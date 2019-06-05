package edu.unlam.boyapp.ui.main;

import android.Manifest;
import android.app.ProgressDialog;
import android.arch.lifecycle.ViewModelProviders;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v4.app.ActivityCompat;
import android.support.v4.app.Fragment;
import android.support.v4.content.ContextCompat;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.List;

import edu.unlam.boyapp.MainActivity;
import edu.unlam.boyapp.R;

public class ConfiguracionFragment extends Fragment {

    Button btnBuscar;

    private ArrayList<BluetoothDevice> mDeviceList = new ArrayList<BluetoothDevice>();

    private BluetoothAdapter mBluetoothAdapter;

    private ProgressDialog mProgressDlg;

    public static final int MULTIPLE_PERMISSIONS = 10; // code you want.

    String[] permissions= new String[]{
            Manifest.permission.BLUETOOTH,
            Manifest.permission.BLUETOOTH_ADMIN,
            Manifest.permission.ACCESS_COARSE_LOCATION,
            Manifest.permission.WRITE_EXTERNAL_STORAGE,
            Manifest.permission.READ_PHONE_STATE,
            Manifest.permission.READ_EXTERNAL_STORAGE};

    private static final String ARG_SECTION_NUMBER = "section_number";
    private ConfiguracionViewModel mViewModel;

