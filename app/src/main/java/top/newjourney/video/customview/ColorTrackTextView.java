package top.newjourney.video.customview;

import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.text.TextUtils;
import android.util.AttributeSet;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.widget.AppCompatTextView;

import top.newjourney.video.R;

/**
 * @author: arvin
 * @CreateTime: 22-3-1
 * @Description: 绘制变色文字
 * @howToUse: ColorTrackTextView colorTrackTextView ;
 * <p>
 * public void setAnimation(ColorTrackTextView.Direction direction){
 * colorTrackTextView.setmDirection(direction);
 * ValueAnimator valueAnimator = ObjectAnimator.ofFloat(0,1) ;
 * valueAnimator.setDuration(2000) ;
 * valueAnimator.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
 * @Override public void onAnimationUpdate(ValueAnimator animation) {
 * colorTrackTextView.setmCurrentProgress((Float) animation.getAnimatedValue());
 * }
 * });
 * }
 */
public class ColorTrackTextView extends AppCompatTextView {

    Paint mOriginColor, mChangeColor;
    float mCurrentProgress = 0.5f;
    private Direction mDirection;

    public enum Direction {
        LEFT_TO_RIGHT, RIGHT_TO_LEFT
    }

    public ColorTrackTextView(@NonNull Context context) {
        this(context, null);
    }

    public ColorTrackTextView(@NonNull Context context, @Nullable AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public ColorTrackTextView(@NonNull Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init(context, attrs);
    }

    private void init(Context context, AttributeSet attrs) {
        TypedArray typedArray = context.obtainStyledAttributes(attrs, R.styleable.ColorTrackTextView);
        //
        int originColor = typedArray.getColor(R.styleable.ColorTrackTextView_originColor, getTextColors().getDefaultColor());
        int changeColor = typedArray.getColor(R.styleable.ColorTrackTextView_changeColor, getTextColors().getDefaultColor());
        mOriginColor = getPaint(originColor);
        mChangeColor = getPaint(changeColor);
        //  recycler
        typedArray.recycle();
    }

    private Paint getPaint(int color) {
        Paint paint = new Paint();
        paint.setColor(color);
        paint.setAntiAlias(true);
        paint.setDither(true);
        paint.setTextSize(getTextSize());
        return paint;
    }

    @Override
    protected void onDraw(Canvas canvas) {
        // getHeight()  文字高度
        // super.onDraw(canvas);
        int currentPoint = (int) (mCurrentProgress * getWidth());
        if (mDirection == Direction.LEFT_TO_RIGHT) {
            drawText(canvas, mOriginColor, currentPoint, getWidth());
            drawText(canvas, mChangeColor, 0, currentPoint);

        } else if (mDirection == Direction.RIGHT_TO_LEFT) {
            drawText(canvas, mOriginColor, 0, getWidth() - currentPoint);
            drawText(canvas, mChangeColor, getWidth() - currentPoint, getWidth());
        }

    }

    private void drawText(Canvas canvas, Paint paint, int start, int end) {
        String text = getText().toString();
        if (TextUtils.isEmpty(text)) {
            return;
        }

        canvas.save();
        Rect clipRect = new Rect(start, 0, end, getWidth());
        //  裁剪区域
        canvas.clipRect(clipRect);
        //  绘制的区域
        Rect textRect = new Rect();
        paint.getTextBounds(text, 0, text.length(), textRect);
        //  获取 x 坐标
        int dx = getWidth() / 2 - textRect.width() / 2;
        canvas.drawText(text, dx, getBaseLine(paint), paint);

        canvas.restore();
    }

    /**
     * 获取基线
     *
     * @param paint
     * @return
     */
    private float getBaseLine(Paint paint) {
        Paint.FontMetricsInt fontMetricsInt = paint.getFontMetricsInt();
        return ((fontMetricsInt.descent - fontMetricsInt.ascent) >> 1) - fontMetricsInt.descent;
        // return ((fontMetricsInt.descent - fontMetricsInt.ascent) /2) -fontMetricsInt.descent;
    }

    public void setmCurrentProgress(float mCurrentProgress) {
        this.mCurrentProgress = mCurrentProgress;
        invalidate();
    }

    public void setmDirection(Direction mDirection) {
        this.mDirection = mDirection;
    }
}
