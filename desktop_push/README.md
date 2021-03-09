# 低延迟桌面传输-推流

### 使用说明
   基于VS2019 FFmpeg4.2.2 libx264 x86开发。(如果配置项目有困难请完整项目下载f5即可一键启动。)

### 效果展示
    ![延迟测试](http://huahua.qn.xlvfan.com/ffmpeg_demo_desktop_stream.png)

### 调试提示
   请先启动推流端,再启动拉流端

### 技术说明
1.  在本地回环推流测试,分辨率:1920*1200,帧率:18 - 24fps,关键帧间距:1 - 6.带宽3 - 5Mbps,延时150±20 ms(包括采集，编解码，传输，显示)。
2.  设计的是1VS多推流，先启动推流端，再启动拉流端，拉流端会先请求传输流分辨率用于初始化转码部分(虽然不请求也行，凑一下)，随后再发送开始推流请求。
3.  libx264编译请参阅[在Windows10下基于VS2019和MSYS2编译libx264](https://github.com/Mushroom0709/ffmpeg_demo/blob/master/third_party_library/WIN%E7%BC%96%E8%AF%91libx264.md)

###### @xMushroom