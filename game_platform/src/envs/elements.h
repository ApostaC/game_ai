#ifndef _ELEMENTS_HH_
#define _ELEMENTS_HH_
#include <string>
#include <vector>
#include <map>
#include <memory>

//#define USE_DEFAULT_CONFIGURE

/**
 * elements.h 
 * defines basic classes to run the game platform
 */

class Environment;
class Agent;
class Platform;

class CppAgent;
class PythonAgent;

typedef std::string Respond_t;      /* Agent's response */
typedef std::string Information_t;  /* Env's info to agnets */

/* Environments */
class Environment
{
    protected:
        typedef std::string __RETURN_TYPE_FOR_UPDATE;
        typedef __RETURN_TYPE_FOR_UPDATE _RTU;
    public:
        virtual _RTU Initialize() = 0;
        virtual void GiveInfo(Agent * pa, Information_t & info);
        virtual Respond_t GetRespond(Agent *pa);
        virtual bool JudgeRespond(const Respond_t & respond) = 0;
        virtual _RTU Update(const Respond_t &step) = 0;

        Environment() = default;
        ~Environment() = default;
};


/* Platforms */
class Configure_t;
typedef std::string AgentID;
class Platform
{
    public:
        virtual void Configure(const Configure_t *conf) = 0;
        virtual void Run() = 0;

        bool AddAgent(const AgentID &id, std::shared_ptr<Agent> agent);
        [[deprecated]]
           std::shared_ptr<Agent> getAgentByID(const AgentID & id);
        
        Platform(std::shared_ptr<Environment> env_)
            :env(std::move(env_)){}
        
    protected:
        std::shared_ptr<Environment> env;
        std::map<AgentID, std::shared_ptr<Agent>> agents;
};



/* Agents */
class Agent
{
    public:
        virtual void Open() = 0;
        virtual void ReceiveInput(const Information_t &input) = 0;
        virtual Respond_t SendResponse() = 0;
        virtual void Close() = 0;
        virtual void Kill();
        virtual void Wait() = 0;

        Agent() = default;
        ~Agent() = default;
};

#endif
