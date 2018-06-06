#include "Agents.h"
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

/* Linux Helpers */

void unix_warning(const char *err)
{
    fprintf(stderr,"\033[33;44mWarning:\033[0;33m %s\033[0m\n",err);
}

void unix_error(const char *err)
{
    perror(err);
    exit(errno);
}

pid_t Fork(void)
{
    pid_t pid;
    if((pid = fork()) < 0)
        unix_error("fork error!");
    return pid;
}

int Pipe(int pipefd[2])
{
    int ret;
    if((ret = pipe(pipefd)) == -1)
        unix_error("fail to open pipe");
    return ret;
}

int Waitpid(pid_t pid, int *statusp, int options)
{
    int ret;
    if((ret = waitpid(pid, statusp, options)) < 0)
        perror("Waitpid error");
    return ret;
}

/* Communicate Protocols */

std::string ProtocolHandler::ParseInput(const std::string &input)
{
    size_t size = input.size();
    char buf[PROTOCOL_HEADER_LENGTH + 1];
    memcpy(buf,(char*)&size,PROTOCOL_HEADER_LENGTH);
    buf[PROTOCOL_HEADER_LENGTH] = 0;

    std::string ret{buf};
    std::copy(input.begin(),input.end(),std::back_inserter(ret));
    return ret;
}

std::string ProtocolHandler::ParseOutput(const std::string &output)
{
    char buf[PROTOCOL_HEADER_LENGTH + 1];
    for(size_t i=0;i<PROTOCOL_HEADER_LENGTH;++i)
        buf[i] = output[i];
    buf[PROTOCOL_HEADER_LENGTH] = 0;
    size_t size = *(size_t *)buf;
    std::string ret;
    for(size_t i=PROTOCOL_HEADER_LENGTH;i<output.length();i++)
        ret.push_back(output[i]);
    return ret;
}

/* CppAgent */


CppAgent::CppAgent(const char *filename):filename_(filename)
{
    state_ = UNINITIALIZED;
    pid_ = 0;
}

CppAgent::CppAgent(const std::string &s):CppAgent(s.c_str())
{
}

void CppAgent::Wait()
{
    UpdateState();
    if(this->state_ != RUNNING)
        return;
    Waitpid(-1,NULL,0);
    this->state_ = FINISHED;
}

int CppAgent::UpdateState()
{
    /*sigset_t mask,prevmask;
    sigemptyset(&mask);
    sigaddset(&mask,SIGCHLD);
    sigprocmask(SIG_BLOCK, &mask, &prevmask);*/

    if(this->state_ != RUNNING) return -1;
    int stat;
    int ret = Waitpid(this->pid_, &stat, WNOHANG|WUNTRACED); 
    if(ret == this->pid_)
        this->state_ = FINISHED;

//    sigprocmask(SIG_SETMASK, &prevmask, NULL);
    return ret;
}

void CppAgent::Open()
{
    switch(this->state_)
    {
        case UNINITIALIZED:
            break;
        case CLOSED:
            break;
        case RUNNING:
            unix_warning("Cannot open the agent during running");
            return;
        case FINISHED:
            unix_warning("Cannot open the agent before close it");
            return;
        default:
            unix_warning("Open error, unknown state");
            return;
    }

    //signal(SIGCHLD, Sigchld_Handler);
    /* Block the SIGCHLD and fork child */
    /*sigset_t mask,prevmask;
    sigemptyset(&mask);
    sigaddset(&mask,SIGCHLD);
    sigprocmask(SIG_BLOCK, &mask, &prevmask);*/

    Pipe(this->sendpipe_);
    Pipe(this->recvpipe_);
    pid_t pid;
    if((pid = Fork()) == 0) /* child process */
    {
        char buf[this->filename_.size() + 5];
        strcpy(buf,this->filename_.c_str());
        char *argv[] = {buf,NULL};

        dup2(sendpipe_[0],0);   /* dup parent's send pipe to stdin */
        close(sendpipe_[1]);    /* close write side for child */
        dup2(recvpipe_[1],1);   /* dup parent's recv pipe to stdout*/
        close(recvpipe_[0]);    /* close read side for child */
        if(execvp(buf,argv) < 0)
        {
            fprintf(stderr,"Cannot open file %s\n",buf);
            return;
        }
    }

    /* Block the SIGCHLD and do some initializations */
    close(sendpipe_[0]);        /* close read side of send pipe */
    close(recvpipe_[1]);        /* close write side of recv pipe*/
    this->state_ = RUNNING;
    this->pid_ = pid;

    //sigprocmask(SIG_SETMASK, &prevmask, NULL);
    
    UpdateState();
    /* Everything should working from here! */
}


void CppAgent::Close()
{
    switch(this->state_)
    {
        case CLOSED:
            unix_warning("Try to close an already-closed agent");
            return;
        case UNINITIALIZED:
            unix_error("(Err) Cannot close an UNINITIALIZED agent");
            return;
        case RUNNING:
            unix_warning("Try to shutdown communication pipe during RUNNING");
        default:
            break;
    }

    this->state_ = CLOSED;
    close(sendpipe_[1]);
    close(recvpipe_[0]);
}

void CppAgent::Kill()
{
    UpdateState();
    if(this->state_ != RUNNING)
    {
        unix_warning("Cannot kill child not being running!");
        return;
    }

    /*sigset_t mask,prevmask;
    sigemptyset(&mask);
    sigaddset(&mask,SIGCHLD);
    sigprocmask(SIG_BLOCK, &mask, &prevmask);*/
    int ret = kill(this->pid_, 9);
    //sigprocmask(SIG_SETMASK, &prevmask, NULL);

    if(ret)
        unix_warning("Kill failed, something may go wrong");
    this->state_ = FINISHED;

}

int CppAgent::GetState()
{
    UpdateState();
    return this->state_;
}


void CppAgent::ReceiveInput(const Information_t &input)
{
    //send input to child using sendpipe
    /*sigset_t mask,prevmask;
    sigemptyset(&mask);
    sigaddset(&mask,SIGCHLD);
    sigprocmask(SIG_BLOCK, &mask, &prevmask);*/

    write(sendpipe_[1], input.c_str(), input.size());
    write(sendpipe_[1], "\n", 1);

    //sigprocmask(SIG_SETMASK, &prevmask, NULL);
}

Respond_t CppAgent::SendResponse()
{
    /*sigset_t mask,prevmask;
    sigemptyset(&mask);
    sigaddset(&mask,SIGCHLD);
    sigprocmask(SIG_BLOCK, &mask, &prevmask);*/

    static ssize_t buffer_size = 2*1024; /* 2KB buffer */
    char buf[buffer_size + 1];
    Respond_t ret;
    ssize_t cnt;
    do
    {
        cnt = read(recvpipe_[0], buf, buffer_size);
        for(int i=0;i<cnt;i++)
            ret.push_back(buf[i]);
    }while(cnt == buffer_size);

    //sigprocmask(SIG_SETMASK, &prevmask, NULL);
    return ret;
}

