#include <stdio.h>
#include <errno.h>
#include "tracelog.h"

using namespace tsync;

Tracelog::Tracelog(char * filepath, int process_id, int min_evnt_dif, int min_msg_dly)
{
    this->filepath = filepath;
    this->process_id = process_id;
    this->min_evnt_dif = min_evnt_dif;
    this->min_msg_dly = min_msg_dly;
}

void Tracelog::Load()
{
    FILE *fp = fopen(this->filepath, "rb");
    if (fp)
    {
        std::string content;

        fseek(fp, 0, SEEK_END);
        content.resize(ftell(fp));
        rewind(fp);

        fread(&content[0], 1, content.size(), fp);
        fclose(fp);

        this->data = content;
    }
    else {
        throw errno;
    }

    char block[2] = { 1, 1 };
    this->pointer = &this->data[0];
    while ( (block[0] != 0) || (block[1] != 0) )
    {
        block[0] = block[1];
        block[1] = *this->pointer;
        printf("%c", block[1]);
        this->pointer++;
    }
}
