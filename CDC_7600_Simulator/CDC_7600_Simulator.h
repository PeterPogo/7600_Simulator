#pragma once

// Declare functions here

#include <iostream>
#include <string>
#include <vector>
using namespace std;

// General functions
void simulate_CDC7600(int test_data_eq);
void create_blank_table(vector<string> instructions_v);
void output_table(vector<string> inst_word, vector<string> inst_sem, vector<string> inst_sem2, vector<string> inst_len, vector<int> issue, vector<int> start, vector<int> result, vector<int> unit_ready, vector<string> fetch, vector<string> store, vector<string> functional_unit_used, vector<string> registers_used, int rows_solved);
unsigned long to_octal(unsigned long binary_num);
string get_unique_registers(string dest, string op1, string op2);


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

