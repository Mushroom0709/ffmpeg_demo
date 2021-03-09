#include "work_center.h"

int main(int _argc, char* _argv[])
{
    WorkCenter center;

    char* ip = (char*)"10.0.0.29";
    int port = 50510;

    if (_argc == 3)
    {
        ip = _argv[1];
        port = atoi(_argv[2]);
    }

    if (false == center.Start(ip, port))
        return-1;

    getchar();

    center.Stop();

    return 0;
}

