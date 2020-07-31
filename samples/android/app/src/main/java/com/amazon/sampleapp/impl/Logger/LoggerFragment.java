package com.amazon.sampleapp.impl.Logger;

import android.app.Activity;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.SwitchCompat;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.CompoundButton;
import android.widget.LinearLayout;
import android.widget.Spinner;
import android.widget.TextView;

import com.amazon.aace.logger.Logger;
import com.amazon.sampleapp.R;
import com.amazon.sampleapp.logView.LogEntry;
import com.amazon.sampleapp.logView.LogRecyclerViewAdapter;

public class LoggerFragment extends Fragment {
    RecyclerView mRecyclerView;
    Activity mActivity;
    LogRecyclerViewAdapter mRecyclerAdapter;

    public LoggerFragment(Activity activity) {
        super();

        this.mActivity = activity;
    }

    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.log_view, container, false);
        mRecyclerView = view.findViewById(R.id.rvLog);
        mRecyclerView.setHasFixedSize(true);
        mRecyclerView.setLayoutManager(new LinearLayoutManager(mActivity));
        mRecyclerAdapter = new LogRecyclerViewAdapter(mActivity.getApplicationContext());
        mRecyclerView.setAdapter(mRecyclerAdapter);

        setUpLogViewOptions();
        return view;
    }

    // Set up log view filtering options
    public void setUpLogViewOptions() {
        LayoutInflater inf = mActivity.getLayoutInflater();

        // Add switch for each log source type
        String[] sources = {"CLI", "AAC", "AVS"};
        LinearLayout sourceContainer = mActivity.findViewById(R.id.sourceSwitchContainer);
        for (final String source : sources) {
            View switchItem = (inf.inflate(R.layout.drawer_switch, sourceContainer, false));
            ((TextView) switchItem.findViewById(R.id.text)).setText(source);
            SwitchCompat drawerSwitch = switchItem.findViewById(R.id.drawerSwitch);
            drawerSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
                public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                    mRecyclerAdapter.setSourceDisplayMode(source, isChecked);
                }
            });
            sourceContainer.addView(switchItem);
        }

        // Add option in dropdown selector for each log level
        Spinner spinner = mActivity.findViewById(R.id.levelSpinner);
        ArrayAdapter<Logger.Level> adapter =
                new ArrayAdapter<Logger.Level>(mActivity, android.R.layout.simple_spinner_item);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);

        for (Logger.Level level : Logger.Level.values()) {
            if (level == Logger.Level.METRIC) {
                continue;
            }
            adapter.add(level);
        }

        spinner.setAdapter(adapter);
        spinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                Logger.Level level = (Logger.Level) parent.getItemAtPosition(position);
                mRecyclerAdapter.setDisplayLevel(level);
                mRecyclerView.scrollToPosition(mRecyclerAdapter.getItemCount() - 1);
            }
            public void onNothingSelected(AdapterView<?> parent) {}
        });

        // Add switch to display or hide card logs
        View cardItem = mActivity.findViewById(R.id.toggleCards);
        ((TextView) cardItem.findViewById(R.id.text)).setText(R.string.log_switch_cards);
        SwitchCompat cardSwitch = cardItem.findViewById(R.id.drawerSwitch);
        cardSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                mRecyclerAdapter.setCardDisplayMode(isChecked);
            }
        });

        // Add switch to display or hide pretty-printed JSON template logs
        View tempItem = mActivity.findViewById(R.id.toggleTemplates);
        ((TextView) tempItem.findViewById(R.id.text)).setText(R.string.log_switch_template);
        SwitchCompat tempSwitch = tempItem.findViewById(R.id.drawerSwitch);
        tempSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                mRecyclerAdapter.setJsonDisplayMode(isChecked);
            }
        });

        // Clear log button
        mActivity.findViewById(R.id.clearLogButton).setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                mRecyclerAdapter.clear();
            }
        });

        // Set initial level selection to INFO
        spinner.setSelection(Logger.Level.INFO.ordinal());
        mRecyclerAdapter.setDisplayLevel(Logger.Level.INFO);
    }

    public void insertItem(final LogEntry entry) {
        // Scroll to bottom of log view
        mRecyclerAdapter.insertItem(entry);
        int count = mRecyclerAdapter.getItemCount();
        int position = count > 0 ? count - 1 : 0;
        if (isScrolledToBottom(mRecyclerView))
            mRecyclerView.scrollToPosition(position);
    }

    static private boolean isScrolledToBottom(RecyclerView recyclerView) {
        int maxScroll = recyclerView.computeVerticalScrollRange();
        int currentScroll = recyclerView.computeVerticalScrollOffset() + recyclerView.computeVerticalScrollExtent();
        return currentScroll >= maxScroll;
    }
}
