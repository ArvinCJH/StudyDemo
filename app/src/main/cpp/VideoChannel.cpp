//
// Created by Administrator on 2022/2/16.
//

#include "VideoChannel.h"

void BaseChannel::start() {
    isPlaying = 1;

}

void BaseChannel::stop() {
    isPlaying = 0;
}

//  解包
void VideoChannel::video_decode() {
}

//  播放
void VideoChannel::video_play() {
}

VideoChannel::VideoChannel(int stream_index, AVCodecContext *codecContext) :
        BaseChannel(stream_index, codecContext) {

}
