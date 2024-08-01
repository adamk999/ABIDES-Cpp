#include <iostream>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <sstream>

struct Timestamp {
    std::chrono::nanoseconds ns_since_epoch;

    // Default constructor to an invalid number.
    Timestamp() : ns_since_epoch(-1) {};

    // Constructor from std::chrono::system_clock::time_point.
    Timestamp(const std::chrono::system_clock::time_point& tp)
        : ns_since_epoch(std::chrono::duration_cast<std::chrono::nanoseconds>(tp.time_since_epoch())) {}

    // Constructor from nanoseconds.
    Timestamp(long long nanoseconds)
        : ns_since_epoch(std::chrono::nanoseconds(nanoseconds)) {}

    // Get the current timestamp.
    static Timestamp now() {
        return Timestamp(std::chrono::system_clock::now());
    }

    // Convert to string representation
    std::string to_string(const std::string& format = "%Y-%m-%d %H:%M:%S") const {
        // Convert nanoseconds to time_point
        auto time_point = std::chrono::system_clock::time_point(std::chrono::duration_cast<std::chrono::system_clock::duration>(ns_since_epoch));
        std::time_t time = std::chrono::system_clock::to_time_t(time_point);
        std::tm tm = *std::localtime(&time);

        std::ostringstream oss;
        oss << std::put_time(&tm, format.c_str());
        return oss.str();
    }

    bool isValid() const {
        return ns_since_epoch.count() >= 0;
    }

    // Get nanoseconds since epoch
    long long to_nanoseconds() const {
        return ns_since_epoch.count();
    }

/* ––––––––––––––––––––––––  Operator Overloading –––––––––––––––––––––––– */
    
    Timestamp operator+(const Timestamp& t_other) const {
        return Timestamp(ns_since_epoch.count() + t_other.ns_since_epoch.count());
    }

    Timestamp& operator+=(const int& delay) {
        ns_since_epoch += std::chrono::nanoseconds(delay);
        return *this;
    }

    bool operator<(const Timestamp& t_other) const {
        return ns_since_epoch < t_other.ns_since_epoch;
    }

    bool operator<=(const Timestamp& t_other) const {
        return ns_since_epoch <= t_other.ns_since_epoch;
    }

    bool operator>(const Timestamp& t_other) const {
        return ns_since_epoch > t_other.ns_since_epoch;
    }

    bool operator>=(const Timestamp& t_other) const {
        return ns_since_epoch >= t_other.ns_since_epoch;
    }

    bool operator==(const Timestamp& t_other) const {
        return ns_since_epoch == t_other.ns_since_epoch;
    }

    bool operator==(const int& nanosecs) const {
        return ns_since_epoch.count() == nanosecs;
    }
};