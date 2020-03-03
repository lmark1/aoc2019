#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <algorithm>

typedef std::pair<int, int> Position;
typedef std::unordered_map<char, int> DistanceMap;

struct hash_pair { 
  template <class T1, class T2> 
  size_t operator()(const std::pair<T1, T2>& p) const
  { 
    auto hash1 = std::hash<T1>{}(p.first); 
    auto hash2 = std::hash<T2>{}(p.second); 
    return hash1 ^ hash2; 
  } 
}; 

std::ostream& operator<<(std::ostream& os, const Position& pos)
{
  os << "(" << pos.first << ", " << pos.second << ")";
  return os;
}

class KeyFinder {
public:
static DistanceMap find_nearest_keys(
  std::vector<std::string>& t_maze,
  const Position& t_startPosition)
{
  DistanceMap keyDistanceMap;
  std::unordered_set<Position, hash_pair> exploredPath;
  const auto originalChar = t_maze[t_startPosition.first][t_startPosition.second];
  // std::cout << "Finding nearest objects for [" << originalChar << "] at " << t_startPosition << "\n"; 
  t_maze[t_startPosition.first][t_startPosition.second] = '.';
  find_nearest_keys_internal(
    t_maze,
    t_startPosition,
    t_startPosition,
    0,
    keyDistanceMap,
    exploredPath
  );
  t_maze[t_startPosition.first][t_startPosition.second] = originalChar;
  return keyDistanceMap;
}

static std::vector<std::size_t> find_all_key_indices(const std::string& t_row)
{
  std::vector<std::size_t> result;
  for (int i = 0; i < t_row.size(); i++) {
    const auto currChar = t_row.at(i);
    if (currChar == '@' || is_key_tile(currChar)) {
      result.push_back(i);
    }
  }
  return result;
}


private:
static inline bool is_wall_tile(char t_tile)
{
  return t_tile == WALL_TILE;
}

static inline bool is_key_tile(char t_tile)
{
  return t_tile != WALL_TILE 
    && t_tile != FREE_TILE;
}

static inline char tile_at(
  const std::vector<std::string>& t_maze,
  const Position & t_currentPosition)
{
  return t_maze.at(t_currentPosition.first).at(t_currentPosition.second);
}

static bool is_key_at_current_position(
  const std::vector<std::string>& t_maze,
  const Position & t_currentPosition)
{
  return is_key_tile(tile_at(t_maze, t_currentPosition));
}

static std::vector<std::pair<char, Position>> get_adjacent_tiles(
  const std::vector<std::string>& t_maze,
  const Position & t_currentPosition)
{
  const auto north_tile = std::make_pair(t_currentPosition.first + 1, t_currentPosition.second);
  const auto south_tile = std::make_pair(t_currentPosition.first - 1, t_currentPosition.second);
  const auto east_tile = std::make_pair(t_currentPosition.first, t_currentPosition.second + 1);
  const auto west_tile = std::make_pair(t_currentPosition.first, t_currentPosition.second - 1);

  return {
    {tile_at(t_maze, north_tile), north_tile},
    {tile_at(t_maze, south_tile), south_tile},
    {tile_at(t_maze, east_tile), east_tile},
    {tile_at(t_maze, west_tile), west_tile}
  };
}

static void find_nearest_keys_internal(
  const std::vector<std::string>& t_maze,
  const Position & t_currentPosition,
  const Position & t_previousPosition,
  int o_numberOfSteps,
  DistanceMap& o_keyDistanceMap,
  std::unordered_set<Position, hash_pair>& o_exploredPath)
{
  // If key is at the current position, add it to the map and step out of recursion
  if (is_key_at_current_position(t_maze, t_currentPosition)) {
    const auto key = tile_at(t_maze, t_currentPosition);
    const auto emplace_struct = o_keyDistanceMap.emplace(key, o_numberOfSteps);
    if (!emplace_struct.second && o_keyDistanceMap[key] > o_numberOfSteps) {
      o_keyDistanceMap[key] = o_numberOfSteps;
    }
    // std::cout << "Key [" << key 
    //   << "] at " << t_currentPosition
    //   << " - distance " << o_keyDistanceMap[key]
    //   << " - found distance " << o_numberOfSteps << "\n";
    
    // Reset the explored path if new best distacne is found
    // if (o_keyDistanceMap[key] == o_numberOfSteps) {
    //   o_exploredPath.clear();
    // }
    return;
  }

  // if (o_numberOfSteps > t_maze.size() * t_maze.front().size()) {
  //   return;
  // }

  const auto adjacent_tiles = get_adjacent_tiles(t_maze, t_currentPosition);
  const auto explore_if_available = [&] 
  (const std::pair<char, Position>& tile_item) {
    // std::cout 
    //   << "Previous position: " << t_previousPosition
    //   << "; Adjacent Tile " << tile_item.first << " at "
    //   << tile_item.second << "\n";
      
    if(
      !is_wall_tile(tile_item.first) 
      && tile_item.second != t_previousPosition
      && o_exploredPath.find(tile_item.second) == o_exploredPath.end()) 
    {
      o_exploredPath.emplace(tile_item.second);
      // std::cout << "Stepping in!\n";
      find_nearest_keys_internal(
        t_maze,
        tile_item.second,
        t_currentPosition,
        o_numberOfSteps + 1,
        o_keyDistanceMap,
        o_exploredPath
      );
      // std::cout << "Stepping out!\n";
      o_exploredPath.erase(tile_item.second);
    }
  };
  std::for_each(adjacent_tiles.begin(), adjacent_tiles.end(), explore_if_available);
} 

static constexpr char WALL_TILE = '#';
static constexpr char FREE_TILE = '.';
static constexpr char ME_TILE = '@';
};

int main()
{
  std::fstream inputFile("day18.txt");
  std::vector<std::string> maze;
  std::string line;
  std::unordered_map<char, DistanceMap>  connectivityMap;
  while (getline(inputFile, line)) {
    maze.push_back(line);
  }
  
  // Build the connectivity map
  for (int i = 0; i < maze.size(); i++) {
    const auto line = maze.at(i);
    const auto keyIndices = KeyFinder::find_all_key_indices(line);
    for (const auto& key : keyIndices) {
      const auto distanceMap = KeyFinder::find_nearest_keys(maze, std::make_pair(i, key));
      // std::cout <<  "[main] Tile [" << line.at(key) << "] ";
      // for (const auto& item : distanceMap) {
      //   std::cout << "{" << item.first << " : " << item.second << "}" << " ";
      // }
      // std::cout << "\n\n";
      connectivityMap.emplace(line.at(key), distanceMap);
    }
  }

  // Check the connectivity map
  int errorCount = 0;
  for (const auto& connItem : connectivityMap) {
    const auto originTile = connItem.first;
    for (const auto& distanceItem : connItem.second) {
      const auto tile = distanceItem.first;
      const auto distance = distanceItem.second;

      const auto checkerDistance = connectivityMap[tile][originTile];
      if (distance != checkerDistance) {
        std::cout << originTile << " - " << tile << " distance " << distance << "\n";
        std::cout << tile << " - " << originTile << " distance " << checkerDistance << "\n\n";
        errorCount++;
      }
    }
  }
  std::cout << "Error count = " << errorCount << "\n";
}