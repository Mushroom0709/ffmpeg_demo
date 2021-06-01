//../../test_video/失眠飞行.mp4
#include "x_player.h"

int main(int argc, char* argv[])
{
    SetProcessDPIAware();

    xM::xPlayer player;
    if (true == player.Initialize("../../test_video/180942863-1-208.mp4"))
        player.Work();
    return 0;
}