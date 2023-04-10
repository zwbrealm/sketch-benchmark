#include <stdio.h>
#include "cm.c"
#include "murmur3.h"
#include <stdarg.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdint.h>
#include <inttypes.h>
// #include <time>

#include <rte_common.h>
#include <rte_vect.h>
#include <rte_byteorder.h>
#include <rte_log.h>
#include <rte_malloc.h>
#include <rte_memory.h>
#include <rte_memcpy.h>
#include <rte_eal.h>
#include <rte_launch.h>
#include <rte_atomic.h>
#include <rte_cycles.h>
#include <rte_prefetch.h>
#include <rte_lcore.h>
#include <rte_per_lcore.h>
#include <rte_branch_prediction.h>
#include <rte_interrupts.h>
#include <rte_random.h>
#include <rte_debug.h>
#include <rte_ether.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>
#include <rte_ip.h>
#include <rte_tcp.h>
#include <rte_udp.h>
#include <rte_string_fns.h>
#include <rte_cpuflags.h>
#include <rte_ethdev.h>
#include <rte_cycles.h>
#include <rte_lcore.h>
#include <rte_mbuf.h>

#define RX_RING_SIZE 1024
#define TX_RING_SIZE 1024

#define NUM_MBUFS 8191
#define MBUF_CACHE_SIZE 250
#define BURST_SIZE 64

#define _WRS_PACK_ALIGN(x) __attribute__((packed, aligned(x)))
#define array_list_SIZE 65536

