package us.fischnet.bt;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Timer;
import java.util.TimerTask;
import java.util.UUID;
import java.util.concurrent.locks.ReentrantLock;

/**
 * Created by lfischer on 3/8/2017.
 * This module is responsible for creating a socket over bluetooth, in order to communicate with
 * the sphinx imaging app, running on another android phone.
 * Start, abort, done commands are passed to sphinx imaging app
 * Messages are received from the sphinx app, and given as control to the arm state machine
 */

public class spCnct{
    private UUID myUUID;
    public Handler mSpHandler;
    public Handler mBtHandler=null;
    public btTerm btTermObj;

    ReentrantLock lock;
    private final String Sphinx_UUID =
            "5a830267-68ec-4c97-a4f9-b9d4f5e89cf3";
    // message codes
    public static final byte COMMAND_CODE = '$'; // precedes a command byte
    public static final byte HELLO_CODE = '#'; // send if there is no command to send - keepalive
    public static final byte ACK_CODE = '%'; // send if there is no command to send - keepalive

    public static final byte CMD_GO_HORIZONTAL = 0; // move horizontally
    public static final byte CMD_GO_UP= CMD_GO_HORIZONTAL+1; // go up
    public static final byte CMD_GO_DOWN= CMD_GO_UP+1; // go back to horizontal
    public static final byte CMD_GO_SCULPT= CMD_GO_DOWN+1; // sculpt
    public static final byte CMD_GO_FACE= CMD_GO_SCULPT+1; // return to face

    // corrective messages from imaging app
    public static final byte CMD_BASE_HINT= 16;
    public static final byte     kHintMsgTooHot                       = CMD_BASE_HINT; // Phone is too hot
    public static final byte     kHintMsgLowBattery                   = CMD_BASE_HINT+1; // Low battery level
    public static final byte     kHintMsgLowStorage                   = CMD_BASE_HINT+2; // Low free disk space
    public static final byte     kHintMsgLowMovementCalib             = CMD_BASE_HINT+3; // Too little movement
    public static final byte     kHintMsgLowMovementSculpt            = CMD_BASE_HINT+4; // Too little movement
    public static final byte     kHintMsgHighMovement                 = CMD_BASE_HINT+5; // Too much movement
    public static final byte     kHintMsgCoverageReminder             = CMD_BASE_HINT+6; // Remind user to cover all parts
    public static final byte     kHintMsgAlmostDone                   = CMD_BASE_HINT+7; // The scan is almost done
    public static final byte     kHintMsgFrameSubject                 = CMD_BASE_HINT+8; // Frame head and shoulder (message shown at the very start)
    public static final byte     kHintMsgOrbitAround                  = CMD_BASE_HINT+9; // Message shown before the calibration phase
    public static final byte     kHintMsgResizeBlob                   = CMD_BASE_HINT+10; // Message shown after the calibration phase
    public static final byte     kHintMsgPreSculpting                 = CMD_BASE_HINT+11; // Message shown during pre-sculpting
    public static final byte     kHintMsgPreSculptingFace             = CMD_BASE_HINT+12; // Message shown during face(180) pre-sculpting
    public static final byte     kHintMsgTrackingLost                 = CMD_BASE_HINT+13; // Tracking lost
    public static final byte     kHintMsgTooClose                     = CMD_BASE_HINT+14; // User is scanning too close
    public static final byte     kHintMsgFaceTutorialStart            = CMD_BASE_HINT+15; // Face tutorial intro message
    public static final byte     kHintMsgFaceTutorialCalibrationStart = CMD_BASE_HINT+16; // Face tutorial calibration start message
    public static final byte     kHintMsgHeadTutorialStart            = CMD_BASE_HINT+17; // Head tutorial intro message
    public static final byte     kHintMsgHeadTutorialCalibrationStart = CMD_BASE_HINT+18; // Head tutorial calibration start message
    public static final byte     kHintMsgInitialSculptingTips         = CMD_BASE_HINT+19; // Hint to the user when starting to sculpt
    public static final byte     CMD_NUM_HINTS = CMD_BASE_HINT+20; // number of hints

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

    private long cntStart=0,cntAbort=0,cntDone=0,cntIdle=0,cntErr=0,cmdDropped=0,cntAck=0;
    private long cntHor=0,cntUp=0,cntDown=0,cntSculpt=0,cntFace=0,cntTooClose=0,cntTooShaky=0,cntTooDark=0,cntTooFast=0;
    private BluetoothAdapter mAdapter=null;
    ThreadConnectBTdevice myThreadConnectBTdevice=null;
    ThreadConnected myThreadConnected=null;
    String address;
    public static final int  MSG_Q_SIZE= 64;



    byte txQ[]=new byte[32];
    byte txqHead=0,txqTail=0;

