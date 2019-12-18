#include <boost/algorithm/string.hpp> 
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <ostream>
#include <iomanip>
#include <chrono>

struct Planet
{
    int id;
    std::array<int, 3> freqVector {-1, -1, -1};
    std::vector<int> position;
    std::vector<int> velocity {0, 0, 0}; 

    bool frequenciesFound() { return freqVector[0] > 0 && freqVector[1] > 0 && freqVector[2] > 0; }
    bool operator== (const Planet& planet)
    {
        return this->position[0] == planet.position[0] &&
            this->position[1] == planet.position[1] &&
            this->position[2] == planet.position[2];
    }

    friend std::ostream& operator<< (std::ostream& o, const Planet& planet)
    {
        o   << "pos:<x= " << std::setw(3) << planet.position[0]
            << ", y= " << std::setw(3) << planet.position[1]
            << ", z= " << std::setw(3) << planet.position[2]
            << ">, vel=<x= " << std::setw(3) << planet.velocity[0]
            << ", y= " << std::setw(3) << planet.velocity[1]
            << ", z= " << std::setw(3) << planet.velocity[2] << ">" << std::endl;
    }
};

void updatePlanets(std::vector<Planet>& planets)
{
    // Update gravity
    for (auto& planet : planets) 
    for (auto& otherPlanet : planets)
    {
        if (planet.id == otherPlanet.id)
            continue;
        for (int i = 0; i < 3; i++)
            if (planet.position[i] != otherPlanet.position[i])
                planet.velocity[i] += planet.position[i] < otherPlanet.position[i] ? 1 : -1;
    }

    // Update positions
    for (auto& planet : planets)
        for (int i = 0; i < 3; i++)
            planet.position[i] += planet.velocity[i];
}

int calculateEnergy(std::vector<Planet>& planets)
{
    int energy = 0;
    for (auto& planet : planets)
    {
        int posSum = 0, velSum = 0;
        for (int i = 0; i < 3; i++)
        {
            posSum += abs(planet.position[i]);
            velSum += abs(planet.velocity[i]);
        }
        energy += posSum * velSum;
    }
    return energy;
}

long long greatestCommonDenominator(long long a, long long b)
{
    if (a == 0)
        return b;

    return greatestCommonDenominator(b % a, a);
}

long long leastCommonMultiple(long long a, long long b)
{
    return a * b / greatestCommonDenominator(a, b);
}

int main ()
{
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    std::fstream inputFile("day12.txt");
    std::string line, token;
    int id = 1;

    std::vector<Planet> planetList;
    while (getline(inputFile, line))
    {
        std::vector<std::string> tokens;
        boost::split(tokens, line, boost::is_any_of("<>,=xyz "));
        
        Planet newPlanet;
        newPlanet.id = id++;
        std::for_each(tokens.begin(), tokens.end(), [&newPlanet](std::string& token)
            { if (!token.empty()) newPlanet.position.push_back(std::stoi(token)); });
        planetList.push_back(newPlanet);
    }


    std::vector< std::vector<int> > sequenceVector;
    for (auto& planet : planetList) 
    {
        sequenceVector.push_back(std::vector<int>());
        sequenceVector.push_back(std::vector<int>());
        sequenceVector.push_back(std::vector<int>());
    }
    // std::vector<Planet> initPlanets(planetList);
    int iter = 0;
    bool done = false;
    while (iter < 1001 || !done)
    {   
        // Push back all planet positions to a sequenceList
        for (int i = 0; i < planetList.size(); i++)
            for (int j = 0; j < 3; j++)
                sequenceVector[i*3 + j].push_back(planetList[i].position[j]);

        // Try to find a repeating sequence
        for (int i = 0; i < sequenceVector.size(); i++)
        {
            int planetIndex = i % planetList.size();
            int frequencyIndex = i % 3;
            if (sequenceVector[i].size() % 2 != 0 ||  // Sequence is not even
                planetList[planetIndex].freqVector[frequencyIndex] > 0) // Sequence frequency is already established
                continue;
            
            // Check if a sequence repeats
            bool repeats = true;
            int halfLen = sequenceVector[i].size() / 2;
            for (int j = 0; j < halfLen; j++)
            {
                if (! (sequenceVector[i][j] == sequenceVector[i][halfLen + j]))
                {
                    repeats = false;
                    break;
                }
            }
            
            // If sequence repeats update the planets frequency vector
            if (repeats) planetList[planetIndex].freqVector[frequencyIndex] = halfLen;
        }

        // Break if all the frequencies are found
        done = true;
        for (auto& planet : planetList)
            done = done && planet.frequenciesFound();

        if (iter == 1000)
        {
            std::printf("After %d steps:\n", iter);
            for (auto& planet : planetList)
                std::cout << planet;
            std::printf("Sum of total energy: %d\n", calculateEnergy(planetList));
        }

        // Update all planet positions
        updatePlanets(planetList);
        iter ++;
    }

    std::printf("\nAfter %d steps:\n", iter);
    for (auto& p : planetList)
    {
        std::cout << p;
        std::printf("Frequencies: (%d, %d, %d)\n", 
            p.freqVector[0], p.freqVector[1], p.freqVector[2]);
    }

    long long result = 1;
    for (int i = 0; i < 3; i++)
        result = leastCommonMultiple(result, planetList[0].freqVector[i]);
    std::cout << "Steps before repeating itself: " << result << std::endl;

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds> (end - begin).count() << "[ms]" << std::endl;
}