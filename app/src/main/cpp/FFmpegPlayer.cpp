//
// Created by Administrator on 2022/2/16.
//

#include "FFmpegPlayer.h"


FFmpegPlayer::FFmpegPlayer(const char *data_source, JNICallbakcHelper *helper) {

    //  strlen 返回的长度不包括 '\0' 这个结束符， 所以需要一个占位符
    this->data_source = new char[strlen(data_source) + 1];
    //  因为在 native-lib.cpp 文件中有释放的操作，为了避免还需要使用时已经被释放，拷贝一份
    strcpy(this->data_source, data_source);
    this->helper = helper;
}

FFmpegPlayer::~FFmpegPlayer() {
    if (data_source) {
        delete data_source;
    }
    if (helper) {
        delete helper;
    }

}

//  文件检测准备的子线程
void *task_prepare(void *args) {
    //  拿到外部的成员变量，因为子线程，所以无法拿到外部变量
    auto *player = static_cast<FFmpegPlayer *> (args);
    player->prepare_();
    //  子线程方法结束加上 return 0 ;
    return 0;
}

//  文件开始播放的子线程
void *task_start(void *args) {
    //  拿到外部的成员变量，因为子线程，所以无法拿到外部变量
    auto *player = static_cast<FFmpegPlayer *> (args);
    player->start_();
    //  子线程方法结束加上 return 0 ;
    return 0;
}

void FFmpegPlayer::prepare() {

    // 创建子线程
    pthread_create(&pid_prepare, 0, task_prepare, this);
}

void FFmpegPlayer::start() {
    isPlaying = 1;

    if (video_channel) {
        video_channel.start();
    }
    if (audio_channel) {
        audio_channel.start();
    }

    // 创建子线程
    pthread_create(&pid_prepare, 0, task_start, this);
}

