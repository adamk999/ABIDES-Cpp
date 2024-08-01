#include <iostream>
#include <fstream>
#include <string>

class Logger
{
private:
    std::ofstream logFile;
    
    // Destructor to close the log file.
    ~Logger() {
        if (logFile.is_open()) 
        {
            logFile.close();
        }
    }
    
public:
    // Constructor to open the log file.
    Logger(const std::string& filepath)
    {
        logFile.open(filepath, std::ios_base::app);
        if (!logFile.is_open()) 
        {
            throw std::runtime_error("Unable to open log file: " + filepath);
        }
    }

    // Method to write a line to the log file.
    void log(const std::string& message) 
    {
        logFile << message << std::endl;
    }
};