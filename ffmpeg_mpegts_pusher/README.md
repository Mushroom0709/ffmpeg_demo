# ffmpeg_mpegts_pusher
+ 利用mpegts封装格式推UDP流

### 使用说明
+ 基于VS2019 FFmpeg4.2.2 x86开发。(如果配置项目有困难请完整项目下载f5即可一键启动。)

### 调试提示
+ 请先在cmd中使用[ffplay -x 1280 -y 720 udp://239.0.1.1:50101 -sync audio]命令，随后启动程序即可(播放屏幕流的时候，不需要 -sync audio )。

### 其他
+ 目前占用内存比较大，推流延迟也比较高，但主要目的在学习mpegts推流，也就没有太计较。

###### @xMushroom