uint64_t rdtsc()
{
    unsigned int lo, hi;
    __asm__ __volatile__("rdtsc"
                         : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

static const struct rte_eth_conf port_conf_default = {
    .rxmode = {
        .max_rx_pkt_len = RTE_ETHER_MAX_LEN,
    },
};

const int num_rx_queues = 4; // 接收队列，最多有8个，这里只设置1个接收队列数量
const int num_tx_queues = 0;

// 低位在前面
void show_ip(uint32_t ip)
{
    uint8_t *p = (uint8_t *)&ip;
    for (int i = 3; i > 0; i--)
    {
        printf("%" PRIu8 ":", p[i]);
    }
    printf("%" PRIu8 "\n", p[0]);
}

static inline int
init_port(uint16_t port, struct rte_mempool *mbuf_pool)
{
    struct rte_eth_conf port_conf = port_conf_default;
    const uint16_t rx_rings = 4, tx_rings = 1;
    uint16_t nb_rxd = RX_RING_SIZE;
    uint16_t nb_txd = TX_RING_SIZE;
    int retval;
    uint16_t q;
    struct rte_eth_dev_info dev_info;
    struct rte_eth_txconf txconf;

    if (!rte_eth_dev_is_valid_port(port))
        return -1;

    retval = rte_eth_dev_info_get(port, &dev_info);
    if (retval != 0)
    {
        printf("Error during getting device (port %u) info: %s\n",
               port, strerror(-retval));
        return retval;
    }

    if (dev_info.tx_offload_capa & DEV_TX_OFFLOAD_MBUF_FAST_FREE)
        port_conf.txmode.offloads |=
            DEV_TX_OFFLOAD_MBUF_FAST_FREE;

    /* Configure the Ethernet device. */
    retval = rte_eth_dev_configure(port, rx_rings, tx_rings, &port_conf);
    if (retval != 0)
        return retval;

    retval = rte_eth_dev_adjust_nb_rx_tx_desc(port, &nb_rxd, &nb_txd);
    if (retval != 0)
        return retval;

    /* Allocate and set up 1 RX queue per Ethernet port. */
    for (q = 0; q < rx_rings; q++)
    {
        retval = rte_eth_rx_queue_setup(port, q, nb_rxd,
                                        rte_eth_dev_socket_id(port), NULL, mbuf_pool);
        if (retval < 0)
            return retval;
    }

    txconf = dev_info.default_txconf;
    txconf.offloads = port_conf.txmode.offloads;
    /* Allocate and set up 1 TX queue per Ethernet port. */
    for (q = 0; q < tx_rings; q++)
    {
        retval = rte_eth_tx_queue_setup(port, q, nb_txd,
                                        rte_eth_dev_socket_id(port), &txconf);
        if (retval < 0)
            return retval;
    }

    /* Start the Ethernet port. */
    retval = rte_eth_dev_start(port);
    if (retval < 0)
        return retval;

    /* Display the port MAC address. */
    struct rte_ether_addr addr;
    retval = rte_eth_macaddr_get(port, &addr);
    if (retval != 0)
        return retval;

    printf("Port %u MAC: %02" PRIx8 " %02" PRIx8 " %02" PRIx8
           " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 "\n",
           port,
           addr.addr_bytes[0], addr.addr_bytes[1],
           addr.addr_bytes[2], addr.addr_bytes[3],
           addr.addr_bytes[4], addr.addr_bytes[5]);

    /* Enable RX in promiscuous mode for the Ethernet device. */
    retval = rte_eth_promiscuous_enable(port);
    if (retval != 0)
        return retval;

    return 0;
}

uint64_t get_time()
{
    struct timespec time1 = {0, 0};
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &time1);
    uint64_t ns = time1.tv_sec * 1000000000 + time1.tv_nsec;

    return ns;
}

typedef struct
{
    int32_t idx;
    int32_t value;
    // int32_t time;
} __attribute__((__packed__)) my_pkt;

int array_list[array_list_SIZE];
int vote[array_list_SIZE];
// int charbuf[100];
// struct timespec time_start = { 0, 0 }, time_end = { 0, 0 };

my_pkt *pkt;
uint64_t total_num = 0;
uint64_t burst_num = 0;
// float tun = 0.0;

int main(int argc, char *argv[])
{
    int ret;
    bool flag;
    uint64_t pkts_count = 0, start1, end1, circle1, start2, end2, circle2, circles;

    // pkt = (my_pkt *)calloc(1, sizeof(my_pkt));

    /* Initialize EAL */
    ret = rte_eal_init(argc, argv);
    if (ret < 0)
    {
        rte_exit(EXIT_FAILURE, "Failed to initialize EAL.\n");
    }

    argc -= ret;
    argv += ret;

    /* Parse application arguments */
    if (argc < 1)
    {
        rte_exit(EXIT_FAILURE, "Invalid number of arguments.\n");
    }

    uint16_t nb_sys_ports = rte_eth_dev_count_avail();
    if (nb_sys_ports < 1)
    {
        rte_exit(EXIT_FAILURE, "No Supported eth found\n");
        return -1;
    }

    // 内存池初始化，发送和接收的数据都在内存池里
    struct rte_mempool *mbuf_pool = rte_pktmbuf_pool_create(
        "mbuf pool", NUM_MBUFS, MBUF_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());

    if (mbuf_pool == NULL)
    {
        rte_exit(EXIT_FAILURE, "Could not create mbuf pool\n");
    }

    // 启动dpdk
    uint16_t portid = 0;
    ret = init_port(portid, mbuf_pool);
    if (ret != 0)
    {
        printf("port init error!\n");
    }

    // printf("size of mypkt: %d", sizeof(my_pkt));
    printf("start to recveve data!\n");
    start1 = get_time();
    printf("start time: %" PRIu64 " ns \n", start1);
    uint64_t start_cycle = rdtsc();
    while (1)
    {
        struct rte_mbuf *mbufs[BURST_SIZE];
        struct rte_ether_hdr *eth_hdr;

        // if (burst_num > 100000000)
        // {
        //     end1 = get_time();
        //     uint64_t end_cycle = rdtsc();
        //     printf("end time: %" PRIu64 " ns \n", end1);
        //     printf("程序运行时间： %" PRIu64 "\n", end1 - start1);
        //     printf("处理数据包： %" PRIu64 "\n", total_num);
        //     printf("吞吐量：%d pps\n", ((total_num * 1000000000) / (end1 - start1)));
        //     printf("avg   :%d\n", (end_cycle - start_cycle) / total_num);
        //     break;
        // }

        for (int r = 0; r < num_rx_queues; r++)
        {
            uint16_t num_recv = rte_eth_rx_burst(portid, r, mbufs, BURST_SIZE);
            if (num_recv > BURST_SIZE)
            {
                // 溢出
                rte_exit(EXIT_FAILURE, "Error receiving from eth\n");
            }
            flag = 1;
            for (int i = 0; i < num_recv; i++, burst_num++)
            {
                if (flag)
                {
                    // printf("recieve %d packet!\n", num_recv);
                    flag = 0;
                }
                struct rte_mbuf *buf = mbufs[i];

                start2 = rdtsc();
                // pkt = rte_pktmbuf_mtod_offset(buf, my_pkt*, sizeof(struct rte_ether_hdr));
                pkt = rte_pktmbuf_mtod_offset(buf, my_pkt *, 0);

                pkt->idx = ntohl(pkt->idx);
                pkt->value = ntohl(pkt->value);
                // pkt->time = ntohl(pkt->time);

                // 对于 cm
                // array_list[(pkt->idx)%array_list_SIZE] += 1;

                // 对于cs
                //  if((pkt->value)&(0x00000001) == 0){
                //      array_list[(pkt->idx)%array_list_SIZE] -= 1;
                //  }else{
                //      array_list[(pkt->idx)%array_list_SIZE] += 1;
                //  }

                // //对于 es
                if ((pkt->value) == array_list[(pkt->idx) % array_list_SIZE])
                {
                    vote[(pkt->idx) % array_list_SIZE] += 1;
                }
                else
                {
                    int index = (pkt->idx) % array_list_SIZE;
                    if (index < 0)
                        index = 0;
                    vote[index] -= 1;
                    if (vote[index] <= 0)
                    {
                        // printf("vote[%d] =  %d\n",(pkt->idx) % array_list_SIZE, vote[(pkt->idx) % array_list_SIZE]);
                        vote[index] = (int)(pkt->value);
                        // vote[(pkt->idx) % array_list_SIZE]++;
                    }
                }
                // //对于第四个
                // array_list[(pkt->idx) % array_list_SIZE] = array_list[(pkt->idx) % array_list_SIZE]^(pkt->value);

                // end2 = rdtsc();

                total_num++;

                if (total_num == 1000000)
                {
                    end1 = get_time();
                    uint64_t end_cycle = rdtsc();
                    printf("end time: %" PRIu64 " ns \n", end1);
                    printf("程序运行时间： %" PRIu64 "\n", end1 - start1);
                    printf("处理数据包： %" PRIu64 "\n", total_num);
                    printf("吞吐量：%d pps\n", ((total_num * 1000000000) / (end1 - start1)));
                    printf("avg   :%d\n", (end_cycle - start_cycle) / total_num);
                    exit(0);
                }

                // if(total_num % 10000000 == 0){
                //     // printf("index = %"PRIu32", value = %"PRIu32", time = %"PRIu32", CPU Cycle = %"PRId64"\n", pkt->idx, pkt->value, pkt->time, end2-start2);
                //     printf("index = %"PRIu32", value = %"PRIu32", CPU Cycle = %"PRId64"\n", pkt->idx, pkt->value, end2-start2);
                //     printf("payload = %s\n", (char*)(pkt+1));
                //     // printf("brust num = %"PRIu16"\n", num_recv);

                // }
                rte_pktmbuf_free(mbufs[i]);
            }
        }
        // for (uint16_t i = 0; i < num_recv; i++) {

        // }
    }
    // free(pkt);
}

// while (!force_quit) {
//   for (i = 0; i < nr_queues; i++) {
//    nb_rx = rte_eth_rx_burst(port_id, i, mbufs, 32);
//    if (nb_rx > 0) {
//     for (j = 0; j < nb_rx; j++) {
//      struct rte_mbuf* m = mbufs[j];

//      struct ether_hdr* eth = rte_pktmbuf_mtod(m, struct ether_hdr*);
//      print_ether_hdr(eth);

//      struct diagnosis_hdr* diagnosis = rte_pktmbuf_mtod_offset(m, struct diagnosis_hdr*, sizeof(struct ether_hdr));
//      uint64_t tstamp = ntoh64(&diagnosis->tstamp);
//      printf("Time (dif) : %llu ns\n", tstamp);

//      num ++;
//      total_latency += tstamp;

//      rte_pktmbuf_free(m);
//     }
//    }
//   }
//  }