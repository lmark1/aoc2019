#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "my_macros.hpp"

#define WIDTH 25
#define HEIGHT 6
#define LOG_VEC_PIC(vec) for (int i = 0; i < HEIGHT; i++) \
                            { std::cout << std::endl; \
                            for (int j = 0; j < WIDTH; j++) \
                                { if (vec[i*WIDTH + j]) std::cout << vec[i*WIDTH + j]; else std::cout << " "; } }

#define TRANSPARENT 2
#define BLACK 0
#define WHITE 1

std::vector<int> decode (std::vector<std::vector<int>>& imageVec)
{
    std::vector<int> decodedImage;
    for (int i = 0; i < WIDTH * HEIGHT; i++)
    {
        for (auto& image : imageVec)
        {
            if (image[i] == TRANSPARENT)
                continue;

            decodedImage.push_back(image[i]);   
            break;
        }
    }
    return decodedImage;
}   

int main () 
{   
    std::fstream inputFile("day08.txt");
    std::string line;
    getline(inputFile, line);

    std::vector<std::vector<int>> imageVec; 
    std::vector<int> image;
    int indexStr = 0, indexImage = 0, zeroesIndex = -1, zeroesCount = 0, minZeroesCount = WIDTH * HEIGHT;
    while(indexStr < line.size() + 1)
    {
        if (indexImage == WIDTH * HEIGHT)
        {
            //LOG_VEC(image);
            imageVec.push_back(std::vector<int> (image));
            image.clear();
            indexImage = 0;

            if (minZeroesCount > zeroesCount)
            {
                minZeroesCount = zeroesCount;
                zeroesIndex = imageVec.size() - 1;
                //LOG("Min zeroes " << minZeroesCount << " at index " << zeroesIndex);
            }
            zeroesCount = 0;

            if (indexStr == line.size())
                break;
        }

        int token_int = line.at(indexStr) - '0';
        image.push_back(token_int);
        indexImage ++;
        indexStr ++; 

        // count the zeroes
        if (token_int == 0)
            zeroesCount++;
    }

    int oneDigits = 0 , twoDigits = 0;
    for (auto number : imageVec[zeroesIndex])
    {
        if (number == 1)
            oneDigits ++;
        
        if (number == 2)
            twoDigits ++;
    }  

    LOG("P1 sol: " << oneDigits * twoDigits);
    LOG_VEC_PIC(decode(imageVec));
} 