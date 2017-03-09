package us.fischnet.bt;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.os.Handler;
import android.os.Message;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.UUID;

/**
 * Created by lfischer on 3/8/2017.
 */

public class spCnct {
    private UUID myUUID;

    private final String Sphinx_UUID =
            "5a830267-68ec-4c97-a4f9-b9d4f5e89cf3";
    // message codes
    private static final byte COMMAND_CODE = '$'; // precedes a command byte
    private static final byte HELLO_CODE = '#'; // send if there is no command to send - keepalive

    private static final byte CMD_GO_HORIZONTAL = 0; // move horizontally
    private static final byte CMD_GO_UP= CMD_GO_HORIZONTAL+1; // go up
    private static final byte CMD_GO_DOWN= CMD_GO_UP+1; // go back to horizontal
    private static final byte CMD_SCULPT= CMD_GO_DOWN+1; // sculpt
    private static final byte CMD_GO_FACE= CMD_SCULPT+1; // return to face

    // corrective messages from imaging app
    private static final byte CMD_TOO_CLOSE= 16;
    private static final byte CMD_TOO_SHAKY= CMD_TOO_CLOSE+1;
    private static final byte CMD_TOO_DARK= CMD_TOO_SHAKY+1;
    private static final byte CMD_TOO_FAST= CMD_TOO_DARK+1;

    // incoming control messages from arm controller
    private static final byte CMD_START = 64;
    private static final byte CMD_ABORT = CMD_START+1;
    private static final byte CMD_DONE = CMD_ABORT+1;

    private long cntStart=0,cntAbort=0,cntDone=0,cntIdle=0,cntErr=0,cmdDropped=0;
    private long cntHor=0,cntUp=0,cntDown=0,cntSculpt=0,cntFace=0,cntTooClose=0,cntTooShaky=0,cntTooDark=0,cntTooFast=0;
    private BluetoothAdapter mAdapter;
    ThreadConnectBTdevice myThreadConnectBTdevice;
    ThreadConnected myThreadConnected;
    String address;

    public  spCnct (String Addr) {
        BluetoothDevice device;
        myUUID = UUID.fromString(Sphinx_UUID);
        mAdapter = BluetoothAdapter.getDefaultAdapter();

        address=Addr;
        device = mAdapter.getRemoteDevice(address);
        myThreadConnectBTdevice = new ThreadConnectBTdevice(device);
        myThreadConnectBTdevice.start();
    }

/*
/*
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        //setContentView(R.layout.activity_term);
        Bundle extras = getIntent().getExtras();
        if (extras != null) {
            address = extras.getString("TARGET_BT_DEVICE");
        }
    }
    */
    //myThreadConnectBTdevice = new ThreadConnectBTdevice(device);
    //myThreadConnectBTdevice.start();
    /*
     ThreadConnectBTdevice:
     Background Thread to handle BlueTooth connecting
     */
    private class ThreadConnectBTdevice extends Thread {

        private BluetoothSocket bluetoothSocket = null;
        private final BluetoothDevice bluetoothDevice;



