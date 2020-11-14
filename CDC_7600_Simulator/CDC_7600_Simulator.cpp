// CDC_7600_Simulator.cpp : This file contains the 'main' function. Program execution begins and ends there.

///////////////////////////////////////////////////////////////////////////////////////
// Desc: CDC 7600 & CDC 6600 Simulator                                               //
// Class: CpE 5120                                                                   //
// Instructor: Dr. Ali Hurson                                                        //
// Date: 11/12/2020                   FS20                                           //
// Piotr Pogorzelski, Kendall Butler                                                 //
///////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <bitset>
#include <iomanip> 
#include <cmath>
#include <algorithm>
#include <chrono>
#include <thread>
#include "CDC_7600_Simulator.h"
using namespace std;
using namespace std::this_thread;     // sleep_for, sleep_until
using namespace std::chrono_literals; // ns, us, ms, s, h, etc.
using std::chrono::system_clock;

class Func_Unit_SB
{
public:
    string funcUnit_name;
    bool busy_tag; // Is the functional unit busy or available? 1 = Busy , 0 = Available
    bool idle;
    int segment_Time; // # clock_pulses (from table)
    int execution_Time; // # clock_pulses (from table)

    int instruction_num;

    // Sources and Destinations being used
    string S1; // source 1
    string S2; // source 2
    string D1; // destination 1 (as if there was more than 1...) destination tag bit?

    // Default constructor
    Func_Unit_SB()
    {
        funcUnit_name = "Default";
        busy_tag = false;
        idle = true;
        segment_Time = 1;
        execution_Time = 1;
    }

    // Set parameters per functional unit
    void SetParameters(string Name, bool Busy, bool Idle, int Seg_time, int Ex_time)
    {
        funcUnit_name = Name;
        busy_tag = Busy;
        segment_Time = Seg_time;
        execution_Time = Ex_time;

        S1 = "--";
        S2 = "--";
        D1 = "--";

        instruction_num = -1;
    }

    void clear_src_dest()
    {
        S1 = "--";
        S2 = "--";
        D1 = "--";

        instruction_num = -1;
    }
};

class Scoreboard
{
public:
    // Rows of scoreboard --> 9 total
    Func_Unit_SB Boolean;
    Func_Unit_SB Shift;
    Func_Unit_SB Fixed_Add; // Probably not gonna use this
    Func_Unit_SB Floating_Add;
    Func_Unit_SB Floating_Multiply;
    Func_Unit_SB Floating_Divide;
    Func_Unit_SB Normalize;
    Func_Unit_SB Pop_count; // Count # 1's in a word
    Func_Unit_SB Increment; // Fetch, Decrement, Set to vector length

    // Default constructor
    Scoreboard()
    {
        Boolean.SetParameters("Boolean", false, true, 1,2);
        Shift.SetParameters("Shift", false, true, 1, 2);
        Fixed_Add.SetParameters("Fixed Add", false, true, 1, 2);
        Floating_Add.SetParameters("Floating Add", false, true, 1, 4);
        Floating_Multiply.SetParameters("Floating Multiply", false, true, 2, 5);
        Floating_Divide.SetParameters("Floating Divide", false, true, 18, 20);
        Normalize.SetParameters("Normalize", false, true, 1, 3);
        Pop_count.SetParameters("Pop_count", false, true, 1, 2);
        Increment.SetParameters("Increment", false, true, 1, 2);
    }
};

#pragma region Operator Overloading
// Overload << operator for Scoreboard Class
ostream& operator<<(ostream& os, const Scoreboard& sb)
{
    os << "\n======================================== ScoreBoard =========================================================";
    os << "\n| Func Unit | busy_tag| idle | segment_time | execution_time | Source 1 | Source 2 | Destination | Inst_Num |";
    os << "\n-------------------------------------------------------------------------------------------------------------";
    os << "\n" << sb.Boolean.funcUnit_name << "           " << sb.Boolean.busy_tag << "      " << sb.Boolean.idle << "\t    " << sb.Boolean.segment_Time << "\t\t    " << sb.Boolean.execution_Time << "\t\t  " << sb.Boolean.S1 << "\t     " << sb.Boolean.S2 << "\t\t " << sb.Boolean.D1 << "\t\t" << sb.Boolean.instruction_num;
    os << "\n" << sb.Shift.funcUnit_name << "             " << sb.Shift.busy_tag << "      " << sb.Shift.idle << "\t    " << sb.Shift.segment_Time << "\t\t    " << sb.Shift.execution_Time << "\t\t  " << sb.Shift.S1 << "\t     " << sb.Shift.S2 << "\t\t " << sb.Shift.D1 << "\t\t" << sb.Shift.instruction_num;
    os << "\n" << sb.Fixed_Add.funcUnit_name << "         " << sb.Fixed_Add.busy_tag << "      " << sb.Fixed_Add.idle << "\t    " << sb.Fixed_Add.segment_Time << "\t\t    " << sb.Fixed_Add.execution_Time << "\t\t  " << sb.Fixed_Add.S1 << "\t     " << sb.Fixed_Add.S2 << "\t\t " << sb.Fixed_Add.D1 << "\t\t" << sb.Fixed_Add.instruction_num;
    os << "\n" << sb.Floating_Add.funcUnit_name << "      " << sb.Floating_Add.busy_tag << "      " << sb.Floating_Add.idle << "\t    " << sb.Floating_Add.segment_Time << "\t\t    " << sb.Floating_Add.execution_Time << "\t\t  " << sb.Floating_Add.S1 << "\t     " << sb.Floating_Add.S2 << "\t\t " << sb.Floating_Add.D1 << "\t\t" << sb.Floating_Add.instruction_num;
    os << "\n" << "Floating Mult" << "     " << sb.Floating_Multiply.busy_tag << "      " << sb.Floating_Multiply.idle << "\t    " << sb.Floating_Multiply.segment_Time << "\t\t    " << sb.Floating_Multiply.execution_Time << "\t\t  " << sb.Floating_Multiply.S1 << "\t     " << sb.Floating_Multiply.S2 << "\t\t " << sb.Floating_Multiply.D1 << "\t\t" << sb.Floating_Multiply.instruction_num;
    os << "\n" << "Floating Div" << "      " << sb.Floating_Divide.busy_tag << "      " << sb.Floating_Divide.idle << "\t    " << sb.Floating_Divide.segment_Time << "\t\t    " << sb.Floating_Divide.execution_Time << "\t\t  " << sb.Floating_Divide.S1 << "\t     " << sb.Floating_Divide.S2 << "\t\t " << sb.Floating_Divide.D1 << "\t\t" << sb.Floating_Divide.instruction_num;
    os << "\n" << sb.Normalize.funcUnit_name << "         " << sb.Normalize.busy_tag << "      " << sb.Normalize.idle << "\t    " << sb.Normalize.segment_Time << "\t\t    " << sb.Normalize.execution_Time << "\t\t  " << sb.Normalize.S1 << "\t     " << sb.Normalize.S2 << "\t\t " << sb.Normalize.D1 << "\t\t" << sb.Normalize.instruction_num;
    os << "\n" << sb.Pop_count.funcUnit_name << "         " << sb.Pop_count.busy_tag << "      " << sb.Pop_count.idle << "\t    " << sb.Pop_count.segment_Time << "\t\t    " << sb.Pop_count.execution_Time << "\t\t  " << sb.Pop_count.S1 << "\t     " << sb.Pop_count.S2 << "\t\t " << sb.Pop_count.D1 << "\t\t" << sb.Pop_count.instruction_num;
    os << "\n" << sb.Increment.funcUnit_name << "         " << sb.Increment.busy_tag << "      " << sb.Increment.idle << "\t    " << sb.Increment.segment_Time << "\t\t    " << sb.Increment.execution_Time << "\t\t  " << sb.Increment.S1 << "\t     " << sb.Increment.S2 << "\t\t " << sb.Increment.D1 << "\t\t" << sb.Increment.instruction_num;
    os << "\n======================================== ScoreBoard =========================================================";

    return os;
}

#pragma endregion

// Global declaration of table vectors
#pragma region Table_Rows
// These tables are purely for storing and outputing data, scoreboard needs to handle the logic
vector<string> instruction_word;
vector<string> instruction_semantics;
vector<string> instruction_semantics_2;
vector<string> instruction_length;
vector<int> issue;
vector<int> start;
vector<int> result;
vector<int> unit_ready;
vector<string> fetch;
vector<string> store;
vector<string> functional_unit_used;
vector<string> registers_used;


vector<vector<bool>> indiv_functional_unit_used;
vector<string> source_1;
vector<string> source_2;
vector<string> destination;

vector<string> instructions;
#pragma endregion

Scoreboard SB;

