#include <jni.h>
#include <string>
#include "FFmpegPlayer.h"
#include <android/native_window_jni.h>


extern "C" {
#include <libavutil/avutil.h>
}

extern "C" JNIEXPORT jstring JNICALL
Java_top_newjourney_video_FFmpegPlayer_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
//    std::string hello = "Hello from C++";
    std::string info = "FFmpeg version is :";
    info.append(av_version_info());
    return env->NewStringUTF(info.c_str());
}

// >>>>>>>>>>>>>>>>>>>>>

FFmpegPlayer *player = nullptr;
JavaVM *vm = nullptr;
ANativeWindow *window = nullptr;
ANativeWindow_Buffer window_buffer;
// 静态初始化锁
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

jint JNI_OnLoad(JavaVM *vm, void *args) {
    ::vm = vm;
    return JNI_VERSION_1_6;
}

void renderFrame(uint8_t *src_data, int width, int height, int src_lineSize) {
    pthread_mutex_lock(&mutex);
    if (!window) {
        pthread_mutex_unlock(&mutex);
        //  释放锁之后需要 return 掉, 因为不持有 navtiveWindow 锁无法再次安全渲染, 会崩溃
        return;
    }

    ANativeWindow_setBuffersGeometry(window, width, height,
                                     WINDOW_FORMAT_RGBA_8888);
    // int error_code = ANativeWindow_setBuffersGeometry(window, width, height,
    //                                                   WINDOW_FORMAT_RGBA_8888);
    // if (0!=error_code) {
    //     //  0 for success, or a negative value on error.
    //     LOGE("renderFrame ========1.2.1============");
    //     pthread_mutex_unlock(&mutex);
    //     return;
    // }

    if (ANativeWindow_lock(window, &window_buffer, nullptr)) {
        ANativeWindow_release(window);
        window = nullptr;
        pthread_mutex_unlock(&mutex);
        return;
    }
    // uint8_t *dst_data = static_cast<uint8_t *>(window_buffer.bits);
    auto *dst_data = static_cast<uint8_t *>(window_buffer.bits);
    int dst_linesize = window_buffer.stride * 4;
    for (int i = 0; i < window_buffer.height; ++i) {
        memcpy(dst_data + i * dst_linesize, src_data + i * src_lineSize, dst_linesize);
    }
    ANativeWindow_unlockAndPost(window);
    pthread_mutex_unlock(&mutex);

}

extern "C"
JNIEXPORT void JNICALL
Java_top_newjourney_video_FFmpegPlayer_prepareNative(JNIEnv *env, jobject thiz,
                                                     jstring data_source) {
    const char *_data_source = env->GetStringUTFChars(data_source, nullptr);
    auto *helper = new JNICallbakcHelper(vm, env, thiz);
    player = new FFmpegPlayer(_data_source, helper);
    player->setRenderCallback(renderFrame);
    player->prepare();
    env->ReleaseStringUTFChars(data_source, _data_source);

}

extern "C"
JNIEXPORT void JNICALL
Java_top_newjourney_video_FFmpegPlayer_startNative(JNIEnv *env, jobject thiz) {
    if (player) {
        player->start();
    }

}

extern "C"
JNIEXPORT void JNICALL
Java_top_newjourney_video_FFmpegPlayer_stopNative(JNIEnv *env, jobject thiz) {


}

extern "C"
JNIEXPORT void JNICALL
Java_top_newjourney_video_FFmpegPlayer_releaseNative(JNIEnv *env, jobject thiz) {


}

extern "C"
JNIEXPORT void JNICALL
Java_top_newjourney_video_FFmpegPlayer_setSurfaceNative(JNIEnv *env, jobject thiz,
                                                        jobject surface) {
    pthread_mutex_lock(&mutex);

    //  先释放之前的显示窗口
    if (window) {
        ANativeWindow_release(window);
        window = nullptr;
    }
    window = ANativeWindow_fromSurface(env, surface);
    pthread_mutex_unlock(&mutex);
}

extern "C"
JNIEXPORT void JNICALL
Java_top_newjourney_video_FFmpegPlayer_seekNative(JNIEnv *env, jobject thiz, jint play_progress) {
    if (player){
        player->seek(play_progress) ;
    }
}

extern "C"
JNIEXPORT jint JNICALL
Java_top_newjourney_video_FFmpegPlayer_getDurationNative(JNIEnv *env, jobject thiz) {

    if (player){
        return player->getDuration();
    }
    return 0 ;

}