package top.newjourney.video;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.SurfaceView;
import android.view.WindowManager;
import android.widget.SeekBar;
import android.widget.TextView;

import top.newjourney.video.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity {
    FFmpegPlayer videoPlayer;

    private ActivityMainBinding binding;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());
        videoPlayer = new FFmpegPlayer();
        // Example of a call to a native method
        TextView tvState = binding.tvState;
        SurfaceView surfaceView = binding.surfaceView;
        TextView tvTime = binding.tvTime;
        SeekBar seekBar = binding.seekBar;
        tvState.setText(videoPlayer.stringFromJNI());
    }


    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    protected void onStop() {
        super.onStop();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }
}