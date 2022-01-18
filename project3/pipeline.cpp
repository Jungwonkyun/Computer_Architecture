#include "pipeline.h" 
#include "instr_info.h"
#include <iostream>
#include <deque>
#include <bitset>
using namespace std;

int output_PC = 0x400000;
int register_ary[32]{ 0 };
int jump_addr = 0;
int alu_result = 0;
int branch_result = 0;
int mem_result = 0;
int A_hazard = 0;
int B_hazard = 0;
int queue_size = 0;
int IF_PC = 0;
int ID_PC = 0;
int EX_PC = 0;
int MEM_PC = 0;
int WB_PC = 0;
int temp_addr = 0;
int same_PC = 0;

bool stall_the_pipeline = false; 
//bool always_taken = false;
bool load_stall = false;
bool branch_hazard = false;
bool prediction_fault = false;
bool jump_delay = false;
deque<instr> pipe_queue(5);

//bool id_ex_update = false;
struct IF_ID  if_id;
struct ID_EX  id_ex;
struct EX_MEM  ex_mem; 
struct MEM_WB  mem_wb;
struct Format format;
struct instr ins;


int find_pc(int binary){
	for (auto it = text_map.begin(); it != text_map.end(); it++) {
		if (it->second == binary)return it->first;
	}
}

void print_reg() {
	std::cout << "Registers:" << std::endl;
	for (int i = 0; i < 32; i++) {
		std::cout << "R" << std::dec << i << ": " << std::hex << "0x" << register_ary[i] << std::endl;
	}
}


int hazard_check_A(ID_EX* id_ex, EX_MEM * ex_mem){	
	
	int check = 0;
	//cout << "type: " << ex_mem->type << endl;
	
	//I format except for LW, LB 
	if (ex_mem->type == "i") {
		if (ex_mem->rt == id_ex->rs&& ex_mem->rt != id_ex->rt) {
			id_ex->fowarded = ex_mem->ALU_OUT; 
			check = 1;
		}
		else if (ex_mem->rt != id_ex->rs && ex_mem->rt == id_ex->rt ) {
			id_ex->fowarded = ex_mem->ALU_OUT;
			check = 2;
		}
		else if (ex_mem->rt == id_ex->rs && ex_mem->rt == id_ex->rt) {
			id_ex->fowarded = ex_mem->ALU_OUT;
			check = 3;
		}
	}
	
 	if (ex_mem->type == "r"&&ex_mem->rd != 0) {
		if (ex_mem->rd == id_ex->rs && ex_mem->rd != id_ex->rt) {
			id_ex->fowarded = ex_mem->ALU_OUT;
			check = 1;
		}
		else if (ex_mem->rd != id_ex->rs && ex_mem->rd == id_ex->rt) {
			id_ex->fowarded = ex_mem->ALU_OUT;
			check = 2;
		}
		else if (ex_mem->rd == id_ex->rs && ex_mem->rd == id_ex->rt) {
			id_ex->fowarded= ex_mem->ALU_OUT;
			check = 3;
		}
	}

	A_hazard = check;
	return check; 
}

