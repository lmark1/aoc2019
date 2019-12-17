#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <unordered_set>
#include <set>
#include <map>
#include <algorithm>
#include <iterator>
#include <functional>

#define _USE_MATH_DEFINES
#include <cmath>

#include "my_macros.hpp"

class IntcodeComputer
{

enum Opcode 
{
    HALT            = 99,
    ADD             = 1,
    MULT            = 2,
    INPUT           = 3,
    OUTPUT          = 4,
    JUMP_IF_TRUE    = 5,
    JUMP_IF_FALSE   = 6,
    LESS_THAN       = 7,
    EQUALS          = 8,
    BASE_OP         = 9
};

enum ParameterMode 
{
    POSITION_MODE   = 0,
    IMMEDIATE_MODE  = 1,
    RELATIVE_MODE   = 2
};

public:
    IntcodeComputer(std::vector<long long> intCode)
        :_intCode(intCode), _intCodeOrig(intCode) 
    {
        _operationsMap.emplace(ADD, [](long long a, long long b) { return a+b; } );
        _operationsMap.emplace(MULT, [](long long a, long long b) { return a*b; } );
        _operationsMap.emplace(LESS_THAN, [](long long a, long long b) { return a < b ? 1 : 0; } );
        _operationsMap.emplace(EQUALS, [](long long a, long long b) { return a == b ? 1 : 0; } );
    }
    
    void reset()
    {
        _intCode = std::vector<long long> (_intCodeOrig);
        _instructionPointer = 0;
        _halted = false;
        _lastOutput = -1;
        _relativeBase = 0;
    }

    int calculate()
    {
        return calculate_internal(-1, _intCode, false, true);
    }

    std::pair<int, int> calcDoubleOutput(int input)
    {
        int first = calculate_internal(input, _intCode, true, false);
        int second = calculate_internal(input, _intCode, true, false);
        return std::make_pair(first, second);
    }

    int getLastOutput() { return _lastOutput; }

    bool isHalted() { return _halted; }
    
private:

    void setMemoryVal(long long index, long long value)
    {
        while (index > _intCode.size() - 1)
            _intCode.push_back(0);    
        _intCode[index] = value; 
    }

    long long getMemoryVal(long long index)
    {
        while (index > _intCode.size() - 1)
            _intCode.push_back(0);    
        return _intCode[index];
    }

    int getArgIndex(int paramMode, int index)
    {
        switch (paramMode)
        {    
            case POSITION_MODE:
                return getMemoryVal(index);                

            case IMMEDIATE_MODE:
                return index;

            case RELATIVE_MODE:
                return _relativeBase + getMemoryVal(index);
            
            default:
                return -1;
        }; 
    }

    int calculate_internal(int input, std::vector<long long> intCode, bool returnOnOutput = false, bool takeUserInput = false)
    {
        long long output = -1;
        bool outputSet = false;
        //std::printf ("Amp input %d, Amp phase %d\n", ampInput, ampPhase);
        while (_instructionPointer < _intCode.size())
        {   
            int paramMode3 = _intCode[_instructionPointer] / 10000,
                paramMode2 = (_intCode[_instructionPointer] % 10000) / 1000,
                paramMode1 = (_intCode[_instructionPointer] % 1000 ) / 100,
                opCode = _intCode[_instructionPointer] % 100;

            // assume index is at operation code
            if (opCode == HALT)
            {
                _halted = true;
                // std::printf("Amplifier %d halted.", _index);
                break;
            }

            if (opCode == INPUT)
            {   
                if (input == -1)
                    throw std::runtime_error("Input called with -1 passed");

                if (takeUserInput)
                {
                    LOG("Please input number: ");
                    std::cin >> input;
                }
                // std::cout << "Current input is: " << input << std::endl;
                setMemoryVal(  getArgIndex(paramMode1, _instructionPointer + 1), input);
                _instructionPointer += 2;
            }
            else if (opCode == OUTPUT)
            {
                output = getMemoryVal( getArgIndex(paramMode1, _instructionPointer + 1) );
                //std::cout << "DIAGNOSTICS output: " << output << std::endl;
                _instructionPointer += 2;
                outputSet = true;
                if (returnOnOutput)
                    break;
            }
            else if (opCode == BASE_OP)
            {
                _relativeBase += getMemoryVal( getArgIndex(paramMode1, _instructionPointer + 1) );
                _instructionPointer += 2;
            }
            else if (opCode == JUMP_IF_TRUE || opCode == JUMP_IF_FALSE)
            {
                int firstArg = getMemoryVal( getArgIndex(paramMode1, _instructionPointer + 1) );
                int secondArg = getMemoryVal( getArgIndex(paramMode2, _instructionPointer + 2) );
                
                if (opCode == JUMP_IF_TRUE && firstArg != 0 ||
                    opCode == JUMP_IF_FALSE && firstArg == 0)
                    _instructionPointer = secondArg;
                else
                    _instructionPointer += 3;
            }
            else
            {
                int firstArgIndex = getArgIndex(paramMode1, _instructionPointer + 1),
                    secondArgIndex = getArgIndex(paramMode2, _instructionPointer + 2),
                    resultIndex = getArgIndex(paramMode3, _instructionPointer + 3);

                //std::printf("Indices [%d, %d, %d]\n", firstArgIndex, secondArgIndex, resultIndex);
                // Do the operation
                setMemoryVal(resultIndex, 
                    _operationsMap[opCode] (
                        getMemoryVal( firstArgIndex ), getMemoryVal( secondArgIndex )
                    ));
                _instructionPointer += 4;
            }
        }

        if (outputSet)
            _lastOutput = output;
        return output;
    }

