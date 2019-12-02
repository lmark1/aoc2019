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

int part1(int noun, int verb,  std::map < int, std::function<int(int, int)> >& map, std::vector<int> intCode)
{
    // initialize opcodes
    intCode[1] = noun;
    intCode[2] = verb;

    int i = 0;
    while (i < intCode.size())
    {
        // assume index is at operation code
        if (intCode[i] == HALT)
            break;

        // Do the operation
        intCode[ intCode[i+3] ] = map[intCode[i]] (
                intCode[ intCode[i+1] ], 
                intCode[ intCode[i+2] ]);
        i += 4;
    }

    return intCode[0];
}

std::pair<int, int> part2(std::map < int, std::function<int(int, int)> >& map, std::vector<int> intCode)
{
    for (int i = 0; i <= 99; i++)
        for (int j = 0; j <=99; j++)
            if (part1(i, j, map, intCode) == 19690720)
                return std::pair<int, int> (i, j);

    return std::pair<int, int> (-1, -1);
}

int main ()
{
    std::fstream inputFile("day02.txt");
    std::string line, token;
    getline(inputFile, line);

    std::vector<int> intCode;
    std::stringstream ss(line);
    while (getline(ss, token, ','))
        intCode.push_back(std::stoi(token));

    std::map < int, std::function<int(int, int)> > operationsMap;
    operationsMap.emplace(ADD, [](int a, int b) { return a+b; } );
    operationsMap.emplace(MULT, [](int a, int b) { return a*b; } );
    
    std::cout << "Position 0 (part1) is: " << part1(12, 2, operationsMap, intCode) << std::endl;
    auto sol = part2(operationsMap, intCode);
    std::printf("Part2 solution is %d\n", 100 * sol.first + sol.second);
    return 0;
}