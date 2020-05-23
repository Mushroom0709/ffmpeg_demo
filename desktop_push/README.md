# 低延迟桌面传输-推流

### 使用说明
   基于VS2019 FFmpeg4.2.2 libx264 x86开发。(如果配置项目有困难请完整项目下载f5即可一键启动。)

### 调试提示
   请先启动推流端,再启动拉流端

### 技术说明
1.  在本地回环推流测试,分辨率:1920*1200,帧率:18~24fps,关键帧间距:1~6.带宽3~5Mbps,延时172ms左右(包括采集，转码，传输，解码，显示)。
2.  设计的是1VS多推流，目前控制部分还未做好，拉流端启动立即开始拉流(即部分参数是硬代码)。余下功能会继续迭代。
3.  libx264编译请参阅[在Windows10下基于VS2019和MSYS2编译libx264](https://github.com/Mushroom0709/ffmpeg_demo/tree/master/ffmpeg_mpegts_pusher/third_party_library/WIN编译libx264.md)

###### @xMushroom