//
// Created by Administrator on 2022/2/16.
//

#ifndef MYSTUDYAPPLICATION_UTIL_H
#define MYSTUDYAPPLICATION_UTIL_H

#define THREAD_MAIN 1 // 主线程
#define THREAD_CHILD 2 // 子线程


#define THREAD_SLEEP_TIME 10*1000   //  10毫秒
#define THREAD_SLEEP_COUNT 100



// 错误代码 >>>>>>>>>>>> start >>>>>>>>>>>>
// 打不开视频
#define FFMPEG_CAN_NOT_OPEN_URL 1
// 找不到流媒体
#define FFMPEG_CAN_NOT_FIND_STREAMS 2
// 找不到解码器
#define FFMPEG_FIND_DECODER_FAIL 3
// 无法根据解码器创建上下文
#define FFMPEG_ALLOC_CODEC_CONTEXT_FAIL 4
// 根据流信息 配置上下文参数失败
#define FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL 6
// 打开解码器失败
#define FFMPEG_OPEN_DECODER_FAIL 7
// 没有音视频
#define FFMPEG_NOMEDIA 8
// 错误代码 >>>>>>>>>>>> end >>>>>>>>>>>>

#define DELETE(object) if(object){delete object; object = 0 ;}

#endif //MYSTUDYAPPLICATION_UTIL_H
