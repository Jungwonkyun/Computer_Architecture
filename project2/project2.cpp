# define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include<vector>
#include<map>
#include <iterator>
#include<sstream>
#include<fstream>
#include<algorithm>
#include<bitset>
#include<string>
#include<string.h>

std::string one_line[100]; //   first and second element is num of text size & num of data size seperately  <input line>
std::map<int, int> text_map; // instruction : address 
std::map<int, int> data_map; // data : address 
std::map<int, int> run_map; // map that contains runned instruction infomation
int binary_ary[100]; // array that string to int 32bit (same information with above array)
int register_ary[32]{ 0 }; // array that has information about register initialize with 0 
char type_ary[100]; // array that identify type
int num_text = 0; //number of instruction that has at binary_ary[0]
int num_data = 0; //number of data that has at binary_ary[1]
int ins_counter = 100; // counter for -n option
int text = 0x400000; // text address start 
int data = 0x10000000; // data address start 
int jump_addr = 0; // jump_addr will save branch target address 
int PC = 0x400000;
int opt_m_ary[2]{ 0 };
int opt_num_ins = 0;
bool opt_m = false;
bool opt_d = false;
bool opt_n = false;

class R_class {
public:
	int op, rs, rt, rd, shamt, funct;
	std::string isa;
	R_class(int r1, int r2, int r3, int r4, int r5, int r6) {
		op = 0;  // r1 = 0 R_type default 
		rs = r2;
		rt = r3;
		rd = r4;
		shamt = r5;
		funct = r6;
	}
};

class I_class {
public:
	int op, rs, rt, off_or_imm;
	std::string isa;
	I_class(int r1, int r2, int r3, int r4) {
		op = r1;
		rs = r2;
		rt = r3;
		off_or_imm = r4;
	}
};

class J_class {
public:
	int op, target;
	std::string isa;
	J_class(int r1, int r2) {
		op = r1;
		target = r2;
	}
};

class ADDU : public R_class // save sum of $rs +$rt in $rd  complete
{
public:
	ADDU(int r1, int r2, int r3, int r4, int r5, int r6) : R_class(r1, r2, r3, r4, r5, r6)
	{
		register_ary[rd] = register_ary[rt] + register_ary[rs];
	}
};

class AND : public R_class //save logical AND of $rs & $rt to $rd  complete
{
public:
	AND(int r1, int r2, int r3, int r4, int r5, int r6) : R_class(r1, r2, r3, r4, r5, r6)
	{
		register_ary[rd] = register_ary[rt] & register_ary[rs];
	}
};

class NOR : public R_class //save logical NOR of $rs & $rt to $rd complete
{
public:
	NOR(int r1, int r2, int r3, int r4, int r5, int r6) : R_class(r1, r2, r3, r4, r5, r6) {
		register_ary[rd] = ~(register_ary[rs] | register_ary[rt]);
	}
};

class OR : public R_class  //save logical OR of $rs & $rt to $rd complete
{
public:
	OR(int r1, int r2, int r3, int r4, int r5, int r6) : R_class(r1, r2, r3, r4, r5, r6)
	{
		register_ary[rd] = register_ary[rt] | register_ary[rs];
	}
};

class SLTU : public R_class // if $rs < $rt save $rd = 1  else save $rd = 0 complete
{
public:
	SLTU(int r1, int r2, int r3, int r4, int r5, int r6) : R_class(r1, r2, r3, r4, r5, r6)
	{
		if (register_ary[rs] < register_ary[rt])register_ary[rd] = 1;
		else register_ary[rd] = 0;
	}
};

class SUBU : public R_class //save $rd sub of $rs - $rt   
{
public:
	SUBU(int r1, int r2, int r3, int r4, int r5, int r6) : R_class(r1, r2, r3, r4, r5, r6)
	{
		register_ary[rd] = register_ary[rs] - register_ary[rt]; 
	}
};

class SLL : public R_class  // left shift for shamt complete
{
public:
	SLL(int r1, int r2, int r3, int r4, int r5, int r6) : R_class(r1, r2, r3, r4, r5, r6)
	{
		register_ary[rd] = register_ary[rt] << shamt;
	}
};

