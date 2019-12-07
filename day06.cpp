#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <iterator>

typedef std::unordered_map<std::string, std::string>::const_iterator map_it;
typedef std::unordered_map<std::string, std::string> myMap;

int part1 ( std::unordered_map<std::string, std::string>& orbitMap)
{
    auto it = orbitMap.begin();
    int indirectOrbit = 0, directOrbit = orbitMap.size();
    while (it != orbitMap.end())
    {
        std::string orbitee = it->second;
        while (true)
        {
            auto newIt = orbitMap.find(orbitee);
            if (newIt == orbitMap.end())
                break;
            indirectOrbit++;
            orbitee = newIt->second;
        }
        it++;
    }
    return indirectOrbit + directOrbit;
}

int part2 (std::string prev, std::string curr, myMap& map)
{
    if (curr.compare("COM") == 0) return map.size();
    if (curr.compare("SAN") == 0) return 0;
    
    int minPath = map.size();
    // Check min path of the orbitee first
    auto newIt = map.find(curr);
    if (newIt != map.end() && 
        newIt->second.compare(prev) != 0) // Dont go back
        minPath = std::min(minPath, 1 + part2(curr, newIt->second, map));

    // Check for its orbiters
    auto check_value = [&curr](std::pair<std::string, std::string> const& p)->bool 
        { return p.second.compare(curr) == 0; };
    auto it = find_if(map.begin(), map.end(), check_value);
    while (it != map.end())
    {
        if (it->first.compare(prev) == 0) // Don't go back, Jack
        {
            it = find_if(std::next(it), map.end(), check_value);
            continue;
        }

        minPath = std::min(minPath, 1 + part2(curr, it->first, map));
        it = find_if(std::next(it), map.end(), check_value);
    }  
    
    return minPath;
}

int main () 
{
    std::fstream inputFile("day06.txt");
    std::string line, token;
    
    std::unordered_map<
        std::string /* Orbiter */, 
        std::string /* Orbitee */> orbitMap;

    while (getline(inputFile, line))
    {
        std::replace(line.begin(), line.end(), ')', ' ');
        std::stringstream ss(line);

        std::string orbiter, orbitee;
        ss >> orbitee;
        ss >> orbiter;

        //std::printf ("Orbiter %s -> Orbitee %s \n", orbiter.c_str(), orbitee.c_str());
        orbitMap.emplace(orbiter,  orbitee);
    }

    std::cout << "Direct + Indirect orbit is: " << part1(orbitMap) << std::endl;
    std::cout << "Min orbit transfer: " << part2("YOU", orbitMap["YOU"], orbitMap) - 1 << std::endl;
    return 0;
}