#define CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <stdlib.h>
#include<map>
#include <iterator>
#include<fstream>
#include<vector>
#include<sstream>
#include <fstream>
#include<algorithm>
#include<string>
#include<regex>
#include<bitset>

using std::string;

std::vector<unsigned int>isa_binary;
std::vector<string>isa_vec;


// R_type to binary 
class R_class {
public:
    int op,rs,rt,rd,shamt,funct; 
    string isa;
    R_class(string oper, int first, int second, int third) {
        if (oper == "sll" || oper == "srl") {   
            isa = oper;
            op = 0;
            rs = 0;
            rt = second;
            rd = first;
            shamt = third; 
            funct = 0;
        }
        else if (oper == "jr") {
            isa = oper;
            op = 0;
            rs = first;
            rt = 0;
            rd = 0;
            shamt = 0;
            funct = 0;
        }
        else {      
            isa = oper;
            op = 0;
            rs = second;
            rt = third;
            rd = first;
            shamt = 0;
            funct = 0;
        }
        make_binary(isa,op,rs,rt,rd,shamt,funct);
    }
        
    void make_binary(string isa, int op,int rs ,int rt ,int rd, int shamt,int funct) {
     
        if (isa == "addu") {
            funct = 33;
        }
        else if (isa == "and") {
            funct = 36;
        }
        else if (isa == "nor") {
            funct = 39;
        }
        else if (isa == "or") {
            funct = 37;
        }
        else if (isa == "sltu") {
            funct = 43;
        }
        else if (isa == "subu") {
            funct = 35;
        }
        else if (isa == "sll") {
            funct = 0;
        }
        else if (isa == "srl") {
            funct = 2;
        }
        else if (isa == "jr") {
            funct = 8;
        }
     std::bitset<32>basic_bit(0);
     std::bitset<32>op_bit(op);
     std::bitset<32>rs_bit(rs);
     std::bitset<32>rt_bit(rt);
     std::bitset<32>rd_bit(rd);
     std::bitset<32>shamt_bit(shamt);
     std::bitset<32>funct_bit(funct);

     std::bitset<32>result_bit = basic_bit | rs_bit << 21 | rt_bit << 16 | rd_bit << 11 | shamt_bit << 6 | funct_bit;
   
     isa_binary.push_back(result_bit.to_ulong());
     //isa_vec.push_back(isa);
     isa_vec.push_back(isa);
    }
};


//I_type to binary
class I_class {
public:
    int op, rs, rt, off_or_imm;
    string isa; 
    I_class(string oper, int first, int second, int third) {
        if (oper == "lui") {
            isa = oper;
            op = 0;
            rs = 0;
            rt = first;
            off_or_imm = second;
        } 
        else if(oper == "lw"||oper == "lb"||oper=="sw"||oper=="sb"){
            isa = oper;
            op = 0;
            rs = third;
            rt = first;
            off_or_imm = second;
          // std::cout << isa <<" "<<rs<<" "<<rt<<" "<<off_or_imm<< std::endl;
        }
        else if (oper == "bne" || oper == "beq") {
            isa = oper;
            op = 0;
            rs = first;
            rt = second;
            off_or_imm = third;
        }       
        else {
            isa = oper;
            op = 0;
            rs = second;
            rt = first;
            off_or_imm = third;
        }
     
        make_binary(isa, op, rs, rt, off_or_imm);
    }


