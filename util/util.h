#include <random>

// Utility function to generate a random uniform variable in [0, 1).
inline double genRandUniform() {
    // Initialise a static random number generator.
    static std::random_device rd;  // Obtain a random number from hardware.
    static std::mt19937 gen(rd()); // Seed the generator with a random number.
    static std::uniform_real_distribution<> dis(0.0, 1.0); // Define the distribution.

    // Generate and return a random number.
    return dis(gen);
}

inline int genRandInt(int min, int max) {
    // Initialise a static random number generator.
    static std::random_device rd;  // Obtain a random number from hardware.
    static std::mt19937 gen(rd()); // Seed the generator with a random number.
    std::uniform_int_distribution<> dis(min, max); // Define the distribution.

    // Generate and return a random integer.
    return dis(gen);
}