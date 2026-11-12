package com.wells.audio;

import android.app.Activity;
import android.content.Intent;
import android.content.Context;
import android.content.pm.PackageManager;
import android.media.projection.MediaProjectionManager;
import android.Manifest;
import android.os.Build;
import android.os.Bundle;
import android.graphics.Color;
import android.graphics.Typeface;
import android.graphics.drawable.GradientDrawable;
import android.view.Gravity;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.Switch;
import android.widget.TextView;

public final class MainActivity extends Activity {
    private static final int BG = Color.rgb(12, 12, 14);
    private static final int MUTED = Color.rgb(175, 175, 180);
    private TextView explanation;
    private RadioGroup qualities;
    private Switch enabled;
    private static final int REQUEST_CAPTURE = 4001;
    private static final int REQUEST_AUDIO_PERMISSION = 4002;
    private boolean changingSwitch;
    private boolean autoStart;

    private int dp(float v) { return Math.round(v * getResources().getDisplayMetrics().density); }
    private TextView text(String value, int size, boolean bold, int color) {
        TextView view = new TextView(this);
        view.setText(value);
        view.setTextSize(size);
        view.setTextColor(color);
        if (bold) view.setTypeface(Typeface.DEFAULT, Typeface.BOLD);
        return view;
    }
    private void add(LinearLayout parent, View child, int top) {
        LinearLayout.LayoutParams p = new LinearLayout.LayoutParams(-1, -2);
        p.topMargin = dp(top);
        parent.addView(child, p);
    }
    private void requestCapture() {
        if (Build.VERSION.SDK_INT >= 23 && checkSelfPermission(Manifest.permission.RECORD_AUDIO) != PackageManager.PERMISSION_GRANTED) {
            requestPermissions(new String[]{Manifest.permission.RECORD_AUDIO}, REQUEST_AUDIO_PERMISSION);
            return;
        }
        MediaProjectionManager manager = (MediaProjectionManager) getSystemService(Context.MEDIA_PROJECTION_SERVICE);
        startActivityForResult(manager.createScreenCaptureIntent(), REQUEST_CAPTURE);
    }
    @Override public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] results) {
        super.onRequestPermissionsResult(requestCode, permissions, results);
        if (requestCode == REQUEST_AUDIO_PERMISSION) {
            if (results.length > 0 && results[0] == PackageManager.PERMISSION_GRANTED) requestCapture();
            else { resetSwitch(); if (autoStart) finish(); }
        }
    }
    private void resetSwitch() {
        changingSwitch = true;
        enabled.setChecked(false);
        changingSwitch = false;
    }
    @Override protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode != REQUEST_CAPTURE) return;
        if (resultCode != RESULT_OK || data == null) { resetSwitch(); if (autoStart) finish(); return; }
        Intent start = new Intent(this, AudioCaptureService.class);
        start.setAction(AudioCaptureService.ACTION_START);
        start.putExtra(AudioCaptureService.EXTRA_RESULT_CODE, resultCode);
        start.putExtra(AudioCaptureService.EXTRA_RESULT_DATA, data);
        if (Build.VERSION.SDK_INT >= 26) startForegroundService(start); else startService(start);
        // The capture lives in AudioCaptureService, not in this Activity.
        // Closing this screen does not stop the foreground service.
        finish();
    }
    @Override public void onCreate(Bundle state) {
        super.onCreate(state);
        autoStart = getIntent() != null && getIntent().getBooleanExtra("auto_start", false);
        getWindow().setStatusBarColor(BG);
        getWindow().setNavigationBarColor(BG);
        LinearLayout root = new LinearLayout(this);
        root.setOrientation(LinearLayout.VERTICAL);
        root.setPadding(dp(24), dp(36), dp(24), dp(24));
        root.setBackgroundColor(BG);
        add(root, text("◖))  Wells Audio", 23, true, Color.WHITE), 0);
        add(root, text("Configurações de transmissão", 14, false, MUTED), 4);
        View line = new View(this);
        line.setBackgroundColor(Color.rgb(50,50,54));
        add(root, line, 28);
        line.getLayoutParams().height = dp(1);
        enabled = new Switch(this);
        enabled.setText("Transmitir áudio");
        enabled.setTextColor(Color.WHITE);
        enabled.setTextSize(16);
        enabled.setChecked(false);
        add(root, enabled, 24);
        add(root, text("Qualidade do áudio", 17, true, Color.WHITE), 26);
        qualities = new RadioGroup(this);
        qualities.setOrientation(LinearLayout.VERTICAL);
        int[] rates = {128,96,32};
        String[] labels = {"128 kbps — Alta qualidade","96 kbps — Equilibrado","32 kbps — Econômico"};
        String[] details = {"Para músicas, filmes e jogos","Boa qualidade com menor consumo","Para conexões mais limitadas"};
        for (int i=0;i<rates.length;i++) {
            final int rate=rates[i];
            RadioButton option = new RadioButton(this);
            option.setId(1000+rate);
            option.setText(labels[i]+"\n"+details[i]);
            option.setTextSize(15);
            option.setTextColor(Color.WHITE);
            option.setPadding(0,dp(10),0,dp(10));
            qualities.addView(option);
        }
        add(root, qualities, 12);
        int saved=getPreferences(0).getInt("bitrate",96);
        if (saved!=128 && saved!=96 && saved!=32) saved=96;
        qualities.check(1000+saved);
        explanation=text("",14,false,MUTED);
        add(root, text("ⓘ  Configuração selecionada", 15, true, Color.WHITE), 26);
        add(root, explanation, 8);
        add(root, text("Transmissão experimental PCM 48 kHz. Os perfis 32/96/128 kbps ainda não comprimem o áudio. O Android solicitará permissão de captura.", 12, false, MUTED), 28);
        qualities.setOnCheckedChangeListener((group,id)->{
            int rate=id-1000;
            getPreferences(0).edit().putInt("bitrate",rate).apply();
            explanation.setText(rate==128?"Prioriza a fidelidade do som.":rate==96?"Equilibra qualidade e economia de dados.":"Prioriza menor consumo de dados, com perda de qualidade.");
        });
        enabled.setOnCheckedChangeListener((button,on)->{
            if (changingSwitch) return;
            if (on) requestCapture(); else {
                Intent stop = new Intent(this, AudioCaptureService.class);
                stop.setAction(AudioCaptureService.ACTION_STOP);
                startService(stop);
            }
        });
        explanation.setText(saved==128?"Prioriza a fidelidade do som.":saved==96?"Equilibra qualidade e economia de dados.":"Prioriza menor consumo de dados, com perda de qualidade.");
        setContentView(root);
        // ADB launch starts capture without requiring the user to touch the switch.
        // Android still shows its mandatory MediaProjection consent dialog.
        if (autoStart) requestCapture();
    }
}
