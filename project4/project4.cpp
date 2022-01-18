#include "cache.h"
#include<sstream>
#include<fstream>
#include <iterator>
#include<algorithm>
#include<string>  
#include<string.h>  
#include<random>


using namespace std;
string read_or_write;
string mem_addr;
bool opt_c = false;
bool opt_a = false;
bool opt_b = false;
bool opt_lru = false;
bool opt_rand= false; 
bool cache1_hit = false;
bool cache2_hit = false;
cache1_info** cache1;
cache2_info** cache2;

int lru_number = 0;
int total_access = 0; 
int read_access = 0;
int write_access = 0;
int L1_R_miss = 0; 
int L1_W_miss = 0;
int L1_clean_evict = 0;
int L1_dirty_evict = 0;
int L2_R_miss = 0;
int L2_W_miss = 0;
int L2_clean_evict = 0;
int L2_dirty_evict = 0;
int L2_R_access = 0;
int L2_W_access = 0;
int random_number = 0;

//make cache1 
void init_cache1() {
	
	cache1 = new  cache1_info * [num_of_set1];

	for (int i = 0; i < num_of_set1; i++) {
		cache1[i] = new  cache1_info[cache1_asso];
		memset(cache1[i], -1, sizeof(cache1_info)*cache1_asso);
	}

}

//make cache2 
void init_cache2() {

	cache2 = new  cache2_info * [num_of_set2];

	for (int i = 0; i < num_of_set2; i++) {
		cache2[i] = new  cache2_info[cache2_asso];
		memset(cache2[i], -1, sizeof(cache2_info) * cache2_asso);
	}
}
void put_into_cache2(memory_info mem_info) {

	int check_block = 0;

	cache2_info* new_cache = new  cache2_info;
	new_cache->address = mem_info.address;
	new_cache->tag = mem_info.tag;
	new_cache->lru_value = lru_number;
	lru_number++;

	if (mem_info.type == "W") {
		new_cache->dirty_bit = 1;
		L2_W_access++;
	}
	if (mem_info.type == "R") {
		new_cache->dirty_bit = 0;
		L2_R_access++;
	}

	for (int i = 0; i < cache2_asso; i++) {
		if (cache2[mem_info.index][i].address != -1)check_block++;
	}

	//cout << hex <<"Type:    "<< mem_info.type <<" address: " << mem_info.address << "  tag: " << mem_info.tag << "   index: " << mem_info.index << endl;

	//when cache hit
	for (int i = 0; i < cache2_asso; i++) {
		if (cache2[mem_info.index][i].tag == new_cache->tag) {
			//cout << "cache hit!!" << endl;
			cache2_hit = true;
			break;
		}
		cache2_hit = false;
	}

	// when cache miss 
	if (cache2_hit != true) {
		//cout << "cache miss!!" << endl;
		if (mem_info.type == "W")L2_W_miss++;
		if (mem_info.type == "R")L2_R_miss++;

		if (check_block != cache2_asso)cache2[mem_info.index][check_block] = *new_cache;  //when block way is not full (don't need replace) 

		//when block way is full so need replace at alu option
		else if (check_block == cache2_asso && opt_lru == true) {
			int temp_num = cache2[mem_info.index][0].lru_value;
			int temp_idx = 0;

			for (int i = 0; i < cache2_asso; i++) {
				if (cache2[mem_info.index][i].lru_value < temp_num) {
					temp_num = cache2[mem_info.index][i].lru_value;
					temp_idx = i;
				};
			}

			if (cache2[mem_info.index][temp_idx].dirty_bit == 1)L2_dirty_evict++;
			else if (cache2[mem_info.index][temp_idx].dirty_bit == 0)L2_clean_evict++;

			cache2[mem_info.index][temp_idx] = *new_cache;
		}

		else if (check_block == cache2_asso && opt_rand == true) {
			if(cache1_asso!=1)random_number = (rand() % (cache1_asso-1));
			if(cache1_asso==1)random_number = 0;
			if (cache2[mem_info.index][random_number].dirty_bit == 2)L2_dirty_evict++;
			else if (cache2[mem_info.index][random_number].dirty_bit == 0)L2_clean_evict++;

			cache2[mem_info.index][random_number] = *new_cache;
		}

	}
}


