#include "mpitracelog.h"
#include <stdlib.h>
#include <errno.h>
#include <stdexcept>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>

using namespace tsync;

uint64_t MpiTracelog::CollectSentTime(ReceiveEvent * event)
{
    MPI_Request req;
    uint64_t senttime;
    MPI_Irecv( &senttime, 1, MPI_UINT64_T, event->GetSender(), MPI_TSYNC_SENTTIME, MPI_COMM_WORLD, &req );
    MPI_Wait( &req, MPI_STATUS_IGNORE );

    return senttime;
}

void MpiTracelog::ForwardSentTime(SendEvent * event)
{
    uint64_t time = event->GetTime();
    // Efficiency of looping all recipients? Current tracelog structure does not allow
    // better solution. Moreover, it seems that <t_num> is always one thus a collective
    // msg is separated into a sequence of msgs within tracelog
    for (auto i = event->Tcbegin(); i != event->Tcend(); i++)
    {
        // Send sent time
        MPI_Request sreq;
        MPI_Isend( &time, 1, MPI_UINT64_T, *i, MPI_TSYNC_SENTTIME, MPI_COMM_WORLD, &sreq );
        MPI_Request_free(&sreq);

        // Recieve received time
        RecvReq rreq;
        rreq.event = event;
        MPI_Irecv( &rreq.buf, 1, MPI_UINT64_T, *i, MPI_TSYNC_RECVTIME, MPI_COMM_WORLD, &rreq.req );
        this->requests.push_back(rreq);

        // Completing received RECVTIME requests
        auto req = this->requests.begin();
        int completed = 0;
        do
        {
            RecvReq rq = *req;
            int flag;
            MPI_Test( &rq.req, &flag, MPI_STATUS_IGNORE );
            if (!flag)
            {
                break;
            }
            rq.event->UpdateRecvTime(rq.buf);
            completed++;
            req++;
        } while ( req != this->requests.end() );

        // Delete completed
        for (int n = 0; n < completed; n++)
        {
            this->requests.pop_front();
        }
    }
}

void MpiTracelog::ForwardRecvTime(ReceiveEvent * event)
{
    uint64_t time = event->GetTime();
    MPI_Request sreq;
    MPI_Isend( &time, 1, MPI_UINT64_T, event->GetSender(), MPI_TSYNC_RECVTIME, MPI_COMM_WORLD, &sreq );
    MPI_Request_free(&sreq);
}

void MpiTracelog::PrepareBackwardAmortization()
{
    for (auto req = this->requests.begin(); req != this->requests.end(); req++)
    {
        RecvReq rq = *req;
        MPI_Wait( &rq.req, MPI_STATUS_IGNORE );
        rq.event->UpdateRecvTime(rq.buf);
    }

    this->requests.clear();
}

void MpiWizard::Run(int argc, char * argv[])
{
    MPI_Init( NULL, NULL );

    int pcount;
    int pid;
    MPI_Comm_size(MPI_COMM_WORLD, &pcount);
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);

    if (argc < 4)
    {
        if (pid == 0)
        {
            printf("Incorrect number of passed arguments!\nMandatory arguments:\n"
            "\t 1. Path to a folder with tracelogs (also used as a save folder)\n"
            "\t 2. Minimum event difference [nanoseconds]\n"
            "\t 3. Minimal message delay [nanoseconds]\n"
            "\nOptional arguments:\n"
            "\t 4. Align spawn times [0 (default)/1]\n");
        }
        exit(0);
    }

    // Source path
    std::string path(argv[1]);
    this->AdjustPath(&path);

    // Save path
    std::string dest(path);
    dest.append("synced");
    if (pid == 0) this->MakeDir(dest.c_str());
    dest.push_back('/');

    MpiTracelog t (path.c_str(), pid, atoi(argv[2]), atoi(argv[3]));
    t.Load();

    this->AlignSpawnTimes(&t, pcount, argc, argv);

    t.Sync();
    t.Store(dest.c_str());


    MPI_Finalize();
}

void MpiWizard::AlignSpawnTimes(MpiTracelog * t, const int p_count, int argc, char * argv[])
{
    if (argc < 5) return;
    if (argv[4][0] != '1') return;

    uint64_t it = t->GetInitTime();
    uint64_t spawntime = t->GetNextEventTime();
    uint64_t times[p_count];

    MPI_Gather( &spawntime, 1, MPI_UINT64_T, times, 1, MPI_UINT64_T, 0, MPI_COMM_WORLD );

    uint64_t maxt = times[0];
    int maxt_pid = 0;
    for (int i = 1; i < p_count; i++)
    {
        if (times[i] > maxt)
        {
            maxt = times[i];
            maxt_pid = i;
        }
    }

    MPI_Bcast( &it, 1, MPI_UINT64_T, maxt_pid, MPI_COMM_WORLD);
    MPI_Bcast( &maxt, 1, MPI_UINT64_T, 0, MPI_COMM_WORLD );

    t->SetInitTime(it);
    t->SetTimeOffset( maxt - spawntime );
}

void MpiWizard::AdjustPath(std::string * path)
{
    if (path->back() != '/') path->push_back('/');
}

void MpiWizard::MakeDir(const char * path)
{
    struct stat sb;
    if ( stat(path, &sb) != 0 || !S_ISDIR(sb.st_mode) )
    {
        if ( mkdir(path, 0700) != 0 )
        {
            std::ostringstream exptn_text;
            exptn_text << "Cannot create a folder for result! Path: '" << path << "'";
            throw std::invalid_argument(exptn_text.str());
        }
    }
}
