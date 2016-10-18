#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include <stdexcept>
#include <sstream>
#include "tracelog.h"

//TODO: necist a jen preskakovat neoupizavana data, nelze u stringu - neznam delku, jak ukladat prectena data?

using namespace tsync;

Tracelog::Tracelog(char * filepath, int process_id, int min_evnt_dif, int min_msg_dly)
{
    this->filepath = filepath;
    this->process_id = process_id;
    this->min_evnt_dif = min_evnt_dif;
    this->min_msg_dly = min_msg_dly;
}

int Tracelog::GetPointerPos()
{
    return this->pointer - &this->data[0];
}

void Tracelog::Load()
{
    FILE *fp = fopen(this->filepath, "rb");
    //FILE *fp = fopen("/home/tom/Dropbox/VSB/ING/Projekt/git/ing-projekt/bin/Debug/trace-0-0.ktt", "rb");
    if (fp)
    {
        std::string content;

        fseek(fp, 0, SEEK_END);
        content.resize(ftell(fp));
        rewind(fp);

        fread(&content[0], 1, content.size(), fp);
        fclose(fp);

        this->data = content;
    }
    else {
        throw errno;
    }

    char block[2] = { 1, 1 };
    this->pointer = &this->data[0];
    while ( (block[0] != 0) || (block[1] != 0) )
    {
        block[0] = block[1];
        block[1] = *this->pointer;
        this->pointer++;
    }
    this->header_end = this->pointer - &this->data[0];
    this->pointer++;
}

void Tracelog::Store()
{
    //TODO: remove fixed name
    FILE *fp = fopen("test.ktt", "wb");
    // Store tracelog header
    fwrite(&this->data[0], sizeof(char), this->header_end + 1, fp);
    // Store events
    for (size_t i = 0; i < this->events.size(); i++)
    {
        this->events[i]->StoreToFile(fp);
    }
    fclose(fp);
}

void Tracelog::Sync()
{
//    for (int i = 0; i < 5; i++)
//    {
//        this->ProcessEvent();
//    }
    while (!this->IsEndReached())
    {
        this->ProcessEvent();
    }
}

void Tracelog::SetTimeOffset(uint64_t offset)
{
    this->time_offset = offset;
}

uint64_t Tracelog::GetTimeOffset()
{
    return this->time_offset;
}

bool Tracelog::IsEndReached()
{
    return this->pointer >= &this->data.back();
}

uint64_t Tracelog::ReadUint64()
{
    uint64_t value;
    memcpy(&value, this->pointer, sizeof(uint64_t));
    this->pointer += sizeof(uint64_t);
    return value;
}

int32_t Tracelog::ReadInt32()
{
    int32_t value;
    memcpy(&value, this->pointer, sizeof(int32_t));
    this->pointer += sizeof(int32_t);
    return value;
}

double Tracelog::ReadDouble()
{
    double value;
    memcpy(&value, this->pointer, sizeof(double));
    this->pointer += sizeof(double);
    return value;
}

std::string Tracelog::ReadString()
{
    std::string value;
    while ((*this->pointer) != 0)
    {
        value += (*this->pointer);
        this->pointer++;
    }
    this->pointer++;
    return value;
}

void Tracelog::ProcessEvent()
{
    char t = *this->pointer;
    this->pointer++;
    //TODO: extra event
    switch (t)
    {
        case 'T':
            this->PETransitionFired();
            break;
        case 'F':
            this->PETransitionFinished();
            break;
        case 'R':
            this->PEReceive();
            break;
        case 'S':
            this->PESpawn();
            break;
        case 'I':
            this->PEIdle();
            break;
        case 'Q':
            this->PEQuit();
            break;
        default:
            std::ostringstream exptn_text;
            exptn_text << "Invalid event type '" << t << "' at position " << this->GetPointerPos() - 1 << " in process " << this->process_id << ".";
            throw std::invalid_argument(exptn_text.str());
    }
}

