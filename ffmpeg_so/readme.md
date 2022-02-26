# 简介

ffmpeg_build.sh	

1. 采用最新 ffmpeg 版本，使用clang的方式编译，里面部分参数可选
2. 使用时需要修改第三行，修改为自己的 NDK 路径
3. 环境介绍: ubuntu 20.04 linux ndk:r20
4. 使用方式: sh ffmpeg_build.sh
5. 脚本输出不包含: libavdevice.so 文件，与aarch64 和 arm数量不相同，是因为之后发现 libavdevice 这个库在 Android 中并未使用到，所以已经去掉。需要可以删除 --disable-avdevice 这一行即可。
6. 脚本暂未支持一键所有架构同时编译，正在完善中...



aarch64 和 arm 为已经编译好的文件，直接使用即可。





### 文件目录树

```
|--ffmpeg_so
   |   |--aarch64
   |   |   |--include
   |   |   |   |--libavutil
   |   |   |   |--libavdevice
   |   |   |   |--libavformat
   |   |   |   |--libavcodec
   |   |   |   |--libavfilter
   |   |   |   |--libswscale
   |   |   |   |--libswresample
   |   |   |--share
   |   |   |   |--ffmpeg
   |   |   |   |   |--examples
   |   |   |--lib
   |   |   |   |--pkgconfig
   |   |--arm
   |   |   |--include
   |   |   |   |--libavutil
   |   |   |   |--libavdevice
   |   |   |   |--libavformat
   |   |   |   |--libavcodec
   |   |   |   |--libavfilter
   |   |   |   |--libswscale
   |   |   |   |--libswresample
   |   |   |--share
   |   |   |   |--ffmpeg
   |   |   |   |   |--examples
   |   |   |--lib
   |   |   |   |--pkgconfig
```

