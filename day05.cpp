#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <map>
#include <algorithm>
#include <iterator>
#include <functional>

#define HALT    99
#define ADD     1
#define MULT    2
#define INPUT   3
#define OUTPUT  4
#define JUMP_IF_TRUE 5
#define JUMP_IF_FALSE 6
#define LESS_THAN 7
#define EQUALS 8

#define POSITION_MODE 0
#define IMMEDIATE_MODE 1

void part1(std::map < int, std::function<int(int, int)> >& map, std::vector<int> intCode)
{
    int i = 0;
    while (i < intCode.size())
    {
        int paramMode3 = intCode[i] / 10000,
            paramMode2 = (intCode[i] % 10000) / 1000,
            paramMode1 = (intCode[i] % 1000 ) / 100,
            opCode = intCode[i] % 100;

        // assume index is at operation code
        if (opCode == HALT)
            break;

        if (opCode == INPUT)
        {
            int input;
            std::cout << "Please give an input: ";
            std::cin >> input;
            std::cout << std::endl;
            if (paramMode1 == POSITION_MODE) intCode[ intCode[i+1] ] = input;
            else intCode[i+1] = input;
            i += 2;
        }
        else if (opCode == OUTPUT)
        {
            int output;
            if (paramMode1 == POSITION_MODE) output = intCode[ intCode[i+1] ];
            else output = intCode[i+1];
            std::cout << "DIAGNOSTICS output: " << output << std::endl;
            i += 2;
        }
        else if (opCode == JUMP_IF_TRUE || opCode == JUMP_IF_FALSE)
        {
            int firstArg = paramMode1 == POSITION_MODE ? intCode[ intCode[i+1] ] : intCode[i+1];
            int secondArg = paramMode2 == POSITION_MODE ? intCode[ intCode[i+2] ] : intCode[i+2];
            
            if (opCode == JUMP_IF_TRUE && firstArg != 0 ||
                opCode == JUMP_IF_FALSE && firstArg == 0)
                i = secondArg;
            else
                i += 3;
        }
        else
        {
            int firstArgIndex = paramMode1 == POSITION_MODE ? intCode[i+1] : i+1;
            int secondArgIndex = paramMode2 == POSITION_MODE ? intCode[i+2] : i+2;
            int resultIndex = paramMode3 == POSITION_MODE ? intCode[i+3] : i+3;

            //std::printf("Indices [%d, %d, %d]\n", firstArgIndex, secondArgIndex, resultIndex);
            // Do the operation
            intCode[ resultIndex ] = map[opCode] (
                intCode[ firstArgIndex ], intCode[ secondArgIndex ]);
            i += 4;
        }
    }
}

int main ()
{
    std::fstream inputFile("day05.txt");
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

    part1(operationsMap, intCode);
    return 0;
}