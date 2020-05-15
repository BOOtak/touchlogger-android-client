package org.leyfer.thesis.touchlogger_dirty.adapter;

import android.content.Context;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

import org.leyfer.thesis.touchlogger_dirty.R;
import org.leyfer.thesis.touchlogger_dirty.activity.MainActivity;

import java.util.ArrayList;
import java.util.List;

public class PayloadViewAdapter extends BaseAdapter {
    private final List<String> logLines;
    private final Context context;

    public PayloadViewAdapter(Context context) {
        this.context = context;
        logLines = new ArrayList<>();
    }

    public void addLogLine(String line) {
        logLines.add(line);
        notifyDataSetChanged();
    }

    public void clearLines() {
        logLines.clear();
        notifyDataSetChanged();
    }

    @Override
    public int getCount() {
        return logLines.size();
    }

    @Override
    public Object getItem(int i) {
        return logLines.get(i);
    }

    @Override
    public long getItemId(int i) {
        return i;
    }

    @Override
    public View getView(int i, View view, ViewGroup viewGroup) {
        if (view == null) {
            View res = LayoutInflater.from(context).inflate(R.layout.log_line_item, viewGroup, false);
            TextView textView = res.findViewById(R.id.logLineTextView);
            textView.setText(logLines.get(i));
            return res;
        } else {
            TextView textView = view.findViewById(R.id.logLineTextView);
            textView.setText(logLines.get(i));
            return view;
        }
    }
}
