#include <string>
#include <stdint.h>
#include <list>
#include "event.h"

namespace tsync
{

    class Tracelog
    {

        public:
            Tracelog(char *, int, int, int);
            ~Tracelog();
            void Load();
            bool IsEndReached();
            int GetPointerPos();
            void Sync();
            void Store();

            void SetTimeOffset(uint64_t);
            uint64_t GetTimeOffset();

        protected:
            int process_id;
            virtual void MakeDir(const char *);
            virtual uint64_t CollectSentTime(ReceiveEvent *)=0;
            virtual void ForwardSentTime(SendEvent *)=0;
            virtual void ForwardRecvTime(ReceiveEvent *)=0;
            virtual void PrepareBackwardAmortization()=0;

        private:
            std::string filepath;
            char * path;
            std::string filename;
            int min_evnt_dif;
            int min_msg_dly;
            std::string data;
            char * pointer;
            int header_end;
            uint64_t time_offset;

            std::vector<BasicEvent*> events;
            std::list<ReceiveEvent*> violating;
            size_t last_violating_index;
            void Synchronize(BasicEvent *);
            void SynchronizeRecv(ReceiveEvent *);
            void BackwardAmortization();

            bool synced;
            bool loaded;

            uint64_t ReadUint64();
            int32_t ReadInt32();
            double ReadDouble();
            std::string ReadString();

            void ReadTransitionTraceFunctionData(TransitionEvent *);
            void PESend();
            void ProcessTokensAdd(TokenEvent *);
            void PEEnd();
            void PEQuit();
            void PETransitionFired();
            void PETransitionFinished();
            void PESpawn();
            void PEReceive();
            void PEIdle();
            void ProcessEvent();
    };

}
