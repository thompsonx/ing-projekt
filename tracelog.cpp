#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <inttypes.h>
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include "tracelog.h"

using namespace tsync;

Tracelog::Tracelog(const char * path, int process_id, int min_evnt_dif, int min_msg_dly)
{
    this->path = path;
    this->process_id = process_id;
    this->min_evnt_dif = min_evnt_dif;
    this->min_msg_dly = min_msg_dly;
    this->last_violating_index = -1;
    this->loaded = false;
    this->synced = false;

    this->filename.append("trace-");
    this->filename.append(std::to_string(process_id));
    this->filename.append("-0.ktt");

    this->filepath.append(path);
    this->filepath.append(filename);
}

Tracelog::~Tracelog()
{
    for (auto i = this->events.begin(); i != this->events.end(); i++)
    {
        delete (*i);
    }
    this->events.clear();
}

int Tracelog::GetPointerPos()
{
    return this->pointer - &this->data[0];
}

void Tracelog::Load()
{
    if (this->loaded) return;

    FILE *fp = fopen(this->filepath.c_str(), "rb");
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
        std::ostringstream exptn_text;
        exptn_text << "Cannot open tracelog file! Path: '" << this->filepath << "'";
        throw std::invalid_argument(exptn_text.str());
    }

    // Read header
    std::string tmp;
    char block[2] = { 1, 1 };
    this->pointer = &this->data[0];
    while ( (block[0] != 0) || (block[1] != 0) )
    {
        block[0] = block[1];
        block[1] = *this->pointer;
        if (!block[1] && !tmp.empty())
        {
            this->header_data.push_back(tmp);
            tmp.clear();
        }
        else if (block[1]) tmp.push_back(block[1]);
        this->pointer++;
    }
    this->pointer++;

    // Parse initial time
    std::string time = this->header_data.back();
    this->inittime = strtoull( time.c_str(), nullptr, 10 );

    this->loaded = true;
}

void Tracelog::Store(const char * dest)
{
    // Prepare store location
    std::string store_path(dest);
    store_path.append(this->filename);

    FILE *fp = fopen(store_path.c_str(), "wb");
    // Store tracelog header
    for (auto str = this->header_data.begin(); str != this->header_data.end(); str++)
    {
        fwrite( (*str).c_str(), sizeof(char), (*str).size() + 1, fp );
    }
    char padding[2] = {0, 0};
    fwrite( padding, sizeof(char), 2, fp );
    // Store events
    for (auto i = this->events.begin(); i != this->events.end(); i++)
    {
        (*i)->StoreToFile(fp);
    }
    fclose(fp);
}

void Tracelog::Sync()
{
    if (this->synced) return;

    while (!this->IsEndReached())
    {
        this->ProcessEvent();
    }
    this->PrepareBackwardAmortization();
    this->BackwardAmortization();

    this->synced = true;
}

int Tracelog::GetPID()
{
    return this->process_id;
}

void Tracelog::SetTimeOffset(uint64_t offset)
{
    this->time_offset = offset;
}

uint64_t Tracelog::GetTimeOffset()
{
    return this->time_offset;
}

void Tracelog::SetInitTime(uint64_t it)
{
    this->inittime = it;
    std::ostringstream oss;
    oss << it;
    std::string time = oss.str();
    this->header_data.pop_back();
    this->header_data.push_back(time);
}

uint64_t Tracelog::GetInitTime()
{
    return this->inittime;
}

