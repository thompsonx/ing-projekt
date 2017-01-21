#include <iostream>
#include "mpitracelog.h"

using namespace std;

int main(int argc, char * argv[])
{
    cout << "Hello world!" << endl;

    tsync::MpiWizard mw;
    mw.Run(argc, argv);

    return 0;
}
