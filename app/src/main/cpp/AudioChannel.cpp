//
// Created by Administrator on 2022/2/16.
//

#include "AudioChannel.h"

void AudioChannel::start() {
    isPlaying = true;
}

void AudioChannel::stop() {
    isPlaying = false;
}

AudioChannel::AudioChannel(int stream_index, AVCodecContext *codecContext) :
        BaseChannel(stream_index, codecContext) {

}

AudioChannel::~AudioChannel() {

}
