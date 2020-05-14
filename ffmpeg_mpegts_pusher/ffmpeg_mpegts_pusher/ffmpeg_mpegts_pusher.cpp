#include "x_input_stream.h"
#include "x_output_stream.h"
int main()
{
    xInputStream input;
    xOutputStream output;

    avformat_network_init();
    avdevice_register_all();

    //if (false == input.OpenFile("../../test_video/失眠飞行.mp4", true))
    //    return -1;

    if (false == input.OpenScreen(true))
        return -1;

    if (false == output.Initialization("udp://239.0.0.1:50101"))
        return -1;

    if (false == output.SetParameters(input.GetStreams(), true))
        return -1;

    input.BlockRead(&output);

    output.Destroy();
    input.Destroy();

    return 0;
}

//ffplay -x 1280 -y 720 udp://239.0.0.1:50101 -sync audio //用来播放视频
//ffplay -x 1280 -y 720 udp://239.0.0.1:50101 //用来播放桌面