    int _index = -1, _lastOutput = -1, _instructionPointer = 0, _relativeBase = 0;
    bool _halted = false;
    std::vector<long long> _intCode;
    const std::vector<long long> _intCodeOrig;
    std::map < long long, std::function<long long(long long, long long)> > _operationsMap;
};

int part1(IntcodeComputer& ic, int startingTile)
{
    std::pair<int, int> currPos(0, 0);
    std::map< std::pair<int, int>, int > colorMap;
    double heading = M_PI_2;
    while (!ic.isHalted())
    {
        colorMap.emplace(currPos, startingTile);
        auto pair = ic.calcDoubleOutput( colorMap[currPos] );

        colorMap[currPos] = pair.first;
        heading += pair.second == 0 ? M_PI_2 : -M_PI_2;
        //std::printf("New heading: %.3f\n", heading);

        currPos = std::make_pair (
            currPos.first + (int)cos(heading),
            currPos.second + (int)sin(heading));

        //std::printf("New position coords: (%d, %d)\n\n", currPos.first, currPos.second);
    }

    return colorMap.size();
}


int part2(IntcodeComputer& ic, int startingTile)
{
    std::pair<int, int> currPos(1, 1);
    std::map< std::pair<int, int>, int > colorMap;
    double heading = -M_PI_2;
    int min_x = 1e5, min_y = 1e5, max_x = -1e5, max_y = -1e5;
    if (currPos.first < min_x)
        min_x = currPos.first;
    if (currPos.second < min_y)
        min_y = currPos.second;
    if (currPos.first > max_x)
        max_x = currPos.first;
    if (currPos.second > max_y)
        max_y = currPos.second;
    
    colorMap.emplace(currPos, startingTile);
    while (!ic.isHalted())
    {
        colorMap.emplace(currPos, 0);
        auto pair = ic.calcDoubleOutput( colorMap[currPos] );

        colorMap[currPos] = pair.first;
        heading += pair.second == 0 ? M_PI_2 : -M_PI_2;
        //std::printf("New heading: %.3f\n", heading);

        currPos = std::make_pair (
            currPos.first - (int)cos(heading),
            currPos.second + (int)sin(heading));

        if (currPos.first < min_x)
            min_x = currPos.first;
        if (currPos.second < min_y)
            min_y = currPos.second;
        if (currPos.first > max_x)
            max_x = currPos.first;
        if (currPos.second > max_y)
            max_y = currPos.second;

        //std::printf("New position coords: (%d, %d)\n\n", currPos.first, currPos.second);
    }

    std::printf("(%d, %d) - (%d, %d)\n", min_x, min_y, max_x, max_y);
    const int width = max_x - min_x;
    const int height = max_y - min_y;
    std::vector <std::vector <char>> grid(height+1, std::vector<char>(width+1, '.'));

    for (auto& item : colorMap)
    {
        if (item.second == 1)
        grid[item.first.second - min_y][item.first.first - min_x] = '#';
    }
    for (auto& line : grid)
    {
        for (auto el : line)
            std::cout << el;
        std::cout << std::endl;
    }
    return colorMap.size();
}

int main ()
{
    std::fstream inputFile("day11.txt");
    std::string line, token;
    getline(inputFile, line);

    std::vector<long long> intCode;
    std::stringstream ss(line);
    while (getline(ss, token, ','))
        intCode.push_back(std::stoll(token));
    
    IntcodeComputer ic(intCode);
    std::printf("Robot visited atleast %d positions.\n",part1(ic, 0));
    ic.reset();
    std::printf("Robot visited atleast %d positions.\n", part2(ic, 1));
}