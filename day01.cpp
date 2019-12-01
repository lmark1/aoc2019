#include <iostream>
#include <fstream>
#include <string>
#include <math.h>

int part1(std::ifstream&);
int part2(std::ifstream&);
int main()
{
    std::ifstream inputFile("day01.txt");
    std::cout << "Total fuel(part1) is: " << part1(inputFile) << std::endl;

    inputFile.clear();
    inputFile.seekg (0, inputFile.beg);
    std::cout << "Total fuel(part2) is: " << part2(inputFile) << std::endl;
    return 0;
}

int part1(std::ifstream& inputFile)
{
    std::string numberStr;
    int totalFuel =  0;
    while (getline(inputFile, numberStr))
    {
        int number = std::stoi(numberStr);
        totalFuel += floor(number/3.0) - 2;
    }
    return totalFuel;
}

int part2(std::ifstream& inputFile)
{
    std::string numberStr;
    int totalFuel =  0;
    while (getline(inputFile, numberStr))
    {
        int number = std::stoi(numberStr);
        while (number > 0)
        {
            number = std::max(floor(number/3.0) - 2, 0.0);
            totalFuel += number;
        }
    }
    return totalFuel;
}