class SRL : public R_class // right shift for shamt complete
{
public:
	SRL(int r1, int r2, int r3, int r4, int r5, int r6) : R_class(r1, r2, r3, r4, r5, r6)
	{
		register_ary[rd] = register_ary[rt] >> shamt;
	}
};

class JR : public R_class //TODO
{
public:
	JR(int r1, int r2, int r3, int r4, int r5, int r6) : R_class(r1, r2, r3, r4, r5, r6)
	{
		jump_addr = register_ary[rs];
		std::map<int, int>::iterator it = text_map.find(jump_addr);
		if (it != text_map.end()) // if jump addresss is in text_ary
		{
			PC = it->first;
		}
	}
};

class ADDIU : public I_class  // save sum of $rs + sign extended imm in $rd  complete
{
public:
	ADDIU(int r1, int r2, int r3, int r4) : I_class(r1, r2, r3, r4)
	{
		std::bitset<32>extended = off_or_imm;  //sign extention to 32bits 
		register_ary[rt] = register_ary[rs] + extended.to_ulong();
	}
};

class ANDI : public I_class // save logical AND with $rs and sign extended imm in $rt complete 
{
public:
	ANDI(int r1, int r2, int r3, int r4) : I_class(r1, r2, r3, r4)
	{
		std::bitset<32>extended = (off_or_imm & 0x0000ffff);
		std::bitset<32> temp = register_ary[rs];
		std::bitset<32> result = extended & temp;

		register_ary[rt] = result.to_ulong();
	}
};

class LUI : public I_class {
public:
	LUI(int r1, int r2, int r3, int r4) : I_class(r1, r2, r3, r4)
	{
		register_ary[rt] = off_or_imm << 16; // upper halfword imm with lower halfword is '0'*16 this mean imm *2^16 
	}
};

class ORI : public I_class {
public:
	ORI(int r1, int r2, int r3, int r4) : I_class(r1, r2, r3, r4)
	{
		std::bitset<32>extended = (off_or_imm & 0x0000ffff);
		std::bitset<32>temp = register_ary[rs];
		std::bitset<32>result = temp | extended;

		register_ary[rt] = result.to_ulong();
	}
};

class SLTIU : public I_class {
public:
	SLTIU(int r1, int r2, int r3, int r4) : I_class(r1, r2, r3, r4)
	{
		if (register_ary[rs] < off_or_imm)register_ary[rt] = 1;   //수정필요 signed extended 생각 강의록엔 unsigned comparison한다고 돼있는데 상관 x?
		else register_ary[rt] = 0;
	}
};

class BEQ : public I_class {
public:
	BEQ(int r1, int r2, int r3, int r4) : I_class(r1, r2, r3, r4)
	{
		if (register_ary[rs] == register_ary[rt]) {
			jump_addr = PC + off_or_imm * 4;
		}

		std::map<int, int>::iterator it = text_map.find(jump_addr);
		if (it != text_map.end()) // if jump addresss is in text_ary
		{
			PC = it->first;
		}
		else if (jump_addr >= text)PC = text;
	}
};

class BNE : public I_class {
public:
	BNE(int r1, int r2, int r3, int r4) : I_class(r1, r2, r3, r4)
	{
		if (register_ary[rs] != register_ary[rt]) {
			jump_addr = PC + off_or_imm * 4;
		}
		std::map<int, int>::iterator it = text_map.find(jump_addr);
		if (it != text_map.end()) // if jump addresss is in text_ary
		{
			PC = it->first;
		}
		else if (jump_addr >= text)PC = text;
	}
};

class LW : public I_class  // 다른 샘플로 한 번더 해보기 offset이 3넘어가는 걸로 
{
public:
	LW(int r1, int r2, int r3, int r4) : I_class(r1, r2, r3, r4)
	{
		int result = 0;
		int share = off_or_imm / 4;
		int rest = off_or_imm % 4;
		int bk_pnt = 0;

		for (auto it = data_map.begin(); it != data_map.end(); it++) {
			if (it->first == register_ary[rs]) {
				if (bk_pnt == share) result = it->second;
				else bk_pnt++;
			}
		}
		register_ary[rt] = result;
	}
};