    void make_binary(string isa, int op, int rs, int rt,int third) {
        
        if (isa == "addiu") {
            op = 9;
        }
        else if (isa == "andi") {
            op = 12;
        }
        else if (isa == "lui") {
            op = 15;
        }
        else if (isa == "ori") {
            op = 13;
        }
        else if (isa == "sltiu") {
            op = 11;
        }
        else if (isa == "beq") {
            op = 4;
        }
        else if (isa == "bne") {
            op = 5;
           // std::cout << "op " << op << " rs " << rs << " rt " << rt << " offset " << third << std::endl;
        }
        else if (isa == "lw") {
            op = 35;
        }
        else if (isa == "lb") {
            op = 32;
        }
        else if (isa == "sw") {
            op = 43;
        }
        else if (isa == "sb") {
            op = 40;
     
        }
        std::bitset<32>basic_bit(0);
        std::bitset<32> signed_bit(65535);
        std::bitset<32>op_bit(op);
        std::bitset<32>rs_bit(rs);
        std::bitset<32>rt_bit(rt);
        std::bitset<32> result_bit;

        if (off_or_imm >= 0) {
            std::bitset<32>off_or_imm_bit(off_or_imm);
             result_bit = basic_bit | op_bit << 26 | rs_bit << 21 | rt_bit << 16 | off_or_imm_bit;
        }
        else {
            std::bitset<32>off_or_imm_bit(off_or_imm);
            std::bitset<32>temp_bit = signed_bit & off_or_imm_bit;
            result_bit =  op_bit << 26 | rs_bit << 21 | rt_bit << 16 | temp_bit;
        }
       
        isa_binary.push_back(result_bit.to_ulong());
        isa_vec.push_back(isa);  
    }
};

class J_class {
public:
    int op, target; 
    string isa;
    J_class(string oper, int addr) {
        isa = oper;
        op = 0;
        target = addr;
        make_binary(isa, op, target);
    }

    void make_binary(string isa, int op, int target) {
        if (isa == "j") {
            op = 2;
        }
        else if (isa == "jal") {
            op = 3;
        }
        std::bitset<32>basic_bit(0);
        std::bitset<32>op_bit(op);
        std::bitset<32>target_bit(target);
        std::bitset<32>result_bit = basic_bit | op_bit << 26 | target_bit;
        isa_binary.push_back(result_bit.to_ulong());
        isa_vec.push_back(isa);

    }
};

//SP_ISA element pharsing function
void SP_func(string isa , int rt = 0 , int imm = 0) {
    
    std::bitset<16>check_bit1(imm);
    std::bitset<16>check_bit2(0);
    std::bitset<16>result_bit = check_bit1 | check_bit2;

    if (result_bit == 0) {
	int high = imm & 0xffff0000;
	std::bitset<32>bit1(high>>16);    	
 
        I_class I("lui",rt, bit1.to_ulong(), 0);
    }
    else {
	int high = imm & 0xffff0000;
        int low = imm & 0x0000ffff;
        std::bitset<32>bit1(high>>16);
        std::bitset<32>bit2(low);	

        I_class I("lui", rt, bit1.to_ulong(), 0);
        I_class I_2("ori", rt, rt,bit2.to_ulong());

    }
    return;
}

