#ifndef _AGENT_HH_
#define _AGENT_HH_

#include "elements.h"
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

class ProtocolHandler
{
    private:
        static const size_t PROTOCOL_HEADER_LENGTH = 8;
    public:
        static std::string ParseInput(const std::string &input);
        static std::string ParseOutput(const std::string &output);
};

class CppAgent : public Agent
{
    protected:
        virtual int UpdateState();  /* return -1 if not being running */
    public:
        static const int RUNNING = 1;
        static const int UNINITIALIZED = 0;
        static const int FINISHED = -1;
        static const int CLOSED = 2;
    public:
        /* standard interfaces */
        virtual void Open() override;
        virtual void ReceiveInput(const Information_t &input) override;
        virtual Respond_t SendResponse() override;
        virtual void Close() override;
        virtual void Kill() override;/* include Close() */

        /* Constructors */
        CppAgent(const char *filename);
        CppAgent(const std::string &str);
        ~CppAgent() = default;

        virtual int GetState();
        virtual void Wait();
        
        /**
         * TODO
         *  Receive & Send: using pipes
         *  DONE. Close: release some buffer
         *  DONE. Kill: kill child process
         *  DONE. updateState: using "statusp" param in waitpid to probe
         *  DONE. getState: call updateState() first
         */

    private:
        int state_;
        pid_t pid_;
        int sendpipe_[2];
        int recvpipe_[2];
        std::string filename_;
};
#endif
