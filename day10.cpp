#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <map>
#include <algorithm>
#include <math.h>
#include <set>

#include "my_macros.hpp"


template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

std::pair<int, int> getBestAsteroid(std::map< std::pair<int, int> , char > asteroidMap)
    {
    int maxAst = 0;
    std::pair<int, int> maxPairVals;
    for (auto asteroid : asteroidMap)
    {   
        std::set<double> degSet;
        for (auto otherAsteroid : asteroidMap)
        {
            // Dont count itself
            if (asteroid.first == otherAsteroid.first)
                continue;
            degSet.emplace(std::atan2(otherAsteroid.first.second - asteroid.first.second, otherAsteroid.first.first - asteroid.first.first));
        }

        if (degSet.size() > maxAst)
        {
            maxAst = degSet.size();
            maxPairVals = asteroid.first;
        }
    }

    LOG("New best ast count: " << maxAst); LOG(" at index: "); LOG_PAIR(maxPairVals); LOG_ENDL;
    return maxPairVals;
}

int main() 
{
    std::fstream inputFile("day10.txt");
    std::string line, token;
    std::map< std::pair<int, int> , char > asteroidMap;
    int rows = 0, cols = 0;
    while (getline(inputFile, line))
    {
        cols = 0;
        std::for_each(line.begin(), line.end(), [&](char c) 
            { if (c == '#') asteroidMap.emplace(std::make_pair(cols, rows), c); cols++;});
        rows--;
    }
    
    int eraseCounter = 0;
    std::pair<int, int> bestAsteroid = getBestAsteroid(asteroidMap);
    while (eraseCounter < 200)
    {
        std::map<double, std::pair<int, int>> degreeMap;
        for (auto& asteroid : asteroidMap)
        {
            if (asteroid.first == bestAsteroid)
                continue;
            
            double angle = std::atan2(asteroid.first.second - bestAsteroid.second, asteroid.first.first - bestAsteroid.first);
            auto it = degreeMap.find(angle);
            
            // Just insert if not in map
            if (it == degreeMap.end())
            {
                degreeMap.emplace(angle, asteroid.first);
                continue;
            }

            double dist_current = sqrt( pow(asteroid.first.first - bestAsteroid.first,2) + pow(asteroid.first.second - bestAsteroid.second, 2));
            double dist_old = sqrt(pow(it->second.first - bestAsteroid.first , 2) + pow(it->second.second - bestAsteroid.second  , 2));

            // If closer
            if (dist_current < dist_old)
            {
                it->second = asteroid.first;
            }
        }

        if (degreeMap.empty())
            break;

        const auto iter_c = std::find_if(degreeMap.rbegin(), degreeMap.rend(), [](auto item)
        {   
            //LOG("Checking for: "); LOG_PAIR(item.second); LOG(item.first); LOG_ENDL;
            return fabs(item.first - 1.5707) < 1e-3;
        });
        LOG_ENDL;
        auto iter(iter_c);
        do
        {
            asteroidMap.erase(iter->second);
            eraseCounter++;
            std::printf("Angle: %.4f, %d-th erase is: ", iter->first, eraseCounter); LOG_PAIR(iter->second); LOG_ENDL;
            iter++;
            if (iter == degreeMap.rend())
                iter = degreeMap.rbegin();
        } while (iter->first != iter_c->first);
        
        LOG_ENDL;
        degreeMap.clear();
    }
}