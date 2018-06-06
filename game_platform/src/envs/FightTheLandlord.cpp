#include "FightTheLandlord.h"
#include <random>
#include <vector>
#include <sstream>
#ifndef OUTPUT
    #include "../factories/AgentFactory.hpp"
#else
    #include "AgentFactory.hpp"
#endif

using Json = nlohmann::json;

/* FTLEnvironment */

FTLEnvironment::Cards_t FTLEnvironment::_transformer(const Respond_t &respond)
{
    Json j;
    std::istringstream sin(respond);
    sin>>j;
    Cards_t ret;
    for(auto cd : j["response"])
        ret.insert((int)cd);
    return ret;
}


void FTLEnvironment::_endGame(int winner)
{
    this->isEnd_ = 1;
    this->winner_ = winner;
}

void FTLEnvironment::_updateRequest(int player)
{
    auto & req = requests_[player];
    Json j;
    if(req.size() == 0)     /* the first request */
    {
        j["own"] = handCard_[player];
        j["publiccard"] = publicCard_;
    }
    j["history"][0] = history_[0];
    j["history"][1] = history_[1];
    req.emplace_back(j);
}

void FTLEnvironment::_updateResponse(int player, const Respond_t &res)
{
    auto & resv = responses_[player];
    Cards_t cds = std::move(_transformer(res));
    resv.emplace_back(cds);
}

Respond_t FTLEnvironment::_outputWrapper(int player)
{
    std::ostringstream sout;
    auto req = requests_[player];
    auto res = responses_[player];
    
    Json j{{"requests",req},{"responses",res}};
    sout.clear();
    sout<<j;
    return sout.str();
}

FTLEnvironment::FTLEnvironment()
{
    isEnd_ = 0;
    lastCardPlayer_ = LANDLORD;
    currPlayer_ = LANDLORD;   
    history_.resize(2);
}

Environment::_RTU FTLEnvironment::Initialize()
{
    lastCardPlayer_ = LANDLORD;
    currPlayer_ = LANDLORD;   

    std::vector<Card_t> cards;
    for(int i=0;i<54;i++)
        cards.push_back(i);

    /* shuffle card */
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(cards.begin(),cards.end(),g);

    /* give card */
    for(int i=0;i<51;i++)
        handCard_[i/17].insert(cards[i]);

    for(int i=51;i<54;i++)
    {
        int aa = LANDLORD;
        handCard_[aa].insert(cards[i]);
        publicCard_.insert(cards[i]);
    }

    /* the very first turn */
    _updateRequest(LANDLORD);
    return _outputWrapper(LANDLORD);
}

bool FTLEnvironment::JudgeRespond(const Respond_t &respond)
{
    Cards_t cards = std::move(_transformer(respond));
    const Cards_t &currhand = handCard_[currPlayer_];

    /* player have those cards? */
    for(auto card : cards)
        if(!currhand.count(card)) return false;

    if(lastCardPlayer_ == currPlayer_)
    {
        if(cards.empty())   /* pass after a round */
            return false;
        return true;
    }

    if(cards.empty()) return true;  /* pass */

    /* same pattern with previous */
    CardCombo last(lastCard_.begin(),lastCard_.end());
    CardCombo now(cards.begin(),cards.end());

    //if(now.comboType!=last.comboType) return false;
    if(!last.canBeBeatenBy(now)) return false;

    return true;
}

Environment::_RTU FTLEnvironment::Update(const Respond_t &step)
{
    // TODO: update response
    _updateResponse(currPlayer_, step);

    _RTU ret;

    bool valid = JudgeRespond(step);
    if(!valid)
    {
        std::fprintf(stderr,"not valid!\n");
        _endGame(currPlayer_ == 0?1:0);
        return "ERROR";
        //return error
    }

    auto cards = std::move(_transformer(step));

    /* delete cards */
    Cards_t &currhand{handCard_[currPlayer_]};
    for(auto cd : cards)
        currhand.erase(cd);

    /* update last*** */
    if(!cards.empty()) // not pass, update
    {
        lastCardPlayer_ = currPlayer_;
        lastCard_ = cards;
    }

    /* update history */
    history_.pop_front();
    history_.emplace_back(cards);
    
    /* Judge game end */
    if(handCard_[currPlayer_].empty())
    {
        _endGame(currPlayer_);
    }

    /* update currplayer */
    currPlayer_ = (currPlayer_ + 1)%3; 

    /* prepare for return */
    _updateRequest(currPlayer_);
    return _outputWrapper(currPlayer_);
}

bool FTLEnvironment::IsEnd()
{
    return this->isEnd_;
}

std::string FTLEnvironment::GetWinnerInfo()
{
    if(this->winner_ == 0)
        return "Landlord";
    else if (this->winner_ > 0)
        return "Farmer";
    else return "ERROR! NO WINNER!";
}



/* FTLPlatform */
class Configure_t
{
    public:
        Json cfg;
};


Configure_t *FTLPlatform::GetConfigure()
{
    return this->_conf;
}

void FTLPlatform::ConfigureAgentAdd(const std::string &type,
                            const std::string &location,
                            int position)
{
    auto & cfg = this->_conf->cfg;
    int num = cfg["NUM_AGENTS"];
    ++num;
    cfg["NUM_AGENTS"]=num;
    cfg["AGENTS"][num-1] = {
        {"type",type},
        {"location",location},
        {"position",position}
    };
}


FTLPlatform::FTLPlatform()
    :Platform(
        std::dynamic_pointer_cast<Environment>(std::make_shared<FTLEnvironment>())
            )
{
    this->_conf = new Configure_t;
    _conf->cfg["NUM_AGENTS"] = 0;
    _conf->cfg["AGENTS"] = Json::array();
}

void FTLPlatform::Configure(const Configure_t *conf)
{
    for(int id = 0;id<3;++id)
    {
        for(auto &agent:_conf->cfg["AGENTS"])
        {
            if(agent["position"] == id)
            {
                const std::string & type = agent["type"],
                     & location = agent["location"];
                this->AddAgent(std::to_string(id),
                        AgentFactory::NewInstance(type,location));
            }
        }
    }
}


void FTLPlatform::Run()
{
    using namespace std;
    auto ftlenv = std::dynamic_pointer_cast<FTLEnvironment>(this->env);
    auto request = ftlenv->Initialize();
    int turn = 0;
    while(!ftlenv->IsEnd())
    {
        auto agentid = std::to_string(turn%3);
        auto agent = this->agents[agentid];
        agent->Open();
        agent->ReceiveInput(request);
        agent->Wait();
        auto str = agent->SendResponse();
        request = ftlenv->Update(str);
        ++turn;
        agent->Close();
    }
    std::cout<<"Game ends! winner is: "<<ftlenv->GetWinnerInfo()<<std::endl;
}

FTLPlatform::~FTLPlatform()
{
    delete this->_conf;

}
