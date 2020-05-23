#include "work_center.h"

int main(int _argc, char* _argv[])
{
    WorkCenter work;

    if (true == work.Start())
    {
        work.Wait();
        //getchar();
        work.Stop();
    }

    return 0;
}
