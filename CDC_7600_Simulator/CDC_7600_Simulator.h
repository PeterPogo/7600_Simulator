#pragma once

// Declare functions here

#include <iostream>
#include <string>
#include <vector>
using namespace std;

// General functions
void simulate_CDC7600(int inst_count);
void create_blank_table(vector<string> instructions_v);
vector<string> get_sources(string inst);
vector<string> get_destination(string inst);
void output_table(vector<string> inst_word, vector<string> inst_sem, vector<string> inst_sem2, vector<string> inst_len, vector<int> issue, vector<int> start, vector<int> result, vector<int> unit_ready, vector<string> fetch, vector<string> store, vector<string> functional_unit_used, vector<string> registers_used, int rows_solved);
unsigned long to_octal(unsigned long binary_num);
string get_unique_registers(string dest, string op1, string op2);

bool check_foc(int inst_num);
bool check_foc2(int inst_num);
bool check_soc(int inst_num);
bool check_toc(int inst_num);


// functional units
void BRANCH(int Opcode, string inst);
void BOOLEAN(int Opcode, string inst); // CDC7600
void SHIFT(int Opcode, string inst); // CDC7600
void ADD(int Opcode, string inst); // CDC7600
void LONG_ADD(int Opcode, string inst); // CDC7600
void DIVIDE(int Opcode, string inst); // CDC7600
void MULTIPLY(int Opcode, string inst); // CDC7600
void INCREMENT(int Opcode, string inst); // CDC7600
void NORMALIZE(int Opcode, string inst); // CDC7600
void POP_COUNT(int Opcode, string inst); // CDC7600

