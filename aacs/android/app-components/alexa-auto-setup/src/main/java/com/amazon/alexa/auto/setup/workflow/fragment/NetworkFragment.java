package com.amazon.alexa.auto.setup.workflow.fragment;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;
import androidx.fragment.app.Fragment;
import androidx.lifecycle.ViewModelProvider;

import com.amazon.alexa.auto.setup.R;

/**
 * Fragment for Network Settings.
 */
public class NetworkFragment extends Fragment {
    private static final String TAG = NetworkFragment.class.getSimpleName();

    private NetworkViewModel mViewModel;

    /**
     * Constructs an instance of NetworkFragment.
     */
    public NetworkFragment() {}

    /**
     * Constructs an instance of NetworkFragment.
     *
     * @param viewModel View Model for Network Settings.
     */
    @VisibleForTesting
    NetworkFragment(@NonNull NetworkViewModel viewModel) {
        this.mViewModel = viewModel;
    }

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if (mViewModel == null) { // It would be non-null for test injected dependencies.
            mViewModel = new ViewModelProvider(this).get(NetworkViewModel.class);
        }
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        return inflater.inflate(R.layout.network_fragment, container, false);
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);

        View fragmentView = requireView();

        TextView getStartedButtonText = fragmentView.findViewById(R.id.network_settings_btn);
        getStartedButtonText.setOnClickListener(view -> mViewModel.goToNetworkSettings());
    }
}
