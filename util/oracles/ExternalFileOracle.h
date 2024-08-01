#pragma once
#include "Oracle.h"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

struct DataPoint
{
public:
    std::string timestamp;
    float value;
};



class ExternalFileOracle : Oracle
/* Oracle using an external price series as the fundamental. The external series are specified files in the ABIDES
   config. If an agent requests the fundamental value in between two timestamps the returned fundamental value is
   linearly interpolated. */
{
public:
    ExternalFileOracle(const std::string& symbol, const std::string& file_path)
    : symbol(symbol)
    {
        load_fundamentals(file_path);
    }

    void print(const int& index)
    {
        if (index > fundamental.size())
        {
            std::cout << "Index too large.\n" << "Max index: " << fundamental.size() << std::endl;
        }
        else
        {
            std::cout << fundamental[index].timestamp << "  " << fundamental[index].value << std::endl;
        }

    }

private:
    int mkt_open;
    std::string symbol;
    std::vector<DataPoint> fundamental;

    void load_fundamentals(const std::string& file_path)
    {
        std::ifstream file(file_path);
        if (!file.is_open()) 
        {
            std::cout << "Unable to open fundamental file." << std::endl;
            return; 
        }
        
        std::string line;
        while (std::getline(file, line))
        {
            std::stringstream lineStream(line);
            std::string timestamp;
            std::string valueStr;

            // Read the timestamp and value.
            if (std::getline(lineStream, timestamp, ',') && std::getline(lineStream, valueStr))
            {
                DataPoint dp;
                dp.timestamp = timestamp;
                dp.value = std::stof(valueStr);
                fundamental.push_back(dp);
            }
        }
        file.close();
    }

};