int main(int argc, char** argv)
{
    // ISA type vector 
    std::vector<string>R_ISA = { "addu","and","jr","nor","or","sltu","sll","srl","subu" };
    std::vector<string>I_ISA = { "addiu","andi","beq","bne","lui","lw","ori","sltiu","sw","lb","sb" };
    std::vector<string>J_ISA = { "j","jal" };
    std::vector<string>SP_ISA = { "la" };

    std::string string1 = ".text";
    std::string string2 = ".word";
    std::string last = " "; // check last branch

    bool check = false;
    bool check2 = false;
    int wnum = 0; // number of word
    int anum = 0; //number of array 
    int labeladd = 0x10000000; // start of data address
    int text_add = 0x400000; // start of text address
    int num_ISA = 0;


    std::map<string, int>label_ary; // label array dictionary
    std::map<string, int> isa_ary; // isa array dictionary
    std::map<string, int> branch_ary;
    std::vector<int> value_ary;

    std::ifstream f(argv[1]); //  std::ifstream f(argv[1]);
    std::ifstream f2(argv[1]);

   // f.open("sample.s");
    //f2.open("sample.s");
    
    if (f.is_open()) {
        for (string line; std::getline(f, line);) {

            std::istringstream iss{ std::regex_replace(line, std::regex("[)(,$]"), " ") };
            std::vector<string> file_line{ std::istream_iterator<string>{iss},std::istream_iterator<string>{} };

            if (line == "")continue;
            if (file_line.at(0) == string1)check = true; // if first element == .text --> text_line will be started 
            if (file_line.at(0) == string2 || (file_line.size() > 1 && file_line.at(1) == string2)) {
                wnum++;

                if (file_line.at(0) == string2) {
                    if (file_line.at(1).find("0x", 0) != string::npos) {
                        value_ary.push_back(stoul(file_line.at(1), NULL, 16));
                    }
                    else {
                        value_ary.push_back(stoul(file_line.at(1)));
                    }
                }
                else if (file_line.at(1) == string2) {
                    if (file_line.at(2).find("0x", 0) != string::npos) {
                        value_ary.push_back(stoul(file_line.at(2), NULL, 16));
                    }
                    else {
                        value_ary.push_back(stoul(file_line.at(2)));
                    }
                }
            }
            if (std::find(R_ISA.begin(), R_ISA.end(), file_line.at(0)) != R_ISA.end())num_ISA++;
            if (std::find(I_ISA.begin(), I_ISA.end(), file_line.at(0)) != I_ISA.end()) {
                num_ISA++;
                if (file_line.at(0) == "bne" || file_line.at(0) == "beq") {
                    text_add = 0x400000 + 4 * (num_ISA - 1);
                    branch_ary.insert(std::pair<std::string, int>(file_line.at(0), text_add));
                }
            }
            if (std::find(J_ISA.begin(), J_ISA.end(), file_line.at(0)) != J_ISA.end())num_ISA++;
            if (std::find(SP_ISA.begin(), SP_ISA.end(), file_line.at(0)) != SP_ISA.end()) {
                std::map<std::string, int>::iterator it;
                int ary_addr = 0;

                for (it = label_ary.begin(); it != label_ary.end(); it++) {
                    if (it->first == file_line.at(2))ary_addr = it->second;
                }
                std::bitset<16>bit1(ary_addr);
                std::bitset<16>bit2(0);
                std::bitset<16>result_bit = bit1 | bit2;

                if (result_bit != 0) {
                    num_ISA += 2;
                }
                if (result_bit == 0) {
                    num_ISA++;
                }
            }

            if (file_line.at(0) == string1)check = true;

            if (file_line.at(0).find(':', 0) <= sizeof(file_line.at(0)) && check == false) // only .data  ~  .text can be label 
            {
                anum++;
                file_line.at(0).erase(std::find(file_line.at(0).begin(), file_line.at(0).end(), ':'));
                std::string labelname = file_line.at(0);
                labeladd = labeladd + 4 * (wnum - 1); // calculate data address
                label_ary.insert(std::pair<std::string, int>(labelname, labeladd)); // dictionary with label name & address    
            }

            if (file_line.at(0).find(':', 0) <= sizeof(file_line.at(0)) && check == true) // after text area,  fine all branch 
            {
                file_line.at(0).erase(std::find(file_line.at(0).begin(), file_line.at(0).end(), ':'));
                text_add = 0x400000 + 4 * (num_ISA);
                branch_ary.insert(std::pair<std::string, int>(file_line.at(0), text_add));

                //if branch is written with isa in one sentence
                if (file_line.size() != 1) num_ISA++;
            }
        }

    if (f2.is_open()){
        for (string line; std::getline(f2, line);) {

            std::istringstream iss{ std::regex_replace(line, std::regex("[)(,$]"), " ") };
            std::vector<string> file_line{ std::istream_iterator<string>{iss},std::istream_iterator<string>{} };

            if (line == "")continue;
            if (file_line.at(0) == string1)check2 = true; // if first element == .text --> text_line will be started 

            //if branch is written with isa in one sentence
            if (check2 == true && file_line.size() != 1 && file_line.at(0).find(':', 0) <= sizeof(file_line.at(0))) {
                if (file_line.size() == 3) {
                    file_line.at(0) = file_line.at(1);
                    file_line.at(1) = file_line.at(2);
                }
                else if (file_line.size() == 4) {
                    file_line.at(0) = file_line.at(1);
                    file_line.at(1) = file_line.at(2);
                    file_line.at(2) = file_line.at(3);
                }
                else if (file_line.size() == 5) {
                    file_line.at(0) = file_line.at(1);
                    file_line.at(1) = file_line.at(2);
                    file_line.at(2) = file_line.at(3);
                    file_line.at(3) = file_line.at(4);
                }
            }

            //distribute ISA type 
            if (std::find(R_ISA.begin(), R_ISA.end(), file_line.at(0)) != R_ISA.end())
            {

                if (file_line.at(0) == "jr") {

                    R_class R(file_line.at(0), stoul(file_line.at(1)), 0, 0);
                }
                else {
                    R_class R(file_line.at(0), stoul(file_line.at(1)), stoul(file_line.at(2)), stoul(file_line.at(3)));
                }
            }

            if (std::find(I_ISA.begin(), I_ISA.end(), file_line.at(0)) != I_ISA.end())
            {
                // num_ISA++;

                if (file_line.at(0) == "lui") {
                    I_class I(file_line.at(0), stoul(file_line.at(1)), stoul(file_line.at(2)), 0);
                }

                else if (file_line.at(0) == "bne" || file_line.at(0) == "beq")
                {
                    int isa_add = branch_ary[file_line.at(0)];
                    int  branch_addr = branch_ary[file_line.at(3)];

                    int offsett = (branch_addr - isa_add - 4) / 4;

                    I_class I(file_line.at(0), stoul(file_line.at(1)), stoul(file_line.at(2)), offsett);
                }

                else {
                    // if 3th element is hexidemical int make it to int 
                    if (file_line.at(3).find("0x") != string::npos) {
                        const char* a = file_line.at(3).c_str();
                        long n = strtoul(a, NULL, 16);
                        I_class I(file_line.at(0), stoul(file_line.at(1)), stoul(file_line.at(2)), n);
                    }
                    else {
                        I_class I(file_line.at(0), stoul(file_line.at(1)), stoul(file_line.at(2)), stoul(file_line.at(3)));
                    }
                }
            }

            if (std::find(J_ISA.begin(), J_ISA.end(), file_line.at(0)) != J_ISA.end())
            {
                int  branch_addr = branch_ary[file_line.at(1)];
                //std::cout << std::hex<<branch_addr << std::endl;

                if (file_line.at(1).find("0x") != string::npos) {
                    const char* a = file_line.at(1).c_str();
                    long n = strtoul(a, NULL, 16);
                    J_class I(file_line.at(0), n);
                }
                else {
                    //J foramt -> target /4 
                    J_class J(file_line.at(0), branch_addr / 4);
                    // std::cout << std::hex << branch_addr / 4 << std::endl;
                }
            }

            if (std::find(SP_ISA.begin(), SP_ISA.end(), file_line.at(0)) != SP_ISA.end())
            {
                std::map<std::string, int>::iterator it;
                int ary_addr;

                for (it = label_ary.begin(); it != label_ary.end(); it++) {
                    if (it->first == file_line.at(2))ary_addr = it->second;
                }
                std::bitset<16>bit1(ary_addr);
                std::bitset<16>bit2(0);
                std::bitset<16>result_bit = bit1 | bit2;

                //  std::cout << file_line.at(0) << "  " << file_line.at(1) << " " <<ary_addr<< std::endl;
                SP_func(file_line.at(0), stoul(file_line.at(1)), ary_addr);
                //std::cout << std::hex << ary_addr << std::endl;
            }
        }
    }
    std::string input_name = argv[1];
    std::string output_name;

    if (input_name.find(".s")) {
        input_name.replace(input_name.find(".s"), 2,".o");
        output_name = input_name;
    }
    std::ofstream output(output_name);

    output << "0x"<<std::hex<<num_ISA*4 << std::endl;    
    output << "0x"<< std::hex << wnum * 4 << std::endl;


   std::vector<unsigned int>::iterator it;
    for (it = isa_binary.begin(); it != isa_binary.end(); it++) {
      
        output <<"0x"<<std::hex<<*it<<std::endl;
    } 

    std::vector<int>::iterator iter;
    for (iter = value_ary.begin(); iter != value_ary.end(); iter++) {

        output <<"0x"<<std::hex<<*iter << std::endl;
    }

    f2.close();
    f.close();
    output.close();
    //std::cout << num_ISA << std::endl;
    return 0;
  }
}


