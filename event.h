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

        protected:
            char type;
            uint64_t time;
    };

    class TokenEvent : BasicEvent
    {
        public:
            TokenEvent(char);
            void SetId(int32_t);
            void AddToken(Token);
            void AddInt(int32_t);
            void AddDouble(double);
            void AddString(std::string);

        private:
            int32_t id;
            std::vector<Token> tokens;
            std::vector<int32_t> ints;
            std::vector<double> doubles;
            std::vector<std::string> strings;
    };

    class SendEvent : BasicEvent
    {
    //TODO: store targets number and then ids
        public:
            SendEvent(char);
            void SetSize(uint64_t);
            void SetEdge(int32_t);
            void AddTarget(int32_t);

        private:
            uint64_t msg_size;
            int32_t edge_id;
            std::vector<int32_t> targets;
    };

    class TransitionEvent : TokenEvent
    {
    //TODO: when storing result use TokenEvent including virtual method for Trasitiondata storing
        public:
            TransitionEvent(char);
            void AddTransitionToken(Token);
            void AddTransitionInt(int32_t);
            void AddTransitionDouble(double);
            void AddTransitionString(std::string);

        private:
            std::vector<Token> t_tokens;
            std::vector<int32_t> t_ints;
            std::vector<double> t_doubles;
            std::vector<std::string> t_strings;
    };

}
