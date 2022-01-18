# define _CRT_SECURE_NO_WARNINGS
#include "pipeline.h" 
#include <iostream>
#include<sstream>
#include<fstream>
#include <iterator>
#include<algorithm>
#include<string>  
#include<map>

using namespace std;

string one_line[100];
map<int, int> text_map;
map<int, int> text_map2;
map<int, int> data_map;
map<int, int> data_map2;
map<int, int> run_map;

int binary_ary[100]{ 0 };
int instr_ary[100]{ 0 };
int text_info = 0x400000;
int text_info2 = 0x400000;
int text_cnt = 0x400000;
int data_info = 0x10000000;
int data_info2 = 0x10000000;
int data_cnt = 0x10000000;
int num_text = 0;
int num_data = 0;

int opt_m_ary[2]{ 0 };
int opt_num_ins = 0;
bool opt_m = false;
bool opt_d = false;
bool opt_n = false;
bool opt_p = false;
bool branch_predictor = false; 

// input, output ó���ϰ� �� option�� ó���� 

void input_function(int num_line) {
	int cnt = 0;

	if (num_line != 1) {
		for (int i = 0; i < num_line; i++) {
			binary_ary[i] = (stoul(one_line[i], 0, 16));

			if (i == 0)num_text = binary_ary[0] / 4;
			if (i == 1)num_data = binary_ary[1] / 4;

		}
	}

	for (int i = 2; i < num_text + 2; i++) {
		
		int a = (0xff000000 & binary_ary[i]) >> 24;
		int b = (0x00ff0000 & binary_ary[i]) >> 16;
		int c = (0x0000ff00 & binary_ary[i]) >>8;
		int d = 0x000000ff & binary_ary[i];
		
		text_map.insert(std::pair<int, int>(text_info, binary_ary[i]));
		text_info += 4;
		
		text_map2.insert(std::pair<int, int>(text_info2, a));
		text_info2 += 1;
		text_map2.insert(std::pair<int, int>(text_info2, b));
		text_info2 += 1;
		text_map2.insert(std::pair<int, int>(text_info2, c));
		text_info2 += 1;
		text_map2.insert(std::pair<int, int>(text_info2, d));
		text_info2 += 1;
		text_cnt += 4;
	}

	for (int i = num_text + 2; i < num_line; i++) {
		
		int a = (0xff000000 & binary_ary[i]) >> 24;
		int b = (0x00ff0000 & binary_ary[i]) >> 16;
		int c = (0x0000ff00 & binary_ary[i]) >> 8;
		int d = 0x000000ff & binary_ary[i];
		
		data_map.insert(std::pair<int, int>(data_info, binary_ary[i]));
		data_info += 4;

		data_map2.insert(std::pair<int, int>(data_info2, a));
		data_info2 += 1;
		data_map2.insert(std::pair<int, int>(data_info2, b));
		data_info2 += 1;
		data_map2.insert(std::pair<int, int>(data_info2, c));
		data_info2 += 1;
		data_map2.insert(std::pair<int, int>(data_info2, d));
		data_info2 += 1;
		data_cnt += 4;
	}

	/*for (int i = 0; i < num_text; i++) {

		cout <<hex<< "binary_info: " << INS[i].binary_info<<endl;
		cout << hex<< "address_info: " << INS[i].address_info << endl << endl;;
	}*/ 

	/*for (int i = 2; i < num_text + 2; i++) {
		instr_ary[i - 2] = binary_ary[i];
	}*/

	start_pipeline(num_text);
	return ;
}

int main(int argc, char* argv[]) {

	ifstream File; 
	File.open(argv[argc - 1]);
	int num_line = 0;

	if (!File.is_open())cout << "File is not opened" << endl;

	if (File.is_open()) {
		while (File.eof() != true) {
			getline(File, one_line[num_line]);
			num_line++;
		}
		num_line -= 1;
	}

	for (int i = 0; i < argc; i++) {
		if (!strcmp(argv[i], "-d")) {
			opt_d = true;
		}

		if (!strcmp(argv[i], "-n")) {
			opt_n = true;
			std::string temp(argv[i + 1]);
			opt_num_ins = stoul(temp, 0, 10);
		}

		if (!strcmp(argv[i], "-p")) {
			opt_p = true;
		}

		if (!strcmp(argv[i], "-m")) {
			opt_m = true;
			std::string temp(argv[i + 1]);
			std::stringstream ss(temp);
			std::string token;
			int i = 0;
			while (std::getline(ss, token, ':'))
			{
				opt_m_ary[i] = stoul(token, 0, 16);   // start addr =  opt_m_ary[0] , end addr =  opt_m_ary[1]
				i++;
			}
		}
		if (!strcmp(argv[i], "-atp"))branch_predictor = true;
		if (!strcmp(argv[i], "-antp"))branch_predictor = false;

	}

	input_function(num_line);
	
	/*for (auto it = text_map2.begin(); it != text_map2.end(); it++) {
		cout << hex<<"PC: " << it->first << " instruction 1byte: " << it->second << endl;
	}*/

	/*for (auto it = data_map2.begin(); it != data_map2.end(); it++) {
		cout << hex << "PC: " << it->first << " instruction 1byte: " << it->second << endl;
	}*/

	if (opt_m) {
		std::cout << "Memory content [0x" << std::hex << opt_m_ary[0] << "..0x" << std::hex << opt_m_ary[1] << "]" << std::endl;
		std::cout << "----------------------------------------------------------------" << std::endl;

		std::map<int, int>::iterator it1 = text_map.find(opt_m_ary[0]);
		std::map<int, int>::iterator it2 = data_map.find(opt_m_ary[0]);
		std::map<int, int>::iterator itt;
		std::map<int, int>::iterator itd;

		//if -m option is in text area 
		if (it1 != text_map.end()) {
			for (itt = it1; itt != text_map.end(); itt++) {
				std::cout << "0x" << std::hex << itt->first << ": 0x" << std::hex << itt->second << std::endl;
				if (itt->first == opt_m_ary[1])break;
			}
			if (opt_m_ary[1] >= text_cnt) {
				int temp = (opt_m_ary[1] - text_cnt) / 4 + 1;
				for (int i = 0; i < temp; i++) {
					std::cout << "0x" << std::hex << text_cnt + 4 * i << ": 0x0" << std::endl;
				}
			}
		}
		else if (it2 != data_map.end()) {
			for (itd = it2; itd != data_map.end(); itd++) {
				std::cout << "0x" << std::hex << itd->first << ": 0x" << std::hex << itd->second << std::endl;
				if (itd->first == opt_m_ary[1])break;
			}
			if (opt_m_ary[1] >= data_cnt) {
				int temp = (opt_m_ary[1] - data_cnt) / 4 + 1;
				for (int i = 0; i < temp; i++) {
					std::cout << "0x" << std::hex << data_cnt + 4 * i << ": 0x0" << std::endl;
				}
			}
		}
		else {
			int temp = (opt_m_ary[1] - opt_m_ary[0]) / 4 + 1;
			for (int i = 0; i < temp; i++) {
				std::cout << "0x" << std::hex << opt_m_ary[0] + 4 * i << ": 0x0" << std::endl;
			}
		}
	}

	return 0; 
}