        private ThreadConnectBTdevice(BluetoothDevice device) {
            bluetoothDevice = device;

            try {
                bluetoothSocket = device.createRfcommSocketToServiceRecord(myUUID);
                //textStatus.setText("bluetoothSocket: \n" + bluetoothSocket);
            } catch (IOException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }

        @Override
        public void run() {
            boolean success = false;
            try {
                bluetoothSocket.connect();
                success = true;
            } catch (IOException e) {
                e.printStackTrace();

                final String eMessage = e.getMessage();
                /*
                runOnUiThread(new Runnable() {

                    @Override
                    public void run() {
                        textStatus.setText("something wrong bluetoothSocket.connect(): \n" + eMessage);
                    }
                });
*/
                try {
                    bluetoothSocket.close();
                } catch (IOException e1) {
                    // TODO Auto-generated catch block
                    e1.printStackTrace();
                }
            }

            if (success) {
                //connect successful
                final String msgconnected = "connect successful:\n"
                        + "BluetoothSocket: " + bluetoothSocket + "\n"
                        + "BluetoothDevice: " + bluetoothDevice;
/*
                runOnUiThread(new Runnable() {

                    @Override
                    public void run() {
                        textStatus.setText(msgconnected);

                        listViewPairedDevice.setVisibility(View.GONE);
                        //inputPane.setVisibility(View.VISIBLE);
                    }
                });
*/
                startThreadConnected(bluetoothSocket);
            } else {
                //fail
            }
        }

        public void cancel() {
/*
            Toast.makeText(getApplicationContext(),
                    "close bluetoothSocket",
                    Toast.LENGTH_LONG).show();
*/
            try {
                bluetoothSocket.close();
            } catch (IOException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }

        }

    }

