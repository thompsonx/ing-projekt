#include <iostream>
#include "mpitracelog.h"

using namespace std;

int main(int argc, char * argv[])
{
    cout << "Hello world!" << endl;

    tsync::MpiTracelog t (argv[1], 0,0,0);
    t.Load();
    t.Sync();
    t.Store();

    return 0;
}
