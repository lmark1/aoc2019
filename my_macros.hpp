#define LOG(x) std::cout << x;
#define LOG_ENDL std::cout << std::endl;
#define LOG_VEC(vec) for (auto i : vec) \
                        std::cout << i; \
                     std::cout << std::endl;
#define LOG_PAIR(pair) std::cout << " (" << pair.first << ", " << pair.second << ") ";