int main()
{
    // Test Data to run
    // Y = AX2 + BX
    // Y = AX2 + BX , X & Y are vectors (limited to 5 entries)
    // Y = AX2 + BX + C

    int instruction_count = 0;
    string instruction;
    string semantic;

    cout << "--- CDC 7600 Simulator ---";


    //cout <<"\n\nChose an equation to simulate:" << "\n1: Y = AX^2 + BX" << "\n2: Y = AX^2 + BX + C" << "\n3: Y = AX^2 + BX (X & Y = vectors)\n\n";
    //cin >> test_data_choice;
        
    // Read in instructions from file
    cout << "\n\nReading instructions from files (binary).....\n\n";


    // Read in semantics
    ifstream sem_input_file("semantics3.txt");
    if (sem_input_file.is_open())
    {
        while (getline(sem_input_file, semantic))
        {
            instruction_semantics_2.push_back(semantic);
        }
        sem_input_file.close();
    }
    else
    {
        cout << "\nUnable to open semantics file";

        return 0;
    }

    // Read in binary
    ifstream inst_input_file("instructions3.txt");
    if (inst_input_file.is_open())
    {
        while (getline(inst_input_file, instruction))
        {
            instructions.push_back(instruction);
            cout << "Instruction " << instruction_count << ": " << instruction << " " << instruction_semantics_2[instruction_count] << '\n';
            instruction_count++;
        }
        inst_input_file.close();
    }
    else
    {
        cout << "\nUnable to open instructions file";

        return 0;
    }

    // Create blank table and fill in vectors by decoding
    create_blank_table(instructions);

    // Simulate the CDC7600
    simulate_CDC7600(instruction_count);

    cout << "\nExiting....\n\n"; // Simulation complete
}


void create_blank_table(vector<string> instructions_v)
{
    //Fill lengths (short/long)
    for (string inst : instructions_v)
    {
        if (inst.length() > 15) { instruction_length.push_back("Long"); }
        else { instruction_length.push_back("Short"); }
    }

    //Fill word splits
    int current_word_length = 0;
    int current_word_count = 2;
    instruction_word.push_back("N1"); // always gotta start at N1

    for (string inst : instructions_v)
    {
        current_word_length += inst.length();

        if (current_word_length >= 60)
        { 
            instruction_word.push_back("N" + to_string(current_word_count));
            current_word_count++;
            current_word_length = 0;
        }
        else { instruction_word.push_back("  "); }
    }

    // Fill Semantics 1 & 2, increment unit, and registers used, (Big ole switchcase)
    for (string inst : instructions_v)
    {
        unsigned long opcode = to_octal(bitset<6>(inst.substr(0, 6)).to_ulong()); // Convert first 6 bits to Op-Code

        // Send Op-code to the switchcase (octal)
        if ((opcode >= 00) && (opcode <= 7)) { BRANCH(opcode, inst); }
        else if ((opcode >= 10) && (opcode <= 17)) { BOOLEAN(opcode, inst); }
        else if ((opcode >= 20) && (opcode <= 27) || (opcode == 43)) { SHIFT(opcode, inst); }
        else if ((opcode >= 30) && (opcode <= 35)) { ADD(opcode, inst); }
        else if ((opcode >= 36) && (opcode <= 37)) { LONG_ADD(opcode, inst); }
        else if ((opcode >= 40) && (opcode <= 42)) { MULTIPLY(opcode, inst); }
        else if ((opcode >= 44) && (opcode <= 47)) { DIVIDE(opcode, inst); }
        else if ((opcode >= 50) && (opcode <= 77)) { INCREMENT(opcode, inst); }
    }

    // Testing values for printing
#pragma region Fake Values
    //issue.push_back(1);
    //issue.push_back(3);
    //issue.push_back(9);
    //issue.push_back(11);
    //issue.push_back(17);
    //issue.push_back(19);
    //issue.push_back(25);
    //issue.push_back(26);
    //issue.push_back(36);
    //issue.push_back(37);
    //issue.push_back(41);
    //issue.push_back(42);
    //issue.push_back(51);
    //issue.push_back(56);
    //issue.push_back(60);

    //start.push_back(1);
    //start.push_back(3);
    //start.push_back(9);
    //start.push_back(11);
    //start.push_back(17);
    //start.push_back(19);
    //start.push_back(25);
    //start.push_back(35);
    //start.push_back(36);
    //start.push_back(37);
    //start.push_back(41);
    //start.push_back(46);
    //start.push_back(51);
    //start.push_back(56);
    //start.push_back(0);

    //result.push_back(4);
    //result.push_back(6);
    //result.push_back(12);
    //result.push_back(14);
    //result.push_back(20);
    //result.push_back(22);
    //result.push_back(35);
    //result.push_back(45);
    //result.push_back(46);
    //result.push_back(40);
    //result.push_back(44);
    //result.push_back(50);
    //result.push_back(55);
    //result.push_back(59);
    //result.push_back(72);

    //unit_ready.push_back(5);
    //unit_ready.push_back(7);
    //unit_ready.push_back(13);
    //unit_ready.push_back(15);
    //unit_ready.push_back(21);
    //unit_ready.push_back(23);
    //unit_ready.push_back(36);
    //unit_ready.push_back(46);
    //unit_ready.push_back(47);
    //unit_ready.push_back(41);
    //unit_ready.push_back(45);
    //unit_ready.push_back(51);
    //unit_ready.push_back(56);
    //unit_ready.push_back(60);
    //unit_ready.push_back(0);

    //fetch.push_back(to_string(9));
    //fetch.push_back(to_string(11));
    //fetch.push_back(to_string(17));
    //fetch.push_back(to_string(19));
    //fetch.push_back(" ");
    //fetch.push_back(" ");
    //fetch.push_back(" ");
    //fetch.push_back(" ");
    //fetch.push_back(" ");
    //fetch.push_back(to_string(45));
    //fetch.push_back(" ");
    //fetch.push_back(" ");
    //fetch.push_back(" ");
    //fetch.push_back(" ");
    //fetch.push_back(" ");

    //store.push_back(" ");
    //store.push_back(" ");
    //store.push_back(" ");
    //store.push_back(" ");
    //store.push_back(" ");
    //store.push_back(" ");
    //store.push_back(" ");
    //store.push_back(" ");
    //store.push_back(" ");
    //store.push_back(" ");
    //store.push_back(" ");
    //store.push_back(" ");
    //store.push_back(" ");
    //store.push_back("27");
    //store.push_back(" ");
#pragma endregion
}


