#include "intcode_computer.hpp"
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <unistd.h>
#include <unordered_set>

// A hash function used to hash a pair of any kind 
struct hash_pair { 
    template <class T1, class T2> 
    size_t operator()(const std::pair<T1, T2>& p) const
    { 
        auto hash1 = std::hash<T1>{}(p.first); 
        auto hash2 = std::hash<T2>{}(p.second); 
        return hash1 ^ hash2; 
    } 
}; 

IntcodeComputer ic(std::fstream {"day15.txt"});
typedef std::unordered_map<
    std::pair<int, int>     /* Grid position */,  
    int                     /* Grid landmark */, 
    hash_pair> gridMap_t;
gridMap_t gridMap;
std::vector< std::vector<char> > matrixMap;
int startX = 0, startY = 0;
const std::string clear( 100, '\n' ) ;

typedef std::unordered_map<
    std::pair<int, int>,
    std::unordered_set<
        std::pair<int, int>, hash_pair>,
    hash_pair>  connectivityMap_t;
connectivityMap_t connectivityMap;

enum TileValues
{
    WALL = 0, 
    FREE = 1,
    OXY = 2
};
std::unordered_map<int, char> tileMap =
    {
      { TileValues::WALL, '#' },
      { TileValues::FREE, '.' },
      { TileValues::OXY,  'O' } 
    };

enum Direction
{
    NORTH = 1,
    SOUTH = 2,
    WEST = 3,
    EAST = 4
};

Direction getOppositeDirection(int dir)
{
    if (dir == Direction::NORTH) return Direction::SOUTH;
    if (dir == Direction::SOUTH) return Direction::NORTH;
    if (dir == Direction::EAST) return Direction::WEST;
    if (dir == Direction::WEST) return Direction::EAST;
}

std::pair<int, int> movePosition(const std::pair<int, int>& currentPosition, int direction)
{
    int xOff = 0, yOff = 0;
    switch (direction)
    {
    case Direction::NORTH:
        yOff++;
        break;
    
    case Direction::EAST:
        xOff++;
        break;

    case Direction::SOUTH:
        yOff--;
        break;

    case Direction::WEST:
        xOff--;
        break;

    default:
        throw std::runtime_error("Undefined movement direction");
        break;
    }
    return std::make_pair(currentPosition.first + xOff, currentPosition.second + yOff);
}

void updateMatrixMap(const std::pair<int, int>& pos, int tileValue)
{
    while (startY > pos.second) 
    {
        startY--;
        matrixMap.insert(matrixMap.begin(), std::vector<char>{tileMap[TileValues::WALL]});
    }

    int mapCoordY = abs(startY) + pos.second;
    while(matrixMap.size() - 1 < mapCoordY)
        matrixMap.push_back( std::vector<char> { tileMap[TileValues::WALL] } );

    if (startX > pos.first) 
    {
        startX--;
        for (auto& line : matrixMap)
            line.insert(line.begin(), tileMap[TileValues::WALL]);
    }

    int mapCoordX = abs(startX) + pos.first;
    while(matrixMap[mapCoordY].size() - 1 < mapCoordX)
        matrixMap[mapCoordY].push_back( tileMap[TileValues::WALL] );
    
    matrixMap[mapCoordY][mapCoordX] = tileMap[tileValue];
}

void printMatrixMap(const std::pair<int, int>& pos, bool enableDroid = true)
{
    std::cout << clear;
    int mapCoordY = abs(startY) + pos.second;
    int mapCoordX = abs(startX) + pos.first;
    int temp = matrixMap[mapCoordY][mapCoordX];
    if (enableDroid) matrixMap[mapCoordY][mapCoordX] = 'D';
    for (auto& line : matrixMap)
    {
        for (auto& el : line)
            std::cout << el;
        std::cout << std::endl;
    }
    std::cout << std::endl;
    matrixMap[mapCoordY][mapCoordX] = temp;
    usleep(5000);
}

int getNewMovement(const std::pair<int, int>& pos, const int& prevMovement)
{
    // Go to a new position that is not yet discovered
    for (int movementDir = Direction::NORTH; movementDir <= 4; movementDir++)
    {
        auto newPos = movePosition(pos, movementDir);
        if (gridMap.find(newPos) == gridMap.end()) return movementDir;
    }

    // If all are discovered return same movement direction if that direction is free
    // if (gridMap.find(movePosition(pos, prevMovement))->second != TileValues::WALL) return prevMovement;

    // Return any free direction as long as its not opposite to previous
    std::vector<int> dirs;
    for (int movementDir = Direction::NORTH; movementDir <= 4; movementDir++)
    {
        auto newPos = movePosition(pos, movementDir);
        auto it = gridMap.find(newPos);
        if (it->second != TileValues::WALL &&
            getOppositeDirection(prevMovement) != movementDir) 
                dirs.push_back(movementDir);
    }
    if (!dirs.empty()) return dirs[rand() % dirs.size()];


    // Return any other movement direction that contains a free block
    dirs.clear();
    for (int movementDir = Direction::NORTH; movementDir <= 4; movementDir++)
    {
        auto newPos = movePosition(pos, movementDir);
        auto it = gridMap.find(newPos);
        if (it->second != TileValues::WALL) dirs.push_back(movementDir);
    }
    return dirs[rand() % dirs.size()];    
}

