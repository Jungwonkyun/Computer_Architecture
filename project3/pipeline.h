#ifndef __PIPELINE_H
#define __PIPELINE_H
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include<map>
#include<string.h>

using namespace std;

extern map<int, int> text_map;
extern map<int, int> text_map2;
extern map<int, int> data_map;
extern map<int, int> data_map2;
extern int register_ary[32];
extern int binary_ary[100];
//extern int instr_ary[100]; 
extern int jump_addr;
extern int text_info;
extern int data_info;

extern  int opt_m_ary[2];
extern int opt_num_ins;
extern bool opt_m;
extern bool opt_d;
extern bool opt_n;
extern bool opt_p;
extern bool branch_predictor; //true = always taken false = always not taken


struct inst {
	int binary_info;
	int address_info;
};

struct IF_ID {
	int Instr = 0;
	int NPC = 0;
	int rs = 0;// �������� ��� 
	int rt = 0;// �������� ��� 
	string isa;// �������� ���
};

struct ID_EX {
	int NPC = 0;
	int rs = 0;
	int rt = 0;
	int rd = 0;
	int shamt = 0; // �������� ���  
	int Imm = 0; 
	int Target = 0; //�������� ���
	string isa;
	int fowarded = 0; //�������� ���
};

struct EX_MEM {
	int ALU_OUT = 0;
	int BR_TARGET = 0;
	int rs = 0; // �������� ���  
	int rt = 0;  //�������� ���
	int rd = 0;   //�������� ���
	int forwarded;//�������� ���
	string type;  //�������� ���
	string isa; //Ȯ�ο�
};

struct MEM_WB {
	int ALU_OUT = 0;
	int MEM_OUT = 0;
	int rs = 0; // �������� ���  
	int rt = 0;  //�������� ���
	int rd = 0; //�������� ���
	string type;  //�������� ���
	string isa; //Ȯ�ο�
};

struct Format{
	int rs = 0;
	int rt = 0;
	int rd = 0;
	int shamt = 0; 
	int imm = 0;
	int target = 0;
	string isa;
	string type;
};

struct instr {
	int PC; 
	int binary;
};

void start_pipeline(int num ); //initialize pipeline 
int ALU_func(ID_EX* id_ex,string isa, int fowarded, int check); //alu unit function 
string find_instr(unsigned int ins, int pc); // indentify instruction by op & funct code


#endif