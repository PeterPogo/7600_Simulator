// CDC_7600_Simulator.cpp : This file contains the 'main' function. Program execution begins and ends there.

// Simulating how to the scoreboard works

#include <iostream>
#include <string>
#include <vector>
using namespace std;

class Func_Unit_SB
{
public:
    string funcUnit_name;
    bool busy_tag; // Is the functional unit busy or available? 1 = Busy , 0 = Available
    bool idle;
    int segment_Time; // # clock_pulses (from table)
    int execution_Time; // # clock_pulses (from table)

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

    // Sources and Destintations per instuction
    int source1 = 1;
    int source2 = 2;
    int desintation = 3; // Destination has tag bit

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

int main()
{
    // Instruction stack of 12 - 60-bit registers (allows for up to 48 previously fetched instructions to be readily available in the instruction stack).

    // 24 total registers - 5 Operand-Address pairs are used for read and 2 are used for write
    int A[8]; // Address registers, paired with x register - 18 bits
    int B[8]; // Index registers - 18 bits
    int x[8]; // Operand register paired with address register - 60 bits

    int clock_pulses = 0;
    Scoreboard SB;

    cout << SB.Boolean.funcUnit_name << "\n";

    // Testing git
}



///////////////////////////////////////////////
/////           *** NOTES ***            //////
///////////////////////////////////////////////

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