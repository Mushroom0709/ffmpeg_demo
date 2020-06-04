#include "work_center.h"

int main(int _argc, char* _argv[])
{
    WorkCenter work;

    char* ip = (char *)"127.0.0.2";
    int port = 50510;

    if (_argc == 3)
    {
        ip = _argv[1];
        port = atoi(_argv[2]);
    }

    if (true == work.Start(ip, port))
    {
        work.Wait();
        //getchar();
        work.Stop();
    }

    return 0;
}