class LB : public I_class {
public:
	LB(int r1, int r2, int r3, int r4) : I_class(r1, r2, r3, r4) // 다른 샘플로 한 번더 해보기 offset이 3넘어가는 걸로 
	{
	  //  register_ary[rt] = register_ary[rs] + off_or_imm;   
		int result = 0;
		int test = 0;
		int share = off_or_imm / 4;
		int rest = off_or_imm % 4;
		int bk_pnt = 0;

		//확인필요 
		for (auto it = data_map.begin(); it != data_map.end(); it++) {
			if (it->first == register_ary[rs] && bk_pnt == share) {
				result = it->second;
				if (bk_pnt == share) {
					if (rest == 0) test = ((int)0xff000000 & result) >> 24;
					else if (rest == 1) test = (0x00ff0000 & result) >> 16;
					else if (rest == 2) test = (0x0000ff00 & result) >> 8;
					else if (rest == 3) test = (0x000000ff & result);
				}
				else bk_pnt++;
			}
		}
		register_ary[rt] = test;
	}
};

class SW : public I_class {

public:
	SW(int r1, int r2, int r3, int r4) : I_class(r1, r2, r3, r4) // MIPS sample 수정해서 시도해봐야함 
	{
		int share = off_or_imm / 4;
		int rest = off_or_imm % 4;
		int bk_pnt = 0;
		int count = 0;
		int last = 0;

		for (auto it = data_map.begin(); it != data_map.end(); it++) {
			// if  register_ary[rs]'s address is in word address change it   

			if (it->first == register_ary[rs]) {
				if (bk_pnt == share)   it->second = register_ary[rt];
				else bk_pnt++;
				count++;
			}
			//if  register_ary[rs]'s address is not in word address update it 
			last = it->first;
		}
		if (count == 0)data_map.insert(std::pair<int, int>(last + 4, register_ary[rt])); // 이거 되는지 확인 해봐야함
	}
};

class SB : public I_class //overflow 고려 안 해도 되나? 예를 들어 마지막 원소 +offset이 5면 그 다음 4byte엔 아무것도 없음  
{
public:
	SB(int r1, int r2, int r3, int r4) : I_class(r1, r2, r3, r4)
	{
		int share = off_or_imm / 4;
		int rest = off_or_imm % 4;
		int bk_pnt = 0;
		int count = 0;
		int test = 0;
		int result = register_ary[rt];
		int temp = register_ary[rs];

		// 얘도 실험해봐야한다 ex) 7? 
		for (auto it = data_map.begin(); it != data_map.end(); it++) {
			// if  register_ary[rs]'s address is in word address change it   
			if (it->first == temp) {
				if (bk_pnt == share) {
					//it->second = register_ary[rt];
					if (rest == 0) test = ((it->second & 0x00ffffff) | (result << 24));
					else if (rest == 1) test = ((it->second & (int)0xff00ffff) | (result << 16));
					else if (rest == 2) test = ((it->second & (int)0xffff00ff) | (result << 8));
					else if (rest == 3) test = ((it->second & (int)0xffffff00) | result);
					it->second = test;
					count++;
				}
				else {
					bk_pnt++;
					temp += 4;
				}
			}
		}
	}
};

class  J : public J_class {
public:
	J(int r1, int r2) : J_class(r1, r2)
	{
		 //target x 4 = jump address 
		jump_addr = 4 * target;
		// std::cout << "target: " <<std::dec<< target << std::endl;
		std::map<int, int>::iterator it = text_map.find(jump_addr);
		if (it != text_map.end()) // if jump addresss is in text_ary
		{
			PC = it->first;
		}
		else if (jump_addr >= text)PC = text;
	}
};

class  JAL : public J_class {
public:
	JAL(int r1, int r2) : J_class(r1, r2)
	{
		jump_addr = 4 * target;
		register_ary[31] = PC;

		std::map<int, int>::iterator it = text_map.find(jump_addr);
		if (it != text_map.end()) // if jump addresss is in text_ary
		{
			PC = it->first;
		}
		else if (jump_addr >= text)PC = text;
	}
};

