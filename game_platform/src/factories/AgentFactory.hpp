#ifndef _AGENTFACTORY_HH_
#define _AGENTFACTORY_HH_

#include <stdexcept>
#ifndef OUTPUT
    #include "../envs/elements.h"
    #include "../envs/Agents.h"
#else
    #include "elements.h"
    #include "Agents.h"
#endif
class AgentFactory
{
    public:
        //static std::shared_ptr<Agent>
        //    NewInstance(const std::string &name, const std::string& position);
        template<typename ...Args>
        static std::shared_ptr<Agent>
            NewInstance(const std::string &name, Args ...args);
};


/*std::shared_ptr<Agent> AgentFactory::NewInstance(const std::string &name, const std::string& position)
{
    if(name == "CppAgent")
    {
        return std::dynamic_pointer_cast<Agent>(std::make_shared<CppAgent>(position.c_str()));
    }
    else
    {
        std::string errmsg = "Class NotFound Execption: " + name;
        throw std::runtime_error(errmsg);
    }
}*/

template<typename ...Args>
std::shared_ptr<Agent> AgentFactory::NewInstance(const std::string &name, Args... args)
{
    if(name == "CppAgent")
    {
        return std::dynamic_pointer_cast<Agent>(std::make_shared<CppAgent>(args...));
    }
    else
    {
        std::string errmsg = "Class NotFound Execption: " + name;
        throw std::runtime_error(errmsg);
    }
}
#endif
