#include <stdio.h>
#include "cms.h"
#include <stdarg.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
//#include <time>

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
#include <stdint.h>
#include <inttypes.h>
#include <rte_ethdev.h>
#include <rte_cycles.h>
#include <rte_lcore.h>
#include <rte_mbuf.h>

#define RX_RING_SIZE 1024
#define TX_RING_SIZE 1024

#define NUM_MBUFS 8191
#define MBUF_CACHE_SIZE 250
#define BURST_SIZE 32

#define _WRS_PACK_ALIGN(x) __attribute__((packed, aligned(x)))


uint64_t rdtsc() {
    unsigned int lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

static const struct rte_eth_conf port_conf_default = {
	.rxmode = {
		.max_rx_pkt_len = RTE_ETHER_MAX_LEN,
	},
};

const int num_rx_queues = 1; //接收队列，最多有8个，这里只设置1个接收队列数量
const int num_tx_queues = 0;

//低位在前面
void show_ip(uint32_t ip){
    uint8_t *p = (uint8_t *)&ip;
    for(int i = 3; i > 0; i--){
        printf("%"PRIu8":", p[i]);
    }
    printf("%"PRIu8"\n", p[0]);
}

static inline int
init_port(uint16_t port, struct rte_mempool *mbuf_pool)
{
	struct rte_eth_conf port_conf = port_conf_default;
	const uint16_t rx_rings = 1, tx_rings = 1;
	uint16_t nb_rxd = RX_RING_SIZE;
	uint16_t nb_txd = TX_RING_SIZE;
	int retval;
	uint16_t q;
	struct rte_eth_dev_info dev_info;
	struct rte_eth_txconf txconf;

	if (!rte_eth_dev_is_valid_port(port))
		return -1;

	retval = rte_eth_dev_info_get(port, &dev_info);
	if (retval != 0) {
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
	for (q = 0; q < rx_rings; q++) {
		retval = rte_eth_rx_queue_setup(port, q, nb_rxd,
				rte_eth_dev_socket_id(port), NULL, mbuf_pool);
		if (retval < 0)
			return retval;
	}

	txconf = dev_info.default_txconf;
	txconf.offloads = port_conf.txmode.offloads;
	/* Allocate and set up 1 TX queue per Ethernet port. */
	for (q = 0; q < tx_rings; q++) {
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





typedef struct {
    uint32_t sip;
    uint32_t dip;
    uint16_t sp;
    uint16_t dp;
    uint8_t proto;
}_WRS_PACK_ALIGN(1) Fivetuble;


//todo 中断处理
CMsketch sketch;
Fivetuble *ft;

// Fivetuble *ft = (Fivetuble *) malloc(sizeof(Fivetuble));


int main(int argc, char *argv[])
{
    int ret;
    bool flag;
    uint64_t pkts_count = 0, start1, end1, circle1, start2, end2, circle2, circles;
    //Fivetuble *ft;
    ft = (Fivetuble *)malloc(sizeof(Fivetuble));
    
    cms_init(&sketch, 7, 10000, NULL);
    /* Initialize EAL */
    ret = rte_eal_init(argc, argv);
    if (ret < 0) {
        rte_exit(EXIT_FAILURE, "Failed to initialize EAL.\n");
    }

    argc -= ret;
    argv += ret;

/* Parse application arguments */
    if (argc < 1) {
        rte_exit(EXIT_FAILURE, "Invalid number of arguments.\n");
    }

    uint16_t nb_sys_ports = rte_eth_dev_count_avail();
    if(nb_sys_ports < 1) {
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
    if(ret != 0){
        printf("port init error!\n");
    }

    printf("start to recveve data!\n");

    while (1){
        struct rte_mbuf *mbufs[BURST_SIZE];
        struct rte_ether_hdr *eth_hdr;


        uint16_t num_recv = rte_eth_rx_burst(portid, 0, mbufs, BURST_SIZE);
        if(num_recv > BURST_SIZE){
            //溢出
            rte_exit(EXIT_FAILURE, "Error receiving from eth\n");
        }
        flag = 1;
        for(int i = 0; i < num_recv; i++){
            if(flag){
               //printf("recieve %d packet!\n", num_recv);
               flag = 0;
            }
            struct rte_mbuf *buf = mbufs[i];

            
            memset(ft, 0, sizeof(Fivetuble));

            uint32_t sip, dip;
            uint16_t sp, dp;
            uint8_t proto_id;

//获取五元组
            start1 = rdtsc();
            struct rte_ipv4_hdr *ipv4_hdr = rte_pktmbuf_mtod_offset(buf, struct rte_ipv4_hdr *, sizeof(struct rte_ether_hdr));
            sip = rte_be_to_cpu_32(ipv4_hdr->src_addr);
            dip = rte_be_to_cpu_32(ipv4_hdr->dst_addr);
            proto_id = ipv4_hdr->next_proto_id;

            struct rte_tcp_hdr *tcp_hdr = rte_pktmbuf_mtod_offset(buf, struct rte_ipv4_hdr *, sizeof(struct rte_ether_hdr) + sizeof(struct rte_ipv4_hdr));
            sp = rte_be_to_cpu_16(tcp_hdr->src_port);
            dp = rte_be_to_cpu_16(tcp_hdr->dst_port);
            end1 = rdtsc();
            circle1 = end1 - start1;

            //给结构体赋值
            ft->sip = sip;
            ft->dip = dip;
            ft->sp = sp;
            ft->dp = dp;
            ft->proto = proto_id;

            unsigned int len = sizeof(Fivetuble) / sizeof(char);

        


            // printf("size of fivetuble: %d\n", sizeof(Fivetuble));
            // printf("size of char %d\n", sizeof(char));
            // printf("size of uint_32_t %d\n", sizeof(uint32_t));

            ///这一段用来打印
            // printf("src_addr = ");
            // show_ip(ft->sip);

            // printf("des_addr = ");
            // show_ip(ft->dip);

            // printf("src_port = %"PRIu32"\n", ft->sp);
            // printf("des_port = %"PRIu32"\n", ft->dp);
        
            // printf("proto_id = %d\n", (int)proto_id);
            // printf("length = %d\n", len);
            ///这一段用来打印

            //这一段是时间瓶颈，因为要计算hash

            start2 = rdtsc();
            cms_add(&sketch, (void*)ft, len);
            uint32_t num = quary(&sketch, (void*)ft, len);
            end2 = rdtsc();
            circle2 = end2 - start2;
            //
            circles = circle1 + circle2;

            pkts_count += 1;

            if(!(num % (100000))){
                printf("num: %d M packets detect\n", (int)num/(100000));
                printf("cpu circle for single packet: %"PRIu64"\n", circles);
            }

            // if(num > 10)
                // printf("10 times detect");

        }
        for (uint16_t i = 0; i < num_recv; i++) {
            rte_pktmbuf_free(mbufs[i]);
        }

    }

    free(ft);
    cms_destroy(&sketch);


}


//五元组：源Ip (source IP), 源端口(source port),目标Ip (destination IP), 目标端口(destination port),4层通信协议 (the layer 4 protocol)


/*
void add(CountMinSketch* cms, const char* key){
    uint64_t* hashs = cms->hash_function(cms->depth, key);
    uint64_t bin;
    for(int i = 0; i < cms->depth; i++){
        bin = hashs[i] % cms->width + i * cms->width;
        cms->bins[bin] += 1;
    }
    free(hashs);
}
*/

//int mainq(){
//    cms_init(&cms, 10000, 7);
//
//    int i, res;
//    uint32_t r;
//    for (i = 0; i < 10; i++) {
////        res = add(&cms, "this is a test");
//        res = cms_add(&cms, "this is a test");
//    }
//
//    r = quary(&cms, "this is a test");
//    printf("%d", r);
//    if (r != 10) {
//        printf("Error with lookup: %d\n", r);
//    }
//    cms_destroy(&cms);
//    return 0;
//};



