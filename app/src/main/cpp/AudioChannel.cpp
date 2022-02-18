//
// Created by Administrator on 2022/2/16.
//

#include "AudioChannel.h"

void BaseChannel::start() {
    isPlaying = 1;
}

void BaseChannel::stop() {
    isPlaying = 0;
}

AudioChannel::AudioChannel(int stream_index, AVCodecContext *codecContext) :
        BaseChannel(stream_index, codecContext) {

}
