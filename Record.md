# Record
 开发中的一些记录
 1. 当C层出现错误时定位具体原因
    方法一: 运行时使用 Debug 'app' 按钮,如果能跳过 Waiting For Debuging 并正常崩溃, 就能得到定位到具体错误的地方(部分手机不可用)
    方法二: 根据错误, 从手机里面拿取文件分析, 能得到定位到具体错误的方法(/data/tombstones/recenttime.log)
    方法三: adb logcat | $NDK/ndk-stack -sym $PROJECT_PATH/obj/local/armeabi; eg. adb logcat | $NDK\ndk\21.4.7075529\ndk-stack -sym StudyDemo\app\build\intermediates\cmake\debug\obj\arm64-v8a
        在输入命令行按回车之后，运行一次app, 就能从logcat中定位到具体错误的地方
