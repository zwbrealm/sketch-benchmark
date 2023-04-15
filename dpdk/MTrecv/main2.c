#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#include <string.h>
#include <sys/queue.h>
#include <stdarg.h>
#include <errno.h>
#include <getopt.h>

#include <rte_common.h>
#include <rte_vect.h>
#include <rte_byteorder.h>
#include <rte_log.h>
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
#include <rte_ethdev.h>
#include <rte_ring.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>
#include <rte_ip.h>
#include <rte_tcp.h>
#include <rte_udp.h>
#include <rte_string_fns.h>
#include <rte_pause.h>
#include <rte_timer.h>

#include <cmdline_parse.h>
#include <cmdline_parse_etheraddr.h>

#include <lthread_api.h>

#define RX_RING_SIZE 128
#define NUM_MBUFS 8191
#define MBUF_CACHE_SIZE 250
#define BURST_SIZE 32

static const struct rte_eth_conf port_conf_default = {
    .rxmode = {.max_rx_pkt_len = RTE_ETHER_MAX_LEN}};

static struct rte_mempool *mbuf_pool;

static int lcore_main(void *arg)
{
    uint16_t port_id = rte_eth_find_next_owned_by(rte_lcore_id(), 0);
    struct rte_mbuf *bufs[BURST_SIZE];

    if (port_id == RTE_MAX_ETHPORTS)
    {
        return 0;
    }

    printf("Core %u receiving packets on port %u\n",
           rte_lcore_id(), port_id);

    while (1)
    {
        const uint16_t nb_rx = rte_eth_rx_burst(port_id, 0, bufs, BURST_SIZE);

        if (nb_rx == 0)
        {
            continue;
        }

        for (int i = 0; i < nb_rx; i++)
        {
            struct rte_mbuf *mbuf = bufs[i];

            struct rte_ether_hdr *eth_hdr = rte_pktmbuf_mtod(mbuf, struct rte_ether_hdr *);
            if (eth_hdr->ether_type == rte_cpu_to_be_16(RTE_ETHER_TYPE_IPV4))
            {
                struct rte_ipv4_hdr *ipv4_hdr = (struct rte_ipv4_hdr *)(eth_hdr + 1);
                uint16_t ip_len = rte_be_to_cpu_16(ipv4_hdr->total_length);
                uint16_t payload_len = ip_len - sizeof(struct rte_ipv4_hdr *);

                void *payload = (void *)ipv4_hdr + sizeof(struct rte_ipv4_hdr *);
                // 在这里添加你的数据包解析逻辑

                rte_pktmbuf_free(mbuf);
            }
            else
            {
                rte_pktmbuf_free(mbuf);
            }
        }
    }

    return 0;
}

int main(int argc, char **argv)
{
    int ret;

    ret = rte_eal_init(argc, argv);
    if (ret < 0)
    {
        rte_exit(EXIT_FAILURE, "Error initializing the DPDK environment\n");
    }

    argc -= ret;
    argv += ret;

    if (rte_eth_dev_count_avail() == 0)
    {
        rte_exit(EXIT_FAILURE, "No Ethernet ports available\n");
    }

    mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL", NUM_MBUFS,
                                        MBUF_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE,
                                        rte_socket_id());

    if (mbuf_pool == NULL)
    {
        rte_exit(EXIT_FAILURE, "Cannot create mbuf pool\n");
    }

    uint16_t port_id = 0;

    ret = rte_eth_dev_configure(port_id, 1, 1, &port_conf_default);
    if (ret < 0)
    {
        rte_exit(EXIT_FAILURE, "Cannot configure Ethernet port\n");
    }

    ret = rte_eth_rx_queue_setup(port_id, 0, RX_RING_SIZE,
                                 rte_eth_dev_socket_id(port_id), NULL, mbuf_pool);

    if (ret < 0)
    {
        rte_exit(EXIT_FAILURE, "Cannot setup RX queue\n");
    }
    printf("21111111111111111111111111111\n");
    ret = rte_eth_dev_start(port_id);
    if (ret < 0)
    {
        rte_exit(EXIT_FAILURE, "Cannot start Ethernet port\n");
    }

    rte_eal_mp_remote_launch(lcore_main, NULL, CALL_MASTER);

    rte_eal_mp_wait_lcore();

    return 0;
}
