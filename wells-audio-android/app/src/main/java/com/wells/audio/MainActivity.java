package com.wells.audio;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.media.projection.MediaProjectionManager;
import android.media.projection.MediaProjectionConfig;
import android.os.Build;
import android.os.Bundle;

/**
 * No settings panel: Windows owns start, Android notification owns controls.
 * MediaProjection consent is Android-controlled and cannot be silently bypassed.
 */
public final class MainActivity extends Activity {
    private static final int REQUEST_CAPTURE = 4001;
    private static final int REQUEST_AUDIO_PERMISSION = 4002;
    private boolean captureRequested;

    private void requestCapture() {
        if (checkSelfPermission(Manifest.permission.RECORD_AUDIO) != PackageManager.PERMISSION_GRANTED) {
            requestPermissions(new String[]{Manifest.permission.RECORD_AUDIO}, REQUEST_AUDIO_PERMISSION);
            return;
        }
        MediaProjectionManager manager = (MediaProjectionManager) getSystemService(Context.MEDIA_PROJECTION_SERVICE);
        Intent request;
        if (Build.VERSION.SDK_INT >= 34) {
            // Default display only: avoids the single-app/full-display picker.
            // Android still presents the mandatory user-consent dialog.
            request = manager.createScreenCaptureIntent(MediaProjectionConfig.createConfigForDefaultDisplay());
        } else {
            request = manager.createScreenCaptureIntent();
        }
        startActivityForResult(request, REQUEST_CAPTURE);
    }

    @Override public void onCreate(Bundle state) {
        super.onCreate(state);
        if (state == null) requestCapture();
    }

    @Override public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] results) {
        super.onRequestPermissionsResult(requestCode, permissions, results);
        if (requestCode == REQUEST_AUDIO_PERMISSION) {
            if (results.length > 0 && results[0] == PackageManager.PERMISSION_GRANTED) requestCapture();
            else finish();
        }
    }

    @Override protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == REQUEST_CAPTURE && resultCode == RESULT_OK && data != null) {
            Intent start = new Intent(this, AudioCaptureService.class);
            start.setAction(AudioCaptureService.ACTION_START);
            start.putExtra(AudioCaptureService.EXTRA_RESULT_CODE, resultCode);
            start.putExtra(AudioCaptureService.EXTRA_RESULT_DATA, data);
            startForegroundService(start);
        }
        finish();
    }
}
