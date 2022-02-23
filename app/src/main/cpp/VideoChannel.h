//
// Created by Administrator on 2022/2/16.
//

#ifndef MYSTUDYAPPLICATION_VIDEOCHANNEL_H
#define MYSTUDYAPPLICATION_VIDEOCHANNEL_H

#include "BaseChannel.h"
#include "AudioChannel.h"

extern "C" {
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/avutil.h>
};

//  定义一个函数指针
typedef void (*RenderCallback)(uint8_t *, int, int, int);

class VideoChannel : public BaseChannel {
private:
    //  解码线程
    pthread_t pid_video_decode;
    //  播放线程
    pthread_t pid_video_play;

    RenderCallback renderCallback;

    int fps ;
    AudioChannel *audio_channel = nullptr ;

public:
    VideoChannel(int stream_index, AVCodecContext *codecContext, AVRational time_base, int fps);

    ~VideoChannel();

    void start();

    void stop();

    //  线程使用, 需要用 publish
    void video_decode();

    void video_play();

    //  回调给 player 处理
    void setRenderCallback(RenderCallback callback);

    void setAudioChannel(AudioChannel *audio_channel) ;

};


#endif //MYSTUDYAPPLICATION_VIDEOCHANNEL_H
