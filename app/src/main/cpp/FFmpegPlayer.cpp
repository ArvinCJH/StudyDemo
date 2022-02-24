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
    pthread_mutex_init(&seek_mutex, nullptr);
}

FFmpegPlayer::~FFmpegPlayer() {
    isPlaying = false;
    if (data_source) {
        delete data_source;
        data_source = nullptr;
    }
    if (helper) {
        delete helper;
        helper = nullptr;
    }
    pthread_mutex_destroy(&seek_mutex);
}


//  错误码基本都是子线程返回
void FFmpegPlayer::onError(int error_code) {
    if (helper) {
        helper->onError(THREAD_CHILD, error_code);
    }
}

//  文件检测准备的子线程
void *task_prepare(void *args) {
    //  拿到外部的成员变量，因为子线程，所以无法拿到外部变量
    auto *player = static_cast<FFmpegPlayer *> (args);
    player->prepare_();
    //  子线程方法结束加上 return 0 ;
    return nullptr;
}

//  文件开始播放的子线程
void *task_start(void *args) {
    //  拿到外部的成员变量，因为子线程，所以无法拿到外部变量
    auto *player = static_cast<FFmpegPlayer *> (args);
    player->start_();
    //  子线程方法结束加上 return 0 ;
    return nullptr;
}

void FFmpegPlayer::prepare() {

    // 创建子线程
    pthread_create(&pid_prepare, nullptr, task_prepare, this);
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

    AVDictionary *dictionary = nullptr;

    /* A different way of passing the options is as key/value pairs in a
     * dictionary. */
    av_dict_set(&dictionary, "timeout", "5000000", 0);   // 设置打开超时, 单位微妙5_000_000
//    av_dict_set(&dictionary, "video_size", "640x480", 0);
//    av_dict_set(&dictionary, "pixel_format", "rgb24", 0);

    //  1.打开媒体地址（文件路径， 直播地址rtmp）
    //    int avformat_open_input(AVFormatContext **ps, const char *url,
    //             const AVInputFormat *fmt, AVDictionary **options);
    //    @return 0 on success, a negative AVERROR on failure.
    resultCode = avformat_open_input(&formatContext, data_source, nullptr, &dictionary);
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
    resultCode = avformat_find_stream_info(formatContext, nullptr);
    if (resultCode < 0) {
        LOGE("查找媒体中的音/视频流的信息失败%s, ", av_err2str(resultCode));
        onError(FFMPEG_CAN_NOT_FIND_STREAMS);
        av_err2str(resultCode);
        return;
    }

    duration = formatContext->duration / AV_TIME_BASE;

    AVCodecContext *avCodecContext = nullptr;

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
        avCodecContext = avcodec_alloc_context3(codec);
        if (!avCodecContext) {
            onError(FFMPEG_ALLOC_CODEC_CONTEXT_FAIL);
            LOGE("获取编解码器上下文失败");
            return;
        }

        //  add self
        avCodecContext->channel_layout = select_channel_layout(codec);
        avCodecContext->sample_rate = select_sample_rate(codec);

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
        resultCode = avcodec_open2(avCodecContext, codec, nullptr);
        if (resultCode) {
            onError(FFMPEG_OPEN_DECODER_FAIL);
            LOGE("打开解码器失败%s, ", av_err2str(resultCode));
            av_err2str(resultCode);
            return;
        }

        AVRational time_base = stream->time_base;

        //  从媒体中读取音/视频包, 编解码的时候都需要 AVCodecContext
        if (parameters->codec_type == AVMediaType::AVMEDIA_TYPE_AUDIO) {
            //  音频包
            audio_channel = new AudioChannel(i, avCodecContext, time_base);
            if (0 != duration) {
                audio_channel->setJNICallbakcHelper(helper);
            }
        } else if (parameters->codec_type == AVMediaType::AVMEDIA_TYPE_VIDEO) {
            if (stream->disposition & AV_DISPOSITION_ATTACHED_PIC) {
                continue;
            }

            AVRational fps_rational = stream->avg_frame_rate;
            int fps = av_q2d(fps_rational);

            //  视频包
            video_channel = new VideoChannel(i, avCodecContext, time_base, fps);
            video_channel->setRenderCallback(renderCallback);
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
        //  内存泄露 -------------> 控制 AVPacket 队列大小
        if ((video_channel && video_channel->packets.size() > THREAD_SLEEP_COUNT) ||
            (audio_channel && audio_channel->packets.size() > THREAD_SLEEP_COUNT)) {
            av_usleep(THREAD_SLEEP_TIME);
            continue;
        }

        //  pkt will be blank (as if it came from av_packet_alloc())
        AVPacket *packet = av_packet_alloc();

        if (!packet) {
            LOGE("av_packet_alloc failer");
            // return;
            continue;
        }
        //  1. 获取压缩包(可能是音频 也可能是视频)
        //  int av_read_frame(AVFormatContext *s, AVPacket *pkt);
        //  return 0 if OK, < 0 on error or end of file. On error, pkt will be blank
        //        (as if it came from av_packet_alloc()).
        int resultCode = av_read_frame(formatContext, packet);
        if (!resultCode) {  //  resultCode == 0
            //  成功的情况, 拿到包了, 把包分类, 然后丢进队列里面去
            if (video_channel && video_channel->stream_index == packet->stream_index) {
                //  视频包, 丢进队列里面去
                // LOGI("insertTo video_channel Queue") ;
                video_channel->packets.insertToQueue(packet);
            } else if (audio_channel && audio_channel->stream_index == packet->stream_index) {
                //  音频包, 丢进队列里面去
                // LOGD("insertTo audio_channel Queue") ;
                audio_channel->packets.insertToQueue(packet);
            }


        } else if (resultCode == AVERROR_EOF) {
            //  读到文件末尾了, 但是并不是播放完毕
            // LOGE("读到文件末尾了") ;
            //  内存泄露 -------------> 临时处理
            // if(video_channel->packets.empty() && audio_channel->packets.empty()){
            //     break;
            // }
            // break;
        } else {
            //  出现了其它错误，结束当前循环
            // LOGE("出现了其它错误，结束当前循环") ;
            break;
        }
    }

    LOGE("isPlaying:%d", isPlaying);
    isPlaying = false;
    video_channel->stop();
    audio_channel->stop();

}

