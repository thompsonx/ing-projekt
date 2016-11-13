#include "event.h"

using namespace tsync;

/* Event Class */

BasicEvent::BasicEvent(char type, uint64_t time)
{
    this->type = type;
    this->time = time;
}

void BasicEvent::SetTime(uint64_t time)
{
    this->time = time;
}

uint64_t BasicEvent::GetTime()
{
    return this->time;
}

char BasicEvent::GetType()
{
    return this->type;
}

void BasicEvent::StoreToFile(FILE* f)
{
    fwrite( &this->type, sizeof(char), 1, f );
    fwrite( &this->time, sizeof(uint64_t), 1, f );
}


/* TokenEvent Class */

TokenEvent::TokenEvent(char type, uint64_t time):BasicEvent(type, time)
{
    this->id = -1;
}

void TokenEvent::SetId(int32_t id)
{
    this->id = id;
}

int32_t TokenEvent::GetId()
{
    return this->id;
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

void TokenEvent::StoreToFile(FILE* f)
{
    BasicEvent::StoreToFile(f);

    if (this->id >= 0)
    {
        fwrite( &this->id, sizeof(int32_t), 1, f );
    }

    this->StoreOwnData(f);

    for (size_t i = 0; i < this->tokens.size(); i++)
    {
        fwrite( "t", 1, 1, f );
        Token t = this->tokens[i];
        fwrite( &t.pointer, sizeof(uint64_t), 1, f );
        fwrite( &t.place_id, sizeof(int32_t), 1, f );
    }

    for (size_t i = 0; i < this->ints.size(); i++)
    {
        fwrite ( "i", 1, 1, f );
        fwrite ( &this->ints[i], sizeof(int32_t), 1, f );
    }

    for (size_t i = 0; i < this->doubles.size(); i++)
    {
        fwrite ( "d", 1, 1, f );
        fwrite ( &this->doubles[i], sizeof(double), 1, f );
    }

    for (size_t i = 0; i < this->strings.size(); i++)
    {
        fwrite ( "s", 1, 1, f );
        fwrite ( this->strings[i].c_str(), sizeof(char), this->strings[i].size() + 1, f );
    }
}

void TokenEvent::StoreOwnData(FILE*) {}


/* SendEvent Class */

SendEvent::SendEvent(uint64_t time):BasicEvent('M', time)
{
    this->tnum = 0;
}

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
    this->tnum++;
}

void SendEvent::StoreToFile(FILE* f)
{
    BasicEvent::StoreToFile(f);

    fwrite( &this->msg_size, sizeof(uint64_t), 1, f );
    fwrite( &this->edge_id, sizeof(int32_t), 1, f );
    fwrite( &this->tnum, sizeof(int32_t), 1, f );

    for (int i = 0; i < tnum; i++)
    {
        fwrite( &this->targets[i], sizeof(int32_t), 1, f );
    }
}



/* ReceiveEvent Class */

ReceiveEvent::ReceiveEvent(uint64_t time, int32_t sender_id):TokenEvent('R', time)
{
    this->SetId(sender_id);
}

int32_t ReceiveEvent::GetSender()
{
    return this->GetId();
}

void ReceiveEvent::SetGap(uint64_t gap)
{
    this->gap = gap;
}

uint64_t ReceiveEvent::GetGap()
{
    return this->gap;
}


/* TransitionEvent Class */

TransitionEvent::TransitionEvent(char type, uint64_t time):TokenEvent(type, time) {}

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

void TransitionEvent::StoreOwnData(FILE* f)
{
    for (size_t i = 0; i < this->t_tokens.size(); i++)
    {
        fwrite( "r", 1, 1, f );
        Token r = this->t_tokens[i];
        fwrite( &r.pointer, sizeof(uint64_t), 1, f );
        fwrite( &r.place_id, sizeof(int32_t), 1, f );
    }

    for (size_t i = 0; i < this->t_ints.size(); i++)
    {
        fwrite ( "i", 1, 1, f );
        fwrite ( &this->t_ints[i], sizeof(int32_t), 1, f );
    }

    for (size_t i = 0; i < this->t_doubles.size(); i++)
    {
        fwrite ( "d", 1, 1, f );
        fwrite ( &this->t_doubles[i], sizeof(double), 1, f );
    }

    for (size_t i = 0; i < this->t_strings.size(); i++)
    {
        fwrite ( "s", 1, 1, f );
        fwrite ( this->t_strings[i].c_str(), sizeof(char), this->t_strings[i].size() + 1, f );
    }
}
