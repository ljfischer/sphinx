package us.fischnet.bt;

import android.app.Activity;
import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.google.android.gms.appindexing.Action;
import com.google.android.gms.appindexing.AppIndex;
import com.google.android.gms.common.api.GoogleApiClient;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Timer;
import java.util.TimerTask;
import java.util.UUID;
import java.util.concurrent.locks.ReentrantLock;

import us.fischnet.myapplication.R;

//import android.R;
//import us.fischnet.R;

/**
 * Created by lfischer on 3/22/2016.
 */
public class btTerm extends Activity {
    // Member fields
    Button b1;
    byte txQ[]=new byte[32];
    byte txqHead=0,txqTail=0;

    byte txM[]=new byte[32];
    byte txmHead=0,txmTail=0;

    public Handler mSpHandler=null;
    spCnct sphinxBTDevice;

    TextView tv1,tv2,tv3,tv4,tv5,tv6,tv7,tv8,tv9,tv10,tv11,tv12;
    private BluetoothAdapter mAdapter;
    Thread BTConnThread;
    ReentrantLock lock;
    private ConnectThread mConnectThread;
    private ConnectedThread mConnectedThread;
    private int mState;

    String address,spAddress;
/*
    public static final byte MSG_PARK = 0x30;       // we're doing nothing
    public static final byte MSG_HOR = 0x31;       // we're doing nothing
    public static final byte MSG_UP = 0x32;       // we're doing nothing
    public static final byte MSG_DOWN = 0x33;       // we're doing nothing
    public static final byte MSG_CCW = 0x34;       // we're doing nothing
    public static final byte MSG_CW = 0x35;       // we're doing nothing
    public static final byte MSG_CAL = 0x36;       // we're doing nothing
    public static final byte MSG_SCULPT = 0x37;       // we're doing nothing
    public static final byte MSG_HOME = 0x38;       // we're doing nothing
*/
///
public static final byte CMD_GO_HORIZONTAL = 0; // move horizontally
    public static final byte CMD_GO_UP= CMD_GO_HORIZONTAL+1; // go up
    public static final byte CMD_GO_DOWN= CMD_GO_UP+1; // go back to horizontal
    public static final byte CMD_GO_SCULPT= CMD_GO_DOWN+1; // sculpt
    public static final byte CMD_GO_FACE= CMD_GO_SCULPT+1; // return to face

    // corrective messages from imaging app
    public static final byte CMD_TOO_CLOSE= 16;
    public static final byte CMD_TOO_SHAKY= CMD_TOO_CLOSE+1;
    public static final byte CMD_TOO_DARK= CMD_TOO_SHAKY+1;
    public static final byte CMD_TOO_FAST= CMD_TOO_DARK+1;

    // arm control messages - these map directly to the RPI commands
    public static final byte CMD_PARK = 0x30;       // we're doing nothing
    public static final byte CMD_HOR = CMD_PARK+1;       // we're doing nothing
    public static final byte CMD_UP = CMD_HOR+1;       // we're doing nothing
    public static final byte CMD_DOWN = CMD_UP+1;       // we're doing nothing
    public static final byte CMD_CCW = CMD_DOWN+1;       // we're doing nothing
    public static final byte CMD_CW = CMD_CCW+1;       // we're doing nothing
    public static final byte CMD_CAL = CMD_CW+1;       // we're doing nothing
    public static final byte CMD_SCULPT = CMD_CAL+1;       // we're doing nothing
    public static final byte CMD_HOME = CMD_SCULPT+1;       // we're doing nothing

    // incoming control messages from arm controller
    public static final byte CMD_START = 64;
    public static final byte CMD_ABORT = CMD_START+1;
    public static final byte CMD_DONE = CMD_ABORT+1;
    ///

    public static final int ACTION_REQUEST_ENABLE = 1;

    public static final String TAG = "MainActivity";


