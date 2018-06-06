#include <iostream>
#include <sstream>
#include <thread>

#ifndef OUTPUT
#include "envs/elements.h"
#include "envs/Agents.h"
#include "utils/json.hpp"
#include "envs/FightTheLandlord.h"
#else
#include "elements.h"
#include "Agents.h"
#include "json.hpp"
#include "FightTheLandlord.h"
#endif
using namespace std;

void testCppAgent()
{
    CppAgent agent("./test2");
    agent.Open();
    agent.ReceiveInput("dsdfa");
    //while(agent.GetState()==CppAgent::RUNNING);
    agent.Wait();
    auto str = agent.SendResponse();
    cout<<"test get from test2: "<<str<<endl;
    agent.Close();
}

void testJson()
{
    using Json = nlohmann::json;
    Json j;
    j["response"] = {{0,1,2},{5,4}};

    cout<<j["response"][0]<<endl;
}

void testEnv()
{
    FTLEnvironment ftlenv;
    auto request = ftlenv.Initialize();
    int turn = 0;
    while(!ftlenv.IsEnd())
    {
        if(turn%3 == 0)
            cerr<<"TURN "<<turn/3<<"---------------------------"<<endl;
        cout<<"request: "<<request<<endl;
        CppAgent agent("../bin/FTL");
        agent.Open();
        agent.ReceiveInput(request);
        agent.Wait();
        auto str = agent.SendResponse();
        cout<<"\tresponse: "<<str<<endl;
        request = ftlenv.Update(str);
        turn++;
    }
    cout<<"Game ends! winner is: "<<ftlenv.GetWinnerInfo()<<endl;
}

void testPlat()
{
    FTLPlatform platform;
    platform.ConfigureAgentAdd("CppAgent","../bin/FTL",FTLEnvironment::LANDLORD);
    platform.ConfigureAgentAdd("CppAgent","../bin/FTL",FTLEnvironment::FARMER_A);
    platform.ConfigureAgentAdd("CppAgent","../bin/FTL",FTLEnvironment::FARMER_B);
    platform.Configure(platform.GetConfigure());
    platform.Run();
}

int main()
{
    testPlat();
    return 0;
}
