#include "work_center.h"

int main()
{
    WorkCenter center;
    if (false == center.Start("127.0.0.2", 50510))
        return-1;

    getchar();

    center.Stop();

    return 0;
}

