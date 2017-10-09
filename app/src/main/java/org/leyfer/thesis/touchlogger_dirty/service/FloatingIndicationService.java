package org.leyfer.thesis.touchlogger_dirty.service;

import android.app.Service;
import android.content.Intent;
import android.graphics.PixelFormat;
import android.graphics.Point;
import android.os.Build;
import android.os.IBinder;
import android.provider.Settings;
import android.support.annotation.Nullable;
import android.util.Log;
import android.view.Display;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.TextView;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;
import org.leyfer.thesis.touchlogger_dirty.R;
import org.leyfer.thesis.touchlogger_dirty.activity.MainActivity;
import org.leyfer.thesis.touchlogger_dirty.event.TouchCoordinatesEvent;

import static android.view.MotionEvent.ACTION_DOWN;
import static android.view.MotionEvent.ACTION_MOVE;
import static android.view.MotionEvent.ACTION_UP;

public class FloatingIndicationService extends Service {
    private WindowManager mWindowManager;
    private View mIndicatorView;
    private WindowManager.LayoutParams indicatorParams;

    private TextView xCoordView;
    private TextView yCoordView;

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        EventBus.getDefault().register(this);

        Log.d(MainActivity.TAG, String.format("Registered: %b",
                EventBus.getDefault().hasSubscriberForEvent(TouchCoordinatesEvent.class)));

        mWindowManager = (WindowManager) getSystemService(WINDOW_SERVICE);

        LayoutInflater inflater = LayoutInflater.from(this);
        mIndicatorView = inflater.inflate(R.layout.indicator_view, null);
        xCoordView = (TextView) mIndicatorView.findViewById(R.id.x_value);
        yCoordView = (TextView) mIndicatorView.findViewById(R.id.y_value);

        xCoordView.setText("????");
        yCoordView.setText("????");

        indicatorParams = getIndicatorParams();

        mIndicatorView.setOnTouchListener(new View.OnTouchListener() {
            int initialX, initialY;
            float initialTouchX, initialTouchY;

            @Override
            public boolean onTouch(View v, MotionEvent event) {
                switch (event.getAction()) {
                    case ACTION_DOWN:
                        initialX = indicatorParams.x;
                        initialY = indicatorParams.y;
                        initialTouchX = event.getRawX();
                        initialTouchY = event.getRawY();
                        return true;

                    case ACTION_MOVE:
                        int newPosX = initialX + (int) (event.getRawX() - initialTouchX);
                        int newPosY = initialY + (int) (event.getRawY() - initialTouchY);
                        indicatorParams.x = newPosX;
                        indicatorParams.y = newPosY;
                        mWindowManager.updateViewLayout(mIndicatorView, indicatorParams);
                        return true;

                    case ACTION_UP:
                        return true;

                    default:
                        return false;
                }
            }
        });

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            if (Settings.canDrawOverlays(this)) {
                mWindowManager.addView(mIndicatorView, indicatorParams);
            }
        } else {
            mWindowManager.addView(mIndicatorView, indicatorParams);
        }

        return START_NOT_STICKY;
    }

    private WindowManager.LayoutParams getIndicatorParams() {
        Display display = mWindowManager.getDefaultDisplay();
        Point size = new Point();
        display.getSize(size);

        WindowManager.LayoutParams params = new WindowManager.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT,
                WindowManager.LayoutParams.TYPE_PRIORITY_PHONE,
                WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE,
                PixelFormat.TRANSLUCENT
        );

        params.gravity = Gravity.TOP | Gravity.START;
        params.alpha = (float) 0.8;
        params.x = size.x / 2;
        params.y = size.y / 2;
        return params;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        EventBus.getDefault().unregister(this);

        Log.d(MainActivity.TAG, String.format("Unregistered: %b",
                EventBus.getDefault().hasSubscriberForEvent(TouchCoordinatesEvent.class)));

        if (mIndicatorView != null) {
            mWindowManager.removeView(mIndicatorView);
        }
    }

    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onTouchCoordinatesEvent(TouchCoordinatesEvent event) {
        xCoordView.setText(String.valueOf(event.x));
        yCoordView.setText(String.valueOf(event.y));
    }
}