    private final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            String readMessage = (String) msg.obj;
            //byte[] readBuf = (byte[]) msg.obj;
            //String readMessage = new String(readBuf, 0, msg.arg1);
            switch (msg.what) {
                case CMD_GO_HORIZONTAL:
                    //tv1.setText("# Hor: "+String.format("%d",cntHor));
                    break;
                case CMD_GO_UP:
                    //tv2.setText("# Up: "+String.format("%d",cntUp));
                    break;
                case CMD_GO_DOWN:
                    //tv3.setText("# Down: "+String.format("%d",cntDown));
                    break;
                case CMD_SCULPT:
                    //tv4.setText("# Sculpt: "+String.format("%d",cntSculpt));
                    break;
                case CMD_GO_FACE:
                    //tv5.setText("# Face: "+String.format("%d",cntFace));
                    break;
                case CMD_TOO_CLOSE:
                    //tv6.setText("# Close: "+String.format("%d",cntTooClose));
                    break;
                case CMD_TOO_SHAKY:
                    //tv7.setText("# Shaky: "+String.format("%d",cntTooShaky));
                    break;
                case CMD_TOO_DARK:
                    //tv8.setText("# Dark: "+String.format("%d",cntTooDark));
                    break;
                case CMD_TOO_FAST:
                    //tv9.setText("# Fast: "+String.format("%d",cntTooFast));
                    break;


                case CMD_START:
                    //tv10.setText("# Fast: "+String.format("%d",cntStart));
                    break;
                case CMD_ABORT:
                    //tv11.setText("# Fast: "+String.format("%d",cntAbort));
                    break;
                case CMD_DONE:
                    //tv12.setText("# Fast: "+String.format("%d",cntDone));
                    break;
            }
        }
    };
    //Called in ThreadConnectBTdevice once connect successed
    //to start ThreadConnected
    private void startThreadConnected(BluetoothSocket socket) {

        myThreadConnected = new ThreadConnected(socket);
        myThreadConnected.start();
    }
    /*
    ThreadConnected:
    Background Thread to handle Bluetooth data communication
    after connected
     */
    private class ThreadConnected extends Thread {
        private final BluetoothSocket connectedBluetoothSocket;
        private final InputStream connectedInputStream;
        private final OutputStream connectedOutputStream;

        public ThreadConnected(BluetoothSocket socket) {
            connectedBluetoothSocket = socket;
            InputStream in = null;
            OutputStream out = null;

            try {
                in = socket.getInputStream();
                out = socket.getOutputStream();
            } catch (IOException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }

            connectedInputStream = in;
            connectedOutputStream = out;
        }

        @Override
        public void run() {
            byte[] buffer = new byte[1024];
            int bytes;
            int idx;

            while (true) {
                try {
                    bytes = connectedInputStream.read(buffer);
                    for (idx=0;idx<bytes;idx++)
                    {
                        if (buffer[idx]==HELLO_CODE)
                        {
                            cntIdle++;
                        }
                        else if (buffer[idx]==COMMAND_CODE)
                        {
                            if (idx+1<bytes)
                            {
                                // we got a cmd code, and there is another byte
                                byte cmd=buffer[++idx]; // get the command
                                switch (cmd)
                                {
                                    case CMD_GO_HORIZONTAL:
                                        cntHor++;
//                                        tv1.setText("# Hor: "+String.format("%d",cntHor));
                                        mHandler.obtainMessage((int) CMD_GO_HORIZONTAL).sendToTarget();
                                        break;
                                    case CMD_GO_UP:
                                        cntUp++;
//                                        tv2.setText("# Up: "+String.format("%d",cntUp));
                                        mHandler.obtainMessage((int) CMD_GO_UP).sendToTarget();
                                        break;
                                    case CMD_GO_DOWN:
                                        cntDown++;
//                                        tv3.setText("# Down: "+String.format("%d",cntDown));
                                        mHandler.obtainMessage((int) CMD_GO_DOWN).sendToTarget();
                                        break;
                                    case CMD_SCULPT:
                                        cntSculpt++;
//                                        tv4.setText("# Sculpt: "+String.format("%d",cntSculpt));
                                        mHandler.obtainMessage((int) CMD_SCULPT).sendToTarget();
                                        break;
                                    case CMD_GO_FACE:
                                        cntFace++;
//                                        tv5.setText("# Face: "+String.format("%d",cntFace));
                                        mHandler.obtainMessage((int) CMD_GO_FACE).sendToTarget();
                                        break;
                                    case CMD_TOO_CLOSE:
                                        cntTooClose++;
//                                        tv6.setText("# Close: "+String.format("%d",cntTooClose));
                                        mHandler.obtainMessage((int) CMD_TOO_CLOSE).sendToTarget();
                                        break;
                                    case CMD_TOO_SHAKY:
                                        cntTooShaky++;
//                                        tv7.setText("# Shaky: "+String.format("%d",cntTooShaky));
                                        mHandler.obtainMessage((int) CMD_TOO_SHAKY).sendToTarget();
                                        break;
                                    case CMD_TOO_DARK:
                                        cntTooDark++;
//                                        tv8.setText("# Dark: "+String.format("%d",cntTooDark));
                                        mHandler.obtainMessage((int) CMD_TOO_DARK).sendToTarget();
                                        break;
                                    case CMD_TOO_FAST:
                                        cntTooFast++;
//                                        tv9.setText("# Fast: "+String.format("%d",cntTooFast));
                                        mHandler.obtainMessage((int) CMD_TOO_FAST).sendToTarget();
                                        break;

                                    // These are simply for display.  Commands received from arm controller
                                    case CMD_START:
//                                        tv10.setText("# Fast: "+String.format("%d",cntStart));
                                        mHandler.obtainMessage((int) CMD_START).sendToTarget();
                                        break;
                                    case CMD_ABORT:
//                                        tv11.setText("# Fast: "+String.format("%d",cntAbort));
                                        mHandler.obtainMessage((int) CMD_ABORT).sendToTarget();
                                        break;
                                    case CMD_DONE:
//                                        tv12.setText("# Fast: "+String.format("%d",cntDone));
                                        mHandler.obtainMessage((int) CMD_DONE).sendToTarget();
                                        break;
                                }

                            }
                            else
                                cntErr++;
                        }
                    }
                } catch (IOException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
/*
                    final String msgConnectionLost = "Connection lost:\n"
                            + e.getMessage();
                    runOnUiThread(new Runnable() {

                        @Override
                        public void run() {
                            textStatus.setText(msgConnectionLost);
                        }
                    });
                    */
                }
            }
        }

        public void write(byte[] buffer) {
            try {
                connectedOutputStream.write(buffer);
            } catch (IOException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }

        public void cancel() {
            try {
                connectedBluetoothSocket.close();
            } catch (IOException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }

}
