#include <iostream>
#include "../util/timestamping.h"
#include "../agents/TradingAgent.h"
#include "../util/logger.h"
#include "../Kernel.h"
#include "../util/oracles/ExternalFileOracle.h"
#include "../util/util.h"

using namespace std;

int main()
{
    Logger log("logger.txt");

    
    Kernel kernel("first_kernel", 1, log);

    //std::vector<Agent> agents(1);
    //gents[0] = Agent(10, "first_agent", "na", 30);
    ExternalFileOracle oracle("AAPL", "data/AAPL.csv");

    //kernel.runner(agents, 10, 11, 1, 1, 50, false, oracle, "/directory");

    // Instantiate the current timestamp
    Timestamp now = Timestamp::now();

    TradingAgent TA(
        1,
        "First_Test",
        "NA",
        1,
        log    
    ); 

    TA.kernelStarting(now);



    return 0;
}