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
    int oreAmount;

    OreCompund(std::string _oreType, int _oreAmount):
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
    int                     /* amount */>   quantityMap_t;

bool isBaseReaction(reactionMap_t::iterator& it, reactionMap_t& chemicalReactionMap)
{
    return it != chemicalReactionMap.end() &&
        it->second.size() == 1 &&
        it->second[0].oreType == "ORE";
}

void calculateQuantities(std::string rootChemical, int rootAmount, reactionMap_t& chemicalReactionMap, 
    quantityMap_t& resultQuantityMap, quantityMap_t& leftoverQMap, bool overrideMul = false)
{
    auto reactionIt = chemicalReactionMap.find(OreCompund(rootChemical, -1));
    int multiplier = std::ceil(1.0 * rootAmount / reactionIt->first.oreAmount);

    // If there is no reaction, assume chamical == ORE
    if (isBaseReaction(reactionIt, chemicalReactionMap))    
    {
        auto quantityIt = resultQuantityMap.emplace(rootChemical, rootAmount);
        if (!quantityIt.second) resultQuantityMap[rootChemical] += rootAmount;
        return;
    }

    for (auto& reagent : reactionIt->second)
    {
        int consumeAmount = multiplier * reagent.oreAmount;

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

        //std::printf("Consume %d %s.\n", consumeAmount, reagent.oreType.c_str());
        calculateQuantities(reagent.oreType, consumeAmount, chemicalReactionMap, resultQuantityMap, leftoverQMap);

        auto reagentIt = chemicalReactionMap.find(reagent);
        if (!isBaseReaction(reagentIt, chemicalReactionMap))
        {
            // Add amount to leftovers
            int actualAmount = std::ceil(1.0 * consumeAmount / reagentIt->first.oreAmount) * reagentIt->first.oreAmount;
            int leftoverAmount = actualAmount - consumeAmount;
            auto empSuccess = leftoverQMap.emplace(reagent.oreType, leftoverAmount);
            if (!empSuccess.second) leftoverQMap[reagent.oreType] += leftoverAmount;
        }
    }
}

int getOreAmount(std::string root, int q, reactionMap_t& chemicalReactionMap)
{
    quantityMap_t 
        resultQuantityMap,      // How many checmical that can be transferred to ORE are needed for reaction ?
        leftoverQMap;           // How many leftover quantities are made.

    calculateQuantities(root, q, chemicalReactionMap, resultQuantityMap, leftoverQMap);
    int totalOre = 0;
    for (auto& chemicalPair : resultQuantityMap)
    {
        auto reactionIt = chemicalReactionMap.find(OreCompund(chemicalPair.first, -1));
        int oreAmount = 
            std::ceil(1.0 * chemicalPair.second / reactionIt->first.oreAmount) * 
            reactionIt->second[0].oreAmount;
        std::printf("CONSUME %d ORE to produce %d %s\n", oreAmount, 
            chemicalPair.second, chemicalPair.first.c_str());
        totalOre += oreAmount;
    }  
    return totalOre;
}

int main  ()
{
    std::fstream inputFile("day14.txt");
    std::string line, token;
    reactionMap_t chemicalReactionMap;
    while(getline(inputFile, line))
    {
        std::vector<std::string> tokens;
        boost::split(tokens, line, boost::is_any_of("[=>], "), boost::algorithm::token_compress_on);
        
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
    int oreNeeded = getOreAmount("FUEL", 1, chemicalReactionMap);
    std::cout << "Total ORE needed: " << oreNeeded << std::endl;
    return 0;
}