void put_into_cache1(memory_info mem_info) {
	
	total_access++; 
	if (mem_info.type == "W")write_access++;
	if (mem_info.type == "R")read_access++;

	int check_block = 0;

	cache1_info* new_cache = new  cache1_info;
	new_cache->address = mem_info.address; 
	new_cache->tag = mem_info.tag;
	new_cache->lru_value = lru_number; 
	lru_number++;

	if (mem_info.type == "W")new_cache->dirty_bit = 1;
	if (mem_info.type == "R")new_cache->dirty_bit = 0;


	for (int i = 0; i < cache1_asso; i++) {
		if (cache1[mem_info.index][i].address != -1)check_block++;
	}
	
	//when cache hit
	for (int i = 0; i < cache1_asso; i++) {
		if (cache1[mem_info.index][i].tag == new_cache->tag) {
			//cout << "cache hit!!" << endl;
			cache1_hit = true;
			break;
		}
		cache1_hit = false;
	}

	// when cache miss 
	if (cache1_hit != true) {
		//cout << "cache miss!!" << endl;
		if (mem_info.type == "W")L1_W_miss++; 
		if (mem_info.type == "R")L1_R_miss++;

		put_into_cache2(mem_info);

		if(check_block != cache1_asso)cache1[mem_info.index][check_block] = *new_cache;  //when block way is not full (don't need replace) 
		
		//when block way is full so need replace at alu option
		else if (check_block == cache1_asso && opt_lru==true) {
			int temp_num = cache1[mem_info.index][0].lru_value;
			int temp_idx = 0;

			for (int i = 0; i < cache1_asso; i++) {
				if (cache1[mem_info.index][i].lru_value < temp_num) {
					temp_num = cache1[mem_info.index][i].lru_value;
					temp_idx = i;
				};
			}
			
			if (cache1[mem_info.index][temp_idx].dirty_bit == 1)L1_dirty_evict++;
			else if (cache1[mem_info.index][temp_idx].dirty_bit == 0)L1_clean_evict++;
			

			cache1[mem_info.index][temp_idx] = *new_cache;
		}

		else if (check_block == cache1_asso && opt_rand == true) {
			if(cache1_asso!=1)random_number = (rand() % (cache1_asso-1));
			if(cache1_asso==1)random_number = 0;

			if (cache1[mem_info.index][random_number].dirty_bit == 1)L1_dirty_evict++;
			else if (cache1[mem_info.index][random_number].dirty_bit == 0)L1_clean_evict++;

			cache1[mem_info.index][random_number] = *new_cache;
		}
	}
}



//decode trace file -> extract tag bit, set_bit
void decode(string str) {
	struct memory_info mem_info;
	mem_info.type = str[0];
	mem_info.address = stoull(str.substr(2, str.size() - 1),nullptr,16); 
	mem_info.tag = mem_info.address >> (blk_offset + index_bit); 
	mem_info.index = (mem_info.address << tag_bit) >> (tag_bit + blk_offset);
	put_into_cache1(mem_info);
	//put_into_cache2(mem_info);
}

