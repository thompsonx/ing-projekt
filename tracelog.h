#include <string>
#include <stdint.h>

namespace tsync
{

    class Tracelog
    {

        public:
            Tracelog(char *, int, int, int);
            void Load();
            bool IsEndReached();
            int GetPointerPos();

            void SetTimeOffset(uint64_t);
            uint64_t GetTimeOffset();

        private:
            char * filepath;
            int process_id;
            int min_evnt_dif;
            int min_msg_dly;
            std::string data;
            char * pointer;
            int header_end;
            uint64_t time_offset;

            uint64_t ReadUint64();
            int32_t ReadInt32();
            double ReadDouble();
            const char * ReadString();

            void PESend();
            void ProcessTokensAdd();
            void ProcessEvent();
            void PETransitionFired();
            void PESpawn();

    };

}
