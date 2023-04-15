#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_lcore.h>
#include <rte_mbuf.h>

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
            rte_pktmbuf_free(bufs[i]);
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
        rte_exit(EXIT_FAILURE, "Cannot configure RX queue\n");
    }

    rte_eth_promiscuous_enable(port_id);

    rte_eal_mp_remote_launch(lcore_main, NULL, SKIP_MASTER);

    rte_eal_mp_wait_lcore();

    return 0;
}
