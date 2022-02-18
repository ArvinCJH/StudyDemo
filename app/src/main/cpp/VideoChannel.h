//
// Created by Administrator on 2022/2/16.
//

#ifndef MYSTUDYAPPLICATION_VIDEOCHANNEL_H
#define MYSTUDYAPPLICATION_VIDEOCHANNEL_H

#include "BaseChannel.h"

extern "C" {
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
};

class VideoChannel : public BaseChannel {
private:
    //  解码线程
    pthread_t pid_video_decode;
    //  播放线程
    pthread_t pid_video_play;

public:
    VideoChannel(int stream_index, AVCodecContext *codecContext) :
            BaseChannel(stream_index, codecContext);

    virtual ~VideoChannel();

    void video_decode();

    void video_play();
};


#endif //MYSTUDYAPPLICATION_VIDEOCHANNEL_H
