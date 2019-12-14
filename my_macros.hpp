#ifndef MY_MACROS_HPP
#define MY_MACROS_HPP

#define LOG(x) std::cout << x << std::endl;
#define LOG_VEC(vec) for (auto i : vec) \
                        std::cout << i; \
                     std::cout << std::endl;

#endif