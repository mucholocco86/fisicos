package com.wells.audio;

// Adapted from rom1v/sndcpy RecordService.java (MIT).

import android.app.Activity;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.widget.RemoteViews;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ServiceInfo;
import android.graphics.drawable.Icon;
import android.media.AudioAttributes;
import android.media.AudioFormat;
import android.media.AudioPlaybackCaptureConfiguration;
import android.media.AudioRecord;
import android.media.projection.MediaProjection;
import android.media.projection.MediaProjectionManager;
import android.net.LocalServerSocket;
import android.net.LocalSocket;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.util.Log;

import java.io.IOException;

public class RecordService extends Service {

    private static final String TAG = "WellsAudio";
    private static final String ACTION_RECORD = "com.wells.audio.RECORD";
    static final String ACTION_STOP = "com.wells.audio.STOP";
    static final String ACTION_RATE = "com.wells.audio.RATE";
    static final String EXTRA_RATE = "rate";
    private static final String EXTRA_MEDIA_PROJECTION_DATA = "mediaProjectionData";

    private static final int MSG_CONNECTION_ESTABLISHED = 1;

    private static final String SOCKET_NAME = "wells_audio";


    private static final int SAMPLE_RATE = 48000;
    private static final int CHANNELS = 2;

    private final Handler handler = new ConnectionHandler(this);
    private MediaProjectionManager mediaProjectionManager;
    private MediaProjection mediaProjection;
    private Thread recorderThread;
    private volatile boolean stopping;
    private volatile LocalSocket activeSocket;
    private volatile AudioRecord activeRecorder;

    public static void start(Context context, Intent data) {
        Intent intent = new Intent(context, RecordService.class);
        intent.setAction(ACTION_RECORD);
        intent.putExtra(EXTRA_MEDIA_PROJECTION_DATA, data);
        context.startForegroundService(intent);
    }

    @Override
    public void onCreate() {
        super.onCreate();

        WellsNotificationManager.ensureChannel(this);
        Notification notification = WellsNotificationManager.create(this, false);

        startForeground(WellsNotificationManager.NOTIFICATION_ID, notification, ServiceInfo.FOREGROUND_SERVICE_TYPE_MEDIA_PROJECTION);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (intent == null) return START_NOT_STICKY;
        String action = intent.getAction();
        if (ACTION_RATE.equals(action)) {
            int rate = intent.getIntExtra(EXTRA_RATE, 96);
            if (rate == 32 || rate == 96 || rate == 128) {
                getSharedPreferences("quality", MODE_PRIVATE).edit().putInt("bitrate", rate).apply();
                WellsNotificationManager.manager(this).notify(WellsNotificationManager.NOTIFICATION_ID, WellsNotificationManager.create(this, isRunning()));
            }
            return START_NOT_STICKY;
        }
        if (ACTION_STOP.equals(action)) {
            stopRecording();
            stopSelf();
            return START_NOT_STICKY;
        }

        if (isRunning()) {
            return START_NOT_STICKY;
        }

        Intent data = intent.getParcelableExtra(EXTRA_MEDIA_PROJECTION_DATA);
        mediaProjectionManager = (MediaProjectionManager) getSystemService(MEDIA_PROJECTION_SERVICE);
        mediaProjection = mediaProjectionManager.getMediaProjection(Activity.RESULT_OK, data);
        if (mediaProjection != null) {
            startRecording();
        } else {
            Log.w(TAG, "Failed to capture audio");
            stopSelf();
        }
        return START_NOT_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    private static LocalSocket connect() throws IOException {
        LocalServerSocket localServerSocket = new LocalServerSocket(SOCKET_NAME);
        try {
            return localServerSocket.accept();
        } finally {
            localServerSocket.close();
        }
    }

    private static AudioPlaybackCaptureConfiguration createAudioPlaybackCaptureConfig(MediaProjection mediaProjection) {
        AudioPlaybackCaptureConfiguration.Builder confBuilder = new AudioPlaybackCaptureConfiguration.Builder(mediaProjection);
        confBuilder.addMatchingUsage(AudioAttributes.USAGE_MEDIA);
        confBuilder.addMatchingUsage(AudioAttributes.USAGE_GAME);
        confBuilder.addMatchingUsage(AudioAttributes.USAGE_UNKNOWN);
        return confBuilder.build();
    }

    private static AudioFormat createAudioFormat() {
        AudioFormat.Builder builder = new AudioFormat.Builder();
        builder.setEncoding(AudioFormat.ENCODING_PCM_16BIT);
        builder.setSampleRate(SAMPLE_RATE);
        builder.setChannelMask(CHANNELS == 2 ? AudioFormat.CHANNEL_IN_STEREO : AudioFormat.CHANNEL_IN_MONO);
        return builder.build();
    }

    private static AudioRecord createAudioRecord(MediaProjection mediaProjection) {
        AudioRecord.Builder builder = new AudioRecord.Builder();
        builder.setAudioFormat(createAudioFormat());
        builder.setBufferSizeInBytes(1024 * 1024);
        builder.setAudioPlaybackCaptureConfig(createAudioPlaybackCaptureConfig(mediaProjection));
        return builder.build();
    }

    private void startRecording() {
        final AudioRecord recorder = createAudioRecord(mediaProjection);
        stopping = false;
        activeRecorder = recorder;

        recorderThread = new Thread(new Runnable() {
            @Override
            public void run() {
                try (LocalSocket socket = connect()) {
                    activeSocket = socket;
                    if (stopping) return;
                    handler.sendEmptyMessage(MSG_CONNECTION_ESTABLISHED);

                    recorder.startRecording();
                    int BUFFER_MS = 15; // do not buffer more than BUFFER_MS milliseconds
                    byte[] buf = new byte[SAMPLE_RATE * CHANNELS * 2 * BUFFER_MS / 1000];
                    while (!stopping) {
                        int r = recorder.read(buf, 0, buf.length);
                        if (r < 0) break;
                        if (r > 0) socket.getOutputStream().write(buf, 0, r);
                    }
                } catch (IOException e) {
                    // ignore
                } finally {
                    try { recorder.stop(); } catch (IllegalStateException ignored) {}
                    recorder.release();
                    activeRecorder = null;
                    activeSocket = null;
                    stopSelf();
                }
            }
        });
        recorderThread.start();
    }

    private boolean isRunning() {
        return recorderThread != null;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        stopForeground(true);
        stopRecording();
    }

    private void stopRecording() {
        stopping = true;
        LocalSocket socket = activeSocket;
        if (socket != null) {
            try { socket.close(); } catch (IOException ignored) {}
        }
        AudioRecord recorder = activeRecorder;
        if (recorder != null) {
            try { recorder.stop(); } catch (IllegalStateException ignored) {}
        }
        if (recorderThread != null) {
            recorderThread.interrupt();
            recorderThread = null;
        }
    }

    private static final class ConnectionHandler extends Handler {

        private RecordService service;

        ConnectionHandler(RecordService service) {
            this.service = service;
        }

        @Override
        public void handleMessage(Message message) {
            if (!service.isRunning()) {
                // if the VPN is not running anymore, ignore obsolete events
                return;
            }

            if (message.what == MSG_CONNECTION_ESTABLISHED) {
                Notification notification = WellsNotificationManager.create(service, true);
                WellsNotificationManager.manager(service).notify(WellsNotificationManager.NOTIFICATION_ID, notification);
            }
        }
    }
}