// Simluation of CDC7600 Processor
void simulate_CDC7600(int inst_count)
{
    #pragma region Old idea for test data

#pragma region Construct base table for Eq1 Y = AX^2 + BX

#pragma endregion

#pragma region Construct base table for Eq2 Y = AX^2 + BX + C
    //instruction_word.push_back("N1");
    //instruction_word.push_back("  ");
    //instruction_word.push_back("N2");
    //instruction_word.push_back("  ");
    //instruction_word.push_back("  ");
    //instruction_word.push_back("N3");
    //instruction_word.push_back("  ");
    //instruction_word.push_back("  ");
    //instruction_word.push_back("N4");
    //instruction_word.push_back("  ");

    //instruction_semantics.push_back("A1 = A1 + K1");
    //instruction_semantics.push_back("A2 = A2 + K2");
    //instruction_semantics.push_back("X0 = X1 * X1");
    //instruction_semantics.push_back("X6 = X0 * X2");
    //instruction_semantics.push_back("A3 = A3 + K3");
    //instruction_semantics.push_back("A4 = A4 + K4");
    //instruction_semantics.push_back("X3 = X3 * X1");
    //instruction_semantics.push_back("X5 = X6 + X3");
    //instruction_semantics.push_back("X7 = X5 + X4");
    //instruction_semantics.push_back("A7 = A7 + K5");

    //instruction_semantics_2.push_back("FETCH X");
    //instruction_semantics_2.push_back("FETCH A");
    //instruction_semantics_2.push_back("FORM X^2");
    //instruction_semantics_2.push_back("FORM AX^2");
    //instruction_semantics_2.push_back("FETCH B");
    //instruction_semantics_2.push_back("FETCH C");
    //instruction_semantics_2.push_back("FORM BX");
    //instruction_semantics_2.push_back("FORM AX^2 + BX");
    //instruction_semantics_2.push_back("FORM Y");
    //instruction_semantics_2.push_back("STORE Y");

    ////////////////////////////////////////////// N1
    //instruction_length.push_back("Long");
    //instruction_length.push_back("Long");
    ////////////////////////////////////////////// N2
    //instruction_length.push_back("Short");
    //instruction_length.push_back("Short");
    //instruction_length.push_back("Long");
    ///////////////////////////////////////////// N3
    //instruction_length.push_back("Long");
    //instruction_length.push_back("Short");
    //instruction_length.push_back("Short");
    //////////////////////////////////////////// N4
    //instruction_length.push_back("Short");
    //instruction_length.push_back("Long");

    //// Fake info for 2 rows
    //issue.push_back(1);
    //issue.push_back(3);
    //issue.push_back(7);
    //issue.push_back(9);
    //issue.push_back(10);
    //issue.push_back(13);
    //issue.push_back(14);
    //issue.push_back(16);
    //issue.push_back(19);
    //issue.push_back(22);

    //start.push_back(1);
    //start.push_back(3);
    //start.push_back(7);
    //start.push_back(12);
    //start.push_back(10);
    //start.push_back(17);
    //start.push_back(21);
    //start.push_back(24);
    //start.push_back(23);
    //start.push_back(27);


    //result.push_back(3);
    //result.push_back(5);
    //result.push_back(12);
    //result.push_back(17);
    //result.push_back(12);
    //result.push_back(21);
    //result.push_back(23);
    //result.push_back(25);
    //result.push_back(28);
    //result.push_back(32);

    //unit_ready.push_back(2);
    //unit_ready.push_back(4);
    //unit_ready.push_back(9);
    //unit_ready.push_back(14);
    //unit_ready.push_back(11);
    //unit_ready.push_back(18);
    //unit_ready.push_back(22);
    //unit_ready.push_back(24);
    //unit_ready.push_back(26);
    //unit_ready.push_back(28);

    //fetch.push_back(to_string(9));
    //fetch.push_back(to_string(11));
    //fetch.push_back(" ");
    //fetch.push_back(" ");
    //fetch.push_back(to_string(16));
    //fetch.push_back(to_string(18));
    //fetch.push_back(" ");
    //fetch.push_back(" ");
    //fetch.push_back(" ");
    //fetch.push_back(" ");

    //store.push_back(" ");
    //store.push_back(" ");
    //store.push_back(" ");
    //store.push_back(" ");
    //store.push_back(" ");
    //store.push_back(" ");
    //store.push_back(" ");
    //store.push_back(" ");
    //store.push_back(" ");
    //store.push_back("27");

    //functional_unit_used.push_back("Increment");
    //functional_unit_used.push_back("Increment");
    //functional_unit_used.push_back("FL multiply, Normalize");
    //functional_unit_used.push_back("FL multiply, Normalize");
    //functional_unit_used.push_back("Increment");
    //functional_unit_used.push_back("Increment");
    //functional_unit_used.push_back("FL multiply, Normalize");
    //functional_unit_used.push_back("FL add, Normalize");
    //functional_unit_used.push_back("FL add, Normalize");
    //functional_unit_used.push_back("Increment");


    //registers_used.push_back("A1, X1");
    //registers_used.push_back("A2, X2");
    //registers_used.push_back("X0, X1");
    //registers_used.push_back("X0, X2, X6");
    //registers_used.push_back("A3, X3");
    //registers_used.push_back("A4, X4");
    //registers_used.push_back("X1, X3");
    //registers_used.push_back("X3, X5, X6");
    //registers_used.push_back("X4, X5, X7");
    //registers_used.push_back("A7, X5, X7");
#pragma endregion

#pragma region Construct base table for Eq3  Y = AX^2 + BX (X & Y = vectors)

#pragma endregion
#pragma endregion

    #pragma region A,B,X registers, and A,B,C contants. Probably wont need these
    // Instruction stack of 12 - 60-bit registers (allows for up to 48 previously fetched instructions to be readily available in the instruction stack).
    // 24 total registers - 5 Operand-Address pairs are used for read and 2 are used for write
    //int A[8]; // Address registers, paired with x register - 18 bits
    //int B[8]; // Index registers - 18 bits,  B0 is always 0
    //int x[8]; // Operand register paired with address register - 60 bits
    //B[0] = 0; // constant within the index resgister
    //double A_Constant = 2.0; // constant stored in memory
    //double B_Constant = 3.0; // constant stored in memory
    //double C_Constant = 4.0; // constant stored in memory
#pragma endregion

    // Processor Declarations
    int clock_pulses = 1; // Always start at clock #1

    bool foc = false; // First order conflict
    bool soc = false; // Second order conflict
    bool toc = false; // Third order conflict


        // First check for 1st order conflict (hardware not available) if so stop pipeline and wait, if not then tag the used functional unit as reserved
        // Then check for 1st order conflict (pt2) (destination in use) (Write after Write), stop pipeline again, if not then add those registers to a "in use" vector
        // Next check sources (2nd order conflict) (Read after Write), are source registers the destination of a previously issued instruction which has not been completed?
        // Finally check for 3rd order conflict (Write after Read)
        // Resolve by delaying the clock

        // 4 cycles to fetch, 6 cycles to store
        // 6 cycles between words (unless branch, then just start a new word)

        // If a functional unit is not available when issue time comes, then issue must be delayed unit unit ready, otherwise
        //    - 2 between long --> long     
        //    - 2 between long --> short
        //    - 1 between short --> short
        //    - 1 between short --> long

    // sources / destinations can be in A or X form

    // Iterate through each instruction within the "instructions" vector
    for (int curr_inst = 0; curr_inst < inst_count; curr_inst++)
    {    
        if (curr_inst == 0) // Treat first instruction differently
        {
            // Get functional unit(s)
            indiv_functional_unit_used.push_back(get_indiv_fu(curr_inst)); // Boolean vector

            // Since no conflicts are possible, reserve those functional units and add in used registers to scoreboard
            reserve_units(curr_inst);
            
            issue.push_back(1);
            start.push_back(1);
            result.push_back(start[start.size() - 1] + get_result_execution_time(curr_inst));
            unit_ready.push_back(start[start.size() - 1] + get_result_segment_time(curr_inst));

            // Fill in fetch entry, This shouldn't be affected by anything
            if (check_fetch_op(curr_inst))
            {
                fetch.push_back(to_string(result[result.size() - 1] + 4));
            }
            else { fetch.push_back("-"); }

            // Fill in store entry, This shouldn't be affected by anything
            if (check_store_op(curr_inst))
            {
                store.push_back(to_string(result[result.size() - 1] + 6));
            }
            else { store.push_back("-"); }
        }
        else
        {
            // First problem is figuring out the issue entry: it can be affected by hardware conflict, must be iterated either +1 or +2 and changing words. Hardware conflict will have the largest affect

            // Default to the above logic for iterating each issue
            if (instruction_word[curr_inst] == "  ")
            {
                if (instruction_length[curr_inst - 1] == "Long") { clock_pulses += 2; }
                else { clock_pulses++; }
            }
            else if (instruction_word[curr_inst] != "  ")
            {
                if (instruction_word[curr_inst - 1] != "  ") { clock_pulses = issue[curr_inst - 1] + 6; }
                else if(instruction_word[curr_inst - 2] != "  ") { clock_pulses = issue[curr_inst - 2] + 6; }
                else if (instruction_word[curr_inst - 3] != "  ") { clock_pulses = issue[curr_inst - 3] + 6; }
                else if (instruction_word[curr_inst - 4] != "  ") { clock_pulses = issue[curr_inst - 4] + 6; }
            }

            indiv_functional_unit_used.push_back(get_indiv_fu(curr_inst)); // Get instruction functional unit(s)

            check_func_unit_complete(curr_inst, clock_pulses);  // now that default issue entry is made, check to see if functional units are free now and remove their busy tags


            // Check for 1st order conflict, this can modify the issue entry
            if (check_foc(curr_inst))
            {
                // find where the conflict is >_> and change issue entry as needed
                // Need to check against all busy hardware for worst-case-senario
                clock_pulses = resolve_foc_conflict(curr_inst);
            }

            reserve_units(curr_inst); // Only allowed to reserve units once foc is resolved or non-existent
            
            issue.push_back(clock_pulses);  // create issue entry based on above logic
            

            // Check for 1st order conflict pt.2 (destination in use) (Write after Write). This will affect the start entry
            if (check_foc2(curr_inst))
            {

            }

  
            
            // each instructions' registers are in vectors source_1, source_2, destination. Need to get pairs X,A for each if they are not unique. Basically will be looking for 6 values within SB

            if (check_soc(curr_inst))
            {

            }
            else if (check_toc(curr_inst))
            {

            }

            // Fill start entry, this affects a lot
            start.push_back(clock_pulses);






            // Fill result entry, This shouldn't be affected by anything
            result.push_back(start[start.size() - 1] + get_result_execution_time(curr_inst));

            // Fill unit ready entry, This shouldn't be affected by anything
            unit_ready.push_back(start[start.size() - 1] + get_result_segment_time(curr_inst));

            // Fill in fetch entry, This shouldn't be affected by anything
            if (check_fetch_op(curr_inst))
            {
                fetch.push_back(to_string(result[result.size() - 1] + 4));
            }
            else { fetch.push_back("-"); }

            // Fill in store entry, This shouldn't be affected by anything
            if (check_store_op(curr_inst))
            {
                store.push_back(to_string(result[result.size() - 1] + 6));
            }
            else { store.push_back("-"); }
        }

        
        cout << "\n\n" << SB;
        output_table(instruction_word, instruction_semantics, instruction_semantics_2, instruction_length, issue, start, result, unit_ready, fetch, store, functional_unit_used, registers_used, curr_inst+1);
        sleep_for(1s);
    }
}


vector<string> get_sources(string inst)
{
    vector<string> yeet;
    
    return yeet;
}

vector<string> get_destination(string inst)
{
    vector<string> yeet;

    return yeet;
}

// Check for first order conflicts
bool check_foc(int inst_num)
{    
    // Check for conflict with Boolean unit
    if (functional_unit_used[inst_num].find("Boolean") != std::string::npos && SB.Boolean.busy_tag)
    {
        return true;
    }


    // Check for conflict with Shift unit
    if (functional_unit_used[inst_num].find("Shift") != std::string::npos && SB.Shift.busy_tag)
    {
        return true;
    }


    // Check for conflict with Long-Add unit
    if (functional_unit_used[inst_num].find("Long-Add") != std::string::npos && SB.Fixed_Add.busy_tag)
    {
        return true;
    }


    // Check for conflict with FL Add unit
    if (functional_unit_used[inst_num].find("FL Add") != std::string::npos && SB.Floating_Add.busy_tag)
    {
        return true;
    }


    // Check for conflict with FL Multiply unit
    if (functional_unit_used[inst_num].find("FL Multiply") != std::string::npos && SB.Floating_Multiply.busy_tag)
    {
        return true;
    }


    // Check for conflict with FL Divide unit
    if (functional_unit_used[inst_num].find("FL Divide") != std::string::npos && SB.Floating_Divide.busy_tag)
    {
        return true;
    }


    // Check for conflict with Normalize unit
    if (functional_unit_used[inst_num].find("Normalize") != std::string::npos && SB.Normalize.busy_tag)
    {
        return true;
    }


    // Check for conflict with Pop_Count unit
    if (functional_unit_used[inst_num].find("Pop_Count") != std::string::npos && SB.Pop_count.busy_tag)
    {
        return true;
    }


    // Check for conflict with Increment unit
    if (functional_unit_used[inst_num].find("Increment") != std::string::npos && SB.Increment.busy_tag)
    {
        return true;
    }

    return false;
}


