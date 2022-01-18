#ifndef __CACHE_H
#define __CACHE_H
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>

int cache1_capa = 0;
int cache1_asso = 0;
int cache2_capa = 0;
int cache2_asso = 0;
int blk_size = 0;
int num_of_block1 = 0;
int num_of_block2 = 0;
int num_of_set1 = 0;
int num_of_set2 = 0;
int tag_bit = 0;
int index_bit = 0;
int blk_offset = 0; 
//unsigned long long tag; 
//unsigned long long index; 

using namespace std; 

struct memory_info {
	string type;
	unsigned long long address;
	unsigned long long tag;
	unsigned long long index;
};

struct cache1_info {
	unsigned long long address;
	unsigned long long tag;
	int lru_value;
	int dirty_bit;
};

struct cache2_info {
	unsigned long long address;
	unsigned long long tag;
	int lru_value;
	int dirty_bit;
};

#endif

