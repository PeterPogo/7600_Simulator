// CDC_7600_Simulator.cpp : This file contains the 'main' function. Program execution begins and ends there.

///////////////////////////////////////////////////////////////////////////////////////
// Desc: CDC 7600 & CDC 6600 Simulator                                               //
// Class: CpE 5120                                                                   //
// Instructor: Dr. Ali Hurson                                                        //
// Date: 11/8/2020                   FS20                                            //
// Piotr Pogorzelski, Kendall Butler                                                 //
///////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <bitset>
#include <iomanip> 
#include "CDC_7600_Simulator.h"
using namespace std;

class Func_Unit_SB
{
public:
    string funcUnit_name;
    bool busy_tag; // Is the functional unit busy or available? 1 = Busy , 0 = Available
    bool idle;
    int segment_Time; // # clock_pulses (from table)
    int execution_Time; // # clock_pulses (from table)

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

        S1 = "B1";
        S2 = "A3";
        D1 = "X3";
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

    // Detect conficts
    // Resolve conflicts
};


#pragma region Operator Overloading
// Overload << operator for Scoreboard Class
ostream& operator<<(ostream& os, const Scoreboard& sb)
{
    os << "\n======================================== ScoreBoard ============================================";
    os << "\n| Func Unit | busy_tag| idle | segment_time | execution_time | Source 1 | Source 2 | Destination";
    os << "\n------------------------------------------------------------------------------------------------";
    os << "\n" << sb.Boolean.funcUnit_name << "           " << sb.Boolean.busy_tag << "      " << sb.Boolean.idle << "\t    " << sb.Boolean.segment_Time << "\t\t    " << sb.Boolean.execution_Time << "\t\t  " << sb.Boolean.S1 << "\t     " << sb.Boolean.S2 << "\t\t " << sb.Boolean.D1;
    os << "\n" << sb.Shift.funcUnit_name << "             " << sb.Shift.busy_tag << "      " << sb.Shift.idle << "\t    " << sb.Shift.segment_Time << "\t\t    " << sb.Shift.execution_Time << "\t\t  " << sb.Shift.S1 << "\t     " << sb.Shift.S2 << "\t\t " << sb.Shift.D1;
    os << "\n" << sb.Fixed_Add.funcUnit_name << "         " << sb.Fixed_Add.busy_tag << "      " << sb.Fixed_Add.idle << "\t    " << sb.Fixed_Add.segment_Time << "\t\t    " << sb.Fixed_Add.execution_Time << "\t\t  " << sb.Fixed_Add.S1 << "\t     " << sb.Fixed_Add.S2 << "\t\t " << sb.Fixed_Add.D1;
    os << "\n" << sb.Floating_Add.funcUnit_name << "      " << sb.Floating_Add.busy_tag << "      " << sb.Floating_Add.idle << "\t    " << sb.Floating_Add.segment_Time << "\t\t    " << sb.Floating_Add.execution_Time << "\t\t  " << sb.Floating_Add.S1 << "\t     " << sb.Floating_Add.S2 << "\t\t " << sb.Floating_Add.D1;;
    os << "\n" << "Floating Mult" << "     " << sb.Floating_Multiply.busy_tag << "      " << sb.Floating_Multiply.idle << "\t    " << sb.Floating_Multiply.segment_Time << "\t\t    " << sb.Floating_Multiply.execution_Time << "\t\t  " << sb.Floating_Multiply.S1 << "\t     " << sb.Floating_Multiply.S2 << "\t\t " << sb.Floating_Multiply.D1;
    os << "\n" << "Floating Div" << "      " << sb.Floating_Divide.busy_tag << "      " << sb.Floating_Divide.idle << "\t    " << sb.Floating_Divide.segment_Time << "\t\t    " << sb.Floating_Divide.execution_Time << "\t\t  " << sb.Floating_Divide.S1 << "\t     " << sb.Floating_Divide.S2 << "\t\t " << sb.Floating_Divide.D1;
    os << "\n" << sb.Normalize.funcUnit_name << "         " << sb.Normalize.busy_tag << "      " << sb.Normalize.idle << "\t    " << sb.Normalize.segment_Time << "\t\t    " << sb.Normalize.execution_Time << "\t\t  " << sb.Normalize.S1 << "\t     " << sb.Normalize.S2 << "\t\t " << sb.Normalize.D1;
    os << "\n" << sb.Pop_count.funcUnit_name << "         " << sb.Pop_count.busy_tag << "      " << sb.Pop_count.idle << "\t    " << sb.Pop_count.segment_Time << "\t\t    " << sb.Pop_count.execution_Time << "\t\t  " << sb.Pop_count.S1 << "\t     " << sb.Pop_count.S2 << "\t\t " << sb.Pop_count.D1;
    os << "\n" << sb.Increment.funcUnit_name << "         " << sb.Increment.busy_tag << "      " << sb.Increment.idle << "\t    " << sb.Increment.segment_Time << "\t\t    " << sb.Increment.execution_Time << "\t\t  " << sb.Increment.S1 << "\t     " << sb.Increment.S2 << "\t\t " << sb.Increment.D1;
    os << "\n================================================================================================";

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
#pragma endregion

int main()
{
    // Test Data to run
    // Y = AX2 + BX
    // Y = AX2 + BX , X & Y are vectors (limited to 5 entries)
    // Y = AX2 + BX + C

    // Start simulation by asking for input
    int test_data_choice;

    bool continue_sim = true;
    char continue_response;

    int inst_count = 0;
    string instruction;
    vector<string> instructions;


    cout << "--- CDC 7600 Simulator ---";
    while (continue_sim)
    {
        //cout <<"\n\nChose an equation to simulate:" << "\n1: Y = AX^2 + BX" << "\n2: Y = AX^2 + BX + C" << "\n3: Y = AX^2 + BX (X & Y = vectors)\n\n";
        //cin >> test_data_choice;
        
        // Read in instructions from file
        cout << "\n\nReading instructions from file (binary).....\n\n";

        ifstream input_file("instructions.txt");
        if (input_file.is_open())
        {
            while ( getline(input_file, instruction))
            {
                instructions.push_back(instruction);
                cout << "Instruction " << inst_count << ": " << instruction << " " << instruction.length() << '\n';
                inst_count++;
            }
            input_file.close();
        }
        else { cout << "\nUnable to open file"; }

        // Create blank table and fill in vectors by decoding
       create_blank_table(instructions);


       // simulate_CDC7600(test_data_choice);

        cout << "\n\nSimulation complete, would you like to simulate another set of instructions? y/n:  ";
        cin >> continue_response;
        if (continue_response == 'n') { continue_sim = false; }
    }
    cout << "\nExiting...."; // Simulation complete
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
        unsigned long opcode = bitset<6>(inst.substr(0, 6)).to_ulong(); // Convert first 6 bits to Op-Code

        // Send Op-code to the switchcase
        if ((opcode >= 00) && (opcode <= 7)) { BRANCH(opcode, inst); }
        else if ((opcode >= 10) && (opcode <= 17)) { BOOLEAN(opcode, inst); }
        else if ((opcode >= 20) && (opcode <= 27) || (opcode == 43)) { SHIFT(opcode, inst); }
        else if ((opcode >= 30) && (opcode <= 35)) { ADD(opcode, inst); }
        else if ((opcode >= 36) && (opcode <= 37)) { LONG_ADD(opcode, inst); }
        else if ((opcode >= 40) && (opcode <= 42)) { MULTIPLY(opcode, inst); }
        else if ((opcode >= 44) && (opcode <= 47)) { DIVIDE(opcode, inst); }
        else if ((opcode >= 50) && (opcode <= 77)) { INCREMENT(opcode, inst); }
    }
}

