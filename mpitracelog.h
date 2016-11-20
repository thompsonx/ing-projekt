#include "tracelog.h"
//#include <mpi.h>

namespace tsync
{
    class MpiTracelog : public Tracelog
    {
        public:
            using Tracelog::Tracelog;

        protected:
            virtual uint64_t CollectSentTime(ReceiveEvent *) override;
            virtual void ForwardSentTime(SendEvent *) override;
            virtual void PrepareBackwardAmortization() override;

    };
}
