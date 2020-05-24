# 低延迟桌面传输-拉流

### 使用说明
   基于VS2019 FFmpeg4.2.2 SDL2 x86开发。(如果配置项目有困难请完整项目下载f5即可一键启动。)

### 效果展示
    ![延迟测试](http://huahua.qn.xlvfan.com/ffmpeg_demo_desktop_stream.png)

### 调试提示
   请先启动推流端,再启动拉流端

### 技术说明
1.  SDL2仅做了简单的UI显示，没有添加拉伸窗口等功能.
2.  解码器的核心是使用 AVCodecParserContext 从h264裸流数据中组装AVPacket.

###### @xMushroom