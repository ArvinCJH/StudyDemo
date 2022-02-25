//
// Created by Administrator on 2022/2/16.
//

#include "AudioChannel.h"


void *task_audio_decode(void *args) {
    auto *audio_channel = static_cast<AudioChannel *>(args);
    audio_channel->audio_decode();
    return nullptr;
}

void *task_audio_play(void *args) {
    auto *audio_channel = static_cast<AudioChannel *>(args);
    audio_channel->audio_play();
    return nullptr;
}

void AudioChannel::start() {
    isPlaying = true;

    packets.setWork(1);
    frames.setWork(1);
    pthread_create(&pid_audio_decode, nullptr, task_audio_decode, this);
    pthread_create(&pid_audio_play, nullptr, task_audio_play, this);
}

void AudioChannel::stop() {
    isPlaying = false;
    packets.setWork(0);
    frames.setWork(0);

    pthread_join(pid_audio_decode, nullptr);
    pthread_join(pid_audio_play, nullptr);

    if (bqPlayerPlay) {
        (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_STOPPED);
        bqPlayerPlay = nullptr;
    }

    if (bqPlayerObject) {
        (*bqPlayerObject)->Destroy(bqPlayerObject);
        bqPlayerObject = nullptr;
        bqPlayerBufferQueue = nullptr;
    }
    if (outputMixObject) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = nullptr;
    }
    if (engineObject) {
        (*engineObject)->Destroy(engineObject);
        engineObject = nullptr;
        engineItf = nullptr;
    }

    // packets.clear();
    // frames.clear();

}

AudioChannel::AudioChannel(int stream_index, AVCodecContext *codecContext, AVRational time_base) :
        BaseChannel(stream_index, codecContext, time_base) {

    out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    out_sample_size = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
    out_sample_rate = 44100;
    // out_channels = av_get_channel_layout_nb_channels(avCodecContext->channel_layout) ;
    // out_sample_size = av_get_bytes_per_sample(avCodecContext->sample_fmt) ;
    // out_sample_rate = avCodecContext->sample_rate ;

    out_buffers_size = out_channels * out_sample_size * out_sample_rate;
    out_buffers = static_cast<uint8_t *>(malloc(out_buffers_size));

    /*struct SwrContext *swr_alloc_set_opts(struct SwrContext *s,
              int64_t out_ch_layout, enum AVSampleFormat out_sample_fmt,
              int out_sample_rate,
              int64_t in_ch_layout, enum AVSampleFormat in_sample_fmt,
              int in_sample_rate,
              int log_offset, void *log_ctx);*/
    swr_ctx = swr_alloc_set_opts(nullptr,
                                 AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, out_sample_rate,
                                 avCodecContext->channel_layout, avCodecContext->sample_fmt,
                                 avCodecContext->sample_rate,
                                 0, nullptr
    );

    swr_init(swr_ctx);

}

AudioChannel::~AudioChannel() {
    if (swr_ctx) {
        swr_free(&swr_ctx);
        swr_ctx = nullptr;
    }
    DELETE(out_buffers);
}


void AudioChannel::audio_decode() {
    AVPacket *pkt = nullptr;
    while (isPlaying) {

        if (isPlaying && frames.size() > THREAD_SLEEP_COUNT) {
            av_usleep(THREAD_SLEEP_TIME);
            continue;
        }

        int result_code = packets.getQueueAndDel(pkt);

        if (!isPlaying) {
            break;
        }
        if (!result_code) {  // ret == 0
            continue;
        }
        result_code = avcodec_send_packet(avCodecContext, pkt);
        if (result_code) {
            break;
        }
        AVFrame *frame = av_frame_alloc();
        result_code = avcodec_receive_frame(avCodecContext, frame);
        if (AVERROR(EAGAIN) == result_code) {
            continue;
        } else if (0 != result_code) {
            if (frame) {
                releaseAVFrame(&frame);
            }
            break;
        }
        frames.insertToQueue(frame);

        av_packet_unref(pkt);
        releaseAVPacket(&pkt);

    }

    av_packet_unref(pkt);
    releaseAVPacket(&pkt);
}

