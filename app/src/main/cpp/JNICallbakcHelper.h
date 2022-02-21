//
// Created by Administrator on 2022/2/16.
//

#ifndef MYSTUDYAPPLICATION_JNICALLBAKCHELPER_H
#define MYSTUDYAPPLICATION_JNICALLBAKCHELPER_H

#include <jni.h>
#include "util.h"

class JNICallbakcHelper {
private:
    JavaVM *vm = 0;
    JNIEnv *env = 0;
    jobject jobj;
    jmethodID jmd_prepared;
    jmethodID jmd_error;

public:
    JNICallbakcHelper(JavaVM *vm, JNIEnv *env, jobject jobj);

    ~JNICallbakcHelper();

    void onPrepared(int thread_mode);
    void onError(int thread_mode, int error_code);
};


#endif //MYSTUDYAPPLICATION_JNICALLBAKCHELPER_H