// Simluation of CDC7600 Processor
void simulate_CDC7600(int test_data_eq)
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

    // Processor Declarations

    // Instruction stack of 12 - 60-bit registers (allows for up to 48 previously fetched instructions to be readily available in the instruction stack).
    // 24 total registers - 5 Operand-Address pairs are used for read and 2 are used for write
    int A[8]; // Address registers, paired with x register - 18 bits
    int B[8]; // Index registers - 18 bits,  B0 is always 0
    int x[8]; // Operand register paired with address register - 60 bits

    B[0] = 0; // constant within the index resgister

    double A_Constant = 2.0; // constant stored in memory
    double B_Constant = 3.0; // constant stored in memory
    double C_Constant = 4.0; // constant stored in memory

    int clock_pulses = 0;
    Scoreboard SB;

    // Testing output
    output_table(instruction_word, instruction_semantics, instruction_semantics_2, instruction_length, issue, start, result, unit_ready, fetch, store, functional_unit_used, registers_used, 10);

    cout << "\n\n" << SB;

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
             << setw(25) << functional_unit_used[i]
             << "   "<< setw(8) << registers_used[i];

        if (((i + 1) < rows_solved) && inst_word[i + 1] != "  ") { cout << "\n"; } // add newline between instruction words
    }
    cout << "\n============================================================================================================================================================";
}

