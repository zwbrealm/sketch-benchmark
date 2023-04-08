#include "flowradar.h"

void FlowRadar::update(uint32_t srcIP, uint32_t dstIP, int val)
{
    bool flag = bloomfilter->query(srcIP, dstIP);
    if (flag == false) bloomfilter->insert(srcIP, dstIP);
    
    // u_char * buf = new u_char[ct_k];
    // u_char * p = (u_char * )&key;
    // for (unsigned i = 0; i < ct_k; i ++)
    // {
    //     buf[i] = p[i % 4];
    //     // printf("%d\n", buf[i]);
    // }
    
    
    for (unsigned i = 0; i < ct_k; i ++)
    {
        // u_int64_t index = hf->getihashval((const u_char*) &key, 4, i) % w_ct;
        // u_int64_t index = crc32(buf, i + 1) % w_ct;
        u_int64_t index = two_tuple_sketch_hash(srcIP, dstIP, i, w_ct);

        uint64_t k1, k2;
        k1 = (uint64_t)srcIP;
        k2 = (uint64_t)dstIP;
        uint64_t combined_k = (k1<<32)+k2;

        if (flag == false)
        {
            countingtable[index].FlowXOR ^= combined_k;
            countingtable[index].FlowCount ++;
        }
        countingtable[index].PacketCount ++;
    }
}

void FlowRadar::dump(map<twoTuple_t, uint32_t> &result)
{
    u_int64_t key_uint64;
    int count;
    
    while (1)
    {
        bool flag = true;
        for (unsigned i = 0; i < w_ct; i ++)
        {
            //cout << countingtable[i].FlowCount << " ";
            if (countingtable[i].FlowCount == 1)
            {
                key_uint64 = countingtable[i].FlowXOR;
                count = countingtable[i].PacketCount;
                //cout << "find key : " << key_uint64 << " val : " << count << endl;
                twoTuple_t* key = from_uint64_to_twotpl(key_uint64);
                result[*key] = count;
                
                // u_char * buf = new u_char[ct_k];
                // u_char * p = (u_char * )&key;
                // for (unsigned i = 0; i < ct_k; i ++)
                // {
                //     buf[i] = p[i % 4];
                //     // printf("%d\n", buf[i]);
                // }
                
                for (unsigned j = 0; j < ct_k; j ++)
                {
                    // u_int64_t index = hf->getihashval((const u_char *) &key, 4, j) % w_ct;
                    // u_int64_t index = crc32(buf, j + 1) % w_ct;
                    u_int64_t index = two_tuple_sketch_hash(key->srcIP, key->dstIP, j, w_ct);

                    countingtable[index].FlowCount --;
                    countingtable[index].FlowXOR ^= key_uint64;
                    countingtable[index].PacketCount -= count;
                }
                flag = false;
            }
        } //cout << "\n";
        if (flag)
            break;
    }
}
