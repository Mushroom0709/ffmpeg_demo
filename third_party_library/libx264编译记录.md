# libx264编译记录

## 环境
+ windows10 + vs2019 + win32
+ 本教程使用 **MSYS2 MSYS**
## 步骤
1. 下载并安装 msys2 [下载地址](https://www.msys2.org/)
   
2. 更换 pacman 源 ([参阅](https://lug.ustc.edu.cn/wiki/mirrors/help/msys2))
   + 在 vscode 中打目录 **"msys2安装目录/etc/pacman.d/mirrorlist.mingw32/"**
  
   + 在 **"mirrorlist.mingw32"** 文件中添加 **"Server = http://mirrors.ustc.edu.cn/msys2/mingw/i686/"**
     + 如下所示
       ```
        ##
        ## 32-bit Mingw-w64 repository mirrorlist
        ##

        ## Primary
        ## msys2.org
        Server = http://mirrors.ustc.edu.cn/msys2/mingw/i686/
        Server = http://repo.msys2.org/mingw/i686/
        Server = https://sourceforge.net/projects/msys2/files/REPOS/MINGW/i686/
        Server = https://www2.futureware.at/~nickoe/msys2-mirror/mingw/i686/
        Server = https://mirror.yandex.ru/mirrors/msys2/mingw/i686/
        Server = https://mirrors.tuna.tsinghua.edu.cn/msys2/mingw/i686/
        Server = http://mirrors.ustc.edu.cn/msys2/mingw/i686/
        Server = http://mirror.bit.edu.cn/msys2/mingw/i686/
        Server = https://mirror.selfnet.de/msys2/mingw/i686/
       ```
   + 在 **"mirrorlist.mingw64"** 文件中添加 **"Server = http://mirrors.ustc.edu.cn/msys2/mingw/x86_64/"** (*位置同 mingw32*)

   + 在 **"mirrorlist.msys"** 文件中添加 **"Server = http://mirrors.ustc.edu.cn/msys2/msys/$arch/"** (*位置同 mingw32*)
   
3. 更新 pacman 软件包数据 并安装 相关依赖
   + `$ pacman -Syu`
   + 关闭窗口并重启系统(虽然我也不知道为啥),再次打开 MSYS
   + `$ pacman -S Su`
   + `$ pacman -S yasm`
   + `$ pacman -S make`
  
4. 编译libx264
   + 下载[libx264](https://www.videolan.org/developers/x264.html)并解压拷贝至 **"msys2安装目录/home/x264-master/"**
    
   + 打开 **"C:\\ProgramData\\Microsoft\\Windows\\Start Menu\\Programs\\Visual Studio 2019\\Visual Studio Tools\\VC\\x86 Native Tools Command Prompt for VS 2019"**(编译64的请选择**x64 Native Tools Command Prompt for VS 2019**) 并进入 **"msys2安装目录"**
   + `> msys2_shell.cmd -mingw32 -full-path` (64位则使用命令  `> msys2_shell.cmd -mingw64 -full-path`
   + 在msys窗口中执行操作
   + `$ cd /home/x264-master/`
   + `$ CC=cl ./configure --enable-shared --prefix=${PWD}/build_shared`
   + `$ make`
   + `$ make install`
   + 编译结果即在 **"msys2安装目录/home/x264-master/build_shared/"** 中
  
###### @xMushroom