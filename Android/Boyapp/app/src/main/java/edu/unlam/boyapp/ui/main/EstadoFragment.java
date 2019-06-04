package edu.unlam.boyapp.ui.main;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import android.support.annotation.Nullable;
import android.support.annotation.NonNull;
import android.support.v4.app.Fragment;
import android.arch.lifecycle.Observer;
import android.arch.lifecycle.ViewModelProviders;

import edu.unlam.boyapp.R;

/**
 * A placeholder fragment containing a simple view.
 */
public class EstadoFragment extends Fragment {

    private static final String ARG_SECTION_NUMBER = "section_number";

    private EstadoViewModel estadoViewModel;

    public static EstadoFragment newInstance(int index) {
        EstadoFragment fragment = new EstadoFragment();
        Bundle bundle = new Bundle();
        bundle.putInt(ARG_SECTION_NUMBER, index);
        fragment.setArguments(bundle);
        return fragment;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        estadoViewModel = ViewModelProviders.of(this).get(EstadoViewModel.class);
        int index = 1;
        if (getArguments() != null) {
            index = getArguments().getInt(ARG_SECTION_NUMBER);
        }
        estadoViewModel.setIndex(index);
    }

    @Override
    public View onCreateView(
            @NonNull LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        View root = inflater.inflate(R.layout.estado_fragment, container, false);
        final TextView nroTempraturaTextView = root.findViewById(R.id.nroTempraturatextView);
        final TextView valorColorTextView = root.findViewById(R.id.valorColorTextView);
        estadoViewModel.getText().observe(this, new Observer<String>() {
            @Override
            public void onChanged(@Nullable String s) {
                nroTempraturaTextView.setText("0");
                valorColorTextView.setText("Turbia");
            }
        });
        return root;
    }
}