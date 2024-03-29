//
// Created by Administrator on 2022/2/16.
//

#include "VideoChannel.h"


void dropAVFrame(queue<AVFrame *> &q) {
    if (!q.empty()) {
        // LOGI("dropAVFrame:%d", q.size());
        AVFrame *frame = q.front();
        // if (AV_PKT_FLAG_KEY != frame->flags) {
        // av_frame_unref(frame);
        BaseChannel::releaseAVFrame(&frame);
        q.pop();
        // }
        // else{
        //     //  如果是关键帧, 不丢此包
        //     break;
        // }
    }
}

void dropAVPacket(queue<AVPacket *> &q) {
    while (!q.empty()) {
        LOGI("dropAVPacket:%d", q.size());
        AVPacket *pkt = q.front();
        if (AV_PKT_FLAG_KEY != pkt->flags) {
            // av_packet_unref(pkt);
            BaseChannel::releaseAVPacket(&pkt);
            q.pop();
        } else {
            //  如果是关键帧, 不丢此包
            break;
        }
    }
}


VideoChannel::VideoChannel(int stream_index, AVCodecContext *codecContext, AVRational time_base,
                           double fps) :
        BaseChannel(stream_index, codecContext, time_base), fps(fps) {
    frames.setSyncCallback(dropAVFrame);
    packets.setSyncCallback(dropAVPacket);
}

VideoChannel::~VideoChannel() {
    DELETE(audio_channel);

}

void VideoChannel::stop() {
    isPlaying = false;
    packets.setWork(0);
    frames.setWork(0);

    pthread_join(pid_video_decode, nullptr);
    pthread_join(pid_video_play, nullptr);


}

void *task_video_decode(void *args) {
    auto *video_channel = static_cast<VideoChannel *>(args);
    video_channel->video_decode();
    return nullptr;
}

void *task_video_play(void *args) {
    auto *video_channel = static_cast<VideoChannel *>(args);
    video_channel->video_play();
    return nullptr;
}

void VideoChannel::start() {
    isPlaying = true;

    packets.setWork(1);
    frames.setWork(1);

    //  需要开启两个线程, 一个用于获取压缩包, 一个用于处理原始包并播放
    //  1.  开启取出压缩包线程
    pthread_create(&pid_video_decode, nullptr, task_video_decode, this);
    //  2.  开启处理原始包并播放线程
    pthread_create(&pid_video_play, nullptr, task_video_play, this);


}


// 解包
void VideoChannel::video_decode() {

    AVPacket *pkt = nullptr;
    while (isPlaying) {
        //  内存泄露 -------------> AVFrame 控制队列大小
        if (isPlaying && frames.size() > THREAD_SLEEP_COUNT) {
            av_usleep(THREAD_SLEEP_TIME);   //  微秒
            continue;
        }

        // 从队列中获取压缩包
        int result_code = packets.getQueueAndDel(pkt);
        if (!isPlaying) {
            // 检测是否在播放状态
            break;
        }
        if (!result_code) {
            //  包取出失败, 继续往下走
            continue;
        }
        //  发送 pkt 到缓冲区(解码缓冲区, ffmpeg内部自带. 会复制一份副本, 所以可以当即释放)
        result_code = avcodec_send_packet(avCodecContext, pkt);

        // releaseAVPacket(&pkt);

        if (result_code) {
            //  发送进缓冲区出现了错误
            LOGE("发送进缓冲区出现了错误, %s", av_err2str(result_code));
            break;
        }

        // 从缓冲区获取已经解压出来的原始包
        AVFrame *frame = av_frame_alloc();
        // avcodec_receive_frame(AVCodecContext *avctx, AVFrame *frame)
        result_code = avcodec_receive_frame(avCodecContext, frame);
        if (AVERROR(EAGAIN) == result_code) {
            // 获取失败, 可能只是没有拿到需要的包, 不是重大错误, 继续进行下一次获取
            continue;
        } else if (0 != result_code) {

            //  内存泄露 -------------> 获取缓冲区的原始包错误,释放  AVFrame
            LOGE("获取缓冲区的原始包错误,%s", av_err2str(result_code));
            if (frame) {
                releaseAVFrame(&frame);
            }
            // 获取缓冲区的原始包错误
            break;
        }
        //  拿到了原始包, 把它加入原始包队列
        frames.insertToQueue(frame);

        //  内存泄露 -------------> 释放 avPacket
        av_packet_unref(pkt);
        releaseAVPacket(&pkt);
    }
    //  内存泄露 -------------> 释放 avPacket
    av_packet_unref(pkt);
    releaseAVPacket(&pkt);

}

