#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
// #include <time>
#include <time.h>
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
#define PKTS_BOUND 10000000

#define NUM_MBUFS 8191
#define MBUF_CACHE_SIZE 250
#define BURST_SIZE 32

#define _WRS_PACK_ALIGN(x) __attribute__((packed, aligned(x)))

#include <stdint.h>

#define MAX_PRIME32 1229
#define MEMORY 10000000
#define D 4
#define LEVEL 4
typedef struct
{
    uint32_t sip;
    uint32_t dip;
    uint16_t sp;
    uint16_t dp;
    uint8_t proto;
} _WRS_PACK_ALIGN(1) Fivetuple;

#define w 156250
#define heap_size 62500000
// int w = MEMORY * 0.25 / 4 / D;
// int heap_size = MEMORY * 0.75 /
//                 ((sizeof(uint32_t) + 4) * 2 + sizeof(uint32_t) + sizeof(uint32_t));

uint32_t hashseed[2 * D] = {12, 45, 12345, 54, 1, 6, 0xd123e567, 0xffff1234};

struct topk_entry
{
    int value;
    Fivetuple tuple;
};

typedef struct CountSketch
{
    unsigned int values[D][w];
    struct topk_entry topks[heap_size];
} cs;

typedef struct Univmon
{
    cs CS[LEVEL];

} um;

static inline uint32_t
rotl32(uint32_t x, int8_t r)
{
    return (x << r) | (x >> (32 - r));
}

static inline uint64_t rotl64(uint64_t x, int8_t r)
{
    return (x << r) | (x >> (64 - r));
}

#define ROTL32(x, y) rotl32(x, y)
#define ROTL64(x, y) rotl64(x, y)

#define getblock(p, i) (p[i])

static inline uint32_t fmix32(uint32_t h)
{
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;

    return h;
}

//-----------------------------------------------------------------------------

uint32_t murmur3(const void *key, int len, uint32_t seed)
{
    const uint8_t *data = (const uint8_t *)key;
    const int nblocks = len / 4;
    int i;

    uint32_t h1 = seed;

    uint32_t c1 = 0xcc9e2d51;
    uint32_t c2 = 0x1b873593;

    //----------
    // body

    const uint32_t *blocks = (const uint32_t *)(data + nblocks * 4);

    for (i = -nblocks; i; i++)
    {
        uint32_t k1 = getblock(blocks, i);

        k1 *= c1;
        k1 = ROTL32(k1, 15);
        k1 *= c2;

        h1 ^= k1;
        h1 = ROTL32(h1, 13);
        h1 = h1 * 5 + 0xe6546b64;
    }

    //----------
    // tail

    const uint8_t *tail = (const uint8_t *)(data + nblocks * 4);

    uint32_t k1 = 0;

    switch (len & 3)
    {
    case 3:
        k1 ^= tail[2] << 16;
    case 2:
        k1 ^= tail[1] << 8;
    case 1:
        k1 ^= tail[0];
        k1 *= c1;
        k1 = ROTL32(k1, 15);
        k1 *= c2;
        h1 ^= k1;
    };

    //----------
    // finalization

    h1 ^= len;

    h1 = fmix32(h1);

    return h1;
}
int cmp_int(const void *_a, const void *_b)
{
    int *a = (int *)_a;
    int *b = (int *)_b;
    return *a - *b;
}
static int __always_inline median(int *vect, int len)
{
    qsort(vect, len, sizeof(int), cmp_int);
    if (len % 2 == 0)
    {
        // if there is an even number of elements, return mean of the two elements in the middle
        // return((vect[len/2] + vect[len/2 - 1]) / 2);
        return ((vect[len / 2] + vect[len / 2 - 1]) / 2);
    }
    else
    {
        // else return the element in the middle
        return vect[len / 2];
    }
}
// {
//     int i, j, x, temp;
//     if (l < r)
//     {
//         i = l;
//         j = r;
//         x = arr[(l + r) / 2];
//         // 以中间元素为轴
//         while (1)
//         {
//             while (i <= r && arr[i] < x)
//                 i++;
//             while (j >= 0 && arr[j] > x)
//                 j--;
//             if (i >= j) // 相遇则跳出
//                 break;
//             else
//             {
//                 temp = arr[i];
//                 arr[i] = arr[j];
//                 arr[j] = temp;
//                 // 交换
//             }
//         }
//         median(arr, l, i - 1); // 对左半部分进行快排
//         median(arr, j + 1, r); // 对右半部分进行快排
//     }
//     int d = r - l;
//     int mid;
//     if ((r - l) % 2 == 0)
//     {
//         mid = (arr[d / 2] + arr[d / 2 - 1]) / 2;
//     }
//     else
//     {
//         mid = arr[d / 2];
//     }
//     return mid;
// }

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

