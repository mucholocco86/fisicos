package com.wells.audio;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.widget.RemoteViews;

/** Wells Audio notification presentation; no audio capture or socket logic. */
final class WellsNotificationManager {
    static final String CHANNEL_ID = "wells_audio";
    static final int NOTIFICATION_ID = 1;

    private WellsNotificationManager() {}

    static void ensureChannel(Context context) {
        NotificationManager manager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
        manager.createNotificationChannel(new NotificationChannel(
            CHANNEL_ID, "Wells Audio", NotificationManager.IMPORTANCE_LOW));
    }

    static NotificationManager manager(Context context) {
        return (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
    }

    static Notification create(Context context, boolean established) {
        int selected = context.getSharedPreferences("quality", Context.MODE_PRIVATE).getInt("bitrate", 96);
        Notification.Builder builder = new Notification.Builder(context, CHANNEL_ID)
            .setContentTitle("Wells Audio")
            .setContentText((established ? "Transmitindo" : "Aguardando conexão") +
                " • PCM original • preferência " + selected + " kbps")
            .setSmallIcon(android.R.drawable.ic_lock_silent_mode_off)
            .setOngoing(true);
        // Android's standard notification shows at most three action buttons.
        // Use an expanded custom view so Stop remains visible alongside all rates.
        RemoteViews expanded = new RemoteViews(context.getPackageName(), R.layout.wells_audio_notification);
        expanded.setTextViewText(R.id.audio_status,
            (established ? "Transmitindo" : "Aguardando conexão") + " • PCM original • preferência " + selected + " kbps");
        for (int rate : new int[]{128, 96, 32}) {
            Intent choose = new Intent(context, RecordService.class);
            choose.setAction(RecordService.ACTION_RATE);
            choose.putExtra(RecordService.EXTRA_RATE, rate);
            PendingIntent pending = PendingIntent.getService(context, rate, choose,
                PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE);
            int viewId = rate == 128 ? R.id.rate_128 : rate == 96 ? R.id.rate_96 : R.id.rate_32;
            expanded.setTextViewText(viewId, (rate == selected ? "✓ " : "") + rate + " kbps");
            expanded.setOnClickPendingIntent(viewId, pending);
        }
        Intent stop = new Intent(context, RecordService.class);
        stop.setAction(RecordService.ACTION_STOP);
        PendingIntent stopAction = PendingIntent.getService(context, 1, stop,
            PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE);
        expanded.setOnClickPendingIntent(R.id.audio_stop, stopAction);
        builder.setStyle(new Notification.DecoratedCustomViewStyle());
        builder.setCustomBigContentView(expanded);
        // Tapping the collapsed notification also provides a way to stop.
        builder.setContentIntent(stopAction);
        return builder.build();
    }

}
