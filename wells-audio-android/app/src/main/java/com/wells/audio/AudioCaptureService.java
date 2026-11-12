package com.wells.audio;

import android.app.Notification;
import android.app.PendingIntent;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.Service;
import android.content.Intent;
import android.content.pm.ServiceInfo;
import android.media.AudioAttributes;
import android.media.AudioFormat;
import android.media.AudioPlaybackCaptureConfiguration;
import android.media.AudioRecord;
import android.media.projection.MediaProjection;
import android.media.projection.MediaProjectionManager;
import android.os.Build;
import android.os.IBinder;
import android.os.Process;
import android.net.LocalServerSocket;
import android.net.LocalSocket;
import android.util.Log;
import java.io.OutputStream;

/**
 * Experimental PCM transport compatible with the existing Windows 48kHz
 * stereo 16-bit playback sink. This is NOT 32/96/128 kbps compression yet.
 */
public final class AudioCaptureService extends Service {
    public static final String ACTION_STOP = "com.wells.audio.STOP";
    public static final String ACTION_START = "com.wells.audio.START";
    public static final String ACTION_QUALITY = "com.wells.audio.QUALITY";
    public static final String EXTRA_BITRATE = "bitrate";
    public static final String EXTRA_RESULT_CODE = "resultCode";
    public static final String EXTRA_RESULT_DATA = "resultData";
    private static final String TAG = "WellsAudio";
    private volatile boolean running;
    private Thread worker;
    private AudioRecord recorder;
    private MediaProjection projection;
    private LocalServerSocket server;
    private LocalSocket client;

    @Override public IBinder onBind(Intent intent) { return null; }

    @Override public int onStartCommand(Intent intent, int flags, int startId) {
        if (intent == null || ACTION_STOP.equals(intent.getAction())) {
            stopSelf();
            return START_NOT_STICKY;
        }
        if (ACTION_QUALITY.equals(intent.getAction())) {
            int selected = intent.getIntExtra(EXTRA_BITRATE, 96);
            if (selected == 32 || selected == 96 || selected == 128) {
                getSharedPreferences("quality", MODE_PRIVATE).edit().putInt("bitrate", selected).apply();
                if (running) getSystemService(NotificationManager.class).notify(103, buildNotification());
            }
            return START_NOT_STICKY;
        }
        if (!ACTION_START.equals(intent.getAction()) || running) return START_NOT_STICKY;
        NotificationManager manager = getSystemService(NotificationManager.class);
        manager.createNotificationChannel(new NotificationChannel("wells_audio", "Wells Audio", NotificationManager.IMPORTANCE_LOW));
        Notification notification = buildNotification();
        if (Build.VERSION.SDK_INT >= 29) {
            startForeground(103, notification, ServiceInfo.FOREGROUND_SERVICE_TYPE_MEDIA_PROJECTION);
        } else {
            startForeground(103, notification);
        }
        int resultCode = intent.getIntExtra(EXTRA_RESULT_CODE, 0);
        Intent data = Build.VERSION.SDK_INT >= 33
            ? intent.getParcelableExtra(EXTRA_RESULT_DATA, Intent.class)
            : (Intent) intent.getParcelableExtra(EXTRA_RESULT_DATA);
        if (data == null || resultCode != android.app.Activity.RESULT_OK) {
            stopSelf();
            return START_NOT_STICKY;
        }
        MediaProjectionManager mp = getSystemService(MediaProjectionManager.class);
        projection = mp.getMediaProjection(resultCode, data);
        if (projection == null) {
            stopSelf();
            return START_NOT_STICKY;
        }
        projection.registerCallback(new MediaProjection.Callback() {
            @Override public void onStop() { stopSelf(); }
        }, new android.os.Handler(getMainLooper()));
        running = true;
        worker = new Thread(this::stream, "WellsAudioPCM");
        worker.start();
        return START_NOT_STICKY;
    }

