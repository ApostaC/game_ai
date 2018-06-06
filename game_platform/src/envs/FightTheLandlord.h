#ifndef _FIGHTTHELANDLORD_HH_
#define _FIGHTTHELANDLORD_HH_

/* @game FightTheLandlord */
#include "elements.h"
#include <map>
#include <set>
#include <deque>
#ifndef OUTPUT
    #include "../utils/FTLCards.hpp"
    #include "../utils/json.hpp"
#else
    #include "FTLCards.hpp"
    #include "json.hpp"
#endif

class FTLEnvironment;
class FTLPlatform;

using Json = nlohmann::json;

class FTLEnvironment : public Environment
{
    public:
        typedef int Card_t;
        using Cards_t = std::set<Card_t>;
        static constexpr int FARMER_A = 1;
        static constexpr int FARMER_B = 2;
        static constexpr int LANDLORD = 0;

    private:
        typedef std::vector<Json> _jsv;
        Cards_t _transformer(const Respond_t &respond);
        void _endGame(int winner);
        void _updateRequest(int player);
        void _updateResponse(int player,const Respond_t &res);
        Respond_t _outputWrapper(int player);
        /**
         * requests: all of the request it received
         * responses: his own respond
         */
    public:
        /* standard interfaces */
        virtual _RTU Initialize() override;
        virtual bool JudgeRespond(const Respond_t &respond) override;
        /* returns the input for the next player */
        virtual _RTU Update(const Respond_t &step) override;

        FTLEnvironment();
        bool IsEnd();
        std::string GetWinnerInfo();
    private:
        int lastCardPlayer_;
        int currPlayer_;
        int isEnd_ = false;
        int winner_ = -1;

        Cards_t publicCard_;
        std::map<int, Cards_t> handCard_;
        std::deque<Cards_t> history_;
        Cards_t lastCard_;

        /* for request and response */
        std::map<int, _jsv> requests_;
        typedef std::vector<Cards_t> _resp_t;
        std::map<int, _resp_t> responses_;
        
        
        /**
         * TODO:
         * Done. _transformer: change json string to Cards
         * Done. Initialize: initialize cards using std random lib
         * Done. JudgeRespond: call _transformer, judge whether it is correct
         *                  1.player have those cards
         *                  2.same pattern with previous
         *                  3.larger than previous
         * Done. Update: call _transformer, and delete some card
         *          update history
         *          update lastcard,lastplayer
         *          update currplayer
         *
         */
};


class FTLPlatform : public Platform
{
    /**
     * To configure
     * {
     *  "NUM_AGENTS": <num_agents>,
     *  "AGENTS":[
     *      {
     *          "type": "<type>"
     *          "location": "<location>"
     *          "position": <position> ### the position in game
     *      },
     *      {
     *          ...
     *      },
     *      {
     *          ...
     *      }
     *  ]
     * }
     */
    public:
        virtual void Configure(const Configure_t *conf) override;
        virtual void Run() override;
        FTLPlatform();
        ~FTLPlatform();

        void ConfigureAgentAdd(const std::string &type,
                                const std::string &location,
                                int position);

        virtual Configure_t *GetConfigure();
    private:
        Configure_t *_conf;
        
};

#endif
