#include "work_center.h"

int main()
{
    WorkCenter center;
    if (false == center.Start())
        return-1;

    getchar();

    center.Stop();

    return 0;
}

