package top.newjourney.video;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import android.animation.ObjectAnimator;
import android.animation.ValueAnimator;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.widget.SeekBar;
import android.widget.TextView;

import java.io.File;
import java.io.IOException;
import java.util.Arrays;

import top.newjourney.video.customview.ColorTrackTextView;
import top.newjourney.video.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity implements SeekBar.OnSeekBarChangeListener {
    private FFmpegPlayer videoPlayer;

    private ActivityMainBinding binding;
    // private SurfaceView surfaceView ;
    TextView tvTime;
    private boolean isTouch;
    private int duration;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());
        // Example of a call to a native method
        TextView tvState = binding.tvState;
        SurfaceView surfaceView = binding.surfaceView;
        tvTime = binding.tvTime;
        SeekBar seekBar = binding.seekBar;
        seekBar.setOnSeekBarChangeListener(this);

        videoPlayer = new FFmpegPlayer();
        videoPlayer.setSurfaceView(surfaceView);
        videoPlayer.setDataSource(new File(Environment.getExternalStorageDirectory() + File.separator + "chengdu.mp4")
                // videoPlayer.setDataSource(new File(Environment.getExternalStorageDirectory() + File.separator + "demo.mp4")
                .getAbsolutePath());


        videoPlayer.setOnPreparedListener(new FFmpegPlayer.OnPreparedListener() {
            @Override
            public void onPreapared() {
                Log.d("TAG", "onPreapared-----------------");
                duration = videoPlayer.getDuration();

                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        if (0 != duration) {
                            setTvTime(0);
                            tvTime.setVisibility(View.VISIBLE);
                            seekBar.setVisibility(View.VISIBLE);
                        }

                        tvState.setText("onPreapared---------finish--------");
                    }
                });
                videoPlayer.start();
            }

            @Override
            public void onError(int errorCode) {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        tvState.setText(errorCode + "");
                    }
                });
            }
        });

        videoPlayer.setOnProgressListener(new FFmpegPlayer.OnProgressListener() {
            @Override
            public void onProgress(int progress) {
                if (!isTouch) {
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            if (duration != 0) {
                                setTvTime(progress);
                                seekBar.setProgress(progress * 100 / duration);
                            }
                        }
                    });
                }
            }
        });

        tvState.setText(videoPlayer.stringFromJNI());
        checkPermission();
    }

    void setTvTime(int n_time) {
        tvTime.setText(getString(R.string.time_duration, getMinutes(n_time), getSeconds(n_time), getMinutes(duration), getSeconds(duration)));
    }

    int getMinutes(int duration) {

        return duration / 60;
    }

    int getSeconds(int duration) {
        return duration % 60;
    }

    final int REQUEST_CODE_CONTACT = 101;

    /**
     * 简单申请个权限, 不做权限获取监听处理
     */
    private void checkPermission() {
        //申请权限
        if (Build.VERSION.SDK_INT >= 23) {
            String[] permissions = {android.Manifest.permission.READ_EXTERNAL_STORAGE,
                    android.Manifest.permission.WRITE_EXTERNAL_STORAGE,
            };
            //  验证是否许可权限
            for (String str : permissions) {
                if (ActivityCompat.checkSelfPermission(this, str) != PackageManager.PERMISSION_GRANTED) {
                    //  申请权限
                    ActivityCompat.requestPermissions(this, permissions, REQUEST_CODE_CONTACT);
                }
            }
        }
    }

//    @Override
//    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
//        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
//        switch (requestCode) {
//            case REQUEST_CODE_CONTACT:
//                break;
//            default:
//                break;
//        }
//    }

    @Override
    protected void onResume() {
        super.onResume();
        videoPlayer.prepare();
    }


    @Override
    protected void onStop() {
        super.onStop();
        videoPlayer.stop();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        videoPlayer.release();
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
        if (fromUser) {
            setTvTime(progress * duration / 100);
        }
    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {
        isTouch = true;
    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        isTouch = false;
        int seekBarProgress = seekBar.getProgress();
        int playProgress = seekBarProgress * duration / 100;
        videoPlayer.seek(playProgress);
    }

}