//fine what instruction is with op_code 
void find_ins(unsigned int ins) {

	PC += 4;

	unsigned int op = ins >> 26;
	std::bitset<32>ins_bit(op);
	std::bitset<32>basic_bit(0);
	std::bitset<32>result_bit = ins_bit | basic_bit;

	//for R type op code is always 0 
	if (result_bit == 0) {

		std::bitset<32> a = (65011712 & ins) >> 21;
		std::bitset<32> b = (2031616 & ins) >> 16;
		std::bitset<32> c = (63488 & ins) >> 11;
		std::bitset<32> d = (1984 & ins) >> 6;
		std::bitset<32> e = (63 & ins);

		int op = 0; //op
		int rs = a.to_ulong(); // rs 
		int rt = b.to_ulong();  // rt 
		int rd = c.to_ulong(); // rd 
		int shamt = d.to_ulong(); // shamt  
		int func = e.to_ulong();  // func         


		if (func == 33)ADDU addu(op, rs, rt, rd, shamt, func);
		else if (func == 36)AND r_and(op, rs, rt, rd, shamt, func);
		else if (func == 39)NOR r_nor(op, rs, rt, rd, shamt, func);
		else if (func == 37)OR r_or(op, rs, rt, rd, shamt, func);
		else if (func == 43)SLTU sltu(op, rs, rt, rd, shamt, func);
		else if (func == 35)SUBU subu(op, rs, rt, rd, shamt, func);
		else if (func == 0)SLL sll(op, rs, rt, rd, shamt, func);
		else if (func == 2)SRL srl(op, rs, rt, rd, shamt, func);
		else if (func == 8)JR jr(op, rs, rt, rd, shamt, func);

	}

	else if (result_bit == 2 || result_bit == 3) {
		std::bitset<32> a = ins >> 26;
		std::bitset<32> b = (67108863 & ins);

		int op = a.to_ulong();
		int rest = b.to_ulong();

		if (op == 2)J j(op, rest);
		else if (op == 3)JAL jal(op, rest);
	}
	else {
		std::bitset<32> a = ins >> 26;
		std::bitset<32> b = (65011712 & ins) >> 21;
		std::bitset<32> c = (2031616 & ins) >> 16;
		std::bitset<32> d = (65535 & ins);

		int op = a.to_ulong();
		int rs = b.to_ulong();
		int rt = c.to_ulong();
		int rest = 0;
		int sign_ck = (65535 & ins) & 0x00008000;  //check if 16 bit imm's MSB is 1 or 0  <일단 맞긴한데 한 번 더 확인해보기>

		// if MSB = negative do 2's complement  
		if (sign_ck != 0) {
			int  temp = (65535 & ins) | 0xffff0000;
			std::bitset<32> d2(~temp);

			int temp2 = d2.to_ulong() + 1;
			rest = -temp2;
		}

		else if (sign_ck == 0) {
			rest = d.to_ulong();
		}

		if (op == 9)ADDIU addiu(op, rs, rt, rest);
		else if (op == 12)ANDI andi(op, rs, rt, rest);
		else if (op == 15)LUI lui(op, rs, rt, rest);
		else if (op == 13)ORI ori(op, rs, rt, rest);
		else if (op == 11)SLTIU sltiu(op, rs, rt, rest);
		else if (op == 4)BEQ beq(op, rs, rt, rest);
		else if (op == 5)BNE bne(op, rs, rt, rest);
		else if (op == 35)LW lw(op, rs, rt, rest);
		else if (op == 32)LB lb(op, rs, rt, rest);
		else if (op == 43)SW sw(op, rs, rt, rest);
		else if (op == 40)SB sb(op, rs, rt, rest);

	}
	return;
}

void print_reg() {
	std::cout << "Registers:" << std::endl;
	for (int i = 0; i < 32; i++) {
		std::cout << "R" << std::dec << i << ": " << std::hex << "0x" << register_ary[i] << std::endl;
	}
}

