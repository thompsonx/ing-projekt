#include <iostream>
#include "tracelog.h"

using namespace std;

int main(int argc, char * argv[])
{
    cout << "Hello world!" << endl;

    tsync::Tracelog t = tsync::Tracelog(argv[1], 0,0,0);
    t.Load();

    return 0;
}