uint64_t Tracelog::GetNextEventTime()
{
    if (!this->loaded)
    {
        throw std::invalid_argument("Tracelog is not loaded! Cannot get the event time.");
    }

    char * ptr = this->pointer;
    this->pointer++;
    uint64_t time = this->ReadUint64();
    this->pointer = ptr;

    return time;
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

void Tracelog::ProcessTokensAdd(TokenEvent * event)
{
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

void Tracelog::PESend()
{
    SendEvent * event = new SendEvent(this->ReadUint64(), this->min_msg_dly);
    event->SetSize(this->ReadUint64());
    event->SetEdge(this->ReadInt32());
    this->Synchronize(event);

    int32_t targets = this->ReadInt32();
    int32_t target_ids[targets];
    for (int32_t i = 0; i < targets; i++)
    {
        target_ids[i] = this->ReadInt32();
        event->AddTarget(target_ids[i]);
    }
    this->ForwardSentTime(event);
}

void Tracelog::PEQuit()
{
    char t = *this->pointer;
    if (t != 'Q')
    {
        return;
    }
    this->pointer++;

    BasicEvent * event = new BasicEvent('Q', this->ReadUint64());
    this->Synchronize(event);
}

void Tracelog::PEEnd()
{
    char t = *this->pointer;
    if (t != 'X')
    {
        return;
    }
    this->pointer++;

    BasicEvent * event = new BasicEvent('X', this->ReadUint64());
    this->Synchronize(event);
}

void Tracelog::PETransitionFired()
{
    TransitionEvent * event = new TransitionEvent('T', this->ReadUint64());
    this->Synchronize(event);
    event->SetId(this->ReadInt32());
    this->ReadTransitionTraceFunctionData(event);
    this->PEQuit();
    this->ProcessTokensAdd(event);
    this->PEEnd();
}

void Tracelog::PETransitionFinished()
{
    TokenEvent * event = new TokenEvent('F', this->ReadUint64());
    this->Synchronize(event);

    this->PEQuit();
    this->ProcessTokensAdd(event);
    this->PEEnd();
}

void Tracelog::PEReceive()
{
    uint64_t time = this->ReadUint64();
    int32_t sender = this->ReadInt32();
    ReceiveEvent * event = new ReceiveEvent(time, sender);
    this->SynchronizeRecv(event);

    this->ProcessTokensAdd(event);
    this->PEEnd();
}

void Tracelog::PESpawn()
{
    uint64_t time = this->ReadUint64() + this->time_offset;
    TokenEvent * event = new TokenEvent('S', time);
    event->SetId(this->ReadInt32());
    // No sync because 'spawn' is an initial event, nothing precedes it
    this->events.push_back(event);
    this->ProcessTokensAdd(event);
}

void Tracelog::PEIdle()
{
    BasicEvent * event = new BasicEvent('I', this->ReadUint64());
    this->Synchronize(event);
}

void Tracelog::Synchronize(BasicEvent * event)
{
    uint64_t time = event->GetTime() + this->time_offset;
    uint64_t prev = this->events.back()->GetTime() + this->min_evnt_dif;

    if (prev > time)
    {
        // Preserve original event spacing (current event and the next)
        uint64_t offset = prev - time;
        this->time_offset += offset;
        time = prev;
    }

    event->SetTime(time);

    this->events.push_back(event);
}

void Tracelog::SynchronizeRecv(ReceiveEvent * event)
{
    uint64_t times[3];
    uint64_t time = event->GetTime() + this->time_offset;
    times[0] = time;
    times[1] = this->events.back()->GetTime() + this->min_evnt_dif;
    times[2] = this->CollectSentTime(event) + this->min_msg_dly;

    uint64_t synced_time = *std::max_element( times, times + 3 );

    if ( synced_time > time )
    {
        uint64_t diff = synced_time - time;
        this->time_offset += diff;
        event->SetGap(diff);
        this->violating.push_back(event);
        this->last_violating_index = this->events.size();
    }

    event->SetTime(synced_time);
    this->ForwardRecvTime(event);
    this->events.push_back(event);
}

void Tracelog::BackwardAmortization()
{
    if ( this->violating.empty() )
        return;

    ReceiveEvent * v_recv = this->violating.back();
    uint64_t offset = v_recv->GetGap();
    this->violating.pop_back();

    for (int64_t i = this->last_violating_index - 1; i >= 0; --i)
    {
        BasicEvent * e = this->events[i];
        if (e->GetType() == 'M')
        {
            SendEvent * se = (SendEvent *) e;
            uint64_t max_offset = se->GetMaxOffset();
            if (max_offset < offset)
            {
                offset = max_offset;
            }
        }

        uint64_t tmp_time = e->GetTime();
        e->SetTime( tmp_time + offset );

        if ( !this->violating.empty() && (e == this->violating.back()) )
        {
            v_recv = this->violating.back();
            offset += v_recv->GetGap();
            this->violating.pop_back();
        }
    }
}
