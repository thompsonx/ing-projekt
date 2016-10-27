#include <string>
#include <stdint.h>
#include "event.h"

namespace tsync
{

    class Tracelog
    {

        public:
            Tracelog(char *, int, int, int);
            void Load();
            bool IsEndReached();
            int GetPointerPos();
            void Sync();
            void Store();

            void SetTimeOffset(uint64_t);
            uint64_t GetTimeOffset();

            virtual void ParsePath(char *, std::string *, std::string *);

        protected:
            virtual void MakeDir(const char *);

        private:
            char * filepath;
            std::string path;
            std::string filename;
            int process_id;
            int min_evnt_dif;
            int min_msg_dly;
            std::string data;
            char * pointer;
            int header_end;
            uint64_t time_offset;

            std::vector<BasicEvent*> events;
            void Synchronize(BasicEvent *);

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
