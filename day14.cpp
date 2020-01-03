#include <boost/algorithm/string.hpp> 
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <cmath>

struct OreCompund
{
    std::string oreType;
    long long oreAmount;

    OreCompund(std::string _oreType, long long _oreAmount):
        oreType(_oreType), oreAmount(_oreAmount) 
    {}

    friend std::ostream& operator << (std::ostream& os, const OreCompund& ore)
    {
        os << "[ " << ore.oreType << ", " << ore.oreAmount << " ]";
        return os;
    }

    friend bool operator < (const OreCompund& or1, const OreCompund& or2)
    {
        std::hash<std::string> hasher;
        return hasher(or1.oreType) < hasher(or2.oreType);
    }
    
    friend bool operator == (const OreCompund& or1, const OreCompund& or2)
    {
        std::hash<std::string> hasher;
        return hasher(or1.oreType) == hasher(or2.oreType);
    }
};

typedef std::map<
    OreCompund              /* result */, 
    std::vector<OreCompund> /* reaction inputs */ > reactionMap_t;

typedef std::unordered_map<
    std::string             /* result */,
    long long               /* amount */>   quantityMap_t;

reactionMap_t chemicalReactionMap;
const long long MAX_ORE = 1e12;
const std::string FUEL_L("FUEL");

bool isBaseReaction(reactionMap_t::iterator& it)
{
    return it != chemicalReactionMap.end() &&
        it->second.size() == 1 &&
        it->second[0].oreType == "ORE";
}

void calculateQuantities(std::string rootChemical, long long rootAmount, 
    quantityMap_t& resultQuantityMap, quantityMap_t& leftoverQMap)
{
    auto reactionIt = chemicalReactionMap.find(OreCompund(rootChemical, -1));
    long long multiplier = std::ceil(1.0 * rootAmount / reactionIt->first.oreAmount);

    // If there is no reaction, assume chamical == ORE
    if (isBaseReaction(reactionIt))    
    {
        auto quantityIt = resultQuantityMap.emplace(rootChemical, rootAmount);
        if (!quantityIt.second) resultQuantityMap[rootChemical] += rootAmount;
        return;
    }

    for (auto& reagent : reactionIt->second)
    {
        long long consumeAmount = multiplier * reagent.oreAmount;

        auto leftoverIt = leftoverQMap.find(reagent.oreType);
        if (leftoverIt != leftoverQMap.end())
        {
            if (leftoverIt->second >= consumeAmount)
            {
                leftoverQMap[reagent.oreType] -= consumeAmount;
                consumeAmount = 0;
            }
            else
            {
                consumeAmount -= leftoverQMap[reagent.oreType];
                leftoverQMap[reagent.oreType] = 0;
            }
        }

        if (consumeAmount == 0)
            continue;

        //std::printf("Consume %ld %s.\n", consumeAmount, reagent.oreType.c_str());
        calculateQuantities(reagent.oreType, consumeAmount, resultQuantityMap, leftoverQMap);
        auto reagentIt = chemicalReactionMap.find(reagent);
        if (!isBaseReaction(reagentIt))
        {
            // Add amount to leftovers
            long long actualAmount = std::ceil(1.0 * consumeAmount / reagentIt->first.oreAmount) * reagentIt->first.oreAmount;
            long long leftoverAmount = actualAmount - consumeAmount;
            auto empSuccess = leftoverQMap.emplace(reagent.oreType, leftoverAmount);
            if (!empSuccess.second) leftoverQMap[reagent.oreType] += leftoverAmount;
        }
    }
}

long long getOreAmount(std::string root, long long q)
{
    quantityMap_t 
        resultQuantityMap,      // How many checmical that can be transferred to ORE are needed for reaction ?
        leftoverQMap;           // How many leftover quantities are made.

    calculateQuantities(root, q, resultQuantityMap, leftoverQMap);
    long long totalOre = 0;
    for (auto& chemicalPair : resultQuantityMap)
    {
        auto reactionIt = chemicalReactionMap.find(OreCompund(chemicalPair.first, -1));
        long long oreAmount = 
            std::ceil(1.0 * chemicalPair.second / reactionIt->first.oreAmount) * 
            reactionIt->second[0].oreAmount;
        //std::printf("CONSUME %ld ORE to produce %ld %s\n", oreAmount, 
        //    chemicalPair.second, chemicalPair.first.c_str());
        totalOre += oreAmount;
    }  
    return totalOre;
}

long long getMaxFuel()
{
    // Initial bounds are not the solution
    long long lowerBoundFuel = 1;

    // Calculate the upper bound
    long long upperBoundFuel = 1;
    long long tempOre = 1;
    while (tempOre <= MAX_ORE)
    {
        upperBoundFuel *= 10;
        tempOre = getOreAmount(FUEL_L, upperBoundFuel);
    }

    while (upperBoundFuel >= lowerBoundFuel)
    {
        long long medFuel = (upperBoundFuel + lowerBoundFuel) / 2;
        long long medOre = getOreAmount(FUEL_L, medFuel);
        if (medFuel == upperBoundFuel || medFuel == lowerBoundFuel)
            break;
        if (medOre >= MAX_ORE)
            upperBoundFuel = medFuel;
        else
            lowerBoundFuel = medFuel;
    } 
    return lowerBoundFuel;
}

int main  ()
{
    std::fstream inputFile("day14.txt");
    std::string line, token;
    while(getline(inputFile, line))
    {
        std::vector<std::string> tokens;
        boost::split(tokens, line, boost::is_any_of("[=>], "), 
            boost::algorithm::token_compress_on);
        
        OreCompund reactionResult (
            tokens[tokens.size()-1], 
            std::stoi(tokens[tokens.size()-2])
        );
        std::vector<OreCompund> reactionInput;
        for (int i = 0; i < tokens.size() - 3; i+=2)
            reactionInput.push_back(
                OreCompund(tokens[i+1], std::stoi(tokens[i]))
            );
        chemicalReactionMap.emplace(reactionResult, reactionInput);
    }  
    std::cout << "Total ORE needed: " << getOreAmount(FUEL_L, 1) << std::endl;
    std::cout << "Fuel for ORE 1e12 needed: " << getMaxFuel() << std::endl;
    return 0;
}