// 子线程函数
void FFmpegPlayer::prepare_() {
    int resultCode = 0;

    formatContext = avformat_alloc_context();
    if (!formatContext) {
        //  获取 context 失败
        LOGE("获取 context 失败");
        resultCode = AVERROR(ENOMEM);
        return;
    }

    AVDictionary *dictionary = 0;

    /* A different way of passing the options is as key/value pairs in a
     * dictionary. */
    av_dict_set(&dictionary, "timeout", "5000000", 0);   // 设置打开超时, 单位微妙5_000_000
//    av_dict_set(&dictionary, "video_size", "640x480", 0);
//    av_dict_set(&dictionary, "pixel_format", "rgb24", 0);

    //  1.打开媒体地址（文件路径， 直播地址rtmp）
    //    int avformat_open_input(AVFormatContext **ps, const char *url,
    //             const AVInputFormat *fmt, AVDictionary **options);
    //    @return 0 on success, a negative AVERROR on failure.
    resultCode = avformat_open_input(&formatContext, data_source, 0, &dictionary);
    av_dict_free(&dictionary);
    if (resultCode) {
        //  打开媒体格式失败
        LOGE("打开媒体格式失败, %s", av_err2str(resultCode));
        onError(FFMPEG_CAN_NOT_OPEN_URL);
//        av_err2str(resultCode);// 根据你的返回值 得到错误详情
        return;
    }

    // 2. 查找媒体中的音/视频流的信息
    // int avformat_find_stream_info(AVFormatContext *ic, AVDictionary **options)
    //  return >=0 if OK, AVERROR_xxx on error
    resultCode = avformat_find_stream_info(formatContext, 0);
    if (resultCode < 0) {
        LOGE("查找媒体中的音/视频流的信息失败%s, ", av_err2str(resultCode));
        onError(FFMPEG_CAN_NOT_FIND_STREAMS);
        av_err2str(resultCode);
        return;
    }

    // 根据流信息, 流的个数,用循环来找编解码器
    for (int i = 0; i < formatContext->nb_streams; ++i) {
        //  获取媒体流（视频/音频）
        AVStream *stream = formatContext->streams[i];
        //  从上面的流中 获取 编码解码的[参数]
        //  由于：后面的编码器 解码器 都需要参数(宽高 等等)
        AVCodecParameters *parameters = stream->codecpar;
        //  3. 根据上面的[参数]获取编解码器  find decoder for the stream
        const AVCodec *codec = avcodec_find_decoder(parameters->codec_id);
        //  >>>>>>>>>>>>>>>>>>>>>> add >>>>>>>>>>>>>>>>>>>>>>
        if (!codec) {
            LOGE("获取编解码器失败");
            onError(FFMPEG_FIND_DECODER_FAIL);
            resultCode = AVERROR(EINVAL);
            return;
        }
        //  4. 获取编解码器上下文
        AVCodecContext *avCodecContext = avcodec_alloc_context3(codec);
        if (!avCodecContext) {
            onError(FFMPEG_ALLOC_CODEC_CONTEXT_FAIL);
            LOGE("获取编解码器上下文失败");
            return;
        }

        //  5. 设置解码器上下文的参数
        //  int avcodec_parameters_to_context(AVCodecContext *codec,
        //      const AVCodecParameters *par);
        //  return >= 0 on success, a negative AVERROR code on failure.
        resultCode = avcodec_parameters_to_context(avCodecContext, parameters);
        if (resultCode < 0) {
            onError(FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL);
            LOGE("设置解码器上下文的参数失败%s, ", av_err2str(resultCode));
            av_err2str(resultCode);
            return;
        }

        //  6. 打开解码器
        // int avcodec_open2(AVCodecContext *avctx, const AVCodec *codec,
        //     AVDictionary **options);
        // return zero on success, a negative value on error
        resultCode = avcodec_open2(avCodecContext, codec, 0);
        if (resultCode) {
            onError(FFMPEG_OPEN_DECODER_FAIL);
            LOGE("打开解码器失败%s, ", av_err2str(resultCode));
            av_err2str(resultCode);
            return;
        }

        //  从媒体中读取音/视频包, 编解码的时候都需要 AVCodecContext
        if (parameters->codec_type == AVMediaType::AVMEDIA_TYPE_AUDIO) {
            //  音频包
            audio_channel = new AudioChannel(i, avCodecContext);
        } else if (parameters->codec_type == AVMediaType::AVMEDIA_TYPE_AUDIO) {
            //  视频包
            video_channel = new VideoChannel(i, avCodecContext);
        }
    }

    //  如果流中 没有音频 也没有 视频
    if (!audio_channel && !video_channel) {
        onError(FFMPEG_NOMEDIA);
        LOGE("流中 没有音频 也没有 视频 ");
        return;
    }

    if (helper) {
        helper->onPrepared(THREAD_CHILD);
    }


}

// 子线程
void FFmpegPlayer::start_() {
    while (isPlaying) {
        //  pkt will be blank (as if it came from av_packet_alloc())
        AVPacket *packet = av_packet_alloc();
        if (!packet) {
            LOGE("av_packet_alloc failer");
            return;
        }
        //  1. 获取压缩包(可能是音频 也可能是视频)
        //  int av_read_frame(AVFormatContext *s, AVPacket *pkt);
        //  return 0 if OK, < 0 on error or end of file. On error, pkt will be blank
        //        (as if it came from av_packet_alloc()).
        int resultCode = av_read_frame(formatContext, packet);
        if (!ret) {
            //  成功的情况, 拿到包了, 把包分类, 然后丢进队列里面去
            if (video_channel && video_channel->steam_index == packet->steam_index) {
                //  视频包, 丢进队列里面去
            } else if (audio_channel && audio_channel->steam_index == packet->steam_index) {
                //  音频包, 丢进队列里面去
            }


        } else if (ret == AVERROR_EOF) {
            //  读到文件末尾了
            break;
        } else {
            //  出现了其它错误，结束当前循环
            break;
        }
    }

    isPlaying = 0;
    video_channel->stop();
    audio_channel->stop();

}

//  错误码基本都是子线程返回
void onError(int error_code) {
    if (helper) {
        helper->onError((THREAD_CHILD, error_code)
    }
}
