//
// Created by Administrator on 2022/2/16.
//

#ifndef MYSTUDYAPPLICATION_AUDIOCHANNEL_H
#define MYSTUDYAPPLICATION_AUDIOCHANNEL_H

#include "BaseChannel.h"

class AudioChannel:public BaseChannel {
private:


public:
    AudioChannel(int stream_index, AVCodecContext *codecContext);

    ~AudioChannel();

    void start();

    void stop();
};


#endif //MYSTUDYAPPLICATION_AUDIOCHANNEL_H
