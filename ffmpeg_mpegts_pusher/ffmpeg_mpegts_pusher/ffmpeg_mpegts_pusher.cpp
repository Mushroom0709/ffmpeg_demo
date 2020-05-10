#include "x_source_process.h"
#include "x_target_process.h"
int main()
{
    xSourceProcess src;
    xTargetProcess tgt;

    if (false == src.OpenSource("../../test_video/失眠飞行.mp4", true))
        return -1;

    if (false == tgt.NewOutput("udp://239.0.1.1:50101", "mpegts"))
        return -1;

    if (false == tgt.BuildStreamByInput(src.FormatContext(), src.StreamIndex(), true))
        return -1;

    src.BlockReadPacket(&tgt);

    src.Destroy();
    tgt.Destroy();

    return 0;
}