    public static ConfiguracionFragment newInstance(int index) {
        ConfiguracionFragment fragment = new ConfiguracionFragment();
        Bundle bundle = new Bundle();
        bundle.putInt(ARG_SECTION_NUMBER, index);
        fragment.setArguments(bundle);
        return fragment;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        //Se crea un adaptador para podermanejar el bluetooth del celular
        mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();

        //Se Crea la ventana de dialogo que indica que se esta buscando dispositivos bluetooth
        mProgressDlg = new ProgressDialog(getActivity());
        mProgressDlg.setMessage("Buscando dispositivos...");
        mProgressDlg.setCancelable(false);

        //se asocia un listener al boton cancelar para la ventana de dialogo ue busca los dispositivos bluetooth
        mProgressDlg.setButton(DialogInterface.BUTTON_NEGATIVE, "Cancelar", btnCancelarDialogListener);

        //

    }

    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container,
                             @Nullable Bundle savedInstanceState) {
        View root = inflater.inflate(R.layout.configuracion_fragment, container, false);

        btnBuscar = (Button) root.findViewById(R.id.btnBuscarDispositivos);


        if (checkPermissions())
        {
            enableComponent();
        }

        return root;
    }

    @Override
    public void onActivityCreated(@Nullable Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        mViewModel = ViewModelProviders.of(this).get(ConfiguracionViewModel.class);
        // TODO: Use the ViewModel
    }

    protected  void enableComponent()
    {
        //se determina si existe bluetooth en el celular
        if (mBluetoothAdapter == null)
        {
            //si el celular no soporta bluetooth
            showUnsupported();
        }
        else
        {
            //si el celular soporta bluetooth, se definen los listener para los botones de la activity
            //btnEmparejar.setOnClickListener(btnEmparejarListener);

            btnBuscar.setOnClickListener(btnBuscarListener);

            //se determina si esta activado el bluetooth
            if (mBluetoothAdapter.isEnabled())
            {
                //se informa si esta habilitado
                showToast("Bluetooth activado");
            }
            else
            {
                //se informa si esta deshabilitado
                showToast("Bluetooth desactivado");
            }
        }


        //se definen un broadcastReceiver que captura el broadcast del SO cuando captura los siguientes eventos:
        IntentFilter filter = new IntentFilter();

        filter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED); //Cambia el estado del bluetooth (Acrtivado /Desactivado)
        filter.addAction(BluetoothDevice.ACTION_FOUND); //Se encuentra un dispositivo bluetooth al realizar una busqueda
        filter.addAction(BluetoothAdapter.ACTION_DISCOVERY_STARTED); //Cuando se comienza una busqueda de bluetooth
        filter.addAction(BluetoothAdapter.ACTION_DISCOVERY_FINISHED); //cuando la busqueda de bluetooth finaliza

        //se define (registra) el handler que captura los broadcast anterirmente mencionados.
        getActivity().registerReceiver(mReceiver, filter);
    }

    //Metodo que chequea si estan habilitados los permisos
    private  boolean checkPermissions() {
        int result;
        List<String> listPermissionsNeeded = new ArrayList<>();

        //Se chequea si la version de Android es menor a la 6
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.M) {
            return true;
        }


        for (String p:permissions) {
            result = ContextCompat.checkSelfPermission(getActivity(),p);
            if (result != PackageManager.PERMISSION_GRANTED) {
                listPermissionsNeeded.add(p);
            }
        }
        if (!listPermissionsNeeded.isEmpty()) {
            ActivityCompat.requestPermissions(getActivity(), listPermissionsNeeded.toArray(new String[listPermissionsNeeded.size()]),MULTIPLE_PERMISSIONS );
            return false;
        }
        return true;
    }

    /**
     * Primero intenta encender bluetooth y luego busca dispositivos
     */
    private View.OnClickListener btnBuscarListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            if (!mBluetoothAdapter.isEnabled()) {
                Intent intent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);

                startActivityForResult(intent, 1000);

                //showToast("Bluetooth activado correctamente");
            }
            mBluetoothAdapter.startDiscovery();
        }
    };

    private void showToast(String message) {
        Toast.makeText(getActivity().getApplicationContext(), message, Toast.LENGTH_SHORT).show();
    }

    //Handler que captura los brodacast que emite el SO al ocurrir los eventos del bluetooth
    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {

            //Atraves del Intent obtengo el evento de bluetooth que informo el broadcast del SO
            String action = intent.getAction();

            //Si cambio de estado el bluetooth(Activado/desactivado)
            if (BluetoothAdapter.ACTION_STATE_CHANGED.equals(action))
            {
                //Obtengo el parametro, aplicando un Bundle, que me indica el estado del bluetooth
                final int state = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, BluetoothAdapter.ERROR);

                //Si esta activado
                if (state == BluetoothAdapter.STATE_ON)
                {
                    showToast("Bluetooth activado correctamente");
                    mBluetoothAdapter.startDiscovery();
                }
            }
            //Si se inicio la busqueda de dispositivos bluetooth
            else if (BluetoothAdapter.ACTION_DISCOVERY_STARTED.equals(action))
            {
                //Creo la lista donde voy a mostrar los dispositivos encontrados
                mDeviceList = new ArrayList<BluetoothDevice>();

                //muestro el cuadro de dialogo de busqueda
                mProgressDlg.show();
            }
            //Si finalizo la busqueda de dispositivos bluetooth
            else if (BluetoothAdapter.ACTION_DISCOVERY_FINISHED.equals(action))
            {
                //se cierra el cuadro de dialogo de busqueda
                mProgressDlg.dismiss();

                //se inicia el activity DeviceListActivity pasandole como parametros, por intent,
                //el listado de dispositovos encontrados
                Intent newIntent = new Intent(getActivity(), DeviceListActivity.class);

                newIntent.putParcelableArrayListExtra("device.list", mDeviceList);

                startActivity(newIntent);
            }
            //si se encontro un dispositivo bluetooth
            else if (BluetoothDevice.ACTION_FOUND.equals(action))
            {
                //Se lo agregan sus datos a una lista de dispositivos encontrados
                BluetoothDevice device = (BluetoothDevice) intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);

                mDeviceList.add(device);
                showToast("Dispositivo Encontrado:" + device.getName());
            }
        }
    };

    private DialogInterface.OnClickListener btnCancelarDialogListener = new DialogInterface.OnClickListener() {
        @Override
        public void onClick(DialogInterface dialog, int which) {
            dialog.dismiss();

            mBluetoothAdapter.cancelDiscovery();
        }
    };

    private void showUnsupported() {

        btnBuscar.setText("Bluetooth no soportado");
        btnBuscar.setEnabled(false);
    }


}