void Tracelog::PESend()
{
    SendEvent * event = new SendEvent('M');
    uint64_t time = this->ReadUint64();
    event->SetTime(time);
    event->SetSize(this->ReadUint64());
    event->SetEdge(this->ReadInt32());
    //TODO: Sync
    int32_t targets = this->ReadInt32();
    int32_t target_ids[targets];
    for (int32_t i = 0; i < targets; i++)
    {
        target_ids[i] = this->ReadInt32();
        event->AddTarget(target_ids[i]);
        //TODO: extra_event_send
    }
    this->events.push_back(event);
}

void Tracelog::ProcessTokensAdd(TokenEvent * event)
{
    //TODO: data storing
    while (!this->IsEndReached())
    {
        char t = *this->pointer;
        this->pointer++;
        if (t == 't')
        {
            Token tkn;
            tkn.pointer = this->ReadUint64();
            tkn.place_id = this->ReadInt32();
            event->AddToken(tkn);
        }
        else if (t == 'i')
        {
            event->AddInt(this->ReadInt32());
        }
        else if (t == 'd')
        {
            event->AddDouble(this->ReadDouble());
        }
        else if (t == 's')
        {
            event->AddString(this->ReadString());
        }
        else if (t == 'M')
        {
            this->PESend();
        }
        else
        {
            this->pointer--;
            break;
        }
    }
}

void Tracelog::ReadTransitionTraceFunctionData(TransitionEvent * event)
{
    while (!this->IsEndReached())
    {
        char t = *this->pointer;
        this->pointer++;
        if (t == 'r')
        {
            Token tkn;
            tkn.pointer = this->ReadUint64();
            tkn.place_id = this->ReadInt32();
            event->AddTransitionToken(tkn);
        }
        else if (t == 'i')
        {
            event->AddTransitionInt(this->ReadInt32());
        }
        else if (t == 'd')
        {
            event->AddTransitionDouble(this->ReadDouble());
        }
        else if (t == 's')
        {
            event->AddTransitionString(this->ReadString());
        }
        else
        {
            this->pointer--;
            break;
        }
    }
}

void Tracelog::PEQuit()
{
    char t = *this->pointer;
    if (t != 'Q')
    {
        return;
    }
    BasicEvent * event = new BasicEvent('Q');
    //TODO: store data
    this->pointer++;
    uint64_t time = this->ReadUint64();
    //TODO: sync
    event->SetTime(time);
    this->events.push_back(event);
}

void Tracelog::ProcessEnd()
{
    char t = *this->pointer;
    if (t != 'X')
    {
        return;
    }
    BasicEvent * event = new BasicEvent('X');
    //TODO: store data
    this->pointer++;
    uint64_t time = this->ReadUint64();
    //TODO: sync
    event->SetTime(time);
    this->events.push_back(event);
}

void Tracelog::PETransitionFired()
{
    TransitionEvent * event = new TransitionEvent('T');
    //TODO: store data and sync
    uint64_t time = this->ReadUint64();
    event->SetTime(time);
    event->SetId(this->ReadInt32());
    this->events.push_back(event);
    this->ReadTransitionTraceFunctionData(event);
    this->PEQuit();
    this->ProcessTokensAdd(event);
    this->ProcessEnd();
}

void Tracelog::PETransitionFinished()
{
    TokenEvent * event = new TokenEvent('F');
    //TODO: store data and sync
    uint64_t time = this->ReadUint64();
    event->SetTime(time);
    this->events.push_back(event);
    this->PEQuit();
    this->ProcessTokensAdd(event);
    this->ProcessEnd();
}

void Tracelog::PEReceive()
{
    TokenEvent * event = new TokenEvent('R');
    //TODO: store data and sync
    uint64_t time = this->ReadUint64();
    event->SetTime(time);
    event->SetId(this->ReadInt32());
    this->events.push_back(event);
    this->ProcessTokensAdd(event);
    this->ProcessEnd();
}

void Tracelog::PESpawn()
{
    TokenEvent * event = new TokenEvent('S');
    uint64_t time = this->ReadUint64();
    event->SetTime(time);
    event->SetId(this->ReadInt32());
    this->events.push_back(event);
    //TODO: sync
    this->ProcessTokensAdd(event);
}

void Tracelog::PEIdle()
{
    BasicEvent * event = new BasicEvent('I');
    uint64_t time = this->ReadUint64();
    //TODO: save and sync
    event->SetTime(time);
    this->events.push_back(event);
}
