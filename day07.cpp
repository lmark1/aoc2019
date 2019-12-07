#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <unordered_set>
#include <map>
#include <algorithm>
#include <iterator>
#include <functional>
#include <math.h>
#include <array>

#define HALT            99
#define ADD             1
#define MULT            2
#define INPUT           3
#define OUTPUT          4
#define JUMP_IF_TRUE    5
#define JUMP_IF_FALSE   6
#define LESS_THAN       7
#define EQUALS          8

#define POSITION_MODE   0
#define IMMEDIATE_MODE  1

class Amplifier
{
public:
    Amplifier(std::vector<int> intCode, std::map < int, std::function<int(int, int)> > map, int index)
        :_intCode(intCode), _intCodeOrig(intCode), _map(map), _index(index) { }
        
    void reset()
    {
        _intCode = std::vector<int> (_intCodeOrig);
        _intructionPointer = 0;
        _halted = false;
        _lastOutput = -1;
    }

    int amplify(int ampInput, int ampPhase)
    {
        return amplify_internal(ampInput, ampPhase, _intCode, false, true);
    }

    int amplifySingle(int ampInput)
    {
        return amplify_internal(ampInput, -1, _intCode, true, true);
    }

    int getLastOutput() { return _lastOutput; }

    bool isHalted() { return _halted; }
    
private:

    int amplify_internal(int ampInput, int ampPhase, std::vector<int> intCode, bool phaseSet = false, bool returnOutput = false)
    {
        int output = -1;
        bool outputSet = false;
        //std::printf ("Amp input %d, Amp phase %d\n", ampInput, ampPhase);
        while (_intructionPointer < _intCode.size())
        {
            int paramMode3 = _intCode[_intructionPointer] / 10000,
                paramMode2 = (_intCode[_intructionPointer] % 10000) / 1000,
                paramMode1 = (_intCode[_intructionPointer] % 1000 ) / 100,
                opCode = _intCode[_intructionPointer] % 100;

            // assume index is at operation code
            if (opCode == HALT)
            {
                _halted = true;
                // std::printf("Amplifier %d halted.", _index);
                break;
            }

            if (opCode == INPUT)
            {
                int input = phaseSet ? ampInput : ampPhase;
                phaseSet = true;
                // std::cout << "Current input is: " << input << std::endl;
                if (paramMode1 == POSITION_MODE) _intCode[ _intCode[_intructionPointer+1] ] = input;
                else _intCode[_intructionPointer+1] = input;
                _intructionPointer += 2;
            }
            else if (opCode == OUTPUT)
            {
                if (paramMode1 == POSITION_MODE) output = _intCode[ _intCode[_intructionPointer+1] ];
                else output = _intCode[_intructionPointer+1];
                // std::cout << "DIAGNOSTICS output: " << output << std::endl;
                _intructionPointer += 2;
                outputSet = true;

                if (returnOutput)
                    break;
            }
            else if (opCode == JUMP_IF_TRUE || opCode == JUMP_IF_FALSE)
            {
                int firstArg = paramMode1 == POSITION_MODE ? _intCode[ _intCode[_intructionPointer+1] ] : _intCode[_intructionPointer+1];
                int secondArg = paramMode2 == POSITION_MODE ? _intCode[ _intCode[_intructionPointer+2] ] : _intCode[_intructionPointer+2];
                
                if (opCode == JUMP_IF_TRUE && firstArg != 0 ||
                    opCode == JUMP_IF_FALSE && firstArg == 0)
                    _intructionPointer = secondArg;
                else
                    _intructionPointer += 3;
            }
            else
            {
                int firstArgIndex = paramMode1 == POSITION_MODE ? _intCode[_intructionPointer+1] : _intructionPointer+1;
                int secondArgIndex = paramMode2 == POSITION_MODE ? _intCode[_intructionPointer+2] : _intructionPointer+2;
                int resultIndex = paramMode3 == POSITION_MODE ? _intCode[_intructionPointer+3] : _intructionPointer+3;

                //std::printf("Indices [%d, %d, %d]\n", firstArgIndex, secondArgIndex, resultIndex);
                // Do the operation
                _intCode[ resultIndex ] = _map[opCode] (
                    _intCode[ firstArgIndex ], _intCode[ secondArgIndex ]);
                _intructionPointer += 4;
            }
        }

        if (outputSet)
            _lastOutput = output;
        return output;
    }

