package com.wells.audio;

import android.Manifest;
import android.app.Activity;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.media.projection.MediaProjectionManager;
import android.media.projection.MediaProjectionConfig;
import android.os.Build;
import android.os.Bundle;

/** Adapted from rom1v/sndcpy MainActivity.java (MIT).
 * Same one-shot entrypoint: request capture, start RecordService, finish.
 */
public final class MainActivity extends Activity {
    private static final int REQUEST_CODE_PERMISSION_AUDIO = 1;
    private static final int REQUEST_CODE_START_CAPTURE = 2;
    private static final int REQUEST_CODE_PERMISSION_NOTIFICATIONS = 3;

    @Override protected void onCreate(Bundle state) {
        super.onCreate(state);
        if (checkSelfPermission(Manifest.permission.RECORD_AUDIO) != PackageManager.PERMISSION_GRANTED) {
            requestPermissions(new String[]{Manifest.permission.RECORD_AUDIO}, REQUEST_CODE_PERMISSION_AUDIO);
        } else {
            requestNotificationPermissionThenCapture();
        }
    }

    private void requestNotificationPermissionThenCapture() {
        // The app targets API 31 to preserve the proven sndcpy capture behavior.
        // On Android 13+, creating the channel while this activity is foreground
        // allows the OS to display its notification consent dialog.
        NotificationManager notifications = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
        notifications.createNotificationChannel(new NotificationChannel(
            "wells_audio", "Wells Audio", NotificationManager.IMPORTANCE_LOW));
        if (Build.VERSION.SDK_INT >= 33
                && checkSelfPermission(Manifest.permission.POST_NOTIFICATIONS)
                    != PackageManager.PERMISSION_GRANTED) {
            requestPermissions(new String[]{Manifest.permission.POST_NOTIFICATIONS},
                REQUEST_CODE_PERMISSION_NOTIFICATIONS);
        } else {
            startCaptureRequest();
        }
    }

    private void startCaptureRequest() {
        MediaProjectionManager manager = (MediaProjectionManager) getSystemService(MEDIA_PROJECTION_SERVICE);
        Intent request = Build.VERSION.SDK_INT >= 34
            ? manager.createScreenCaptureIntent(MediaProjectionConfig.createConfigForDefaultDisplay())
            : manager.createScreenCaptureIntent();
        startActivityForResult(request, REQUEST_CODE_START_CAPTURE);
    }

    @Override public void onRequestPermissionsResult(int code, String[] permissions, int[] results) {
        super.onRequestPermissionsResult(code, permissions, results);
        if (code == REQUEST_CODE_PERMISSION_AUDIO) {
            if (results.length > 0 && results[0] == PackageManager.PERMISSION_GRANTED) requestNotificationPermissionThenCapture();
            else finish();
        } else if (code == REQUEST_CODE_PERMISSION_NOTIFICATIONS) {
            // Notification permission is optional for audio capture. Continue
            // even if denied; Android may hide the notification action controls.
            startCaptureRequest();
        }
    }

    @Override public void onActivityResult(int code, int result, Intent data) {
        super.onActivityResult(code, result, data);
        if (code == REQUEST_CODE_START_CAPTURE && result == RESULT_OK && data != null) {
            RecordService.start(this, data);
        }
        finish();
    }
}