// Check for first order conflicts
bool check_foc2(int inst_num)
{
    // Check for conflict with Boolean unit
    if (functional_unit_used[inst_num].find("Boolean") != std::string::npos && SB.Boolean.busy_tag)
    {
        return true;
    }


    // Check for conflict with Shift unit
    if (functional_unit_used[inst_num].find("Shift") != std::string::npos && SB.Shift.busy_tag)
    {
        return true;
    }


    // Check for conflict with Long-Add unit
    if (functional_unit_used[inst_num].find("Long-Add") != std::string::npos && SB.Fixed_Add.busy_tag)
    {
        return true;
    }


    // Check for conflict with FL Add unit
    if (functional_unit_used[inst_num].find("FL Add") != std::string::npos && SB.Floating_Add.busy_tag)
    {
        return true;
    }


    // Check for conflict with FL Multiply unit
    if (functional_unit_used[inst_num].find("FL Multiply") != std::string::npos && SB.Floating_Multiply.busy_tag)
    {
        return true;
    }


    // Check for conflict with FL Divide unit
    if (functional_unit_used[inst_num].find("FL Divide") != std::string::npos && SB.Floating_Divide.busy_tag)
    {
        return true;
    }


    // Check for conflict with Normalize unit
    if (functional_unit_used[inst_num].find("Normalize") != std::string::npos && SB.Normalize.busy_tag)
    {
        return true;
    }


    // Check for conflict with Pop_Count unit
    if (functional_unit_used[inst_num].find("Pop_Count") != std::string::npos && SB.Pop_count.busy_tag)
    {
        return true;
    }


    // Check for conflict with Increment unit
    if (functional_unit_used[inst_num].find("Increment") != std::string::npos && SB.Increment.busy_tag)
    {
        return true;
    }

    return false;
}


// Check for first order conflicts
bool check_soc(int inst_num)
{
    // Check for conflict with Boolean unit
    if (functional_unit_used[inst_num].find("Boolean") != std::string::npos && SB.Boolean.busy_tag)
    {
        return true;
    }


    // Check for conflict with Shift unit
    if (functional_unit_used[inst_num].find("Shift") != std::string::npos && SB.Shift.busy_tag)
    {
        return true;
    }


    // Check for conflict with Long-Add unit
    if (functional_unit_used[inst_num].find("Long-Add") != std::string::npos && SB.Fixed_Add.busy_tag)
    {
        return true;
    }


    // Check for conflict with FL Add unit
    if (functional_unit_used[inst_num].find("FL Add") != std::string::npos && SB.Floating_Add.busy_tag)
    {
        return true;
    }


    // Check for conflict with FL Multiply unit
    if (functional_unit_used[inst_num].find("FL Multiply") != std::string::npos && SB.Floating_Multiply.busy_tag)
    {
        return true;
    }


    // Check for conflict with FL Divide unit
    if (functional_unit_used[inst_num].find("FL Divide") != std::string::npos && SB.Floating_Divide.busy_tag)
    {
        return true;
    }


    // Check for conflict with Normalize unit
    if (functional_unit_used[inst_num].find("Normalize") != std::string::npos && SB.Normalize.busy_tag)
    {
        return true;
    }


    // Check for conflict with Pop_Count unit
    if (functional_unit_used[inst_num].find("Pop_Count") != std::string::npos && SB.Pop_count.busy_tag)
    {
        return true;
    }


    // Check for conflict with Increment unit
    if (functional_unit_used[inst_num].find("Increment") != std::string::npos && SB.Increment.busy_tag)
    {
        return true;
    }

    return false;
}


// Check for first order conflicts
bool check_toc(int inst_num)
{
    // Check for conflict with Boolean unit
    if (functional_unit_used[inst_num].find("Boolean") != std::string::npos && SB.Boolean.busy_tag)
    {
        return true;
    }


    // Check for conflict with Shift unit
    if (functional_unit_used[inst_num].find("Shift") != std::string::npos && SB.Shift.busy_tag)
    {
        return true;
    }


    // Check for conflict with Long-Add unit
    if (functional_unit_used[inst_num].find("Long-Add") != std::string::npos && SB.Fixed_Add.busy_tag)
    {
        return true;
    }


    // Check for conflict with FL Add unit
    if (functional_unit_used[inst_num].find("FL Add") != std::string::npos && SB.Floating_Add.busy_tag)
    {
        return true;
    }


    // Check for conflict with FL Multiply unit
    if (functional_unit_used[inst_num].find("FL Multiply") != std::string::npos && SB.Floating_Multiply.busy_tag)
    {
        return true;
    }


    // Check for conflict with FL Divide unit
    if (functional_unit_used[inst_num].find("FL Divide") != std::string::npos && SB.Floating_Divide.busy_tag)
    {
        return true;
    }


    // Check for conflict with Normalize unit
    if (functional_unit_used[inst_num].find("Normalize") != std::string::npos && SB.Normalize.busy_tag)
    {
        return true;
    }


    // Check for conflict with Pop_Count unit
    if (functional_unit_used[inst_num].find("Pop_Count") != std::string::npos && SB.Pop_count.busy_tag)
    {
        return true;
    }


    // Check for conflict with Increment unit
    if (functional_unit_used[inst_num].find("Increment") != std::string::npos && SB.Increment.busy_tag)
    {
        return true;
    }

    return false;
}


// Get individual vectors from functional_unit_used vector (because it's in sentence form)
vector<bool> get_indiv_fu(int inst_num)
{
    vector<bool> temp;
    
    // Check for conflict with Boolean unit
    if (functional_unit_used[inst_num].find("Boolean") != std::string::npos)
    {
        temp.push_back(true);
    }
    else { temp.push_back(false); }


    // Check for conflict with Shift unit
    if (functional_unit_used[inst_num].find("Shift") != std::string::npos)
    {
        temp.push_back(true);
    }
    else { temp.push_back(false); }


    // Check for conflict with Long-Add unit
    if (functional_unit_used[inst_num].find("Long-Add") != std::string::npos)
    {
        temp.push_back(true);
    }
    else { temp.push_back(false); }


    // Check for conflict with FL Add unit
    if (functional_unit_used[inst_num].find("FL Add") != std::string::npos)
    {
        temp.push_back(true);
    }
    else { temp.push_back(false); }


    // Check for conflict with FL Multiply unit
    if (functional_unit_used[inst_num].find("FL Multiply") != std::string::npos)
    {
        temp.push_back(true);
    }
    else { temp.push_back(false); }


    // Check for conflict with FL Divide unit
    if (functional_unit_used[inst_num].find("FL Divide") != std::string::npos)
    {
        temp.push_back(true);
    }
    else { temp.push_back(false); }


    // Check for conflict with Normalize unit
    if (functional_unit_used[inst_num].find("Normalize") != std::string::npos)
    {
        temp.push_back(true);;
    }
    else { temp.push_back(false); }


    // Check for conflict with Pop_Count unit
    if (functional_unit_used[inst_num].find("Pop_Count") != std::string::npos)
    {
        temp.push_back(true);
    }
    else { temp.push_back(false); }


    // Check for conflict with Increment unit
    if (functional_unit_used[inst_num].find("Increment") != std::string::npos)
    {
        temp.push_back(true);
    }
    else { temp.push_back(false); }

    return temp;
}


// get worst case execution time given the func units   
int get_result_execution_time(int inst_num)
{
    vector<bool> temp = indiv_functional_unit_used[inst_num];
   
    if (temp[5]) // Floating Divide
    {
        return 20;
    }
    
    if (temp[4]) // Floating Multiply
    {
        return 5;
    }
    
    if (temp[3]) // Floating Add
    {
        return 4;
    }
    
    if (temp[6]) // Normalize
    {
        return 3;
    }
    
    return 2; // Everything else has this execution time

}


// get worst case segment time given the func units   
int get_result_segment_time(int inst_num)
{
    vector<bool> temp = indiv_functional_unit_used[inst_num];

    if (temp[5]) // Floating Divide
    {
        return 18;
    }

    if (temp[4]) // Floating Multiply
    {
        return 2;
    }

    return 1; // Everything else has this segment time
}


// Check to see if the current operation fetches by using semantics
bool check_fetch_op(int inst_num)
{
    if (instruction_semantics_2[inst_num].find("Fetch") != std::string::npos)
    {
        return true;
    }

    return false;
}


// Check to see if the current operation stores by using semantics
bool check_store_op(int inst_num)
{
    if (instruction_semantics_2[inst_num].find("Store") != std::string::npos)
    {
        return true;
    }

    return false;
}