    public  spCnct (String Addr) {
        BluetoothDevice device;
        myUUID = UUID.fromString(Sphinx_UUID);
        mAdapter = BluetoothAdapter.getDefaultAdapter();

        address=Addr;
        device = mAdapter.getRemoteDevice(address);
        myThreadConnectBTdevice = new ThreadConnectBTdevice(device);
        myThreadConnectBTdevice.start();

    }

    public  void close () {
        if (myThreadConnectBTdevice!=null)
            myThreadConnectBTdevice.cancel();
    };

    /*
     ThreadConnectBTdevice:
     Background Thread to handle BlueTooth connecting
     */
    private class ThreadConnectBTdevice extends Thread {

        private BluetoothSocket bluetoothSocket = null;
        private final BluetoothDevice bluetoothDevice;


        private ThreadConnectBTdevice(BluetoothDevice device) {
            bluetoothDevice = device;
            lock=new ReentrantLock();
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
                // error
                try {
                    bluetoothSocket.close();
                } catch (IOException e1) {
                    // TODO Auto-generated catch block
                    e1.printStackTrace();
                }
            }

            if (success) {
                //connect successful
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

    public void setBtHandler(Handler hand) {
        mBtHandler=hand;
    }
    public void setBtObj(btTerm btT) {
        btTermObj=btT;
        //btTermObj.addMsg((byte) 1);
    }

     //Called in ThreadConnectBTdevice once connect successed
    //to start ThreadConnected
    public void startThreadConnected(BluetoothSocket socket) {

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

        boolean sec=false;
        int ms=0;
        public static final int TXMIT_TIMER_DELAY	=25; // ms to sleep between tx timer

        public ThreadConnected(BluetoothSocket socket) {
            // constructor
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

            // Create recv Thread Class
            Thread spRcv = new Thread(new Runnable() {
                public void run() {
                    byte[] buffer = new byte[1024];
                    byte msg;
                    int bytes;
                    int idx;
                    int state=0; // what is our state

                    while (true) {
                        try {
                            bytes = connectedInputStream.read(buffer);  // this blocks until receipt
                            for (idx=0;idx<bytes;idx++)
                            {
                                msg=buffer[idx]; // get the command
                                if (msg==HELLO_CODE)
                                {
                                    cntIdle++;
                                    state=0; // clear the state
                                }
                                else if (msg==ACK_CODE)
                                {
                                    state=0x01; // set the ack state
                                }
                                else if (msg==COMMAND_CODE)
                                {
                                    state=0x02; // set the command state
                                }
                                else
                                {
                                    // not a hello, ack or command byte.  Data.  whats our state
                                    if (state ==1)
                                    {
                                        // ack state
//                                        btTermObj.addMsg(msg); // tell the main thread
//                                        if (mBtHandler!=null)
//                                            mBtHandler.obtainMessage(msg).sendToTarget();
                                    }
                                    else if (state==2)
                                    {
                                        // command state
                                        msg|=CMD_BASE_HINT; //
                                        btTermObj.addMsg(msg); // tell the main thread
                                        if (mBtHandler!=null)
                                            mBtHandler.obtainMessage(msg).sendToTarget();

                                    }
                                    state=0; // clear the state
                                }
                            }
                        } catch (IOException e) {
                            // TODO Auto-generated catch block
                            e.printStackTrace();

                        }
                    }

                }
            });

            spRcv.start(); // start the recv thread
            Timer timer = new Timer();
            timer.schedule(new TimerTask() {
                @Override
                public void run() {
                    TimerMethod(); // the timer method is responsible for transmitting commands to the Sphinx app/phone
                }

            }, 0, TXMIT_TIMER_DELAY); // no delay, run every 25ms

            //  OK - as I research all this, I realize that android has interfaces, designed for this
            // but Im rushed now, so Im doing it quick and dirty. :(

        }

        public boolean addCmd(byte cmd)
        {
            // add a command into the buffer to ship to the Sphinx phone
            // is it full?


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

        @Override
        public void run() {




            Looper.prepare();

            mSpHandler = new Handler() {
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

                        case CMD_START:
                            //tv10.setText("# Fast: "+String.format("%d",cntStart));
                            addCmd(CMD_START);
                            break;
                        case CMD_ABORT:
                            //tv11.setText("# Fast: "+String.format("%d",cntAbort));
                            addCmd(CMD_ABORT);
                            break;
                        case CMD_DONE:
                            //tv12.setText("# Fast: "+String.format("%d",cntDone));
                            addCmd(CMD_DONE);
                            break;
                    }
                }
            };
            Looper.loop(); // this blocks and feeds messages to the handler

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
                            buffer[0]=COMMAND_CODE;
                            write(buffer);
                            cmd=txQ[txqTail];
                            buffer[0]=cmd;
                            write(buffer);
                            txqTail++;
                            if (txqTail==32)
                                txqTail=0; // wrapped
                        }
                        else {
                            // queue is empty
                            buffer[0]=HELLO_CODE;
                            write(buffer);
                        }
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


        public void write(byte[] buffer) {
            // does the writing over the bluetooth socket
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
