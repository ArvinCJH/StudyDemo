package top.newjourney.video;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.SurfaceView;
import android.view.WindowManager;
import android.widget.SeekBar;
import android.widget.TextView;

import java.io.File;
import java.io.IOException;
import java.util.Arrays;

import top.newjourney.video.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity {
    private FFmpegPlayer videoPlayer;

    private ActivityMainBinding binding;
    private SurfaceView surfaceView ;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());
        // Example of a call to a native method
        TextView tvState = binding.tvState;
        surfaceView = binding.surfaceView;
        TextView tvTime = binding.tvTime;
        SeekBar seekBar = binding.seekBar;

        videoPlayer = new FFmpegPlayer();
        videoPlayer.setSurfaceView(surfaceView);
        videoPlayer.setDataSource(new File(Environment.getExternalStorageDirectory() + File.separator + "demo.mp4")
                .getAbsolutePath());
        videoPlayer.setOnPreparedListener(new FFmpegPlayer.OnPreparedListener() {
            @Override
            public void onPreapared() {
                Log.d("TAG", "onPreapared-----------------");

                tvState.setText("onPreapared---------finish--------");
                videoPlayer.start();
            }

            @Override
            public void onError(int errorCode) {

            }
        });
        tvState.setText(videoPlayer.stringFromJNI());
        checkPermission();
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
}