// Reserve all units for a given instruction (Assumes no hardware conflict)
void reserve_units(int inst_num)
{
    vector<bool> temp = indiv_functional_unit_used[inst_num];
    
    // Only write sources, destination to the main func_unit (organized specifically)
    if (temp[4]) // FL Multiply
    {
        SB.Floating_Multiply.S1 = source_1[inst_num];
        SB.Floating_Multiply.S2 = source_2[inst_num];
        SB.Floating_Multiply.D1 = destination[inst_num];
    }
    else if (temp[3]) // FL Add
    {
        SB.Floating_Add.S1 = source_1[inst_num];
        SB.Floating_Add.S2 = source_2[inst_num];
        SB.Floating_Add.D1 = destination[inst_num];
    }
    else if (temp[6]) // Normalize or Branch
    {
        SB.Normalize.S1 = source_1[inst_num];
        SB.Normalize.S2 = source_2[inst_num];
        SB.Normalize.D1 = destination[inst_num];
    }
    else if (temp[5]) // Divide 
    { 
        SB.Floating_Divide.S1 = source_1[inst_num];
        SB.Floating_Divide.S2 = source_2[inst_num];
        SB.Floating_Divide.D1 = destination[inst_num];
    }
    else if (temp[0]) // Boolean
    {
        SB.Boolean.S1 = source_1[inst_num];
        SB.Boolean.S2 = source_2[inst_num];
        SB.Boolean.D1 = destination[inst_num];
    }
    else if (temp[1]) // Shift
    {
        SB.Shift.S1 = source_1[inst_num];
        SB.Shift.S2 = source_2[inst_num];
        SB.Shift.D1 = destination[inst_num];
    }
    else if (temp[2]) // Fixed Add
    {
        SB.Fixed_Add.S1 = source_1[inst_num];
        SB.Fixed_Add.S2 = source_2[inst_num];
        SB.Fixed_Add.D1 = destination[inst_num];
    }
    else if (temp[7]) // Pop_Count
    {
        SB.Pop_count.S1 = source_1[inst_num];
        SB.Pop_count.S2 = source_2[inst_num];
        SB.Pop_count.D1 = destination[inst_num];
    }
    else if (temp[8]) // Increment
    {
        SB.Increment.S1 = source_1[inst_num];
        SB.Increment.S2 = source_2[inst_num];
        SB.Increment.D1 = destination[inst_num];
    }

    // Reserve those units by changing the "Busy_tag"
    if (temp[4]) // FL Multiply
    {
        SB.Floating_Multiply.busy_tag = true;
        SB.Floating_Multiply.instruction_num = inst_num;
    }
    

    if (temp[3]) // FL Add
    {
        SB.Floating_Add.busy_tag = true;
        SB.Floating_Add.instruction_num = inst_num;
    }
    
    if (temp[6]) // Normalize or Branch
    {
        SB.Normalize.busy_tag = true;
        SB.Normalize.instruction_num = inst_num;
    }
    
    if (temp[5]) // Divide 
    {
        SB.Floating_Divide.busy_tag = true;
        SB.Floating_Divide.instruction_num = inst_num;
    }
    
    if (temp[0]) // Boolean
    {
        SB.Boolean.busy_tag = true;
        SB.Boolean.instruction_num = inst_num;
    }
    
    if (temp[1]) // Shift
    {
        SB.Shift.busy_tag = true;
        SB.Shift.instruction_num = inst_num;
    }
    
    if (temp[2]) // Fixed Add
    {
        SB.Fixed_Add.busy_tag = true;
        SB.Fixed_Add.instruction_num = inst_num;
    }
    
    if (temp[7]) // Pop_Count
    {
        SB.Pop_count.busy_tag = true;
        SB.Pop_count.instruction_num = inst_num;
    }
    
    if (temp[8]) // Increment
    {
        SB.Increment.busy_tag = true;
        SB.Increment.instruction_num = inst_num;
    }
}

// Clear busy flag and operators used in SB if the instruction is complete
void clear_func_unit(int inst_num)
{
    vector<bool> temp = indiv_functional_unit_used[inst_num];

    // Clear all if any used functional units once an instruction is complete
    if (temp[0]) // Boolean
    {
        SB.Boolean.clear_src_dest();
        SB.Boolean.busy_tag = false;
    }

    if (temp[1]) // Shift
    {
        SB.Shift.clear_src_dest();
        SB.Shift.busy_tag = false;
    }

    if (temp[2]) // Fixed Add
    {
        SB.Fixed_Add.clear_src_dest();
        SB.Fixed_Add.busy_tag = false;
    }
    
    if (temp[3]) // FL Add
    {
        SB.Floating_Add.clear_src_dest();
        SB.Floating_Add.busy_tag = false;
    }

    if (temp[4]) // FL Multiply
    {
        SB.Floating_Multiply.clear_src_dest();
        SB.Floating_Multiply.busy_tag = false;
    }
   
    if (temp[5]) // Divide 
    {
        SB.Floating_Divide.clear_src_dest();
        SB.Floating_Divide.busy_tag = false;
    }

    if (temp[6]) // Normalize or Branch
    {
        SB.Normalize.clear_src_dest();
        SB.Normalize.busy_tag = false;
    }

    if (temp[7]) // Pop_Count
    {
        SB.Pop_count.clear_src_dest();
        SB.Pop_count.busy_tag = false;
    }

    if (temp[8]) // Increment
    {
        SB.Increment.clear_src_dest();
        SB.Increment.busy_tag = false;
    }
}

// Check to see if any functional units above the current instruction have compelted and reset their busy tags / holds on data
void check_func_unit_complete(int inst_num, int curr_clock)
{
    // Go through SB and check any func unit that has busy tag. if busy, then check its unit_ready against the current clock, if unit ready < curr clock then clear the busy tag / info

    if (SB.Boolean.busy_tag && (unit_ready[SB.Boolean.instruction_num] < curr_clock)) // Check Boolean
    {
        clear_func_unit(SB.Boolean.instruction_num);
    }

    if (SB.Shift.busy_tag && (unit_ready[SB.Shift.instruction_num] < curr_clock)) // Check Shift
    {
        clear_func_unit(SB.Shift.instruction_num);
    }

    if (SB.Fixed_Add.busy_tag && (unit_ready[SB.Fixed_Add.instruction_num] < curr_clock)) // Check Fixed Add
    {
        clear_func_unit(SB.Fixed_Add.instruction_num);
    }

    if (SB.Floating_Add.busy_tag && (unit_ready[SB.Floating_Add.instruction_num] < curr_clock)) // Check Floating Add
    {
        clear_func_unit(SB.Floating_Add.instruction_num);
    }

    if (SB.Floating_Multiply.busy_tag && (unit_ready[SB.Floating_Multiply.instruction_num] < curr_clock)) // Check Floating Multiply
    {
        clear_func_unit(SB.Floating_Multiply.instruction_num);
    }

    if (SB.Floating_Divide.busy_tag && (unit_ready[SB.Floating_Divide.instruction_num] < curr_clock)) // Check Floating Divide
    {
        clear_func_unit(SB.Floating_Divide.instruction_num);
    }

    if (SB.Normalize.busy_tag && (unit_ready[SB.Normalize.instruction_num] < curr_clock)) // Check Normalize
    {
        clear_func_unit(SB.Normalize.instruction_num);
    }

    if (SB.Pop_count.busy_tag && (unit_ready[SB.Pop_count.instruction_num] < curr_clock)) // Check Pop Count
    {
        clear_func_unit(SB.Pop_count.instruction_num);
    }

    if (SB.Increment.busy_tag && (unit_ready[SB.Increment.instruction_num] < curr_clock)) // Check Increment
    {
        clear_func_unit(SB.Increment.instruction_num);
    }


    // this only checks for similar instructions....
    //for (int i = inst_num; i > 0; i--)
    //{
    //    if (functional_unit_used[inst_num] == functional_unit_used[i]) // Possible conflict bc similar instruction
    //    {
    //        if (unit_ready[i-1] < curr_clock) // then that instruction is complete
    //        {
    //            clear_func_unit(i); // Clear its busy_tag and sources, destination
    //        }
    //    }
    //}
}

// Find where first order conflict is and modify the issue entry as needed
int resolve_foc_conflict(int inst_num)
{
    int worst_clock_senario = 0;

    vector<bool> temp_funcs_used = indiv_functional_unit_used[inst_num];

    // Find where the conflicts are
    // Look through scoreboard to find which busy tags collide
    // Check all functional units to incase multiple are used
    if ((temp_funcs_used[0] == SB.Boolean.busy_tag) && (SB.Boolean.busy_tag == true)) // Boolean
    {
        if (worst_clock_senario < unit_ready[SB.Boolean.instruction_num])
        {
            worst_clock_senario = unit_ready[SB.Boolean.instruction_num];
        }
    }

    if ((temp_funcs_used[1] == SB.Shift.busy_tag) && (SB.Shift.busy_tag == true)) // Shift
    {
        if (worst_clock_senario < unit_ready[SB.Shift.instruction_num])
        {
            worst_clock_senario = unit_ready[SB.Shift.instruction_num];
        }
    }

    if ((temp_funcs_used[2] == SB.Fixed_Add.busy_tag) && (SB.Fixed_Add.busy_tag == true)) // Fixed Add
    {
        if (worst_clock_senario < unit_ready[SB.Fixed_Add.instruction_num])
        {
            worst_clock_senario = unit_ready[SB.Fixed_Add.instruction_num];
        }
    }

    if ((temp_funcs_used[3] == SB.Floating_Add.busy_tag) && (SB.Floating_Add.busy_tag == true)) // Floating Add
    {
        if (worst_clock_senario < unit_ready[SB.Floating_Add.instruction_num])
        {
            worst_clock_senario = unit_ready[SB.Floating_Add.instruction_num];
        }
    }

    if ((temp_funcs_used[4] == SB.Floating_Multiply.busy_tag) && (SB.Floating_Multiply.busy_tag == true)) // Floating Multiply
    {
        if (worst_clock_senario < unit_ready[SB.Floating_Multiply.instruction_num])
        {
            worst_clock_senario = unit_ready[SB.Floating_Multiply.instruction_num];
        }
    }

    if ((temp_funcs_used[5] == SB.Floating_Divide.busy_tag) && (SB.Floating_Divide.busy_tag == true)) // Floating Divide
    {
        if (worst_clock_senario < unit_ready[SB.Floating_Divide.instruction_num])
        {
            worst_clock_senario = unit_ready[SB.Floating_Divide.instruction_num];
        }
    }

    if ((temp_funcs_used[6] == SB.Normalize.busy_tag) && (SB.Normalize.busy_tag == true)) // Normalize
    {
        if (worst_clock_senario < unit_ready[SB.Normalize.instruction_num])
        {
            worst_clock_senario = unit_ready[SB.Normalize.instruction_num];
        }
    }

    if ((temp_funcs_used[7] == SB.Pop_count.busy_tag) && (SB.Pop_count.busy_tag == true)) // Pop Count
    {
        if (worst_clock_senario < unit_ready[SB.Pop_count.instruction_num])
        {
            worst_clock_senario = unit_ready[SB.Pop_count.instruction_num];
        }
    }

    if ((temp_funcs_used[8] == SB.Increment.busy_tag) && (SB.Increment.busy_tag == true)) // Increment
    {
        if (worst_clock_senario < unit_ready[SB.Increment.instruction_num])
        {
            worst_clock_senario = unit_ready[SB.Increment.instruction_num];
        }
    }

    return worst_clock_senario;
}

