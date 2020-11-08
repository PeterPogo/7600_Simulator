#pragma once

#include <iostream>
#include <string>
#include <vector>
using namespace std;

// Declare functions here



// General functions
void simulate_CDC7600(int test_data_eq);

void output_table(vector<string> inst_word, vector<string> inst_sem, vector<string> inst_sem2, vector<string> inst_len, vector<int> issue, vector<int> start, vector<int> result, vector<int> unit_ready, vector<string> fetch, vector<string> store, vector<string> functional_unit_used, vector<string> registers_used, int rows_solved);

// functional units
void Branch(int Opcode);
void Boolean(int Opcode);
void Shift(int Opcode);
void ADD(int Opcode);
void LONG_ADD(int Opcode);
void DIVIDE(int Opcode);
void MULTIPLY(int Opcode);
void INCREMENT(int Opcode);