const int num_rx_queues = 1; // 接收队列，最多有8个，这里只设置1个接收队列数量
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

uint64_t get_time()
{
    struct timespec time1 = {0, 0};
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &time1);
    uint64_t ns = time1.tv_sec * 1000000000 + time1.tv_nsec;

    return ns;
}

static void __always_inline ns_um_add(cs *CS, void *element, uint64_t len, uint32_t row_to_update)
{
    uint32_t hash;

    if (row_to_update >= D)
    {
        return;
    }
    hash = murmur3((const void *)element, len, row_to_update * row_to_update);
    unsigned int target_idx = hash & (w - 1);
    // We should probably split the coin here to swap the sign for the countsketch implementation
    // 小于2^n的，CHECK_BIT(hash, n)为0，2*2^n ~ 3*2^n CHECK_BIT(hash, n)为0
    if (hash << 31 & 1)
    {
        CS->values[row_to_update][target_idx]++;
    }
    else
    {
        CS->values[row_to_update][target_idx]--;
    }
}

static int __always_inline query_sketch(cs *CS, void *element, uint64_t len)
{
    // const unsigned int hashes[] = {
    //	xxhash32(element, len, 0x2d31e867),
    //	xxhash32(element, len, 0x6ad611c4),
    //	xxhash32(element, len, 0x00000000),
    //	xxhash32(element, len, 0xffffffff)
    // };

    //_Static_assert(ARRAY_SIZE(hashes) == D, "Missing hash function");

    // int *value = malloc(D * sizeof(int));
    int value[D];
    for (int i = 0; i < D; i++)
    {
        unsigned int hash = murmur3(element, len, i * i);
        unsigned int target_idx = hash & (w - 1);
        if (hash << 31 & 1)
        {
            value[i] = CS->values[i][target_idx];
        }
        else
        {
            value[i] = -CS->values[i][target_idx];
        }
    }
    // printf("123\n");
    // value这个数组储存了查询结果
    // 找到value这个数组的中位数
    // 返回value数组的中位数作为此时查询的结果
    return median(value, D);
    // return 1234;
}
// 数组模拟堆，插入排序
static void __always_inline insertionSort(cs *md)
{
    int i, j;
    struct topk_entry key;

#pragma clang loop unroll(full)
    for (i = 1; i < heap_size; i++)
    {
        // __builtin_memcpy(&key, &arr[i], sizeof(struct topk_entry));
        key = md->topks[i];
        j = i - 1;
        while (j >= 0 && md->topks[j].value < key.value)
        {
            md->topks[j + 1] = md->topks[j];
            j = j - 1;
        }
        // __builtin_memcpy(&arr[j + 1], &key, sizeof(struct topk_entry));
        md->topks[j + 1] = key;
    }
}

