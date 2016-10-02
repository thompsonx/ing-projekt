#include "event.h"

using namespace tsync;

/* Event Class */

BasicEvent::BasicEvent(char type)
{
    this->type = type;
}

void BasicEvent::SetTime(uint64_t time)
{
    this->time = time;
}


/* TokenEvent Class */

TokenEvent::TokenEvent(char type):BasicEvent(type) {}

void TokenEvent::SetId(int32_t id)
{
    this->id = id;
}

void TokenEvent::AddToken(Token t)
{
    this->tokens.push_back(t);
}

void TokenEvent::AddInt(int32_t i)
{
    this->ints.push_back(i);
}

void TokenEvent::AddDouble(double d)
{
    this->doubles.push_back(d);
}

void TokenEvent::AddString(std::string s)
{
    this->strings.push_back(s);
}


/* SendEvent Class */

SendEvent::SendEvent(char type):BasicEvent(type) {}

void SendEvent::SetSize(uint64_t s)
{
    this->msg_size = s;
}

void SendEvent::SetEdge(int32_t e)
{
    this->edge_id = e;
}

void SendEvent::AddTarget(int32_t t)
{
    this->targets.push_back(t);
}

/* TransitionEvent Class */

TransitionEvent::TransitionEvent(char type):TokenEvent(type) {}

void TransitionEvent::AddTransitionToken(Token t)
{
    this->t_tokens.push_back(t);
}

void TransitionEvent::AddTransitionInt(int32_t i)
{
    this->t_ints.push_back(i);
}

void TransitionEvent::AddTransitionDouble(double d)
{
    this->t_doubles.push_back(d);
}

void TransitionEvent::AddTransitionString(std::string s)
{
    this->t_strings.push_back(s);
}
