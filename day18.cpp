#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <algorithm>

struct hash_pair { 
  template <class T1, class T2> 
  size_t operator()(const std::pair<T1, T2>& p) const
  { 
    auto hash1 = std::hash<T1>{}(p.first); 
    auto hash2 = std::hash<T2>{}(p.second); 
    return hash1 ^ hash2; 
  } 
}; 

class KeyFinder {
public:
static std::unordered_map<char, int> find_nearest_keys(
  const std::vector<std::string>& t_maze,
  const std::pair<int, int>& t_startPosition)
{
  std::unordered_map<char, int> keyDistanceMap;
  std::unordered_set<std::pair<int, int>, hash_pair> exploredPath;
  find_nearest_keys_internal(
    t_maze,
    t_startPosition,
    t_startPosition,
    0,
    keyDistanceMap,
    exploredPath
  );
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
    && t_tile != FREE_TILE
    && t_tile != ME_TILE;
}

static inline char tile_at(
  const std::vector<std::string>& t_maze,
  const std::pair<int, int> & t_currentPosition)
{
  return t_maze.at(t_currentPosition.first).at(t_currentPosition.second);
}

static bool is_key_at_current_position(
  const std::vector<std::string>& t_maze,
  const std::pair<int, int> & t_currentPosition)
{
  return is_key_tile(tile_at(t_maze, t_currentPosition));
}

static std::vector<std::pair<char, std::pair<int, int>>> get_adjacent_tiles(
  const std::vector<std::string>& t_maze,
  const std::pair<int, int> & t_currentPosition)
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
  const std::pair<int, int> & t_currentPosition,
  const std::pair<int, int> & t_previousPosition,
  int o_numberOfSteps,
  std::unordered_map<char, int>& o_keyDistanceMap,
  std::unordered_set<std::pair<int, int>, hash_pair>& o_exploredPath)
{
  // If key is at the current position, add it to the map and step out of recursion
  if (is_key_at_current_position(t_maze, t_currentPosition)) {
    const auto key = tile_at(t_maze, t_currentPosition);
    const auto emplace_struct = o_keyDistanceMap.emplace(key, o_numberOfSteps);
    if (!emplace_struct.second && o_keyDistanceMap[key] > o_numberOfSteps) {
      o_keyDistanceMap[key] = o_numberOfSteps;
    }
    // std::cout << "Key [" << key 
    //   << "] @ [" << t_currentPosition.first 
    //   << ", " << t_currentPosition.second << "]"
    //   << " - distance " << o_keyDistanceMap[key] << "\n";
    return;
  }

  const auto adjacent_tiles = get_adjacent_tiles(t_maze, t_currentPosition);
  const auto explore_if_available = [&] 
  (const std::pair<char, std::pair<int, int>>& tile_item) {
    // std::cout 
    //   << "Previous position: [" << t_previousPosition.first 
    //   << ", " << t_previousPosition.second << "]; ";
    // std::cout 
    //   << "Adjacent Tile " << tile_item.first << " @ ["
    //   << tile_item.second.first << ", "
    //   << tile_item.second.second << "]\n";
      
    if(
      !is_wall_tile(tile_item.first) 
      && tile_item.second != t_previousPosition
      && o_exploredPath.find(tile_item.second) == o_exploredPath.end()) {
      //  std::cout << "Going in!\n";
      o_exploredPath.emplace(tile_item.second);
      find_nearest_keys_internal(
        t_maze,
        tile_item.second,
        t_currentPosition,
        ++o_numberOfSteps,
        o_keyDistanceMap,
        o_exploredPath
      );
      // std::cout << "Coming out!\n";
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
  std::unordered_map<char, std::unordered_map<char, int>>  connectivityMap;
  while (getline(inputFile, line)) {
    maze.push_back(line);
  }
  
  int i = 0;
  for (const auto& line : maze) {
    const auto keyIndices = KeyFinder::find_all_key_indices(line);
    for (const auto& key : keyIndices) {
      const auto distanceMap = KeyFinder::find_nearest_keys(maze, std::make_pair(i, key));
      std::cout <<  "For [" << line.at(key) << "]\n";
      for (const auto& item : distanceMap) {
        std::cout << "{" << item.first << " : " << item.second << "}" << " ";
      }
      std::cout << "\n";
      connectivityMap.emplace(line.at(key), distanceMap);
    }
    i++;
  }
}