    private Notification buildNotification() {
        int selected = getSharedPreferences("quality", MODE_PRIVATE).getInt("bitrate", 96);
        Intent stopIntent = new Intent(this, AudioCaptureService.class);
        stopIntent.setAction(ACTION_STOP);
        PendingIntent stop = PendingIntent.getService(this, 1, stopIntent,
            PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE);
        Notification.Builder builder = new Notification.Builder(this, "wells_audio")
            .setSmallIcon(android.R.drawable.ic_lock_silent_mode_off)
            .setContentTitle("Wells Audio — transmitindo")
            .setContentText("PCM sem compressão • preferência: " + selected + " kbps")
            .setOngoing(true);
        // Quality buttons save the future codec preference; PCM transport is
        // currently fixed at 1536 kbps. Do not claim these buttons compress.
        for (int rate : new int[]{128, 96, 32}) {
            Intent choose = new Intent(this, AudioCaptureService.class);
            choose.setAction(ACTION_QUALITY);
            choose.putExtra(EXTRA_BITRATE, rate);
            PendingIntent action = PendingIntent.getService(this, rate, choose,
                PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE);
            builder.addAction(android.R.drawable.ic_menu_manage,
                (selected == rate ? "✓ " : "") + rate + " kbps", action);
        }
        builder.addAction(android.R.drawable.ic_media_pause, "Parar", stop);
        return builder.build();
    }

    private void stream() {
        Process.setThreadPriority(Process.THREAD_PRIORITY_AUDIO);
        try {
            AudioPlaybackCaptureConfiguration capture = new AudioPlaybackCaptureConfiguration.Builder(projection)
                .addMatchingUsage(AudioAttributes.USAGE_MEDIA)
                .addMatchingUsage(AudioAttributes.USAGE_GAME)
                .addMatchingUsage(AudioAttributes.USAGE_UNKNOWN)
                .build();
            AudioFormat format = new AudioFormat.Builder()
                .setEncoding(AudioFormat.ENCODING_PCM_16BIT)
                .setSampleRate(48000)
                .setChannelMask(AudioFormat.CHANNEL_IN_STEREO).build();
            int min = AudioRecord.getMinBufferSize(48000, AudioFormat.CHANNEL_IN_STEREO, AudioFormat.ENCODING_PCM_16BIT);
            if (min <= 0) throw new IllegalStateException("Unsupported PCM format");
            recorder = new AudioRecord.Builder()
                .setAudioFormat(format)
                .setAudioPlaybackCaptureConfig(capture)
                .setBufferSizeInBytes(Math.max(min * 2, 19200)).build();
            if (recorder.getState() != AudioRecord.STATE_INITIALIZED) throw new IllegalStateException("AudioRecord not initialized");
            server = new LocalServerSocket("wells_audio");
            recorder.startRecording();
            byte[] buffer = new byte[3840]; // 20 ms, 48 kHz stereo 16-bit
            while (running) {
                client = server.accept();
                try {
                    OutputStream out = client.getOutputStream();
                    while (running) {
                        int count = recorder.read(buffer, 0, buffer.length, AudioRecord.READ_BLOCKING);
                        if (count < 0) throw new IllegalStateException("AudioRecord read error: " + count);
                        if (count > 0) out.write(buffer, 0, count);
                    }
                } catch (java.io.IOException disconnected) {
                    Log.i(TAG, "Receiver disconnected: " + disconnected.getMessage());
                } finally {
                    try { client.close(); } catch (Exception ignored) {}
                    client = null;
                }
            }
        } catch (Exception e) {
            Log.e(TAG, "Audio capture stopped", e);
        } finally {
            stopSelf();
        }
    }

    @Override public void onDestroy() {
        running = false;
        if (client != null) try { client.close(); } catch (Exception ignored) {}
        if (server != null) try { server.close(); } catch (Exception ignored) {}
        if (recorder != null) {
            try { recorder.stop(); } catch (Exception ignored) {}
            recorder.release();
            recorder = null;
        }
        if (projection != null) {
            MediaProjection old = projection;
            projection = null;
            old.stop();
        }
        super.onDestroy();
    }
}
