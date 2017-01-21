#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <string>

namespace tsync
{

    struct Token
    {
        uint64_t pointer;
        int32_t place_id;
    };

    class BasicEvent
    {
        public:
            BasicEvent(char, uint64_t);
            void SetTime(uint64_t);
            uint64_t GetTime();
            char GetType();
            virtual void StoreToFile(FILE*);

        protected:
            char type;
            uint64_t time;
    };

    class TokenEvent : public BasicEvent
    {
        public:
            TokenEvent(char, uint64_t);
            void SetId(int32_t);
            int32_t GetId();
            void AddToken(Token);
            void AddInt(int32_t);
            void AddDouble(double);
            void AddString(std::string);

            void StoreToFile(FILE*) override;

        protected:
            virtual void StoreOwnData(FILE*);

        private:
            int32_t id;
            std::vector<Token> tokens;
            std::vector<int32_t> ints;
            std::vector<double> doubles;
            std::vector<std::string> strings;
    };

    class SendEvent : public BasicEvent
    {
        public:
            SendEvent(uint64_t);
            void SetSize(uint64_t);
            void SetEdge(int32_t);
            void AddTarget(int32_t);
            void UpdateRecvTime(uint64_t);
            uint64_t GetRecvTime();
            uint64_t GetMaxOffset();
            std::vector<int32_t>::const_iterator Tcbegin();
            std::vector<int32_t>::const_iterator Tcend();

            virtual void StoreToFile(FILE*) override;

        private:
            uint64_t msg_size;
            int32_t edge_id;
            int32_t tnum;
            std::vector<int32_t> targets;
            uint64_t received_time;
    };

    class ReceiveEvent : public TokenEvent
    {
        public:
            ReceiveEvent(uint64_t, int32_t);
            int32_t GetSender();
            void SetGap(uint64_t);
            uint64_t GetGap();

        private:
            uint64_t gap;
    };

    class TransitionEvent : public TokenEvent
    {
        public:
            using TokenEvent::TokenEvent;
            void AddTransitionToken(Token);
            void AddTransitionInt(int32_t);
            void AddTransitionDouble(double);
            void AddTransitionString(std::string);

        protected:
            virtual void StoreOwnData(FILE*) override;

        private:
            std::vector<Token> t_tokens;
            std::vector<int32_t> t_ints;
            std::vector<double> t_doubles;
            std::vector<std::string> t_strings;
    };

}
