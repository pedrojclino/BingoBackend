#include "utils.h"
#include <random>
#include <chrono>

int Utils::random(int min, int max)
{
    std::random_device rd;  

    // 2. Initialize the Mersenne Twister engine with the seed
    std::mt19937 gen(rd()); 

    // 3. Define the distribution range [min, max] (inclusive)
    std::uniform_int_distribution<> distrib(min, max);

    // 4. Generate the random number
    return distrib(gen);
}

int Utils::epochTime()
{
    auto now = std::chrono::system_clock::now();
    return static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count());
}