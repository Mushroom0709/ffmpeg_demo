#include "work_center.h"

int main(int _argc, char* _argv[])
{
    WorkCenter work;

    if (true == work.Start("127.0.0.2", 50510))
    {
        work.Wait();
        //getchar();
        work.Stop();
    }

    return 0;
}
