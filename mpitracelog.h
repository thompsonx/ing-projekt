#include "tracelog.h"
#include <mpi.h>
#include <list>

#define MPI_TSYNC_SENTTIME 100
#define MPI_TSYNC_RECVTIME 101

namespace tsync
{
    struct RecvReq
    {
        uint64_t buf;
        MPI_Request req;
        SendEvent * event;
    };


    class MpiTracelog : public Tracelog
    {
        public:
            using Tracelog::Tracelog;

        protected:
            virtual uint64_t CollectSentTime(ReceiveEvent *) override;
            virtual void ForwardSentTime(SendEvent *) override;
            virtual void ForwardRecvTime(ReceiveEvent *) override;
            virtual void PrepareBackwardAmortization() override;

        private:
            std::list<RecvReq> requests;
    };


    class MpiWizard
    {

        public:
            void AlignSpawnTimes(MpiTracelog *, const int, int, char **);
            void AdjustPath(std::string *);
            void MakeDir(const char *);
            void Run(int, char **);

    };

}