const std::pair<int, int> getOxygenPosition()
{
    std::pair<int, int> currentPosition {0, 0},
        oxyPosition {0, 0};
    gridMap.emplace(currentPosition, TileValues::FREE);
    matrixMap.push_back( std::vector<char> {tileMap[TileValues::FREE]} );

    int movementDir = Direction::EAST;
    char currentTileValue = TileValues::FREE;
    int movesUntilNewArea = 0;
    bool foundOxy = false;
    while (movesUntilNewArea < 500 || !foundOxy) // currentTileValue != TileValues::OXY)
    {  
        if (currentTileValue == TileValues::OXY)
        {
            foundOxy = true;
            oxyPosition = currentPosition;
        }

        //std::printf("Dir: %d [%d, %d] -> %d\n", movementDir, currentPosition.first, currentPosition.second, currentTileValue);
        int newTileValue = ic.calculateSingle(movementDir);
        if (ic.isHalted()) break;
        auto newPos = movePosition(currentPosition, movementDir);
        updateMatrixMap(newPos, newTileValue);

        // Insert new grid position
        auto success = gridMap.emplace(newPos, newTileValue);
        // If insertion is not successful, this means we have already seen this area
        if (!success.second)
            movesUntilNewArea++;
        else
            movesUntilNewArea = 0;
        
        //std::printf("Found: [%d, %d] -> %d\n", newPos.first, newPos.second, newTileValue);
        printMatrixMap(currentPosition);
        
        // Check if position is valid
        if (!success.second && gridMap[newPos] != newTileValue)
            throw std::runtime_error("Inconsistent tiles ...");
        
        // Movement was successful
        if (newTileValue != TileValues::WALL)
        {
            // Single link
            auto success = connectivityMap.emplace(
                currentPosition, std::unordered_set<std::pair<int, int>, hash_pair> {newPos});
            if (!success.second)
                connectivityMap[currentPosition].emplace(newPos);

            // Double link
            success = connectivityMap.emplace(
                newPos, std::unordered_set< std::pair<int, int>, hash_pair> {currentPosition});
            if (!success.second)
                connectivityMap[newPos].emplace(currentPosition);

            currentPosition = newPos;
            currentTileValue = newTileValue;
        }
        
        movementDir = getNewMovement(currentPosition, movementDir);
    }
    return oxyPosition;
}

int shortestPath(std::pair<int, int> currentPosition, std::pair<int, int> previousPosition)
{
    // If we found the oxygen break !
    if (gridMap[currentPosition] == TileValues::OXY)
        return 0;

    // Find all the connected positions
    auto connectIt = connectivityMap.find(currentPosition);

    if (connectIt->second.size() == 1 &&                    // If there is only 1 way go there...
        *connectIt->second.begin() != previousPosition)     // ... And that element is not the previous element
        return 1 +  shortestPath(*connectIt->second.begin(), currentPosition);
    
    
    // Go through all the elements and calc path
    int minPath = 1e5;
    for (auto& candidate : connectIt->second)
    {   
        if (candidate == previousPosition)
            continue;

        minPath = std::min(
            minPath, 
            1 + shortestPath(candidate, currentPosition));
    }
    return minPath;
}

int part2(const std::pair<int, int> oxyPosition)
{
    std::vector< std::pair<int, int> > oxyRooms;
    oxyRooms.push_back(oxyPosition);

    int elapsedMin = 0;
    while (oxyRooms.size() != 0)
    {
        std::vector< std::pair<int, int> > updatedRooms;

        // Go though all available connected oxy rooms
        for (auto& oxyRoom : oxyRooms)
        for (auto& connectedRoom : connectivityMap[oxyRoom])
        {
            if (gridMap[connectedRoom] != TileValues::FREE)
                continue;
            
            // Update grid
            gridMap[connectedRoom] = TileValues::OXY;
            updateMatrixMap(connectedRoom, TileValues::OXY);
            updatedRooms.push_back(connectedRoom);
        }
        
        if (updatedRooms.size() == 0) break;
        elapsedMin++;
        printMatrixMap(std::make_pair(0, 0), false);
        std::cout << "Elapsed: " << elapsedMin << std::endl;
        oxyRooms = std::move(updatedRooms);
    }
    return elapsedMin;    
}

int main ()
{
    ic.setVerbosity(false);
    auto oxyPosition = getOxygenPosition();
    int minPath = shortestPath(std::make_pair(0, 0), std::make_pair(-1, -1));
    printMatrixMap(std::make_pair(0, 0), true);
    std::cout << "Shortest path: " << minPath << std::endl;

    int oxyTime = part2(oxyPosition);
    std::cout << "Oxygen time: " << oxyTime << std::endl;
}     