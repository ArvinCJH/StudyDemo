//
// Created by Administrator on 2022/2/16.
//

#ifndef MYSTUDYAPPLICATION_SAFE_QUEUE_H
#define MYSTUDYAPPLICATION_SAFE_QUEUE_H

#include <queue>
#include <pthread.h>

using namespace std;


template<typename T>    // 泛型：存放任意类型
class SafeQueue {
private:
    typedef void (*ReleaseCallback)(T *);  // 函数指针定义 做回调 用来释放T里面的内容的
    queue<T> queue;
    pthread_mutex_t mutex; // 互斥锁
    pthread_cond_t cond;   // 等待 和 唤醒
    int work;   // 标记队列是否工作
    ReleaseCallback releaseCallback;
public:
    SafeQueue() {
        //  初始化互斥锁
        pthread_mutex_init(&mutex, 0);
        //  初始化唤醒
        pthread_cond_init(&cond, 0);

    }

    ~SafeQueue() {
        //  释放互斥锁
        pthread_mutex_destroy(&mutex);
        //  释放唤醒
        pthread_cond_destroy(&cond);
    }

    int empty() {
        return queue.empty();
    }

    int size() {
        return queue.size();
    }

    //  入队
    void insertToQueue(T value) {
        //  多线程访问, 加锁
        pthread_mutex_lock(&mutex);
        if (work) {
            //  工作状态
            queue.push(value);
            // 每次入队后要唤醒阻塞
            pthread_cond_signal(&cond);
        } else {
            //  如果未设置工作状态, 直接丢包
            if (releaseCallback) {
                // 让外界去释放堆区空间(释放数据空间)
                releaseCallback(&value);
            }
        }
        //  多线程访问, 解锁
        pthread_mutex_unlock(&mutex);
    }

    //  出队
    int getQueueAndDel(T &value) {
        int ret = 0;   // 默认是 false
        //  多线程访问, 加锁
        pthread_mutex_lock(&mutex);
        while (work && empty()) {
            // 如果是工作状态 并且 队列里面没有数据，阻塞等待数据
        }

        if (!empty()) {
            //  队列里面有数据
            value = queue.front();
            //  未使用完, 先不释放数据
            queue.pop();   //  数据出队
            ret = 1;
        }
        //  多线程访问, 解锁
        pthread_mutex_unlock(&mutex);
        return ret;
    }


    void setWork(int work) {
        //  多线程访问, 加锁
        pthread_mutex_lock(&mutex);
        this->work = work;
        // 每次设置状态后，唤醒阻塞
        pthread_cond_signal(&cond);
        //  多线程访问, 解锁
        pthread_mutex_unlock(&mutex);
    }


    //  删除队列中得所有数据, 一个个删除
    void clear() {
        //  多线程访问, 加锁
        pthread_mutex_lock(&mutex);
        for (int i = 0; i < size(); ++i) {
            //  循环释放队列中的数据
            T value = queue.front();
            if (releaseCallback) {
                // 让外界去释放堆区空间(释放数据空间)
                releaseCallback(&value);
            }
            // 删除队列中的数据(出队, 释放队列空间)
            queue.pop();
        }

        //  多线程访问, 解锁
        pthread_mutex_unlock(&mutex);
    }

    /**
     * 设置此函数指针的回调，让外界去释放
     * @param releaseCallback
     */
    void setReleaseCallback(ReleaseCallback releaseCallback) {
        this->releaseCallback = releaseCallback;
    }
};


#endif //MYSTUDYAPPLICATION_SAFE_QUEUE_H