    // Unique UUID for this application
    private static final UUID MY_UUID_SECURE =
            UUID.fromString("fa87c0d0-afac-11de-8a39-0800200c9a66");
//    private static final UUID MY_UUID_INSECURE =
//            UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
    private static final UUID MY_UUID_INSECURE =
            UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
    /**
     * ATTENTION: This was auto-generated to implement the App Indexing API.
     * See https://g.co/AppIndexing/AndroidStudio for more information.
     */
    private GoogleApiClient client;
    //private final Handler spMsgHandler;
    private final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            String readMessage = (String) msg.obj;
            //byte[] readBuf = (byte[]) msg.obj;
            //String readMessage = new String(readBuf, 0, msg.arg1);

        }
    };

    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_term);

        b1 = (Button) findViewById(R.id.button7);
        lock=new ReentrantLock();


        Bundle extras = getIntent().getExtras();
        if (extras != null) {
            address = extras.getString("TARGET_BT_DEVICE");
            spAddress=extras.getString("TARGET_SP_DEVICE");
            sphinxBTDevice=new spCnct(spAddress);
            if (sphinxBTDevice!=null)
                sphinxBTDevice.setBtObj(this);
            connect(false); // connects to the RPI
        }

        // ATTENTION: This was auto-generated to implement the App Indexing API.
        // See https://g.co/AppIndexing/AndroidStudio for more information.
        client = new GoogleApiClient.Builder(this).addApi(AppIndex.API).build();
    }



    public void data(View v) {

        //mConnectedThread.startMsg();
        sphinxBTDevice.mSpHandler.obtainMessage((int) CMD_PARK).sendToTarget();
    }

    public boolean addMsg(byte msg) {
        if (txmHead+1==txmTail)
            return false;
        if (txmHead==32-1 && txmTail==0)
            return false;
        // we have room
        txM[txmHead]=msg;
        txmHead++;
        if (txmHead==32) // wrapped
            txmHead=0;
        return true;
    }

    public boolean addCmd(byte cmd) {
        if (txqHead+1==txqTail)
            return false;
        if (txqHead==32-1 && txqTail==0)
            return false;
        // we have room
        txQ[txqHead]=cmd;
        txqHead++;
        if (txqHead==32) // wrapped
            txqHead=0;
        return true;
    }
    public void Park(View v) {

        //addCmd(CMD_PARK);
        if (mConnectedThread!=null) {
            //mConnectedThread.sndPacket(MSG_PARK);
            mConnectedThread.mBtHandler.obtainMessage((int) CMD_PARK).sendToTarget();
        }

    }
    public void Up(View v) {
        addCmd(CMD_UP);
        if (mConnectedThread!=null) {
            //mConnectedThread.sndPacket(MSG_UP);
        }

    }
    public void Down(View v) {
        mConnectedThread.sndPacket(CMD_DOWN);
    }
    public void Hor(View v) {

        addCmd(CMD_HOR);
        if (mConnectedThread!=null) {
           // mConnectedThread.sndPacket(MSG_HOR);
        }

    }
    public void cw(View v) {

/*
        byte [] buffer = "#".getBytes();  // const
        Toast.makeText(getApplicationContext(), "Data", Toast.LENGTH_LONG).show();
        mConnectedThread.write(buffer);
*/
        addCmd(CMD_CW);
        if (mConnectedThread!=null) {
            //mConnectedThread.sndPacket(CMD_CW);
        }
    }
    public void ccw(View v) {

/*
        byte [] buffer = "#".getBytes();  // const
        Toast.makeText(getApplicationContext(), "Data", Toast.LENGTH_LONG).show();
        mConnectedThread.write(buffer);
*/
        addCmd(CMD_CCW);
        if (mConnectedThread!=null) {
           // mConnectedThread.sndPacket(CMD_CCW);
        }

    }
    public void cal(View v) {

/*
        byte [] buffer = "#".getBytes();  // const
        Toast.makeText(getApplicationContext(), "Data", Toast.LENGTH_LONG).show();
        mConnectedThread.write(buffer);
*/
        // we need to send a start command to the sphinx app, and wait for the ack that tells us to start moving horizontally

        //addCmd(CMD_CAL);
         if (mConnectedThread!=null) {
            //mConnectedThread.sndPacket(CMD_CAL);
             mConnectedThread.mBtHandler.obtainMessage((int) CMD_CAL).sendToTarget();
        }
    }
    public void sculpt(View v) {

/*
        byte [] buffer = "#".getBytes();  // const
        Toast.makeText(getApplicationContext(), "Data", Toast.LENGTH_LONG).show();
        mConnectedThread.write(buffer);
*/
        addCmd(CMD_SCULPT);
        if (mConnectedThread!=null) {
            //mConnectedThread.sndPacket(CMD_SCULPT);
        }
    }
    public void home(View v) {

/*
        byte [] buffer = "#".getBytes();  // const
        Toast.makeText(getApplicationContext(), "Data", Toast.LENGTH_LONG).show();
        mConnectedThread.write(buffer);
*/
        addCmd(CMD_HOME);
        if (mConnectedThread!=null) {
            //mConnectedThread.sndPacket(CMD_HOME);
        }
    }
    public void close(View v) {

/*
        byte [] buffer = "#".getBytes();  // const
        Toast.makeText(getApplicationContext(), "Data", Toast.LENGTH_LONG).show();
        mConnectedThread.write(buffer);
*/
        if (mConnectedThread!=null)
            mConnectedThread.cancel();

    }
    /**
     * Start the ConnectThread to initiate a connection to a remote device.
     *
     * @param device The BluetoothDevice to connect
     * @param secure Socket Security type - Secure (true) , Insecure (false)
     */
    public synchronized void connect(boolean secure) {

        BluetoothDevice device;
        mAdapter = BluetoothAdapter.getDefaultAdapter();

        if (!mAdapter.isEnabled()) {
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableBtIntent, ACTION_REQUEST_ENABLE);
        }

        device = mAdapter.getRemoteDevice(address);
        Log.d(TAG, "connect to: " + device);
        BTConnThread = new ConnectThread(device);
        BTConnThread.start();
    }

    @Override
    public void onStart() {
        super.onStart();

        // ATTENTION: This was auto-generated to implement the App Indexing API.
        // See https://g.co/AppIndexing/AndroidStudio for more information.
        client.connect();
        Action viewAction = Action.newAction(
                Action.TYPE_VIEW, // TODO: choose an action type.
                "btTerm Page", // TODO: Define a title for the content shown.
                // TODO: If you have web page content that matches this app activity's content,
                // make sure this auto-generated web page URL is correct.
                // Otherwise, set the URL to null.
                Uri.parse("http://host/path"),
                // TODO: Make sure this auto-generated app deep link URI is correct.
                Uri.parse("android-app://us.fischnet.bt/http/host/path")
        );
        AppIndex.AppIndexApi.start(client, viewAction);
    }

    @Override
    public void onStop() {
        super.onStop();

        // ATTENTION: This was auto-generated to implement the App Indexing API.
        // See https://g.co/AppIndexing/AndroidStudio for more information.
        Action viewAction = Action.newAction(
                Action.TYPE_VIEW, // TODO: choose an action type.
                "btTerm Page", // TODO: Define a title for the content shown.
                // TODO: If you have web page content that matches this app activity's content,
                // make sure this auto-generated web page URL is correct.
                // Otherwise, set the URL to null.
                Uri.parse("http://host/path"),
                // TODO: Make sure this auto-generated app deep link URI is correct.
                Uri.parse("android-app://us.fischnet.bt/http/host/path")
        );
        AppIndex.AppIndexApi.end(client, viewAction);
        client.disconnect();
    }

    /****************************************************************************/

    private class ConnectThread extends Thread {
        private final BluetoothSocket mmSocket;
        private final BluetoothDevice mmDevice;

        public ConnectThread(BluetoothDevice device) {
            // Use a temporary object that is later assigned to mmSocket,
            // because mmSocket is final
            BluetoothSocket tmp = null;
            mmDevice = device;

            // Get a BluetoothSocket to connect with the given BluetoothDevice
            try {
                // MY_UUID is the app's UUID string, also used by the server code
                tmp = device.createRfcommSocketToServiceRecord(MY_UUID_INSECURE);
            } catch (IOException e) {
            }
            mmSocket = tmp;
        }

        public void run() {
            // Cancel discovery because it will slow down the connection
            mAdapter.cancelDiscovery();

            try {
                // Connect the device through the socket. This will block
                // until it succeeds or throws an exception
                mmSocket.connect();

            } catch (IOException connectException) {
                // Unable to connect; close the socket and get out
                try {
                    mmSocket.close();
                } catch (IOException closeException) {
                }
                return;
            }

            // Do work to manage the connection (in a separate thread)
            manageConnectedSocket(mmSocket); //
        }

        /**
         * Will cancel an in-progress connection, and close the socket
         */
        public void cancel() {
            try {
                mmSocket.close();
            } catch (IOException e) {
            }
        }
    }

    public void manageConnectedSocket(BluetoothSocket sock) {
// Start the thread to manage the connection and perform transmissions
        mConnectedThread = new ConnectedThread(sock);
        if (mConnectedThread!=null)
            mConnectedThread.start();
    }

    /************************************************************************************/

    private class ConnectedThread extends Thread {
        private final BluetoothSocket mmSocket;
        private final InputStream mmInStream;
        private final OutputStream mmOutStream;
        private int  msgState=0;
        byte[] dataBuf=new byte[MSG_MAX_DATA]; // buffer for msg
        int msgType,msgChk,msgLen;
        public Handler mBtHandler;


        int bytesSent,bytesRcved,dataRcved,msgsRcved,msgsSent,msgRty;
        int syncSent,syncLost,ackRcved;
        byte [] pktBuf=new byte[PACKET_BUF_SIZE];
        int ms=0;
        boolean sec=false;

        public static final byte HDR_CMD ='$';
        public static final byte  HDR_HELLO ='#';

        public static final int MSG_MAX_DATA	=0x8; // how much data we can pass
        public static final int MSG_CMD_MASK	=0x7F; // 127 different cmd types
        public static final int MSG_ACK_MASK	=0x80; // high bit set for a response

        public static final int TXMIT_TIMER_DELAY	=25; // ms to sleep between tx timer

        public static final byte   MSG_CMD_PING=1; // start at 1
        public static final byte   MSG_RSP_PING=2;
        public static final byte   MSG_NUM_TYPES=MSG_RSP_PING+1;


        public static final byte PACKET_BUF_SIZE=16;


/////////
        public ConnectedThread(BluetoothSocket socket) {
            mmSocket = socket;
            InputStream tmpIn = null;
            OutputStream tmpOut = null;

            // Get the input and output streams, using temp objects because
            // member streams are final
            try {
                tmpIn = socket.getInputStream();
                tmpOut = socket.getOutputStream();
            } catch (IOException e) {
            }

            mmInStream = tmpIn;
            mmOutStream = tmpOut;
            Timer timer = new Timer();
            timer.schedule(new TimerTask() {
                @Override
                public void run() {
                    TimerMethod();
                }

            }, 0, TXMIT_TIMER_DELAY); // no delay, run every 25ms
        }
        private void txByteSend(byte data)
        {
            byte[] buffer = new byte[1];  // buffer store for the stream
            buffer[0]=data;
            bytesSent++;
            write(buffer);
        }



        private void sndPacket(byte type) {
//
            if (lock.tryLock()){
                try {
                    // manipulate protected state
                    txByteSend(HDR_CMD);
                    txByteSend(type);
                } finally {
                    lock.unlock();
                }
            } else {
                // perform alternative actions
            }
        }
        private void TimerMethod()
        {
            byte[] buffer = new byte[1];  // buffer store for the stream
            byte cmd;
            // This routine gets called once every 25ms.
            if (sec) {
                if (lock.tryLock()){
                    try {
                        // manipulate protected state
                        if (txqHead!=txqTail)
                        {
                            // there are cmds to transmit
                            txByteSend(HDR_CMD);
                            cmd=txQ[txqTail];
                            txByteSend(cmd);
                            txqTail++;
                            if (txqTail==32)
                                txqTail=0; // wrapped
                        }
                        else
                            txByteSend(HDR_HELLO);
                    } finally {
                        lock.unlock();
                    }
                } else {
                    // perform alternative actions
                }
            }
            ms+=TXMIT_TIMER_DELAY;
            if (ms==1000) {
                sec = true;
                ms=0; // reset
            }
            else
                sec=false;
        }

        public void run() {
            byte[] buffer = new byte[1024];  // buffer store for the stream
            String buf;
            int bytes,i; // bytes returned from read()


            Looper.prepare();

            mBtHandler = new Handler() {
                @Override
                public void handleMessage(Message msg) {
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
                            addCmd(CMD_START);
                            break;
                        case CMD_ABORT:
                            //tv11.setText("# Fast: "+String.format("%d",cntAbort));
                            break;
                        case CMD_DONE:
                            //tv12.setText("# Fast: "+String.format("%d",cntDone));
                            break;

                        case CMD_PARK:
                            //tv12.setText("# Fast: "+String.format("%d",cntDone));
                            addCmd(CMD_PARK);
                            break;
                        case CMD_CAL:
                            //tv12.setText("# Fast: "+String.format("%d",cntDone));
                            addCmd(CMD_CAL);
                            break;
                    }
                }
            };
            // give the sphinx thread, our handler so he can send us messages
            // - sigh.  using this handler from another thread caused crashes in the handler code
            if (sphinxBTDevice!=null)
                sphinxBTDevice.setBtHandler(mBtHandler);

            Looper.loop(); // this blocks and feeds messages to the handler

            // Keep listening to the InputStream until an exception occurs
            // not currently taking chars back from the RPI.  Will implement separate thread if needed
            // thus, this code is superfluous
            while (true) {
                try {
                    // Read from the InputStream
                    bytes = mmInStream.read(buffer);
                    bytesRcved+=bytes;
                    // if we are running as a handler, send it to processor
                    // else send it to UI
                    /*
                    if ((msgState&MST_RUNNING)==MST_RUNNING)
                    {
                        for (i=0;i<bytes;i++)
                            msgRcvChar(buffer[i]);

                    }
                    else {
                        // Send the obtained bytes to the UI activity
                        buf=new String(buffer, "UTF-8");
                        mHandler.obtainMessage(MESSAGE_DATA, bytes, -1, buf)
                                .sendToTarget();
                    }
                    */
                } catch (IOException e) {
                    break;
                }
            }
    }

        /* Call this from the main activity to send data to the remote device */
        public void write(byte[] bytes) {
            try {
                mmOutStream.write(bytes);
            } catch (IOException e) {
            }
        }

        /* Call this from the main activity to shutdown the connection */
        public void cancel() {
            try {
                mmSocket.close();
            } catch (IOException e) {
            }
        }
    }
}
