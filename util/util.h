#pragma once
#include <random>
#include <iostream>
#include <unordered_map>
#include <sstream>  // For std::ostringstream
#include <string>

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

template<typename K, typename V>
std::string unorderedMapToString(const std::unordered_map<K, V>& map) {
    std::ostringstream oss;
    oss << "{ ";  // Start the string with a brace for a map-like representation

    for (const auto& pair : map) {
        oss << pair.first << ": " << pair.second << ", ";  // Append key-value pairs
    }

    std::string result = oss.str();

    // Remove the last comma and space, then close the brace
    if (result.length() > 2) {
        result = result.substr(0, result.length() - 2);
    }
    result += " }";

    return result;
}

std::string dollarise(int cents) {
        /*
        Used to dollarize an int-cents price for printing.
        */
        return "$" + std::to_string(cents/100) + "." + std::to_string(cents%100);
}

template <typename T>
std::string str(T val) { return std::to_string(T); }