int hazard_check_B(ID_EX* id_ex, MEM_WB* mem_wb) {
	int check = 0;
	//I format except for LW, LB 
	if (mem_wb->isa == "lw"|| mem_wb->isa == "lb") {
		
		if (mem_wb->rt == id_ex->rs && mem_wb->rt != id_ex->rt) {
			id_ex->fowarded = mem_wb->MEM_OUT;
			check = 1;
		}
		else if (mem_wb->rt != id_ex->rs && mem_wb->rt == id_ex->rt) {
			id_ex->fowarded = mem_wb->MEM_OUT;
			check = 2;
		}
		else if (mem_wb->rt == id_ex->rs && mem_wb->rt == id_ex->rt) {
			id_ex->fowarded = mem_wb->MEM_OUT;
			check = 3;
		}
	}

	else if (mem_wb->type == "r") {
		if (mem_wb->rd == id_ex->rs && mem_wb->rt != id_ex->rt) {
			id_ex->fowarded = mem_wb->ALU_OUT;
			check = 1;
		}
		else if (mem_wb->rd != id_ex->rs && mem_wb->rt == id_ex->rt) {
			id_ex->fowarded = mem_wb->ALU_OUT;
			check = 2;
		}
		else if (mem_wb->rd == id_ex->rs && mem_wb->rt == id_ex->rt) {
			id_ex->fowarded = mem_wb->ALU_OUT;
			check = 3;
		}
	}

	if (mem_wb->type == "i" && mem_wb->isa != "lw" && mem_wb->isa != "lb") {
		if (mem_wb->rt == id_ex->rs && mem_wb->rt != id_ex->rt) {
			id_ex->fowarded = mem_wb->ALU_OUT;
			/*cout << endl;
			cout << "forwarded:  " << id_ex->fowarded << endl << endl;*/
			check = 1;
		}
		else if (mem_wb->rt != id_ex->rs && mem_wb->rt == id_ex->rt) {
			id_ex->fowarded = mem_wb->ALU_OUT;
			check = 2;
		}
		else if (mem_wb->rt == id_ex->rs && mem_wb->rt == id_ex->rt) {
			id_ex->fowarded = mem_wb->ALU_OUT;
			check = 3;
		}
	}

	B_hazard = check;
	return check;
}

bool branch_hazard_check(ID_EX *id_ex, EX_MEM *ex_mem) {
	int check = false;

	//I format except for LW, LB 
	if (ex_mem->type == "i") {
		if (ex_mem->rt == id_ex->rs || ex_mem->rt == id_ex->rt) {
			check = true;
		}
	}

	else if (ex_mem->type == "r" ) {
		if (ex_mem->rt == id_ex->rs || ex_mem->rt == id_ex->rt) {
			check = true;
		}
	}

	A_hazard = check;
	return check;
}


string find_instr(unsigned int ins) {

	unsigned int op = ins >> 26;
	bitset<32>ins_bit(op);
	bitset<32>basic_bit(0);
	bitset<32>result_bit = ins_bit | basic_bit;

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

		R_class r_class;
		r_class.set_ins(op, rs, rt, rd, shamt, func);
		//r_class.print_ins();
		r_class.find_ins(op);
		
			format.rs = r_class.rs;
			format.rt = r_class.rt;
			format.rd = r_class.rd;
			format.shamt = r_class.shamt;
			format.type = "r";

		return r_class.isa;
	}

	else if (result_bit == 2 || result_bit == 3) {
		std::bitset<32> a = ins >> 26;
		std::bitset<32> b = (67108863 & ins);

		int op = a.to_ulong();
		int rest = b.to_ulong();
		
		J_class j_class;
		j_class.set_ins(op, rest);
		//j_class.print_ins();
		j_class.find_ins(op);
	
		format.target = j_class.target;
		format.type = "j";
		
		return j_class.isa;
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
		int sign_ck = (65535 & ins) & 0x00008000;  //check if 16 bit imm's MSB is 1 or 0  

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

		I_class i_class;
		i_class.set_ins(op, rs,rt,rest);
		//i_class.print_ins();
		i_class.find_ins(op);

		format.rs = i_class.rs;
		format.rt = i_class.rt;
		format.imm = i_class.off_or_imm;
		format.type = "i"; 
		
		return i_class.isa;
	}
}


void WB(struct instr ins) {
	
	string isa3 = find_instr(ins.binary);
	if (ins.binary == 1) {
		format.type = "no";
	}

	if (isa3 == "jal") {
		register_ary[31] = mem_wb.ALU_OUT;
	}

	if (format.type == "r") {
		register_ary[format.rd] = mem_wb.ALU_OUT;
	}

	else if (format.type == "i") {
		if (isa3 == "lb" || isa3 == "lw") {
			register_ary[format.rt] = mem_wb.MEM_OUT;
		}
		else if (isa3 != "lw" && isa3 != "lb" && isa3 != "sw" && isa3 != "sb") {
			register_ary[format.rt] = mem_wb.ALU_OUT;
		}
	}


	if (ins.binary == 1)isa3 = "NOOP";
	WB_PC = ins.PC;
	//std::cout << "At WB stage  " << isa3 << endl;
	//cout << "PC: " << hex << ins.PC << endl;

	return;
}



