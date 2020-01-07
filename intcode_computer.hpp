#ifndef INTCODE_COMPUTER_HPP
#define INTCODE_COMPUTER_HPP

#include <vector>
#include <map>
#include <iostream>
#include <functional>
#include "my_macros.hpp"
#include <fstream>
#include <sstream>

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

    IntcodeComputer(std::fstream&& intCodeFileStream) :
        IntcodeComputer([&intCodeFileStream]() 
        {
            std::vector<long long> intCode;
            std::string line, token;
            getline(intCodeFileStream, line);
            std::stringstream ss(line);
            while (getline(ss, token, ',')) intCode.push_back(std::stoll(token));
            return intCode;
        }())
    { }

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
    int i = 0;
        _intCode = std::vector<long long> (_intCodeOrig);
        _instructionPointer = 0;
        _halted = false;
        _lastOutput = -1;
        _relativeBase = 0;
    }

    void setVerbosity(bool value) { _verbose = value; }
    int calculate()
    {
        return calculate_internal(-1, _intCode, false, true);
    }

    int calculateSingle()
    {
        return calculate_internal(-1, _intCode, true, true);
    }

    int calculateSingle(int input)
    {
        return calculate_internal(input, _intCode, true, false);
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
                LOG_COND(_verbose, "Computer halted\n");
                break;
            }

            if (opCode == INPUT)
            {   
                if (takeUserInput)
                {
                    LOG("Please input number: ");
                    std::cin >> input;
                }

                LOG_COND(_verbose, "Current input is: " << input << std::endl);
                setMemoryVal(  getArgIndex(paramMode1, _instructionPointer + 1), input);
                _instructionPointer += 2;
            }
            else if (opCode == OUTPUT)
            {
                output = getMemoryVal( getArgIndex(paramMode1, _instructionPointer + 1) );
                LOG_COND(_verbose, "DIAGNOSTICS output: " << output << std::endl);
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
    bool _verbose = true;
    std::vector<long long> _intCode;
    const std::vector<long long> _intCodeOrig;
    std::map < long long, std::function<long long(long long, long long)> > _operationsMap;
};

#endif /* INTCODE_COMPUTER_HPP */