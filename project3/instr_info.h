#ifndef __INSTR_INFO_H
#define __INSTR_INFO_H
# define _CRT_SECURE_NO_WARNINGS
#include<string>

class R_class {
public:
	int op, rs, rt, rd, shamt, funct;
	std::string  isa;
	void set_ins(int r1, int r2, int r3, int r4, int r5, int r6);
	void print_ins();
	void find_ins(int op);
};

class I_class {
public:
	int op, rs, rt, off_or_imm;
	std::string  isa;
	void set_ins(int r1, int r2, int r3, int r4);
	void print_ins();
	void find_ins(int op);
};

class J_class {
public:
	int op, target;
	std::string  isa;
	void set_ins(int r1, int r2);
	void print_ins();
	void find_ins(int op);
};


#endif