int main(int argc, char* argv[])
{
	std::ifstream File;

	File.open(argv[argc - 1]);  // last element of argument is file name

	if (!File.is_open())std::cout << "File is not opened" << std::endl;

	int num_line = 0;

	// option check 
	for (int i = 0; i < argc; i++) {
		if (!strcmp(argv[i], "-d")) {
			opt_d = true;
		}
		if (!strcmp(argv[i], "-n")) {
			opt_n = true;
			std::string temp(argv[i + 1]);
			opt_num_ins = stoul(temp, 0, 10);
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
	}

	//get binary information one by one sentence 
	if (File.is_open()) {
		while (File.eof() != true) {
			std::getline(File, one_line[num_line]);
			num_line++;
		}
		num_line -= 1;
	}
	//convert input hex string to int 
	int cnt = 0;

	if (num_line != 1) {
		for (int i = 0; i < num_line; i++) {
			binary_ary[i] = (stoul(one_line[i], 0, 16));

			if (i == 0)num_text = binary_ary[0] / 4;
			if (i == 1)num_data = binary_ary[1] / 4;
		}
	}

	for (int i = 2; i < num_text + 2; i++) {
		text_map.insert(std::pair<int, int>(text, binary_ary[i]));
		text += 4;
	}

	for (int i = num_text + 2; i < num_line; i++) {
		data_map.insert(std::pair<int, int>(data, binary_ary[i]));
		data += 4;
	}

	int counter = 0;
	int last_check = 0;

	std::cout << "Current register values:" << std::endl;
	std::cout << "----------------------------------------------------------------" << std::endl;

	for (int i = 2; i < num_text + 2; i++) {
		if (opt_d == true) {
			std::cout << "PC: 0x" << std::hex << PC << std::endl;
			print_reg();
			std::cout << std::endl;
			std::cout << std::endl;
		}
		if (opt_n == true && counter >= opt_num_ins)break; // when -n option, if counter >= ins break this loop
		if ((counter < opt_num_ins) && opt_n == true)find_ins(binary_ary[i]);  //if and only if -n option, if counter < ins run instruction
		if (opt_n == false)  // if there's no -n option run all instruction 
		{
			find_ins(binary_ary[i]);
		}
		counter++;

		if (jump_addr != 0) {
			std::map<int, int>::iterator it = text_map.find(jump_addr);
			if (it != text_map.end()) // if jump addresss is in text_ary
			{
				i = (((it->first) - 0x400000) / 4) + 1;
				PC = it->first;
				jump_addr = 0;
			}

			else if (it == text_map.end()) {
				break;
			}
		}
	}

	if (PC == text||!opt_d) {
		std::cout << "PC: 0x" << std::hex << PC << std::endl;
		print_reg();
		std::cout << std::endl;
	}


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
			if (opt_m_ary[1] >= text) {
				int temp = (opt_m_ary[1] - text) / 4 + 1;
				for (int i = 0; i < temp; i++) {
					std::cout << "0x" << std::hex << text + 4 * i << ": 0x0" << std::endl;
				}
			}
		}
		else if (it2 != data_map.end()) {
			for (itd = it2; itd != data_map.end(); itd++) {
				std::cout << "0x" << std::hex << itd->first << ": 0x" << std::hex << itd->second << std::endl;
				if (itd->first == opt_m_ary[1])break;
			}
			if (opt_m_ary[1] >= data) {
				int temp = (opt_m_ary[1] - data) / 4 + 1;
				for (int i = 0; i < temp; i++) {
					std::cout << "0x" << std::hex << data + 4 * i << ": 0x0" << std::endl;
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

	//std::cout <<std::dec<< counter << std::endl;
  /* for (int i = 2; i < num_text + 2; i++) {

		std::cout <<std::hex<< binary_ary[i] << std::endl;
	} */

	/* std::map<int, int>::iterator it;

	 for (it = text_map.begin(); it != text_map.end(); it++)
	 {
		 std::cout << "Key : " << it->first << "   Value : "<< std::hex << it->second << std::endl;
	 }*/
	 //std::cout << std::hex<<last_check << std::endl;
	 // std::cout << counter << std::endl;

	File.close();
	return 0;
}


