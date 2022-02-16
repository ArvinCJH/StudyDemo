//
// Created by Administrator on 2022/2/16.
//

#ifndef MYSTUDYAPPLICATION_FFMPEGPLAYER_H
#define MYSTUDYAPPLICATION_FFMPEGPLAYER_H

#include <string>
#include <pthread.h>
#include <AudioChannel.h>
#include <VideoChannel.h>
#include <util.h>

// ffmpeg是纯c写的，必须采用c的编译方式
extern "C" {
#include <libavformat/avformat.h>
};

class FfmpegPlayer {
private:
    char *data_source = 0 ;
    pthread_t pid_prepare;

    AudioChannel *audioChannel = 0 ;
    VideoChannel *videoChannel = 0 ;
    JNICallbakcHelper *helper = 0 ;

public:
    FfmpegPlayer(const char *data_source, JNICallbakcHelper *helper) ;
    ~FfmpegPlayer() ;
    void prepare() ;
    void prepare_() ;

};


#endif //MYSTUDYAPPLICATION_FFMPEGPLAYER_H
