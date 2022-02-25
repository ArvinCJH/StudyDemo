//
// Created by Administrator on 2022/2/16.
//

#ifndef MYSTUDYAPPLICATION_AUDIOCHANNEL_H
#define MYSTUDYAPPLICATION_AUDIOCHANNEL_H

#include "BaseChannel.h"
#include <SLES/OpenSLES_Android.h>

extern "C" {
#include <libswresample/swresample.h>
};


class AudioChannel : public BaseChannel {
private:
    //  两个线程
    // 1. 压缩包转为原始包线程
    pthread_t pid_audio_decode;
    // 2. 重采样与播放线程
    pthread_t pid_audio_play;


public:
    AudioChannel(int stream_index, AVCodecContext *codecContext, AVRational time_base);

    ~AudioChannel();

    void start();

    void stop();

    void audio_decode();

    void audio_play();

    int getPCM();

    // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    // 音频参数相关
    int out_channels;       // STEREO:双声道类型 == 获取 声道数 2
    int out_sample_size;    // 每个sample是16 bit == 2字节 位数
    int out_sample_rate;    // 采样率
    int out_buffers_size;
    uint8_t *out_buffers = 0;
    SwrContext *swr_ctx = 0;
    // 音频 SLES 相关
    SLObjectItf engineObject = 0;

    SLEngineItf engineItf = 0;

    SLObjectItf outputMixObject = 0;

    SLObjectItf bqPlayerObject = 0;

    SLPlayItf bqPlayerPlay = 0;

    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue = 0;

    double audio_time; //  音视频同步
};


#endif //MYSTUDYAPPLICATION_AUDIOCHANNEL_H
