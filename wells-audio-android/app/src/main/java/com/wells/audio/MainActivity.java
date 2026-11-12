package com.wells.audio;

import android.app.Activity;
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
    @Override public void onCreate(Bundle state) {
        super.onCreate(state);
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
        enabled.setChecked(getPreferences(0).getBoolean("enabled", false));
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
        add(root, text("Protótipo da interface. A captura e transmissão ainda não estão implementadas.", 12, false, MUTED), 28);
        qualities.setOnCheckedChangeListener((group,id)->{
            int rate=id-1000;
            getPreferences(0).edit().putInt("bitrate",rate).apply();
            explanation.setText(rate==128?"Prioriza a fidelidade do som.":rate==96?"Equilibra qualidade e economia de dados.":"Prioriza menor consumo de dados, com perda de qualidade.");
        });
        enabled.setOnCheckedChangeListener((button,on)->getPreferences(0).edit().putBoolean("enabled",on).apply());
        explanation.setText(saved==128?"Prioriza a fidelidade do som.":saved==96?"Equilibra qualidade e economia de dados.":"Prioriza menor consumo de dados, com perda de qualidade.");
        setContentView(root);
    }
}