    int _index = -1;
    int _lastOutput = -1;
    bool _halted = false;
    int _intructionPointer = 0;
    std::vector<int> _intCode;
    const std::vector<int> _intCodeOrig;
    std::map < int, std::function<int(int, int)> > _map;
};

int part1(std::vector<int> totalInput, std::vector<Amplifier>& amps)
{
    int i = 0;
    int ampInput = 0;
    while (i < 5)
    {
        ampInput = amps[i].amplify(ampInput, totalInput[i]);
        i++;
    }
    return ampInput;
}

int part2(std::vector<int> phaseInput, std::vector<Amplifier>& amps)
{
    int i = 0;
    int ampInput = 0;
    bool firstPass = true;
    bool allHalted = true;
    do {
        allHalted = true;
        i = 0;
        while (i < 5)
        {
            if (firstPass) ampInput = amps[i].amplify(ampInput, phaseInput[i]);
            else ampInput = amps[i].amplifySingle(ampInput);
            allHalted = allHalted && amps.at(i).isHalted();
            // std::cout << std::endl;
            i++;
        }
        firstPass = false;
    } while (!allHalted);

    return amps.at(4).getLastOutput();
}

int calcMax(std::vector<Amplifier>& amps)
{
    int max = 0;
    std::array<int, 5> perm = {0, 1, 2, 3, 4};
    do {
        max = std::max(max, part1(std::vector<int> {
                perm[0], perm[1], perm[2], perm[3], perm[4]}, amps));
        for (auto& amp : amps)
            amp.reset();
    } while (std::next_permutation(perm.begin(), perm.end()));
    return max;
}

int calcMaxFeedBack(std::vector<Amplifier>& amps)
{
    std::array<int, 5> perm = {5, 6, 7, 8, 9};
    int max = 0;
    do {
        max = std::max(max, part2(std::vector<int> {
                perm[0], perm[1], perm[2], perm[3], perm[4]}, amps));
        for (auto& amp : amps)
            amp.reset();
    } while (std::next_permutation(perm.begin(), perm.end()));
    return max;
}

int main () 
{
    std::fstream inputFile("day07.txt");
    std::string line, token;
    getline(inputFile, line);

    std::vector<int> intCode;
    std::stringstream ss(line);
    while (getline(ss, token, ','))
        intCode.push_back(std::stoi(token));

    std::map < int, std::function<int(int, int)> > operationsMap;
    operationsMap.emplace(ADD, [](int a, int b) { return a+b; } );
    operationsMap.emplace(MULT, [](int a, int b) { return a*b; } );
    operationsMap.emplace(LESS_THAN, [](int a, int b) { return a < b ? 1 : 0; } );
    operationsMap.emplace(EQUALS, [](int a, int b) { return a == b ? 1 : 0; } );

    std::vector<Amplifier> amps;
    amps.push_back(Amplifier(intCode, operationsMap, 1));
    amps.push_back(Amplifier(intCode, operationsMap, 2));
    amps.push_back(Amplifier(intCode, operationsMap, 3));
    amps.push_back(Amplifier(intCode, operationsMap, 4));
    amps.push_back(Amplifier(intCode, operationsMap, 5));
    
    std::cout << "Max thruster:\n" << calcMax(amps) << std::endl;
    std::cout << "Max thruster feedback:\n" << calcMaxFeedBack(amps) << std::endl;
    return 0; 
}