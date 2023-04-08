#ifndef FlowRadar_h
#define FlowRadar_h

#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <cstring>
#include <string>
#include <string.h>

#include "bloomfilter.h"
#include "murmur3.h"

struct FRBucket
{
    u_int64_t FlowXOR;
    u_int32_t FlowCount;
    u_int32_t PacketCount;
    FRBucket()
    {
        FlowXOR = 0;
        FlowCount = 0;
        PacketCount = 0;
    }
};

class FlowRadar
{
private:
    int bf_k, ct_k;
    u_int32_t bf_size_in_bytes;
    u_int32_t ct_size_in_bytes;

    u_int32_t w_bf;
    u_int32_t w_ct;
    // Hashfamily * hf;
    BloomFilter *bloomfilter;
    vector<FRBucket> countingtable;

public:
    int num_flow = 0;
    FlowRadar(u_int32_t tot_mem, u_int32_t bf_k, u_int32_t ct_k, double bf_size_percentage)
    {
        this->bf_k = bf_k;
        this->ct_k = ct_k;

        // hf = new Hashfamily(ct_k);

        bf_size_in_bytes = tot_mem * bf_size_percentage;
        ct_size_in_bytes = tot_mem * (1 - bf_size_percentage);

        w_bf = bf_size_in_bytes * 8;
        w_ct = ct_size_in_bytes / sizeof(FRBucket);

        bloomfilter = new BloomFilter(w_bf, bf_k);

        countingtable = vector<FRBucket>(w_ct, FRBucket());
    }
    void update(uint32_t srcIP, uint32_t dstIP, int val);
    void dump(map<twoTuple_t, uint32_t> &result);
};

#endif /* FlowRadar_hpp */