// Format and print the Table as rows are solved
void output_table(vector<string> inst_word, vector<string> inst_sem, vector<string> inst_sem2, vector<string> inst_len, vector<int> issue, vector<int> start, vector<int> result, vector<int> unit_ready, vector<string> fetch, vector<string> store, vector<string> functional_unit_used, vector<string> registers_used, int rows_solved)
{
    cout << "\n============================================================================================================================================================";
    cout << "\n| Word # |  Semantics   | Semantics cont.  | Inst. type | Issue | Start | Result | Unit ready | Fetch | Store |   Functional Unit(s)   |     Registers     |";
    cout << "\n------------------------------------------------------------------------------------------------------------------------------------------------------------";

    for (int i = 0; i < rows_solved; i++)
    {
        cout << "\n" << left
             << "    " << setw(5)  << inst_word[i] 
             << "  " << setw(10) << inst_sem[i] << "\t"
             << "   " << setw(15) << inst_sem2[i] << "\t"
             << setw(5) << inst_len[i] << "\t"
             << "    " << setw(3) << issue[i] << "\t"
             << "    " << setw(3) << start[i] << "\t"
             << "    " << setw(3) << result[i] << "\t"
             << "       " << setw(3) << unit_ready[i] << "\t"
             << " " << setw(3) << fetch[i] << "\t"
             << " " << setw(3) << store[i] << "\t"
             << setw(26) << functional_unit_used[i]
             << "   "<< setw(8) << registers_used[i];

        if (((i + 1) < rows_solved) && inst_word[i + 1] != "  ") { cout << "\n"; } // add newline between instruction words
    }
    cout << "\n============================================================================================================================================================";
}


// Convert decimal form of binary opcode to an octal number
unsigned long to_octal(unsigned long binary_num)
{
    int rem, i = 1, octalNumber = 0;
    while (binary_num != 0)
    {
        rem = binary_num % 8;
        binary_num /= 8;
        octalNumber += rem * i;
        i *= 10;
    }
    return octalNumber;
}

// given a string of reigsters, return a list with no repeats
string get_unique_registers(string dest, string op1, string op2)
{
    string result = "";
    vector<string> temp_vector;


    // This section is for sources/destinations within the scoreboard
    if ((dest != "") && (dest[0] != 'K') && dest[0] != 'B')
    {
        destination.push_back(dest);
    }

    if ((op1 != "") && (op1[0] != 'K') && op1[0] != 'B')
    {
        source_1.push_back(op1);
    }

    if ((op2 != "") && (op2[0] != 'K') && op2[0] != 'B')
    {
        source_2.push_back(op2);
    }
    else if (op2[0] == 'K')
    {
        source_2.push_back("--");
    }

    // this section is for registers entry in the table
    if (dest != "" && dest[0] != 'K')
    {
        temp_vector.push_back(dest);

        if (dest[0] == 'A')
        {
            string extra_operand = "X";
            extra_operand += dest[1];
            temp_vector.push_back(extra_operand);
        }
    }

    if (op1 != "" && op1[0] != 'K')
    {
        temp_vector.push_back(op1);

        if(op1[0] == 'A')
        {
            string extra_operand = "X";
            extra_operand += op1[1];
            temp_vector.push_back(extra_operand);
        }
    }

    if (op2 != "" && op2[0] != 'K')
    {
        temp_vector.push_back(op2);

        if (op2[0] == 'A')
        {
            string extra_operand = "X";
            extra_operand += op2[1];
            temp_vector.push_back(extra_operand);
        }
    }
    
    sort(temp_vector.begin(), temp_vector.end());
    temp_vector.erase(unique(temp_vector.begin(), temp_vector.end()), temp_vector.end());

    for (string reg : temp_vector)
        result += (reg + " ");

    return result;
}


// Functional units and functions
#pragma region Functional Units and implementations

