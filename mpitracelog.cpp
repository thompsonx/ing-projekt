#include "mpitracelog.h"
#include <stdlib.h>

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

        // TODO: pravidelnost kontroly? velikosti bufferu MPI?
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

    if (argc != 4)
    {
        if (!pid)
        {
            printf("Incorrect number of passed arguments!\nMandatory arguments:\n"
            "\t 1. Path to a folder with tracelogs (also used as a save folder)\n"
            "\t 2. Minimum event difference [nanoseconds]\n"
            "\t 3. Minimal message delay [nanoseconds]\n");
        }
        exit(0);
    }

    MpiTracelog t (argv[1], pid, atoi(argv[2]), atoi(argv[3]));
    t.Load();
    t.Sync();
    t.Store();


    MPI_Finalize();
}
