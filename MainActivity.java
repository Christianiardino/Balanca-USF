package com.lhamaintergalatica.balaausf;

import androidx.appcompat.app.AppCompatActivity;

import android.annotation.SuppressLint;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.util.Set;
import java.util.UUID;

public class MainActivity extends AppCompatActivity {

    Thread thread; // Variável tipo Thread para utilizar tempo
    BluetoothSocket bluetoothSocket;
    BluetoothDevice bluetoothDevice;// Aparelhos conectados ao Bluetooth
    OutputStream outputStream;// Envio de dados via Bluetooth
    InputStream inputStream;// Recebimento de dados via Bluetooth
    BluetoothAdapter bluetoothAdapter;// Adaptador de Bluetooth

    Button calibragem, blueConect;// Botões de calibragem e conectar
    TextView peso, blueText;// Textos peso e estado da coneção
    byte [] buffer;
    int bufferPos;

    @SuppressLint("SetTextI18n")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Vinculando a variavel com o botão criado na activity_main.xml
        calibragem = findViewById(R.id.button_calibragem);
        blueConect = findViewById(R.id.bluetooth_confir);
        peso = findViewById(R.id.peso_calculado);
        blueText = findViewById(R.id.texto_bluetooth);

        bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();

        if(bluetoothAdapter == null){
            blueText.setText("Não conectado a balança!");
        }else{
            blueText.setText("Conectado a balança!");
        }


        blueConect.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                if(bluetoothAdapter.isEnabled()){
                    blueConect.setBackgroundColor(Color.GREEN);

                    Set<BluetoothDevice> connectedDevices = bluetoothAdapter.getBondedDevices();
                    if(connectedDevices.size() > 0){
                        for(BluetoothDevice device : connectedDevices){
                            if(device.getName().equals("balanca ACGT nova")){
                                bluetoothDevice = device;
                                break;
                            }
                        }
                    }

                    Toast.makeText(getApplicationContext(), "Conectado a balança!", Toast.LENGTH_SHORT).show();
                    try {
                        openBT();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }

                }else{
                    blueConect.setBackgroundColor(Color.RED);
                }

            }
        });

        calibragem.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                try {
                    calibragemClass();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        });

    }

    void openBT() throws IOException {
        UUID uuid = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
        bluetoothSocket = bluetoothDevice.createRfcommSocketToServiceRecord(uuid);
        bluetoothSocket.connect();
        outputStream = bluetoothSocket.getOutputStream();
        inputStream = bluetoothSocket.getInputStream();

        dataListener();

        blueText.setText("Conectado");
    }

    void dataListener(){

        final Handler handler = new Handler();
        final byte limit = 10;

        buffer = new byte[256];

        thread = new Thread(new Runnable() {
            @Override
            public void run() {
                while(!Thread.currentThread().isInterrupted()){
                    try {
                        int bytesAvailable = inputStream.available();
                        if(bytesAvailable > 0) {
                            byte[] packetBytes = new byte[bytesAvailable];
                            inputStream.read(packetBytes);
                            for(int i=0;i<bytesAvailable;i++)
                            { byte b = packetBytes[i];
                                if(b == limit) {

                                    byte[] encodedBytes = new byte[bufferPos];
                                    System.arraycopy(buffer, 0, encodedBytes, 0, encodedBytes.length);
                                    final String data = new String(encodedBytes, StandardCharsets.US_ASCII);

                                    bufferPos = 0;
                                    handler.post(new Runnable() {
                                        @SuppressLint("SetTextI18n")
                                        public void run()
                                        {
                                            peso.setText(String.format("%.3f", (Float.valueOf(data)/1000)));
                                        }
                                    });
                                }
                                else {
                                    buffer[bufferPos++] = b;
                                }
                            }
                        }
                    }
                    catch (IOException ex){
                        Toast.makeText(getApplicationContext(), "Coneção Perdida!", Toast.LENGTH_SHORT).show();
                    }
                }
            }
        });

        thread.start();

    }

    void calibragemClass() throws IOException{
        outputStream.write('c');
        Toast.makeText(getApplicationContext(), "Calibragem iniciada!", Toast.LENGTH_SHORT).show();
    }
}