void FFmpegPlayer::start() {
    isPlaying = true;

    if (video_channel) {
        video_channel->setAudioChannel(audio_channel);
        video_channel->start();
    }
    if (audio_channel) {
        audio_channel->start();
    }

    // 创建子线程
    pthread_create(&pid_start, nullptr, task_start, this);
}

void FFmpegPlayer::setRenderCallback(RenderCallback renderCallback) {
    this->renderCallback = renderCallback;
}

long FFmpegPlayer::getDuration() {
    return duration;
}

void FFmpegPlayer::seek(int progress) {
    if (progress < 0 || progress > duration) {
        return;
    }
    if (!audio_channel && !video_channel) {
        return;
    }
    if (!formatContext) {
        return;
    }

    pthread_mutex_lock(&seek_mutex);

    int result_code = av_seek_frame(formatContext, -1, progress * AV_TIME_BASE, AVSEEK_FLAG_FRAME);
    if (result_code < 0) {
        pthread_mutex_unlock(&seek_mutex);
        return;
    }


    if (audio_channel) {
        audio_channel->packets.setWork(0);
        audio_channel->frames.setWork(0);
        audio_channel->packets.clear();
        audio_channel->frames.clear();
        audio_channel->packets.setWork(1);
        audio_channel->frames.setWork(1);
    }
    if (video_channel) {
        video_channel->packets.setWork(0);
        video_channel->frames.setWork(0);
        video_channel->packets.clear();
        video_channel->frames.clear();
        video_channel->packets.setWork(1);
        video_channel->frames.setWork(1);
    }
    pthread_mutex_unlock(&seek_mutex);

}


/* select layout with the highest channel count */
uint64_t FFmpegPlayer::select_channel_layout(const AVCodec *codec) {
    const uint64_t *p;
    uint64_t best_ch_layout = 0;
    int best_nb_channels = 0;

    if (!codec->channel_layouts)
        return AV_CH_LAYOUT_STEREO;

    p = codec->channel_layouts;
    while (*p) {
        int nb_channels = av_get_channel_layout_nb_channels(*p);

        if (nb_channels > best_nb_channels) {
            best_ch_layout = *p;
            best_nb_channels = nb_channels;
        }
        p++;
    }
    return best_ch_layout;
}


/* just pick the highest supported samplerate */
uint64_t FFmpegPlayer::select_sample_rate(const AVCodec *codec) {
    const int *p;
    int best_samplerate = 0;

    if (!codec->supported_samplerates)
        return 44100;

    p = codec->supported_samplerates;
    while (*p) {
        if (!best_samplerate || abs(44100 - *p) < abs(44100 - best_samplerate))
            best_samplerate = *p;
        p++;
    }
    return best_samplerate;
}

void *task_stop(void *args) {
    auto player = static_cast<FFmpegPlayer *>(args);
    player->stop_(player);
    return nullptr;
}

void FFmpegPlayer::stop() {
    pthread_create(&pid_stop, nullptr, task_stop, this);


}

void FFmpegPlayer::stop_(FFmpegPlayer *) {
    isPlaying = false;
}