#pragma region Branch Unit
void BRANCH(int Opcode, string inst)
{
    string semantic_string = "";
    string register_string = "";
    string destination = "";
    string operand1 = "";
    string operand2 = "";

    switch (Opcode)
    {
        case 00: // Stop, 0 clocks  // Done
        {
            instruction_semantics.push_back("-");
            functional_unit_used.push_back("Branch, Increment, Boolean");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 01: // RETURN JUMP to K, 14 clocks // Done
        {
            destination = "K" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());

            semantic_string = "RETURN JUMP to " + destination;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Branch, Increment, Boolean");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 02: // GO TO K + Bi (note 1), 14 clocks     // Done
        {
            operand1 = "B" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());

            operand2 = "K" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());

            semantic_string = "GO TO " + operand2 + " + " + operand1;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Branch, Increment, Boolean");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        #pragma region Not used according to "Test Data", assume increment unit is used as partner
        //case 030: // GO TO K if Xj == 0, 9 clocks *add 6 if branch to instruction is out of the stack (no memory conflict considered)
        //{

        //}
        //case 031: // GO TO K if Xj != 0, 9 clocks *add 6 if branch to instruction is out of the stack (no memory conflict considered)
        //{

        //}
        //case 032: // GO TO K if Xj == positive, 9 clocks *add 6 if branch to instruction is out of the stack (no memory conflict considered)
        //{

        //}
        //case 033: // GO TO K if Xj == negative, 9 clocks *add 6 if branch to instruction is out of the stack (no memory conflict considered)
        //{

        //}
        //case 034: // GO TO K if Xj is in range, 9 clocks *add 6 if branch to instruction is out of the stack (no memory conflict considered)
        //{

        //}
        //case 035: // GO TO K if Xj is out of range, 9 clocks *add 6 if branch to instruction is out of the stack (no memory conflict considered)
        //{

        //}
        //case 036: // GO TO K if Xj is definite, 9 clocks *add 6 if branch to instruction is out of the stack (no memory conflict considered)
        //{
    
        //}
        //case 037: // GO TO K if XJ is indefinite, 9 clocks *add 6 if branch to instruction is out of the stack (no memory conflict considered)
        //{

        //}
        #pragma endregion

        case 04: // GO TO K if Bi == Bj, 8 clocks *add 6 if branch to instruction is out of the stack (no memory conflict considered)  // Done
        {
            operand1 = "B" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand2 = "B" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                destination = "K" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                destination = "K" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = "Go to " + destination + " if " + operand1 + " == " + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Branch, Increment, Boolean");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 05: // GO TO K if Bi != Bj, 8 clocks *add 6 if branch to instruction is out of the stack (no memory conflict considered)
        {
            operand1 = "B" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand2 = "B" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                destination = "K" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                destination = "K" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = "Go to " + destination + " if " + operand1 + " != " + operand2;
            instruction_semantics.push_back("");

            functional_unit_used.push_back("Branch, Increment, Boolean");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 06: // Go TO K if Bi >= Bj, 8 clocks *add 6 if branch to instruction is out of the stack (no memory conflict considered)
        {
            operand1 = "B" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand2 = "B" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                destination = "K" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                destination = "K" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = "Go to " + destination + " if " + operand1 + " >= " + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Branch, Increment, Boolean");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 07: // GO TO K if Bi < Bj, 8 clocks *add 6 if branch to instruction is out of the stack (no memory conflict considered)
        {
            operand1 = "B" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand2 = "B" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                destination = "K" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                destination = "K" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = "Go to " + destination + " if " + operand1 + " < " + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Branch, Increment, Boolean");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        default: // If invalid opcode
        {
            instruction_semantics.push_back("Invalid Opcode");
            functional_unit_used.push_back("Invalid Opcode");
            registers_used.push_back("Invalid Opcode");
        }
    }
}

#pragma endregion

#pragma region Boolean Unit
void BOOLEAN(int Opcode, string inst)
{
    string semantic_string = "";
    string register_string = "";
    string destination = "";
    string operand1 = "";
    string operand2 = "";

    switch (Opcode)
    {
        case 10: // TRANSMIT Xj to Xi, 3 clocks
        {
            destination = "X" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "X" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            semantic_string = operand1 + " --> " + destination;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Boolean");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 11: // LOGICAL PRODUCT of Xj and Xk to Xi, 3 clocks
        {
            destination = "X" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "X" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand2 = "X" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "X" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = destination + " = " + operand1 + " ^ " + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Boolean");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 12: // LOGICAL SUM of Xj and Xk to Xi, 3 clocks
        {
            destination = "X" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "X" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand2 = "X" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "X" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = destination + " = " + operand1 + " + " + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Boolean");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 13: // LOGICAL DIFFERENCE of Xj and Xk to Xi, 3 clocks
        {
            destination = "X" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "X" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand2 = "X" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "X" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = destination + " = " + operand1 + " - " + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Boolean");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 14: // TRANSMIT Xk COMP. to Xi, 3 clocks
        {
            destination = "X" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand1 = "X" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand1 = "X" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = "!" + operand1 + " --> " + destination;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Boolean");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 15: // LOGICAL PRODUCT of Xj and Xk COMP to Xi, 3 clocks
        {
            destination = "X" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "X" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand2 = "X" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "X" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = destination + " = " + operand1 + " ^ !" + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Boolean");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 16: // LOGICAL SUM of Xj and Xk COMP to Xi, 3 clocks
        {
            destination = "X" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "X" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand2 = "X" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "X" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = destination + " = " + operand1 + " + !" + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Boolean");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 17: // LOGICAL DIFFERENCE of Xj and Xk COMP to Xi, 3 clocks 
        {
            destination = "X" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "X" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand2 = "X" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "X" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = destination + " = " + operand1 + " - !" + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Boolean");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        default: // If invalid opcode
        {
            instruction_semantics.push_back("Invalid Opcode");
            functional_unit_used.push_back("Invalid Opcode");
            registers_used.push_back("Invalid Opcode");
        }
    }
}
#pragma endregion

#pragma region Shift Unit
void SHIFT(int Opcode, string inst)
{
    string semantic_string = "";
    string register_string = "";
    string destination = "";
    string operand1 = "";
    string operand2 = "";

    switch (Opcode)
    {
        case 20: // Shift Xi LEFT jk places, 3 clocks
        {
            destination = "X" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = to_string(bitset<6>(inst.substr(9, 6)).to_ulong());

            semantic_string = destination + " = " + destination + " SHIFT LEFT " + operand1;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Shift");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 21: // SHIFT Xi RIGHT jk places, 3 clocks
        {
            destination = "X" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = to_string(bitset<6>(inst.substr(9, 6)).to_ulong());

            semantic_string = destination + " = " + destination + " SHIFT RIGHT " + operand1;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Shift");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 22: // SHIFT Xi NOMINALLY LEFT Bj places, 3 clocks
        {
            destination = "X" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "B" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            semantic_string = destination + " = " + destination + " SHIFT NOM. LEFT " + operand1;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Shift");
            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 23: // SHIFT Xi, NOMINALLY RIGHT Bj places, 3 clocks
        {
            destination = "X" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "B" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            semantic_string = destination + " = " + destination + " SHIFT NOM. RIGHT " + operand1;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Shift");
            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 24: // NORMALIZE Xk in Xi and Bj, 4 clocks
        {
            destination = "X" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "B" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());;

            if (inst.length() <= 15)
            {
                operand2 = "X" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "X" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = "NORM. " + operand2 + " in " + destination + " and " + operand1;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Shift");
            registers_used.push_back(get_unique_registers(destination,operand1,operand2));

            break;
        }

        case 25: // ROUND AND NORMALIZE Xk in Xi and Bj, 4 clocks
        {
            destination = "X" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "B" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());;

            if (inst.length() <= 15)
            {
                operand2 = "X" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "X" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = "Round and NORM. " + operand2 + " in " + destination + " and " + operand1;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Shift");
            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 26: // UNPACK Xk to Xi and Bj, 3 clocks
        {
            destination = "X" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "B" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());;

            if (inst.length() <= 15)
            {
                operand2 = "X" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "X" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = "UNPACK " + operand2 + " in " + destination + " and " + operand1;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Shift");
            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 27: // PACK Xi from Xk and Bj, 3 clocks
        {
            destination = "X" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "B" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());;

            if (inst.length() <= 15)
            {
                operand2 = "X" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "X" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = "PACK " + destination + " from " + operand2 + " and " + operand1;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Shift");
            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 43: // FORM jk MASK in Xi, 3 clocks
        {
            destination = "X" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = to_string(bitset<6>(inst.substr(9, 6)).to_ulong());

            semantic_string = "FORM " + operand1 + " MASK in " + destination;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Shift");
            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        default: // If invalid opcode
        {
            instruction_semantics.push_back("Invalid Opcode");
            functional_unit_used.push_back("Invalid Opcode");
            registers_used.push_back("Invalid Opcode");
        }
    }
}
#pragma endregion

#pragma region ADD Unit
void ADD(int Opcode, string inst)
{
    string semantic_string = "";
    string register_string = "";
    string destination = "";
    string operand1 = "";
    string operand2 = "";

    switch (Opcode)
    {
        case 30: // FLOATING SUM of Xj and Xk to Xi, 4 clocks
        {
            destination = "X" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "X" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand2 = "X" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "X" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = destination + " = " + operand1 + " + " + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("FL Add, Normalize");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 31: // FLOATING DIFFERENCE of Xj and Xk to Xi, 4 clocks
        {
            destination = "X" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "X" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand2 = "X" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "X" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = destination + " = " + operand1 + " - " + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("FL Add, Normalize");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 32: // FLOATING DP SUM of Xj and Xk to Xi, 4 clocks
        {
            destination = "X" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "X" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand2 = "X" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "X" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = "DP " + destination + " = " + operand1 + " + " + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("FL Add, Normalize");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 33: // FLOATING DP DIFFERENCE of Xj and Xk to Xi, 4 clocks
        {
            destination = "X" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "X" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand2 = "X" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "X" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = "DP " + destination + " = " + operand1 + " - " + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("FL Add, Normalize");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 34: // ROUND FLOATING SUM of Xi and Xk to Xi, 4 clocks
        {
            destination = "X" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "X" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand2 = "X" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "X" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = "ROUND " + destination + " = " + operand1 + " + " + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("FL Add, Normalize");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 35: // ROUND FLOATNG DIFFERENCE of Xj and XK to Xi, 4 clocks
        {
            destination = "X" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "X" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand2 = "X" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "X" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = "ROUND " + destination + " = " + operand1 + " - " + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("FL Add, Normalize");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        default: // If invalid opcode
        {
            instruction_semantics.push_back("Invalid Opcode");
            functional_unit_used.push_back("Invalid Opcode");
            registers_used.push_back("Invalid Opcode");
        }
    }
}
#pragma endregion

#pragma region LONG ADD Unit
void LONG_ADD(int Opcode, string inst)
{
    string semantic_string = "";
    string register_string = "";
    string destination = "";
    string operand1 = "";
    string operand2 = "";

    switch (Opcode)
    {
        case 36: // INTEGER SUM of Xj and Xk to Xi, 3 clocks
        {
            destination = "X" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "X" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand2 = "X" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "X" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = destination + " = " + operand1 + " + " + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Long-Add, Normalize");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 37: // INTEGER DIFFERENCE of Xj and Xk to Xi, 3 clocks
        {
            destination = "X" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "X" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand2 = "X" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "X" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = destination + " = " + operand1 + " - " + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Long-Add, Normalilze");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        default: // If invalid opcode
        {
            instruction_semantics.push_back("Invalid Opcode");
            functional_unit_used.push_back("Invalid Opcode");
            registers_used.push_back("Invalid Opcode");
        }
    }
}
#pragma endregion

#pragma region DIVIDE Unit
void DIVIDE(int Opcode, string inst)
{
    string semantic_string = "";
    string register_string = "";
    string destination = "";
    string operand1 = "";
    string operand2 = "";

    switch (Opcode)
    {
        case 44: // FLOATING DIVIDE Xj by Xk to Xi, 29 clocks
        {
            destination = "X" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "X" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand2 = "X" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "X" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = destination + " = " + operand1 + " / " + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("FL Divide, Normalize");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 45: // ROUND FLOATING DIVIDE  Xj by Xk to Xi, 29 clocks
        {
            destination = "X" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "X" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand2 = "X" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "X" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = "Round " + destination + " = " + operand1 + " / " + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("FL Divide, Normalize");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 46: // PASS, 0 clocks
        {
            semantic_string = "Pass";
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("FL Divide, Normalize");

            registers_used.push_back(" ");
        }

        case 47: // SUM of 1's in Xk to Xi, 8 clocks
        {
            destination = "X" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "X" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand2 = "X" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "X" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = "SUM 1's in " + operand2 + " to " + destination;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("FL Divide, Normalize");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        default: // If invalid opcode
        {
            instruction_semantics.push_back("Invalid Opcode");
            functional_unit_used.push_back("Invalid Opcode");
            registers_used.push_back("Invalid Opcode");
        }
    }
}
#pragma endregion

#pragma region MULTIPLY Unit
void MULTIPLY(int Opcode, string inst)
{
    string semantic_string = "";
    string register_string = "";
    string destination = "";
    string operand1 = "";
    string operand2 = "";

    switch (Opcode)
    {
        case 40: // FLOATING PRODUCT of Xj and Xk to Xi, 10 clocks
        {
            destination = "X" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "X" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand2 = "X" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "X" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = destination + " = " + operand1 + " * " + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("FL Multiply, Normalize");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 41: // ROUND FLOATING PRODUCT of Xj and Xk to Xi, 10 clocks
        {
            destination = "X" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "X" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand2 = "X" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "X" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = "Round " + destination + " = " + operand1 + " * " + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("FL Multiply, Normalize");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 42: // FLOATING DP PRODUCT of Xj and Xk to Xi, 10 clocks
        {
            destination = "X" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "X" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand2 = "X" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "X" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = "DP " + destination + " = " + operand1 + " * " + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("FL Multiply, Normalize");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        default: // If invalid opcode
        {
            instruction_semantics.push_back("Invalid Opcode");
            functional_unit_used.push_back("Invalid Opcode");
            registers_used.push_back("Invalid Opcode");
        }
    }
}
#pragma endregion

#pragma region INCREMENT Unit
void INCREMENT(int Opcode, string inst)
{
    string semantic_string = "";
    string register_string = "";
    string destination = "";
    string operand1 = "";
    string operand2 = "";

    switch (Opcode)
    {
        case 50: // SUM of Aj and K to Ai, 3 clocks
        {
            destination = "A" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "A" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand2 = "K" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "K" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = destination + " = " + operand1 + " + " + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Increment");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 51: // SUM of Bj and K to Ai, 3 clocks
        {
            destination = "A" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "B" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand2 = "K" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "K" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = destination + " = " + operand1 + " + " + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Increment");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 52: // SUM of Xj and K to Ai, 3 clocks
        {
            destination = "A" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "X" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand2 = "K" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "K" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = destination + " = " + operand1 + " + " + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Increment");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 53: // SUM of Xj and Bk to Ai, 3 clocks
        {
            destination = "A" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "X" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand2 = "B" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "B" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = destination + " = " + operand1 + " + " + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Increment");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 54: // SUM of Aj and Bk to Ai, 3 clocks
        {
            destination = "A" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "A" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand2 = "B" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "B" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = destination + " = " + operand1 + " + " + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Increment");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 55: // DIFFERENCE of Aj and Bk to Ai, 3 clocks
        {
            destination = "A" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "A" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand2 = "B" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "B" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = destination + " = " + operand1 + " - " + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Increment");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 56: // SUM of Bj and Bk to Zi, 3 clocks
        {
            destination = "Z" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "B" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand2 = "B" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "B" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = destination + " = " + operand1 + " + " + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Increment");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 57: // DIFFERENCE of Bj and Bk to Zi, 3 clocks
        {
            destination = "Z" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "B" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand2 = "B" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "B" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = destination + " = " + operand1 + " - " + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Increment");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 60: // SUM of Aj and K to Bi, 3 clocks
        {
            destination = "B" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "A" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand2 = "K" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "K" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = destination + " = " + operand1 + " + " + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Increment");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 61: // SUM of Bj and K to Bi, 3 clocks
        {
            destination = "B" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "B" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand2 = "K" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "K" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = destination + " = " + operand1 + " + " + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Increment");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 62: // SUM of Xj and K to Bi, 3 clocks
        {
            destination = "B" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "X" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand2 = "K" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "K" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = destination + " = " + operand1 + " + " + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Increment");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 63: // SUM of Xj and Bk to Bi, 3 clocks
        {
            destination = "B" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "X" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand2 = "B" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "B" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = destination + " = " + operand1 + " + " + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Increment");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 64: // SUM of Aj and Bk to Bi, 3 clocks
        {
            destination = "B" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "A" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand2 = "B" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "B" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = destination + " = " + operand1 + " + " + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Increment");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 65: // DIFFERENCE of Aj and Bk to Bi, 3 clocks
        {
            destination = "B" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "A" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand2 = "B" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "B" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = destination + " = " + operand1 + " - " + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Increment");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 66: // SUM of Bj and Bk to Bi, 3 clocks
        {
            destination = "B" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "B" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand2 = "B" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "B" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = destination + " = " + operand1 + " + " + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Increment");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 67: // DIFFERENCE of Bj and Bk to Bi, 3 clocks
        {
            destination = "B" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "B" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand2 = "B" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "B" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = destination + " = " + operand1 + " - " + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Increment");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 70: // SUM of Aj and K to Xi, 3 clocks
        {
            destination = "X" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "A" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand2 = "K" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "K" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = destination + " = " + operand1 + " + " + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Increment");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 71: // SUM of Bj and K to Xi, 3 clocks
        {
            destination = "X" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "B" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand2 = "K" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "K" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = destination + " = " + operand1 + " + " + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Increment");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 72: // SUM of Xj and K to Xi, 3 clocks
        {
            destination = "X" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "X" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand2 = "K" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "K" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = destination + " = " + operand1 + " + " + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Increment");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 73: // SUM of Xj and Bk to Xi, 3 clocks
        {
            destination = "X" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "X" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand2 = "B" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "B" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = destination + " = " + operand1 + " + " + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Increment");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 74: // SUM of Aj and Bk to Xi, 3 clocks
        {
            destination = "X" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "A" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand2 = "B" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "B" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = destination + " = " + operand1 + " + " + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Increment");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 75: // DIFFERENCE of Aj and Bk to Xi, 3 clocks
        {
            destination = "X" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "A" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand2 = "B" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "B" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = destination + " = " + operand1 + " - " + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Increment");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 76: // SUM of Bj and Bk to Xi, 3 clocks
        {
            destination = "X" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "B" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand2 = "B" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "B" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = destination + " = " + operand1 + " + " + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Increment");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        case 77: // DIFFERENCE of Bj and Bk to Xi, 3 clocks
        {
            destination = "X" + to_string(bitset<3>(inst.substr(6, 3)).to_ulong());
            operand1 = "B" + to_string(bitset<3>(inst.substr(9, 3)).to_ulong());

            if (inst.length() <= 15)
            {
                operand2 = "B" + to_string(bitset<3>(inst.substr(12, 3)).to_ulong());
            }
            else
            {
                operand2 = "B" + to_string(bitset<18>(inst.substr(12, 18)).to_ulong());
            }

            semantic_string = destination + " = " + operand1 + " - " + operand2;
            instruction_semantics.push_back(semantic_string);

            functional_unit_used.push_back("Increment");

            registers_used.push_back(get_unique_registers(destination, operand1, operand2));

            break;
        }

        default: // If invalid opcode
        {
            instruction_semantics.push_back("Invalid Opcode");
            functional_unit_used.push_back("Invalid Opcode");
            registers_used.push_back("Invalid Opcode");
        }
    }
}
#pragma endregion



#pragma region NORMALIZE Unit
void NORMALIZE(int Opcode, string inst)
{
    string semantic_string = "";
    string register_string = "";
    string destination = "";
    string operand1 = "";
    string operand2 = "";

    switch (Opcode)
    {
        case 40: // FLOATING PRODUCT of Xj and Xk to Xi, 10 clocks
        {

        }
        case 41: // ROUND FLOATING PRODUCT of Xj and Xk to Xi, 10 clocks
        {

        }
        case 42: // FLOATING DP PRODUCT of Xj and Xk to Xi, 10 clocks
        {

        }
    }
}
#pragma endregion Performs the same as Branch Unit in CDC6600 (for this project)

#pragma region POP_COUNT Unit
void POP_COUNT(int Opcode, string inst)
{
    string semantic_string = "";
    string register_string = "";
    string destination = "";
    string operand1 = "";
    string operand2 = "";

    // count # 1's in operand register Xk and store lower order 6 bits of operand register Xi. Bits 6 through 59 are cleared to 0
    // This deals with the 60bit A,X,B registers

    switch (Opcode)
    {
        case 40: // FLOATING PRODUCT of Xj and Xk to Xi, 10 clocks
        {

        }
        case 41: // ROUND FLOATING PRODUCT of Xj and Xk to Xi, 10 clocks
        {

        }
        case 42: // FLOATING DP PRODUCT of Xj and Xk to Xi, 10 clocks
        {

        }
    }
}
#pragma endregion // What do, he didn't really answer this question.... Probably just not going to implement

#pragma endregion











///////////////////////////////////////////////
/////           *** NOTES ***            //////
///////////////////////////////////////////////

// Assume there is no memory conflicts

// Simluator will only do floating point numbers

// Floating point number is represented by fractional number (mantissa) multiplied by a base to a power (ie. +.1522 x10^3), normalizing means when the left most digit
//  is non-zero (0.0523x10^4 is not normalized --> 0.523x10^3 to normalize). Anytime there is arithmetic we should normalize.

// X and Y are vectors?? Meaning --> they are arrays of numbers  n =5 for this project
// Structure of program should be loop

// "Set B1 to 1" is used for index addressing because of the vectors

// Some operations (like arithmetic) can use multiple functional units --> scoreboard will have to make sure all functional units are available. If not, then 1st order conflict

// Instuction Opcode given to scoreboard
// 1.) Check whether functional unit is available
//     - if not available, 1st order confilit, pipeline stops
//         else, functional unit tagged as reserved

// 2.) Scoreboard checks sources/destinations
//      - Check distination first, is it used before? If yes then 1st order conflict (hardware unavailable) - pipeline stops
//      - If destination is good, check sources. Are source registers destination of previously issued insturction which has not completed 2nd order conflict?
//      - look for 3rd order conflicts (write after read) after 2nd order conflict (read after write)
//          *resolve by delaying execution until conflic is resolved

// 3.) Clock table will reflect actions of scoreboard (there is a counter --> clock_pulses)

// Lecture 10/20 39:41 starts filling out the example table

// B0 is always 0