static void __always_inline insert_into_heap(cs *md, int median, Fivetuple *pkt)
{
    int index = -1;

    for (int i = 0; i < heap_size; i++)
    {
        Fivetuple origin_pkt = md->topks[i].tuple;
        // bpf_probe_read_kernel(&origin, sizeof(origin), &md->topks[layer][i].tuple);
        if (origin_pkt.dip == pkt->dip &&
            origin_pkt.sip == pkt->sip &&
            origin_pkt.proto == pkt->proto &&
            origin_pkt.dp == pkt->dp &&
            origin_pkt.sp == pkt->sp)
        {
            index = i;
            break;
        }
    }

    if (index >= 0)
    {
        if (md->topks[index].value < median)
        {
            md->topks[index].value = median;
            md->topks[index].tuple = *pkt;
        }
        else
        {
            return;
        }
    }
    else
    {
        // The element is not in the array, let's insert a new one.
        // What I do is to insert in the last position, and then sort the array
        // 如果查找不到对应的pkt,就将在最后的数组元素，赋中位数值（模拟堆）
        if (md->topks[heap_size - 1].value < median)
        {
            md->topks[heap_size - 1].value = median;
            md->topks[heap_size - 1].tuple = *pkt;
        }
        else
        {
            return;
        }
    }
    insertionSort(md);
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

// todo 中断处理
// CMsketch sketch;
Fivetuple *ft;
um Univ;

// Fivetuple *ft = (Fivetuple *) malloc(sizeof(Fivetuple));

uint64_t total_cycles = 0;
uint64_t total_ns = 0;
int64_t pkts_count = 0, pkts_num = 1000000000;
int main(int argc, char *argv[])
{
    int ret;
    bool flag;
    // Fivetuple *ft;
    ft = (Fivetuple *)malloc(sizeof(Fivetuple));

    // cms_init(&sketch, 7, 10000, NULL);
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

    printf("start to recveve data!\n");
    uint64_t start_cycle = rdtsc();
    uint64_t start_time = get_time();
    while (1)
    {

        struct rte_mbuf *mbufs[BURST_SIZE];
        struct rte_ether_hdr *eth_hdr;

        uint16_t num_recv = rte_eth_rx_burst(portid, 0, mbufs, BURST_SIZE);
        if (num_recv > BURST_SIZE)
        {
            // 溢出
            rte_exit(EXIT_FAILURE, "Error receiving from eth\n");
        }
        flag = 1;
        for (int i = 0; i < num_recv; i++)
        {
            if (flag)
            {
                // printf("recieve %d packet!\n", num_recv);
                flag = 0;
            }
            struct rte_mbuf *buf = mbufs[i];

            memset(ft, 0, sizeof(Fivetuple));

            uint32_t sip, dip;
            uint16_t sp, dp;
            uint8_t proto_id;

            // 获取五元组

            struct rte_ipv4_hdr *ipv4_hdr = rte_pktmbuf_mtod_offset(buf, struct rte_ipv4_hdr *, sizeof(struct rte_ether_hdr));
            sip = rte_be_to_cpu_32(ipv4_hdr->src_addr);
            dip = rte_be_to_cpu_32(ipv4_hdr->dst_addr);
            proto_id = ipv4_hdr->next_proto_id;

            struct rte_tcp_hdr *tcp_hdr = rte_pktmbuf_mtod_offset(buf, struct rte_ipv4_hdr *, sizeof(struct rte_ether_hdr) + sizeof(struct rte_ipv4_hdr));
            sp = rte_be_to_cpu_16(tcp_hdr->src_port);
            dp = rte_be_to_cpu_16(tcp_hdr->dst_port);

            // 给结构体赋值
            ft->sip = sip;
            ft->dip = dip;
            ft->sp = sp;
            ft->dp = dp;
            ft->proto = proto_id;

            unsigned int len = sizeof(Fivetuple) / sizeof(char);

            // printf("size of Fivetuple: %d\n", sizeof(Fivetuple));
            // printf("size of char %d\n", sizeof(char));
            // printf("size of uint_32_t %d\n", sizeof(uint32_t));

            /// 这一段用来打印
            // printf("src_addr = ");
            // show_ip(ft->sip);

            // printf("des_addr = ");
            // show_ip(ft->dip);

            // printf("src_port = %"PRIu32"\n", ft->sp);
            // printf("des_port = %"PRIu32"\n", ft->dp);

            // printf("proto_id = %d\n", (int)proto_id);
            // printf("length = %d\n", len);
            /// 这一段用来打印

            // 这一段是时间瓶颈，因为要计算hash

            // unsigned int *index = malloc(sizeof(unsigned int) * D);
            // for (int i = 0; i < D; i++)
            // {
            //     index[i] = murmur3((const void *)ft, sizeof(ft), hashseed[i]) % w;
            //     g = murmur3(str, key_len, hashseed[i + D]) % 2;

            //     if (g == 0)
            //     {
            //         Univ.CS[0][i][index[i]]++;
            //     }
            //     else
            //     {
            //         Univ.CS[0][i][index[i]]--;
            //     }
            // }

            // // int estimate = sketch->query(str, key_len);
            // int temp;
            // int *res = malloc(sizeof(int) * D);
            // int *index = malloc(sizeof(int) * D);
            // int g;
            // for (int i = 0; i < D; i++)
            // {
            //     index[i] = murmur3((const void *)ft, sizeof(ft), hashseed[i]) % w;
            //     temp = Univ.CS[0][i][index[i]];
            //     g = murmur3((const void *)ft, sizeof(ft), hashseed[i + D]) % 2;

            //     res[i] = (g == 0 ? temp : -temp);
            // }

            // median(res, res + d);
            // int r;
            // if (d % 2 == 0)
            // {
            //     r = (res[d / 2] + res[d / 2 - 1]) / 2;
            // }
            // else
            // {
            //     r = res[d / 2];
            // }

            // cms_add(&sketch, (void *)ft, len);
            // uint32_t num = quary(&sketch, (void *)ft, len);
            int median;
            unsigned int pos = murmur3((const void *)ft, sizeof(ft), 199);
            ns_um_add(&Univ.CS[0], ft, sizeof(ft), 0);
            median = query_sketch(&Univ.CS[0], ft, sizeof(ft));
            for (int i = 1; i < LEVEL; i++)
            {
                if (pos & 1)
                {
                    // str = (const char *)str;
                    ns_um_add(&Univ.CS[i], ft, sizeof(ft), i);
                }
                else
                    break;
                pos >>= 1;
                // Update the sketch
            }

            // int value = median;
            // int value = 1234;
            // insert_into_heap(&Univ.CS[0], value, ft);
            // printf("a\n");
            pkts_count++;

            if (pkts_count % 100000000 == 0)
            {
                uint64_t end_cycle = rdtsc();
                uint64_t end_time = get_time();

                printf("avg throughoutput:%ld\n", pkts_count * 1000000000 / (end_time - start_time));
                printf("avg cpu/packet:%ld\n", (end_cycle - start_cycle) / pkts_count);
                // printf("%ld\n", (end_cycle - start_cycle));
                if (pkts_count / 100000000 == 2)
                {
                    for (uint16_t i = 0; i < num_recv; i++)
                    {
                        rte_pktmbuf_free(mbufs[i]);
                    }
                    free(ft);
                    exit(0);
                }
            }

            // if(num > 10)
            // printf("10 times detect");
        }

        for (uint16_t i = 0; i < num_recv; i++)
        {
            rte_pktmbuf_free(mbufs[i]);
        }
    }

    free(ft);
    // cms_destroy(&sketch);
}

// 五元组：源Ip (source IP), 源端口(source port),目标Ip (destination IP), 目标端口(destination port),4层通信协议 (the layer 4 protocol)

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

// int mainq(){
//     cms_init(&cms, 10000, 7);
//
//     int i, res;
//     uint32_t r;
//     for (i = 0; i < 10; i++) {
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