void bqPlayerCallback(SLAndroidSimpleBufferQueueItf queueItf, void *args) {
    auto *audio_channel = static_cast<AudioChannel *>(args);
    int pcm_size = audio_channel->getPCM();
    (*queueItf)->Enqueue(queueItf, audio_channel->out_buffers, pcm_size);

}

void AudioChannel::audio_play() {
    SLresult result;
    result = slCreateEngine(&engineObject, 0, nullptr, 0, nullptr, nullptr);
    if (SL_RESULT_SUCCESS != result) {
        // create engine fail
        LOGE("create slCreateEngine error");
        return;
    }
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE); // SL_BOOLEAN_FALSE:延时等待你创建成功
    if (SL_RESULT_SUCCESS != result) {
        // create engine fail
        LOGE("create Realize error");
        return;
    }
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineItf);
    if (SL_RESULT_SUCCESS != result) {
        // create engine fail
        LOGE("create GetInterface error");
        return;
    }
    if (!engineItf) {
        LOGE("create engineItf error");
        return;
    }

    result = (*engineItf)->CreateOutputMix(engineItf, &outputMixObject, 0, nullptr, nullptr);
    if (SL_RESULT_SUCCESS != result) {
        // create engine fail
        LOGE("create engineItf CreateOutputMix error");
        return;
    }


    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        // create engine fail
        LOGE("create outputMixObject Realize error");
        return;
    }
    LOGI("2、SET outputMixObject ---");
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 10};

    SLDataFormat_PCM format_pcm = {
            SL_DATAFORMAT_PCM,
            2,
            SL_SAMPLINGRATE_44_1,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
            SL_BYTEORDER_LITTLEENDIAN};
    SLDataSource audio_src = {&loc_bufq, &format_pcm};

    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, nullptr};

    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};

    result = (*engineItf)->CreateAudioPlayer(
            engineItf,
            &bqPlayerObject,
            &audio_src,
            &audioSnk,
            1,
            ids,
            req
    );

    if (SL_RESULT_SUCCESS != result) {
        // create engine fail
        LOGE("create CreateAudioPlayer error");
        return;
    }

    result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        // create engine fail
        LOGE("create bqPlayerObject Realize error");
        return;
    }

    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);
    if (SL_RESULT_SUCCESS != result) {
        // create engine fail
        LOGE("create bqPlayerObject GetInterface SL_IID_PLAY error");
        return;
    }

    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE,
                                             &bqPlayerBufferQueue);
    if (SL_RESULT_SUCCESS != result) {
        // create engine fail
        LOGE("create bqPlayerObject GetInterface SL_IID_BUFFERQUEUE error");
        return;
    }

    (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, this);

    (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);

    bqPlayerCallback(bqPlayerBufferQueue, this);

}

/**
 * pcm data
 * @return
 */
int AudioChannel::getPCM() {
    int pcm_data_size = 0;
    AVFrame *frame = nullptr;
    while (isPlaying) {
        int result_code = frames.getQueueAndDel(frame);
        if (!isPlaying) {
            break;
        }
        if (!result_code) {
            continue;
        }

        //  开始重采样

        int dst_nb_samples = av_rescale_rnd(
                swr_get_delay(swr_ctx, frame->sample_rate) + frame->nb_samples,
                out_sample_rate, frame->sample_rate, AV_ROUND_UP

        );

        int samples_per_channel = swr_convert(
                swr_ctx,
                &out_buffers, dst_nb_samples,
                (const uint8_t **) frame->data, frame->nb_samples);
        pcm_data_size = samples_per_channel * out_sample_size * out_channels;

        audio_time = frame->best_effort_timestamp * av_q2d(time_base);
        // LOGI("audio_time1:%d", audio_time) ;
        // LOGI("audio_time:%d, best_effort_timestamp:%d", audio_time, frame->best_effort_timestamp ) ;
        if (jniCallbakcHelper) {
            jniCallbakcHelper->onSeek(THREAD_CHILD, audio_time);
        }
        break;

    }
    av_frame_unref(frame);
    releaseAVFrame(&frame);

    return pcm_data_size;
}

// double* AudioChannel::getDoubleTime() {
//     // LOGI("audio_time:%d", audio_time) ;
//     return audio_time;
// }
