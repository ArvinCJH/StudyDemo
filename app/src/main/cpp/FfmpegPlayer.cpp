//
// Created by Administrator on 2022/2/16.
//

#include "FfmpegPlayer.h"


FfmpegPlayer::FfmpegPlayer(const char *data_source, JNICallbakcHelper *helper){

}
FfmpegPlayer::~FfmpegPlayer(){
    if(data_source){
        delete data_source ;
    }
    if(helper){
        delete helper;
    }

}

void FfmpegPlayer::prepare() {

}


// 子线程函数
void FfmpegPlayer::prepare_() {

}