//  播放
void VideoChannel::video_play() {
    //  从原始包队列中取出原始包进行播放
    AVFrame *frame = nullptr;
    uint8_t *dst_data[4];
    int dst_linesize[4];

    // int av_image_alloc(uint8_t *pointers[4], int linesizes[4],
    //                    int w, int h, enum AVPixelFormat pix_fmt, int align);
    // return the size in bytes required for the image buffer, a negative
    // error code in case of failure
    av_image_alloc(dst_data, dst_linesize,
                   avCodecContext->width, avCodecContext->height,
                   AV_PIX_FMT_RGBA, 1);

//    struct SwsContext *sws_getContext(int srcW, int srcH, enum AVPixelFormat srcFormat,
//                                      int dstW, int dstH, enum AVPixelFormat dstFormat,
//                                      int flags, SwsFilter *srcFilter,
//                                      SwsFilter *dstFilter, const double *param);
//    return a pointer to an allocated context, or NULL in case of error
    SwsContext *sws_ctx = sws_getContext(
            //  输入数据的参数
            avCodecContext->width, avCodecContext->height,
            avCodecContext->pix_fmt,

            //  输出的参数
            avCodecContext->width, avCodecContext->height,
            AV_PIX_FMT_RGBA,
            //  转换期间使用的算法
            SWS_BILINEAR, nullptr, nullptr, nullptr
    );
    int result_code = 0;
    double extra_delay = 0;
    double fps_delay = 0;
    double real_delay = 0;
    double video_time = 0;
    double audio_time = 0;
    double time_diff = 0;
    while (isPlaying) {
        result_code = frames.getQueueAndDel(frame);

        if (!isPlaying) {
            break;
        }
        if (!result_code) {
            //  拿取失败, 继续下一次拿取
            LOGI("拿取失败, 继续下一次拿取");
            continue;
        }
        //  格式转换    yuv -> rgba
        sws_scale(sws_ctx,
                //  输出的 YUV 数据
                  frame->data, frame->linesize,
                  0, avCodecContext->height,
                //  输出参数
                  dst_data, dst_linesize
        );
        // 音视频同步
        extra_delay = frame->repeat_pict / (2 * fps);
        fps_delay = 1.0 / fps;
        real_delay = extra_delay + fps_delay;

        video_time = frame->best_effort_timestamp * av_q2d(time_base);

        // LOGI("video_time2:%d", video_time) ;
        audio_time = audio_channel->audio_time;

        //  差值
        time_diff = video_time - audio_time;
        if (time_diff > 0) {
            if (time_diff > 1) {
                LOGI("continue========= >1");
                av_usleep(real_delay * 2 * 1000000);
            } else {
                LOGI("continue========= >0");
                av_usleep((real_delay + time_diff) * 1000000);
            }
        } else if (time_diff < 0) {
            if (fabs(time_diff) <= 0.05) {
                LOGI("continue========= <= 0.05");
                frames.sync();
                continue;
            }
        } else {
            LOGI("百分百同步了, impossibility!!!");
        }

        renderCallback(dst_data[0], avCodecContext->width, avCodecContext->height, dst_linesize[0]);

        //  内存泄露 -------------> 释放 AVFrame
        av_frame_unref(frame);
        releaseAVFrame(&frame);
    }
    //  内存泄露 -------------> 释放 AVFrame
    av_frame_unref(frame);
    releaseAVFrame(&frame);
    isPlaying = false;
    av_free(&dst_data[0]);
    sws_freeContext(sws_ctx);
}


void VideoChannel::setRenderCallback(RenderCallback callback) {
    this->renderCallback = callback;
}

void VideoChannel::setAudioChannel(AudioChannel *audio_channel) {
    this->audio_channel = audio_channel;
}