// Functional units and functions
#pragma region Functional Units and implementations

#pragma region Branch Unit
void BRANCH(int Opcode, string inst)
{
    switch (Opcode)
    {
        case 00: // Stop, 0 clocks
        {
            //instruction_semantics;
            //instruction_semantics_2;
            //functional_unit_used;
            //registers_used;
        }
        case 01: // RETURN JUMP to K, 14 clocks
        {

        }
        case 02: // GO TO K + Bi (note 1), 14 clocks
        {

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


        case 04: // GO TO K if Bi == Bj, 8 clocks *add 6 if branch to instruction is out of the stack (no memory conflict considered)
        {

        }
        case 05: // GO TO K if Bi != Bj, 8 clocks *add 6 if branch to instruction is out of the stack (no memory conflict considered)
        {

        }
        case 06: // Go TO K if Bi >= Bj, 8 clocks *add 6 if branch to instruction is out of the stack (no memory conflict considered)
        {

        }
        case 07: // GO TO K if Bi < Bj, 8 clocks *add 6 if branch to instruction is out of the stack (no memory conflict considered)
        {

        }
    }
}

#pragma endregion

#pragma region Boolean Unit
void BOOLEAN(int Opcode, string inst)
{
    switch (Opcode)
    {
        case 10: // TRANSMIT Xj to Xi, 3 clocks
        {

        }
        case 11: // LOGICAL PRODUCT of Xj and Xk to Xi, 3 clocks
        {

        }
        case 12: // LOGICAL SUM of Xj and Xk to Xi, 3 clocks
        {

        }
        case 13: // LOGICAL DIFFERENCE of Xj and Xk to Xi, 3 clocks
        {
    
        }
        case 14: // TRANSMIT Xk COMP. to Xi, 3 clocks
        {

        }
        case 15: // LOGICAL PRODUCT of XJ and Xk COMP to Xi, 3 clocks
        {

        }
        case 16: // LOGICAL SUM of XJ and Xk COMP ti Xi, 3 clocks
        {
    
        }
        case 17: // LOGICAL DIFFERENCE of Xj and Xk COMP to Xi, 3 clocks
        {

        }
    }
}
#pragma endregion

#pragma region Shift Unit
void SHIFT(int Opcode, string inst)
{
    switch (Opcode)
    {
        case 20: // Shift Xi LEFT jk places, 3 clocks
        {

        }
        case 21: // SHIFT Xi RIGHT jk places, 3 clocks
        {

        }
        case 22: // SHIFT Xi NOMINALLY LEFT Bj places, 3 clocks
        {

        }
        case 23: // SHIFT Xi, NOMINALLY RIGHT Bj places, 3 clocks
        {

        }
        case 24: // NORMALIZE Xk in Xi and Bj, 4 clocks
        {

        }
        case 25: // ROUND AND NORMALIZE Xk in Xi and Bj, 4 clocks
        {

        }
        case 26: // UNPACK Xk to Xi and Bj, 3 clocks
        {

        }
        case 27: // PACK Xi from Xk and Bj, 3 clocks
        {

        }
        case 43: // FORM jk MASK in Xi, 3 clocks
        {

        }
    }
}
#pragma endregion

#pragma region ADD Unit
void ADD(int Opcode, string inst)
{
    switch (Opcode)
    {
        case 30: // FLOATING SUM of Xj and Xk to Xi, 4 clocks
        {

        }
        case 31: // FLOATING DIFFERENCE of XK and Xk to Xi, 4 clocks
        {

        }
        case 32: // FLOATING DP SUM of Xj and Xk to Xi, 4 clocks
        {

        }
        case 33: // FLOATING DP DIFFERENCE of Xj and Xk to Xi, 4 clocks
        {

        }
        case 34: // ROUND FLOATING SUM of Xi and Xk to Xi, 4 clocks
        {

        }
        case 35: // ROUND FLOATNG DIFFERENCE of Xj and XK to Xi, 4 clocks
        {

        }
    }
}
#pragma endregion

#pragma region LONG ADD Unit
void LONG_ADD(int Opcode, string inst)
{
    switch (Opcode)
    {
        case 36: // INTEGER SUM of Xj and Xk to Xi, 3 clocks
        {

        }
        case 37: // INTEGER DIFFERENCE of Xj and Xk to Xi, 3 clocks
        {

        }
    }
}
#pragma endregion

#pragma region DIVIDE Unit
void DIVIDE(int Opcode, string inst)
{
    switch (Opcode)
    {
        case 44: // FLOATING DIVIDE Xj by Xk to Xi, 29 clocks
        {

        }
        case 45: // ROUND FLOATING DIVIDE  Xj by Xk to Xi, 29 clocks
        {

        }
        case 46: // PASS, 0 clocks
        {

        }
        case 47: // SUM of 1's in Xk to Xi, 8 clocks
        {

        }
    }
}
#pragma endregion

#pragma region MULTIPLY Unit
void MULTIPLY(int Opcode, string inst)
{
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
#pragma endregion

#pragma region INCREMENT Unit
void INCREMENT(int Opcode, string inst)
{
    switch (Opcode)
    {
        case 50: // SUM  of Aj and K to Ai, 3 clocks
        {

        }
        case 51: // SUM of Bj and K to Ai, 3 clocks
        {

        }
        case 52: // SUM of Xj and K to Ai, 3 clocks
        {

        }
        case 53: // SUM of Xj and Bk to Ai, 3 clocks
        {

        }
        case 54: // SUM of Aj and Bk to Ai, 3 clocks
        {

        }
        case 55: // DIFFERENCE of Aj and Bk to Ai, 3 clocks
        {

        }
        case 56: // SUM of Bj and Bk to Zi, 3 clocks
        {

        }
        case 57: // DIFFERENCE of Bj and Bk to Zi, 3 clocks
        {

        }
        case 60: // SUM of Aj and K to Bi, 3 clocks
        {

        }
        case 61: // SUM of Bj and K to Bi, 3 clocks
        {

        }
        case 62: // SUM of Xj and K to Bi, 3 clocks
        {

        }
        case 63: // SUM of Xj and Bk to Bi, 3 clocks
        {

        }
        case 64: // SUM of Aj and Bk to Bi, 3 clocks
        {

        }
        case 65: // DIFFERENCE of Aj and Bk to Bi, 3 clocks
        {

        }
        case 66: // SUM of Bj and Bk to Bi, 3 clocks
        {

        }
        case 67: // DIFFERENCE of Aj and Bk to Bi, 3 clocks
        {

        }
        case 70: // SUM of Aj and K to Xi, 3 clocks
        {

        }
        case 71: // SUM of Bj and K to Xi, 3 clocks
        {

        }
        case 72: // SUM of Xj and K to Xi, 3 clocks
        {

        }
        case 73: // SUM of Xj and Bk to Xi, 3 clocks
        {

        }
        case 74: // SUM of Aj and Bk to Xi, 3 clocks
        {

        }
        case 75: // DIFFERENCE of Aj and Bk to Xi, 3 clocks
        {

        }
        case 76: // SUM of Bj and Bk to Xi, 3 clocks
        {

        }
        case 77: // DIFFERENCE of Bj and Bk to Xi, 3 clocks
        {

        }
    }
}
#pragma endregion

#pragma endregion













///////////////////////////////////////////////
/////           *** NOTES ***            //////
///////////////////////////////////////////////

// Assume there is no memory conflicts

// Simluator will only do floating point numbers

// Floating point number is represented by fractional number (mantissa) multiplied by a base to a power (ie. +.1522 x10^3), normalizing means when the left most digit
//  is non-zero (0.0523x10^4 is not normalized --> 0.523x10^3 to normalize). Anytime there is arithmetic we should normalize.

// X and Y are vectors?? Meaning --> they are arrays of numbers
// Structure of program should be loop

// "Set B1 to 1" is used for index addressing because of the arrays

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