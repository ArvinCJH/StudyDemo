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

public:
    JNICallbakcHelper(JavaVM *vm, JNIEnv *env, jobject jobj);

    ~JNICallbakcHelper();

    void onPrepared(int i);
};


#endif //MYSTUDYAPPLICATION_JNICALLBAKCHELPER_H
