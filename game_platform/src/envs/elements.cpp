#include "elements.h"


void Environment::GiveInfo(Agent *pa, Information_t &info)
{
    pa->ReceiveInput(info);
}

Respond_t Environment::GetRespond(Agent *pa)
{
    return std::move(pa->SendResponse());
}



void Agent::Kill()
{
    /* nothing todo here */
    /* but this function shouldn't be pure virtual */
    /* because some agent don't need Kill method */
}


bool Platform::AddAgent(const AgentID &id, std::shared_ptr<Agent> agent)
{
    auto result = this->agents.insert({id,agent});
    return result.second;
}


std::shared_ptr<Agent> Platform::getAgentByID(const AgentID &id)
{
    try{
        auto result = this->agents.at(id);
        return result;
    }catch(...)
    {
        //TODO: give warning
        return std::shared_ptr<Agent>(nullptr);
    }
}
