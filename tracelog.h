#include <string>

namespace tsync
{

    class Tracelog
    {

        public:
            Tracelog(char *, int, int, int);
            void Load();

        private:
            char * filepath;
            int process_id;
            int min_evnt_dif;
            int min_msg_dly;
            std::string data;
            char * pointer;
    };

}