int main(int argc, char* argv[]) {

	int n = 0;
	string mem_info;

	ifstream File;
	File.open(argv[argc - 1]);
	if (!File.is_open())cout << "File is not opened" << endl;


	if (File.is_open()) {
		for (int i = 0; i < argc; i++) {
			if (!strcmp(argv[i], "-c")) {
				std::string temp(argv[i + 1]);
				cache2_capa = stoul(temp, 0, 10);
				if (cache2_capa >= 4)cache1_capa = cache2_capa / 4;     // if cache2_capa is over 4, cache1_capa =  cache2_capa / 4  
				else if (cache2_capa <= 2)cache1_capa = cache2_capa;    // if cache2_capa is under 4, cache1_capa =  cache2_capa 
				opt_c = true;
			}

			if (!strcmp(argv[i], "-a")) {
				std::string temp(argv[i + 1]);
				cache2_asso = stoul(temp, 0, 10);
				if (cache2_asso >= 4)cache1_asso = cache2_asso / 4;     // if cache2_asso is over 4, cache1_asso =  cache2_asso  / 4  
				else if (cache2_asso <= 2)cache1_asso = cache2_asso;    // if cache2_asso is under 4, cache1_asso =  cache2_asso 
				opt_a = true;
			}

			if (!strcmp(argv[i], "-b")) {
				std::string temp(argv[i + 1]);
				blk_size = stoul(temp, 0, 10);
				opt_b = true; 
			}
			if (!strcmp(argv[i], "-lru")) {
				opt_lru = true;
			}
			if (!strcmp(argv[i], "-random")) {
				opt_rand = true;
			}
		}

		bool option = opt_lru || opt_rand;
		if (opt_a == false || opt_b == false || opt_c == false || option == false) {
			cout << "Exception Argument" << endl;
			return 0;
		}

		num_of_block1 = 1024 * cache1_capa / blk_size;     //num of cache 1 's blocks
		num_of_block2 = 1024 * cache2_capa / blk_size;    //num of cache 2 's blocks
		num_of_set1 = num_of_block1 / cache1_asso;     //num of cache 1's sets 
		num_of_set2 = num_of_block2 / cache2_asso;     //num of cache 1's sets 
	
		int temp_blk_size = blk_size; 
		int temp_set_size = num_of_set1;

		while (temp_blk_size != 1) {
			temp_blk_size /= 2;
			blk_offset++;
		}

		while (temp_set_size != 1) {
			temp_set_size /= 2;
			index_bit++;
		}
	
		tag_bit = 64 - blk_offset - index_bit;
		
		init_cache1();
		init_cache2();
		
		while (File.eof() != true) {
			getline(File, mem_info);	
			if(mem_info.length() < 2)break;		
			decode(mem_info);
		}
	}
	string newfile_name = argv[8];
	newfile_name.erase(newfile_name.find("."));
	newfile_name.append("_");
	newfile_name.append(to_string(cache2_capa));
	newfile_name.append("_");
	newfile_name.append(to_string(cache2_asso));
	newfile_name.append("_");
	newfile_name.append(to_string(blk_size)); 
	newfile_name.append(".out");

	ofstream newfile(newfile_name);
	
		//cout << newfile_name << endl;

	float L1_R_miss_rate = (float)L1_R_miss / (float)read_access;
	float L1_W_miss_rate = (float)L1_W_miss / (float)write_access;
	float L2_R_miss_rate = (float)L2_R_miss / (float)L2_R_access;
	float L2_W_miss_rate = (float)L2_W_miss / (float)L2_W_access;


	newfile<< "-- General Stats --" << endl;
	newfile<< "L1 capacity:  " << cache1_capa << endl;
	newfile<< "L1 way:  " << cache1_asso << endl;
	newfile<< "L2 capacity:  " << cache2_capa << endl;
	newfile<< "L2 way:  " << cache2_asso << endl;
	newfile<< "Block Size:      " << blk_size << endl;
	newfile<< "Total accesses:  " << total_access << endl;
	newfile<< "Read accesses:  " << read_access << endl;
	newfile<< "Write accesses:  " << write_access << endl;
	newfile<< "L1 Read misses:  " << L1_R_miss << endl;
	newfile<< "L2 Read misses:  " << L2_R_miss << endl;
	newfile<< "L1 Write misses:  " << L1_W_miss << endl;
	newfile<< "L2 Write misses:  " << L2_W_miss << endl;
	newfile<< "L1 Read miss rate:  " << L1_R_miss_rate*100<<"%"<<endl;
	newfile<< "L2 Read miss rate:  " << L2_R_miss_rate * 100 << "%" << endl;
	newfile<< "L1 Write miss rate:  " << L1_W_miss_rate*100<<"%" << endl;
	newfile<< "L2 Write miss rate:  " << L2_W_miss_rate * 100 << "%" << endl;
	newfile<< "L1 Clean eviction:  " << L1_clean_evict << endl;
	newfile<< "L2 Clean eviction:  " << L2_clean_evict << endl;
	newfile<< "L1 Dirty eviction:  " << L1_dirty_evict << endl;
	newfile<< "L2 Dirty eviction:  " << L2_dirty_evict << endl;
	
	newfile.close();

	for (int i = 0; i < num_of_set1; i++) {
		delete[] cache1[i];
	}
	delete[] cache1;
	

	for (int i = 0; i < num_of_set2;i++) {
		delete[] cache2[i];
	}
	delete[] cache2;


	//cout << "Cache1 capa: " << cache1_capa << "  Cache1 asso:  " << cache1_asso << "  Cache2 capa: " << cache2_capa << "  Cache2 asso:  " << cache2_asso << "  Block_size: " << blk_size << endl << endl;
	//cout << "num_of_block1: " << num_of_block1 << "  num_of_block2:  " << num_of_block2 << "  num_of_set1:  " << num_of_set1 << "  num_of_set2  " << num_of_set2 << endl;
	
	File.close();
    return 0;
}
