package top.newjourney.video;

import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import androidx.annotation.NonNull;

public class FFmpegPlayer implements SurfaceHolder.Callback {
    // C++层准备情况的接口
    private OnPreparedListener onPreparedListener;

    private SurfaceHolder surfaceHolder;

    // 媒体源（文件路径， 直播地址rtmp）
    private String dataSource;

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }


    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();

    public void setDataSource(String dataSource) {
        this.dataSource = dataSource;
    }

    /**
     * 播放前的 准备工作
     */
    public void prepare() {
        prepareNative(dataSource);
    }

    /**
     * 开始播放
     */
    public void start() {
        startNative();
    }

    /**
     * 停止播放
     */
    public void stop() {
        stopNative();
    }

    /**
     * 释放资源
     */
    public void release() {
        releaseNative();
    }

    public void setSurfaceView(SurfaceView surfaceView) {
        if (null != surfaceHolder) {
            surfaceHolder.removeCallback(this);
        }
        surfaceHolder = surfaceView.getHolder();
        surfaceHolder.addCallback(this);
    }

    /**
     * 设置准备OK的监听方法
     */
    public void setOnPreparedListener(OnPreparedListener onPreparedListener) {
        this.onPreparedListener = onPreparedListener;
    }

    /**
     * 给jni反射调用的
     */
    private void onPrepared() {
        if (null != onPreparedListener) {
            onPreparedListener.onPreapared();
        }
    }

    /**
     * 给jni反射调用的
     */
    private void onError(int errorCode) {
        onPreparedListener.onError(errorCode);
    }

    @Override
    public void surfaceCreated(@NonNull SurfaceHolder holder) {

    }

    @Override
    public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {
        setSurfaceNative(surfaceHolder.getSurface());
    }

    @Override
    public void surfaceDestroyed(@NonNull SurfaceHolder holder) {

    }

    interface OnPreparedListener {
        void onPreapared();

        void onError(int errorCode);
    }


    //region native 函数区域
    private native void prepareNative(String dataSource);

    //  开始播放
    private native void startNative();
    // 停止播放
    private native void stopNative();
    // 释放
    private native void releaseNative();
    // 设置播放窗口
    private native void setSurfaceNative(Surface surface);
    //endregion

}
