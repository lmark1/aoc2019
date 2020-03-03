#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <algorithm>
#include <chrono>

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
  t_maze[t_startPosition.first][t_startPosition.second] = FREE_TILE;
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
    if (currChar == ME_TILE || is_key_tile(currChar)) {
      result.push_back(i);
    }
  }
  return result;
}

static std::unordered_map<char, DistanceMap> build_connectivity_map(
  std::vector<std::string>& t_maze, bool t_verbose = false)
{
  std::unordered_map<char, DistanceMap>  connectivityMap;
  for (int i = 0; i < t_maze.size(); i++) {
    const auto line = t_maze.at(i);
    const auto keyIndices = KeyFinder::find_all_key_indices(line);
    for (const auto& key : keyIndices) {
      const auto distanceMap = KeyFinder::find_nearest_keys(t_maze, std::make_pair(i, key));
      connectivityMap.emplace(line.at(key), distanceMap);

      if (t_verbose) {
        std::cout <<  "[main] Tile [" << line.at(key) << "] ";
        for (const auto& item : distanceMap) {
          std::cout << "{" << item.first << " : " << item.second << "}" << " ";
        }
        std::cout << "\n";
      }
    }
  }
  return connectivityMap;
}

static void check_connectivity_map(const std::unordered_map<char, DistanceMap>& t_connectivityMap)
{
  int errorCount = 0;
  for (const auto& connItem : t_connectivityMap) {
    const auto originTile = connItem.first;
    for (const auto& distanceItem : connItem.second) {
      const auto tile = distanceItem.first;
      const auto distance = distanceItem.second;

      const auto checkerDistance = t_connectivityMap.at(tile).at(originTile);
      if (distance != checkerDistance) {
        std::cout << originTile << " - " << tile << " distance " << distance << "\n";
        std::cout << tile << " - " << originTile << " distance " << checkerDistance << "\n\n";
        errorCount++;
      }
    }
  }
  std::cout << "Error count = " << errorCount << "\n";
}

static std::unordered_map<char, Position> get_position_map(
  std::vector<std::string>& t_maze)
{
  std::unordered_map<char, Position> positionMap;
  for (int i = 0; i < t_maze.size(); i++) {
    const auto line = t_maze.at(i);
    const auto keyIndices = KeyFinder::find_all_key_indices(line);
    for (const auto& index : keyIndices) {
      positionMap.emplace(line.at(index), std::make_pair(i, index));
    }
  }
  return positionMap;
}

static int minimize(std::vector<std::string>& o_maze)
{
  const auto positionMap = get_position_map(o_maze);
  std::unordered_set<char> collectedKeys;
  return minimize_internal(ME_TILE, positionMap, o_maze, collectedKeys, false);
}

private:

static int minimize_internal(
  const char t_currentChar,
  const std::unordered_map<char, Position>& t_positionMap,
  std::vector<std::string>& o_maze,
  std::unordered_set<char>& o_collectedKeys,
  bool t_verbose = false)
{
  // 0.5) Invalidate the current tile
  const auto currentPosition = t_positionMap.at(t_currentChar);

  // 1) Obtain the distance map from the current tile
  const auto distanceMap = find_nearest_keys(o_maze, currentPosition);
  
  if (t_verbose) {
    std::cout <<  "[minimize] Tile [" << t_currentChar << "] ";
    for (const auto& item : distanceMap) {
      std::cout << "{" << item.first << " : " << item.second << "}" << " ";
    }
    std::cout << "\n";
  }

  // 2) Find all the available keys
  if (t_verbose) { std::cout << "[minimize] Valid tiles are: "; }
  std::unordered_set<char> validTiles;
  for (const auto& distanceItem : distanceMap) {
    const auto tile = distanceItem.first;
    
    if (is_tile_key(tile)
      || is_door_unlocked(tile, o_collectedKeys)) {
      validTiles.emplace(tile);
      if (t_verbose) { std::cout << tile << ", "; }
    }
  }
  if (t_verbose) { std::cout << "\n"; }

  if (validTiles.empty()) {
    return 0;
  }

  // 3) Recursion through all the available keys
  int minPathCost = 1e4;
  for (const auto& tileCandidate : validTiles) {

    const auto originalTile = o_maze[currentPosition.first][currentPosition.second];
    o_maze[currentPosition.first][currentPosition.second] = FREE_TILE;
    if (is_tile_key(tileCandidate)) {
      o_collectedKeys.emplace(tileCandidate);
    }

    if (t_verbose) {std::cout << "[minimize] Moving from " << t_currentChar << " to " << tileCandidate << "\n\n";}
    const auto pathCost =
      distanceMap.at(tileCandidate)
      + minimize_internal(
        tileCandidate,
        t_positionMap,
        o_maze,
        o_collectedKeys,
        t_verbose
      );

    // std::cout << "Path cost: " << pathCost << "\n";
    if (pathCost < minPathCost) {
      minPathCost = pathCost;
    }

    o_collectedKeys.erase(tileCandidate);
    o_maze[currentPosition.first][currentPosition.second] = originalTile;    
  }

  // std::cout << "Returning " << minPathCost << "\n";
  return minPathCost;
}

static inline bool is_tile_key(const char tile)
{
  return islower(tile) != 0;
}

static inline bool is_door_unlocked(const char tile, const std::unordered_set<char>& collectedKeys)
{
  return collectedKeys.find(tolower(tile)) != collectedKeys.end();
}

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
    return;
  }

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
  while (getline(inputFile, line)) {
    maze.push_back(line);
  }

  auto start = std::chrono::steady_clock::now();
  auto connectivityMap = KeyFinder::build_connectivity_map(maze, true);
  KeyFinder::check_connectivity_map(connectivityMap);

  const auto minPathCost = KeyFinder::minimize(maze);
  std::cout << "Min path cost: " << minPathCost << "\n";

  auto end = std::chrono::steady_clock::now();
  std::cout << "Elapsed time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms\n";
}