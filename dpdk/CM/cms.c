//
// Created by DELL on 2023/3/22.
//
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <rte_malloc.h>
#include "cms.h"

uint64_t* default_hash(unsigned int num_hashes, const void* str, unsigned int len);
uint64_t fnv_1a(const void* key, int seed, unsigned int len);
uint64_t results[10];

void cms_init(CMsketch *cms, uint32_t depth, uint32_t width, cms_hash_function hashFunction)
{
    cms->depth = depth;
    cms->width = width;
    if(hashFunction != NULL)
        cms->hash_function = hashFunction;
    else
        cms->hash_function = default_hash;
    cms->bins = (int32_t*) calloc(width*depth, sizeof(int32_t));
}

void cms_add(CMsketch *cms, void *key, unsigned int len)
{
    uint64_t *hashes = cms->hash_function(cms->depth, key, len);
    int num_add = INT32_MAX;
    uint64_t bin;
    for(int i = 0; i < cms->depth; i++){
        bin = (hashes[i] % cms->width) + (i * cms->width);
        cms->bins[bin] += 1;
    }

    //free(hashes);
}

uint32_t quary(CMsketch *cms, const void* key, int len){
    uint64_t *hashes = cms->hash_function(cms->depth, key, len);
    uint32_t hash = 0;
    uint32_t num = INT32_MAX;
    for(int i = 0; i < cms->depth; i++){
//        hash = hashs[i];
        hash = cms->bins[hashes[i] % cms->width + i * cms->width];
        if(hash < num) num = hash;
    }
    //free(hashes);
    return num;
}


int cms_destroy(CMsketch * cms) {
    free(cms->bins);
    cms->width = 0;
    cms->depth = 0;
    cms->hash_function = NULL;
    cms->bins = NULL;

    free(cms->bins);

    return 1;
}



//NOTE: The caller will free the results
uint64_t* default_hash(unsigned int num_hashes, const void* str, unsigned int len) {
    //uint64_t* results = (uint64_t*)calloc(num_hashes, sizeof(uint64_t));
    for (int i = 0; i < num_hashes; ++i){
        results[i] = fnv_1a(str, i, len);
    }
    return results;
}

// uint64_t* default_hash(unsigned int num_hashes, const void* str, unsigned int len) {
//     uint64_t* results = (uint64_t*)calloc(num_hashes, sizeof(uint64_t));
//     for (int i = 0; i < num_hashes; ++i){
//         results[i] = fnv_1a(str, i, len);
//     }
//     return results;
// }



uint64_t fnv_1a(const void* key, int seed, unsigned int len) {
    // FNV-1a hash (http://www.isthe.com/chongo/tech/comp/fnv/)
    uint64_t h = 14695981039346656037ULL + (31 * seed); // FNV_OFFSET 64 bit with magic number seed
    for (int i = 0; i < len; ++i){
        h = h ^ (unsigned char) ((char*)key)[i];
        h = h * 1099511628211ULL; // FNV_PRIME 64 bit
    }
    return h;
}