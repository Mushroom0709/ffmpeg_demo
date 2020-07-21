#include "process_center.h"

int main()
{
    ProcessCenter center;

    if (false == center.Init())
        return -1;

    ////建立图像库 可单独使用
    ////LoadingImageLibrary(原图的根目录,略缩图边长(默认640))
    //if (center.LoadingImageLibrary("F:\\爱豆\\"))
    //    return -1;

    //生成大图 可单独使用 
    //MakeThumbMatrix(小图路径，目标大图文件名称，小图缩放尺寸——宽，小图缩放尺寸——高，填充略缩图边长)
    //注意，小图缩放尺寸 填入-1即代表当前参数自动适配，此处将按照小图原始比例等比适配
    //if (true == center.MakeThumbMatrix("C:\\小图.jpg", ".\\大图.jpg", -1, 70, 320))
    //    return -1;

    //1.jpg

    if (true == center.MakeThumbMatrix("C:\\Users\\xMushroom\\Desktop\\1.png", ".\\jb-test.jpg", 150, -1, 60))
        return -1;

    return 0;
}