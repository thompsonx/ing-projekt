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
            BasicEvent(char);
            void SetTime(uint64_t);
            virtual void StoreToFile(FILE*);

        protected:
            char type;
            uint64_t time;
    };

    class TokenEvent : public BasicEvent
    {
        public:
            TokenEvent(char);
            void SetId(int32_t);
            void AddToken(Token);
            void AddInt(int32_t);
            void AddDouble(double);
            void AddString(std::string);

            void StoreToFile(FILE*);

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
            SendEvent(char);
            void SetSize(uint64_t);
            void SetEdge(int32_t);
            void AddTarget(int32_t);

            void StoreToFile(FILE*);

        private:
            uint64_t msg_size;
            int32_t edge_id;
            int32_t tnum;
            std::vector<int32_t> targets;
    };

    class TransitionEvent : public TokenEvent
    {
        public:
            TransitionEvent(char);
            void AddTransitionToken(Token);
            void AddTransitionInt(int32_t);
            void AddTransitionDouble(double);
            void AddTransitionString(std::string);

        protected:
            void StoreOwnData(FILE*);

        private:
            std::vector<Token> t_tokens;
            std::vector<int32_t> t_ints;
            std::vector<double> t_doubles;
            std::vector<std::string> t_strings;
    };

}
