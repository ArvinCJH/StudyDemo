#include <jni.h>
#include <string>
#include "FfmpegPlayer.h"
#include "JNICallbakcHelper.h"


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

FfmpegPlayer *player = 0;
JavaVM *vm = 0;


extern "C"
JNIEXPORT void JNICALL
Java_top_newjourney_video_FFmpegPlayer_prepareNative(JNIEnv *env, jobject thiz,
                                                     jstring data_source) {
    const char* _data_source = env.GetStringUTFChars(data_source, 0) ;
    player = new FfmpegPlayer() ;
    player.prepare() ;
    env.ReleaseStringUTFChars(data_source, _data_source) ;

}

extern "C"
JNIEXPORT void JNICALL
Java_top_newjourney_video_FFmpegPlayer_startNative(JNIEnv *env, jobject thiz) {


}

extern "C"
JNIEXPORT void JNICALL
Java_top_newjourney_video_FFmpegPlayer_stopNative(JNIEnv *env, jobject thiz) {


}

extern "C"
JNIEXPORT void JNICALL
Java_top_newjourney_video_FFmpegPlayer_releaseNative(JNIEnv *env, jobject thiz) {


}