//
// Created by Administrator on 2022/2/17.
//
#include "JNICallbakcHelper.h"



JNICallbakcHelper::JNICallbakcHelper(JavaVM *vm, JNIEnv *env, jobject jobj) {
    this->vm = vm ;
    this->env = env ;
    //   jobject 不能跨越线程，不能跨越函数，必须全局引用 所以不能使用  this->jobj = jobj ;
    this->jobj = env->NewGlobalRef(jobj) ;   // 需要使用全局引用
    jclass clazz = env->GetObjectClass(jobj) ;
    //  使用映射的方式, 拿取 java 层的对应方法
    jmd_prepared = env->GetMethodID(clazz, "onPrepared", "()V") ;
    jmd_error = env->GetMethodID(clazz, "onError", "(I)V") ;
    jmd_seek = env->GetMethodID(clazz, "onProgressListener", "(I)V") ;
}

JNICallbakcHelper::~JNICallbakcHelper() {
    vm = nullptr;
    env->DeleteGlobalRef(jobj);
    jobj = nullptr;
    env = nullptr;
}

void JNICallbakcHelper::onPrepared(int thread_mode) {
    if(thread_mode == THREAD_MAIN){
        // 主线程
        env->CallVoidMethod(jobj, jmd_prepared);
    }else if (thread_mode == THREAD_CHILD) {
        //  子线程 env也不可以跨线程 需要一个全新的env
        JNIEnv * env_child;
        vm->AttachCurrentThread(&env_child,nullptr) ;
        env_child->CallVoidMethod(jobj, jmd_prepared) ;
        vm->DetachCurrentThread() ;
    }
}

//  返回错误码到上层
void JNICallbakcHelper::onError(int thread_mode, int error_code) {
    if(thread_mode == THREAD_MAIN){
        // 主线程
        env->CallVoidMethod(jobj, jmd_error, error_code);
    }else if (thread_mode == THREAD_CHILD) {
        //  子线程 env也不可以跨线程 需要一个全新的env
        JNIEnv * env_child;
        vm->AttachCurrentThread(&env_child, nullptr) ;
        env_child->CallVoidMethod(jobj, jmd_error, error_code) ;
        vm->DetachCurrentThread() ;
    }
}

void JNICallbakcHelper::onSeek(int thread_mode, int seek_progress) {

    if(thread_mode == THREAD_MAIN){
        // 主线程
        env->CallVoidMethod(jobj, jmd_seek, seek_progress);
    }else if (thread_mode == THREAD_CHILD) {
        //  子线程 env也不可以跨线程 需要一个全新的env
        JNIEnv * env_child;
        vm->AttachCurrentThread(&env_child, nullptr) ;
        env_child->CallVoidMethod(jobj, jmd_seek, seek_progress) ;
        vm->DetachCurrentThread() ;
    }
}