//���ɾ� ����� �۵��ϴ��� Ȯ���ϱ� 
void MEM(struct instr ins) {

	string isa2 = find_instr(ins.binary);
    //cout << "MEM_ALU_OUT: " << hex<<ex_mem.ALU_OUT << endl;


	if (isa2 == "lw") {
		int addr = ex_mem.ALU_OUT;
		int check = 0;
		int result = 0;

		//cout << "alu_out:  "<<addr << endl;
		for (auto it = data_map2.begin(); it != data_map2.end(); it++) {
			if (it->first == addr) {
				if (check == 0) result +=(it->second) << 24;
				else if(check == 1)result += (it->second) << 16;
				else if (check == 2)result += (it->second) << 8;
				else if (check == 3)result += it->second;
				check++;
				addr++;
			}
			if (check == 4)break;
		}
		mem_result = result; 
	}

	else if (isa2 == "lb") {
		int addr = ex_mem.ALU_OUT;
		int result = 0;

		for (auto it = data_map2.begin(); it != data_map2.end(); it++) {
			if (it->first == addr) {
				result = it->second;
			}
		}
		mem_result = result;
	}

	else if (isa2 == "sw") {
		int addr = ex_mem.ALU_OUT;
		int check = 0;
		int result = 0;
		int last = 0;

		int a = (register_ary[format.rt] & 0xff000000) >> 24;  
		int b = (register_ary[format.rt] & 0x00ff0000) >> 16;
		int c = (register_ary[format.rt] & 0x0000ff00) >> 8;
		int d = register_ary[format.rt] & 0x000000ff;

		for (auto it = data_map2.begin(); it != data_map2.end(); it++) {
			if (it->first == addr) {
				if (check == 0)it->second = a;
				else if (check == 1)it->second = b;
				else if (check == 2)it->second = c;
				else if (check == 3)it->second = d;
				check++;
				addr++;
			}
			if (check == 4)break; 
			last = it->first;
		}
	}

	else if (isa2 == "sb") {


		int share = format.imm / 4;
		int rest = format.imm % 4;
		int bk_pnt = 0;
		int count = 0;
		int test = 0;
		int result = register_ary[format.rt];
		int temp = register_ary[format.rs];

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

	else {
		//cout << imm << endl;
		mem_result = 0;
	}

	mem_wb.ALU_OUT = ex_mem.ALU_OUT; 
	mem_wb.MEM_OUT = mem_result;
	mem_wb.type = format.type; 
	mem_wb.rd = format.rd;
	mem_wb.rt = format.rt;
	mem_wb.rs = format.rs; 
	mem_wb.isa = isa2;

	if (ins.binary == 1) {
		isa2 = "NOOP";
		mem_wb.ALU_OUT = 0;
		mem_wb.MEM_OUT = 0;
		mem_wb.type = "NO";
		mem_wb.rd = 0;
		mem_wb.rt = 0;
		mem_wb.rs = 0;
	}

	MEM_PC = ins.PC ;
	//std::cout << "At MEM stage  " << isa2 << endl;

	return;
}


void EX(struct instr ins) {

	//id_ex_update = true; 
	string isa = find_instr(ins.binary);

	//cout <<"hazard check: "<<A_hazard<< "  forward value " << id_ex.fowarded_by_EX_MEM << endl;
	if (isa == "lb") {
	}

	int hazard_type = A_hazard | B_hazard;
	alu_result = ALU_func(&id_ex, isa, id_ex.fowarded,hazard_type );
	

	if (ins.binary != 1) {
		ex_mem.ALU_OUT = alu_result;
		ex_mem.BR_TARGET = branch_result;
		ex_mem.rd = format.rd;
		ex_mem.rs = format.rs;
		ex_mem.rt = format.rt;
		ex_mem.type = format.type;
		ex_mem.isa = isa;
	}
	if (ins.binary == 1) {
		ex_mem.type ="No";
		ex_mem.ALU_OUT = 0;
		ex_mem.BR_TARGET = 0;
		ex_mem.rd = 0;
		ex_mem.rs = 0;
		ex_mem.rt = 0;
	}

	if (ins.binary == 1)isa = "NOOP";
	EX_PC = ins.PC;
	//std::cout << "At EX stage  " << isa << endl;
	//cout << "PC: " << hex << ins.PC << endl;
	return;
}

void ID(struct instr ins) {

	string isa = find_instr(ins.binary);

	int rs_forward = 0;
	int rt_forward = 0;

	/*cout << "IF_ID Register" << endl;
	cout << hex << "NPC:  " << if_id_reg->NPC << "    ";
	cout << hex << "Instruction:  " << if_id_reg->Instr << endl;*/
	
	//�б� ���� ������Ѵ�. beq, bne�� �б� ������ �����

	if (stall_the_pipeline == false) {
		id_ex.NPC = ins.PC+4;
		id_ex.rs = format.rs;
		id_ex.rt = format.rt;
		id_ex.rd = format.rd;
		id_ex.Imm = format.imm;
		id_ex.shamt = format.shamt;
		id_ex.Target = format.target;
		id_ex.isa = isa;
	}
	
	int check2 = hazard_check_B(&id_ex, &mem_wb);
	int check = hazard_check_A(&id_ex, &ex_mem);

	if (isa == "j" ) {
		//stall_the_pipeline = true;
		jump_addr = format.target * 4;
	}

	if (isa == "jal") {
		//stall_the_pipeline = true;
		jump_addr = format.target * 4;
		temp_addr = ins.PC+4;
	}

	if (isa == "jr") {
		//stall_the_pipeline = true;
		if (check == 0 && check2 == 0)jump_addr = temp_addr;
		if (check == 1 || check2 == 1)jump_addr = id_ex.fowarded;
	}

	if (isa == "bne" || isa == "beq") {
		bool check_hazard = branch_hazard_check(&id_ex, &ex_mem);
		
		if (check_hazard == true) {
			pipe_queue[0] = pipe_queue[1];
			pipe_queue[1].PC = 0;
			pipe_queue[1].binary = 1;
		
			branch_hazard = true;
			isa = "NOOP";
			id_ex.NPC = 0;
			id_ex.rs = 0;
			id_ex.rt = 0;
			id_ex.rd = 0;
			id_ex.Imm = 0;
			id_ex.shamt = 0;
			id_ex.Target = 0;
			ins.PC = 0;
			ins .binary = 0;
		}
		
		if (branch_hazard == false&&check != 0) {
			if (check == 1)rs_forward = id_ex.fowarded;
			else if (check == 2)rt_forward = id_ex.fowarded;
			else if (check == 3) {
				rt_forward = id_ex.fowarded;
				rs_forward = id_ex.fowarded;
			}
		}
		
		if (branch_hazard == false&&check2 != 0) {
			if (check2 == 1) rs_forward = id_ex.fowarded;			
			else if (check2 == 2)rt_forward = id_ex.fowarded;
			else if (check2 == 3) {
				rt_forward = id_ex.fowarded;
				rs_forward = id_ex.fowarded;
			}
		}
		
		if (branch_hazard == false&&check == 0 && check2 == 0) {
			rs_forward = register_ary[format.rs];
			rt_forward = register_ary[format.rt];
		}

		if (branch_predictor == false && isa== "bne") {		
			if (rs_forward != rt_forward) {
				//if branch prediction fault 3 cycle stall  
				prediction_fault = true;
				jump_addr = ins.PC+4 + format.imm * 4;	
			}
		}
		if (branch_predictor == false && isa == "beq") {
			if (rs_forward == rt_forward) {
				//if branch prediction fault 3 cycle stall  
				prediction_fault = true;
				jump_addr = ins.PC + 4 + format.imm * 4;
			}
		}

		if (branch_predictor == true && isa == "bne") {
			//branch prediction fault
			if (rs_forward == rt_forward) {
			//if branch prediction fault 3 cycle stall  
			prediction_fault = true;
			}
			
			//branch prediction success
			else if (rs_forward != rt_forward) {
				//if branch prediction success 1 cycle stall  
				jump_addr = ins.PC + 4 + format.imm * 4;
			}
		}
		
		if (branch_predictor == true && isa == "beq") {
			//branch prediction fault
			if (rs_forward != rt_forward) {
				//if branch prediction fault 3 cycle stall  
				prediction_fault = true;
			}

			//branch prediction success
			else if (rs_forward == rt_forward) {
				//if branch prediction success 1 cycle stall  
				jump_addr = ins.PC + 4 + format.imm * 4;
			}
		}


	}
	if (ins.binary == 1)isa = "NOOP";

	ID_PC = ins.PC;
	//std::cout << "At ID stage  " << isa << endl;
	//cout << "PC: " << hex << pipe_queue[1].PC << endl;
	
	return;
}

void IF(struct instr ins) {
	
	string a = find_instr(ins.binary);

	
	//load use hazard
	if (id_ex.isa == "lw" || id_ex.isa == "lb" || id_ex.isa == "sw" || id_ex.isa == "sb") {
		
		if (id_ex.rt == format.rs || id_ex.rt == format.rt) {
			load_stall = true;  
			pipe_queue[0].binary = 1;
			pipe_queue[0].PC = 0;
		}
	}
	if (load_stall == false && stall_the_pipeline == false) {
		if_id.Instr = ins.binary;
		if_id.NPC = ins.PC + 4;
		if_id.rs = format.rs;
		if_id.rt = format.rt;
		if_id.isa = a;
	}

	if (a == "jal" || a == "jr" || a == "j") {
		stall_the_pipeline = true;
		same_PC = ins.PC;
	}
	
	if (pipe_queue[0].binary == 1)a = "NOOP";

	IF_PC = ins.PC;
	//std::cout << "At IF stage  " << a << endl;
	//cout << "PC: " << hex << ins.PC << endl;
	output_PC = ins.PC;
	return;
}


void start_pipeline(int num) {
	if (opt_p == true) {
		cout << dec << "==== Completion cycle: " << 0 << " ====" << endl << endl;
		cout << "Current pipeline PC state:" << endl;
		std::cout << hex << "{ " << 0 << " | " << 0 << " | " << 0 << " | " << 0 << " | " << 0 << " }" << endl << endl;
	}

	if (opt_d == true) {
		std::cout << "PC: 0x" << std::hex << 0x400000 << std::endl;
		print_reg();
		cout << endl;
	}

	bool end_signal = false;
	bool cc = false;
	int prediction_stall = 0;
	static int cycle = 0;


	for (int i = 2; i < num + 2; i++) {
		// if normal state enqueue next instruction 

		if (pipe_queue.size() >= 5)pipe_queue.pop_back();

		if (prediction_stall == 3) {
			prediction_fault = false;
			prediction_stall = 0;
			same_PC = pipe_queue[3].PC + 4;
		}

		if (prediction_fault == true) {
			same_PC = pipe_queue[1].PC;
			ins.PC = 0;
			ins.binary = 1;
			pipe_queue.push_front(ins);
		}

		if (load_stall == true) {
			same_PC = pipe_queue[0].PC;
			pipe_queue[0].PC = 0;
			pipe_queue[0].binary = 1;
			i -= 2;
			load_stall = false;
			continue;
		}

		if (stall_the_pipeline == false && jump_addr == 0 && prediction_fault == false) {
			ins.PC = find_pc(binary_ary[i]);
			ins.binary = binary_ary[i];
			pipe_queue.push_front(ins);
		}


		//if jump instruction need 1 cycle flush for jump 
		if (stall_the_pipeline == true) {
			ins.PC = 0;
			ins.binary = 1;
			pipe_queue.push_front(ins);
			stall_the_pipeline = false;
			cc = true;
			same_PC = pipe_queue[1].PC;
			i--;
		}


		//if At ID state instruction is junp instruction enqueue jumped instruction
		if (stall_the_pipeline == false && prediction_fault == false && jump_addr != 0) {
			map<int, int>::iterator it = text_map.find(jump_addr);
			if (it != text_map.end()) // if jump addresss is in text_ary
			{
				i = (((it->first) - 0x400000) / 4) + 2;
				ins.PC = jump_addr;
				ins.binary = binary_ary[i];
				pipe_queue.push_front(ins);
				//cout << hex << "jump_addr: " << jump_addr << endl << endl;	
				jump_addr = 0;
				//PC = jump_addr;
				same_PC = pipe_queue[1].PC;
			}
			else if (it == text_map.end()) {
				end_signal = true;
				ins.PC = 0;
				ins.binary = 1;
				pipe_queue.push_front(ins);
			}
		}

		//cout << "queue info:  " <<hex<< pipe_queue[0].PC <<"    "<< pipe_queue[1].PC << "    " << pipe_queue[2].PC << "    " << pipe_queue[3].PC << "    " << pipe_queue[4].PC << endl;

		int zero = 0;
		for (int i = 0; i < 5; i++) {
			if (pipe_queue[i].binary == 0)zero++;
		}
		queue_size = pipe_queue.size() - zero;

		if (queue_size == 2) {
			ID(pipe_queue[1]);
		}

		else if (queue_size == 3) {
			EX(pipe_queue[2]);
			ID(pipe_queue[1]);
		}

		else if (queue_size == 4) {
			MEM(pipe_queue[3]);
			EX(pipe_queue[2]);
			ID(pipe_queue[1]);
		}

		else if (queue_size >= 5) {
			WB(pipe_queue[4]);
			MEM(pipe_queue[3]);
			EX(pipe_queue[2]);
			ID(pipe_queue[1]);
		}


		if (branch_hazard == true) {
			same_PC = pipe_queue[1].PC;
			branch_hazard = false;
			i--;
		}

		if (prediction_fault == true) {
			if (prediction_stall == 0)same_PC = pipe_queue[1].PC;
			pipe_queue[0].PC = 0;
			pipe_queue[0].binary = 1;
			prediction_stall++;
			i--;
		}

		IF(pipe_queue[0]);
		cycle++;
		

		if (stall_the_pipeline == false && pipe_queue[0].binary == binary_ary[num + 1] && end_signal == false) {
			while (true) {
				ins.PC = 0;
				ins.binary = 1;
				pipe_queue.push_front(ins);

				WB(pipe_queue[4]);
				MEM(pipe_queue[3]);
				EX(pipe_queue[2]);
				ID(pipe_queue[1]);
				IF(pipe_queue[0]);

	
				if (opt_p == true) {
					cout <<dec<< "==== Completion cycle: " << cycle << " ====" << endl << endl;
					cout << "Current pipeline PC state:" << endl;
					std::cout << hex << "{ " << IF_PC << " | " << ID_PC << " | " << EX_PC << " | " << MEM_PC << " | " << WB_PC << " }" << endl << endl;
				}

				if (opt_d == true) {
					cout << dec << "Current register values:" << endl;
					//if (IF_PC == 0 && ID_PC == 0 && EX_PC == 0 && MEM_PC == 0 && WB_PC == 0)output_PC = 0x400000 + 4 * num ;
					std::cout << "PC: 0x" << std::hex << 0x400000 + 4 * num << std::endl;
					print_reg();
				}
				if (IF_PC == 0 && ID_PC == 0 && EX_PC == 0 && MEM_PC == 0 && WB_PC == 0) {
					cout << dec << "==== Completion cycle: " << cycle << " ====" << endl << endl;
					cout << "Current pipeline PC state:" << endl;
					std::cout << hex << "{ " << " | "<< " | " <<" | " << " | " <<" }" << endl << endl;
					std::cout << "PC: 0x" << std::hex << 0x400000 + 4 * num << std::endl;
					print_reg();
					break;
				}
				cycle++;
				if (cycle >= 314)break;
			}
			break;
		}


		if ((stall_the_pipeline == false && pipe_queue[0].binary == binary_ary[num + 1]) || end_signal == true) {
			while (true) {
				ins.PC = 0;
				ins.binary = 1;
				pipe_queue.push_front(ins);

				WB(pipe_queue[4]);
				MEM(pipe_queue[3]);
				EX(pipe_queue[2]);
				ID(pipe_queue[1]);
				IF(pipe_queue[0]);

				if (opt_p == true) {
					cout << dec << "==== Completion cycle: " << cycle << " ====" << endl << endl;
					cout << "Current pipeline PC state:" << endl;
					std::cout << hex << "{ " << IF_PC << " | " << ID_PC << " | " << EX_PC << " | " << MEM_PC << " | " << WB_PC << " }" << endl << endl;
				}
				if (opt_d == true) {
					cout << "Current register values:" << endl;
					//if (IF_PC == 0 && ID_PC == 0 && EX_PC == 0 && MEM_PC == 0 && WB_PC == 0)output_PC = 0x400000 + 4 * num ;
					std::cout << "PC: 0x" << std::hex << 0x400000 + 4 * num << std::endl;
					print_reg();
				}
				std::cout << endl;

				if (IF_PC == 0 && ID_PC == 0 && EX_PC == 0 && MEM_PC == 0 && WB_PC == 0) {
					cout << dec << "==== Completion cycle: " << cycle << " ====" << endl << endl;
					cout << "Current pipeline PC state:" << endl;
					std::cout << hex << "{ " << " | "  << " | " <<  " | " <<  " | " <<  " }" << endl << endl;
					std::cout << "PC: 0x" << std::hex << 0x400000 + 4 * num << std::endl;
					print_reg();
					break;
				}
				cycle++;
				if (cycle >= 314)break;
			}
			break;
		}

		if (stall_the_pipeline == true && pipe_queue[0].binary == binary_ary[num + 1]) {
			same_PC = pipe_queue[1].PC + 4;
			i--;
		}

		//cout << "queue info:  " <<hex<< pipe_queue[0].PC <<"    "<< pipe_queue[1].PC << "    " << pipe_queue[2].PC << "    " << pipe_queue[3].PC << "    " << pipe_queue[4].PC << endl;
		if (opt_p == true) {
			cout << dec << "==== Completion cycle: " << cycle << " ====" << endl << endl;
			cout << "Current pipeline PC state:" << endl;
			std::cout << hex << "{ " << IF_PC << " | " << ID_PC << " | " << EX_PC << " | " << MEM_PC << " | " << WB_PC << " }" << endl << endl;
		}
		if (opt_d == true) {
			cout << "Current register values:" << endl;
			if (prediction_fault == true || stall_the_pipeline == true||cc==true)output_PC = same_PC;
			std::cout << "PC: 0x" << std::hex << output_PC + 4 << std::endl;
			print_reg();
		}
		std::cout << endl;

		if (opt_n == true && cycle >= opt_num_ins) {
			cout << dec << "==== Completion cycle: " << cycle << " ====" << endl << endl;
			cout << "Current pipeline PC state:" << endl;
			std::cout << hex << "{ " << IF_PC << " | " << ID_PC << " | " << EX_PC << " | " << MEM_PC << " | " << WB_PC << " }" << endl << endl;
			
			cout << "Current register values:" << endl;
			if (prediction_fault == true || stall_the_pipeline == true||cc==true)output_PC = same_PC;
			std::cout << "PC: 0x" << std::hex << output_PC + 4 << std::endl;
			print_reg();
			break;
		}
		cc = false;
		if (cycle >= 314)break;
	}
}