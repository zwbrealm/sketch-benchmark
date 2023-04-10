//
// Created by DELL on 2023/3/22.
//

#ifndef C_SKETCH_CMS_H
#define C_SKETCH_CMS_H

#include <stdint.h>

typedef uint64_t* (*cms_hash_function)(unsigned int num_hashs, const void *key, unsigned int len);

typedef struct
{
    uint32_t depth;
    uint32_t width;
    cms_hash_function hash_function;
    int32_t* bins;

} CMsketch;

void cms_init(CMsketch *cms, uint32_t depth, uint32_t width, cms_hash_function hashFunction);
void cms_add(CMsketch *cms, void *key, unsigned int len);
uint32_t quary(CMsketch *cms, const void* key, int len);
int cms_destroy(CMsketch * cms);

#endif //C_SKETCH_CMS_H
