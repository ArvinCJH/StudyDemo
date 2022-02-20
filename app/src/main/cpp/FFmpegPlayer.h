//
// Created by Administrator on 2022/2/16.
//

#ifndef MYSTUDYAPPLICATION_FFMPEGPLAYER_H
#define MYSTUDYAPPLICATION_FFMPEGPLAYER_H

#include <string>
#include <pthread.h>
#include "JNICallbakcHelper.h"
#include "AudioChannel.h"
#include "VideoChannel.h"
#include "util.h"
#include "log4c.h"

// ffmpeg是纯c写的，必须采用c的编译方式
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/error.h>
};

class FFmpegPlayer {
private:
    char *data_source = 0;
    pthread_t pid_prepare;

    AVFormatContext *formatContext = 0;
    AudioChannel *audio_channel = 0;
    VideoChannel *video_channel = 0;
    JNICallbakcHelper *helper = 0;
    bool isPlaying = 0 ;

    void onError(int error_code) ;


    RenderCallback renderCallback;


public:
    FFmpegPlayer(const char *data_source, JNICallbakcHelper *helper);

    ~FFmpegPlayer();

    void prepare();

    void prepare_();

    void start();

    void start_();

    void setRenderCallback(RenderCallback renderCallback) ;
};


#endif //MYSTUDYAPPLICATION_FFMPEGPLAYER_H
