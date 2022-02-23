//
// Created by Administrator on 2022/2/16.
//

#ifndef MYSTUDYAPPLICATION_BASECHANNEL_H
#define MYSTUDYAPPLICATION_BASECHANNEL_H

#include "safe_queue.h"
#include "log4c.h"
#include "util.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/time.h>
}


class BaseChannel {

public:
    int stream_index;    //  音/视频下标
    //  压缩包队列
    SafeQueue<AVPacket *> packets;
    //  原始包队列
    SafeQueue<AVFrame *> frames;


    bool isPlaying; // 音频 和 视频 都会有的标记 是否播放

    AVRational time_base ;      //  音视频同步(时间单位)

    AVCodecContext *avCodecContext = 0;    // 音频 视频 都需要的 解码器上下文
    BaseChannel(int stream_index, AVCodecContext *avCodecContext, AVRational time_base) :
            stream_index(stream_index), avCodecContext(avCodecContext) , time_base(time_base){
        packets.setReleaseCallback(releaseAVPacket); // 给队列设置Callback，Callback释放队列里面的数据
        frames.setReleaseCallback(releaseAVFrame); // 给队列设置Callback，Callback释放队列里面的数据
    }

    //  父类析构一定要加 virtual
    virtual ~BaseChannel() {
        packets.clear();
        frames.clear();
    }

    /**
     * 释放 AVPacket *
     * @param p
     */
    static void releaseAVPacket(AVPacket **p) {
        if (p) {
            av_packet_free(p);
            *p = 0;
        }
    }

    /**
     * 释放 AVFrame *
     * @param f
     */
    static void releaseAVFrame(AVFrame **f) {
        if (f) {
            av_frame_free(f);
            *f = 0;
        }
    }
};


#endif //MYSTUDYAPPLICATION_BASECHANNEL_H
