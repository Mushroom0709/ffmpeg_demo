#include <stdio.h>
#include <Windows.h>
#include "xplayer.h"

int main(int _argc, char* _argv[])
{
    SetProcessDPIAware(); //解决windows 缩放问题

    xPlayer player;

    player.Start("../../test_video/失眠飞行.mp4");
    player.Stop();
    return 0;
}