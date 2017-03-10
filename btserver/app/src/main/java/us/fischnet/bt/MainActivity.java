package us.fischnet.btserver;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothServerSocket;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStreamWriter;
import java.util.UUID;

public class MainActivity extends Activity {
    Button b1,b2;
    TextView tv10,tv11,tv12;
    TextView tv1,tv2,tv3,tv4,tv5,tv6,tv7,tv8,tv9;

    // message codes
    private static final byte COMMAND_CODE = '$'; // precedes a command byte
    private static final byte HELLO_CODE = '#'; // send if there is no command to send - keepalive

    private static final byte WRITER_SLEEP_MS = 100; // how many ticks we wait based on 100ms thread sleep
    private static final byte KEEPALIVE_TIMER = 1000/WRITER_SLEEP_MS; // how many iterations we wait based on thread sleep ms count


    // outgoing messages
    /* // sigh - Java doesnt give us the ability to assign numbers to enums
    enum Messages {
        CMD_GO_HORIZONTAL=1,
                CMD_GO_UP,
                CMD_GO_DOWN,
                CMD_SCULPT,
                CMD_TOO_CLOSE=16,
                CMD_TOO_SHAKY,
                CMD_TOO_DARK,
                CMD_TOO_FAST,
                CMD_START=128,
                CMD_ABORT,
                CMD_DONE;
    }
    */

    // various messages are
    // comamands from imaging app
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

    private static final int DISCOVERABLE_REQUEST_CODE = 0x1;
    private boolean CONTINUE_READ_WRITE = true;
    private long cntStart=0,cntAbort=0,cntDone=0,cntIdle=0,cntErr=0,cmdDropped=0;
    private long cntHor=0,cntUp=0,cntDown=0,cntSculpt=0,cntFace=0,cntTooClose=0,cntTooShaky=0,cntTooDark=0,cntTooFast=0;

    private static final int CMD_BUF_SIZE= 64;
    byte[] cmdBuf = new byte[CMD_BUF_SIZE];  // buffer store for the stream
    int cmdHead=0,cmdTail=0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        b1 = (Button) findViewById(R.id.button1);
        b2 = (Button) findViewById(R.id.button2);


        tv10 = (TextView) findViewById(R.id.start);
        tv11 = (TextView) findViewById(R.id.Abort);
        tv12 = (TextView) findViewById(R.id.Done);

        tv1 = (TextView) findViewById(R.id.tv1);
        tv2 = (TextView) findViewById(R.id.textView11);
        tv3 = (TextView) findViewById(R.id.textView12);
        tv4 = (TextView) findViewById(R.id.textView13);
        tv5 = (TextView) findViewById(R.id.textView14);
        tv6 = (TextView) findViewById(R.id.textView15);
        tv7 = (TextView) findViewById(R.id.textView16);
        tv8 = (TextView) findViewById(R.id.textView17);
        tv9 = (TextView) findViewById(R.id.textView18);

        //Always make sure that Bluetooth server is discoverable during listening...
        Intent discoverableIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_DISCOVERABLE);
        startActivityForResult(discoverableIntent, DISCOVERABLE_REQUEST_CODE);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        android.util.Log.e("TrackingFlow", "Creating thread to start listening...");
        new Thread(reader).start();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if(socket != null){
            try{
                is.close();
                os.close();
                socket.close();
            }catch(Exception e){}
            CONTINUE_READ_WRITE = false;
        }
    }


    public void clkHor(View v) {
        // test code to simulate imaging app commands
        // this shows how to send a message to the BT thread
        mHandler.obtainMessage((int) CMD_GO_HORIZONTAL)
                .sendToTarget();
    }

    public void clkTooClose(View v) {
        // test code to simulate imaging app commands
        // this shows how to send a message to the BT thread
        mHandler.obtainMessage((int) CMD_TOO_CLOSE)
                .sendToTarget();
    }


    private BluetoothSocket socket;
    private InputStream is;
    private OutputStreamWriter os;

    private Runnable reader = new Runnable() {
        public void run() {
            BluetoothServerSocket serverSocket=null;
            BluetoothAdapter adapter = BluetoothAdapter.getDefaultAdapter();
            UUID MY_UUID_INSECURE =
                    UUID.fromString("5a830267-68ec-4c97-a4f9-b9d4f5e89cf3");
            while (true) {
                // always try to wait on this socket
                try {
                    serverSocket = adapter.listenUsingRfcommWithServiceRecord("BTServer", MY_UUID_INSECURE);
                    android.util.Log.e("TrackingFlow", "Listening...");

                    socket = serverSocket.accept(); // blocks waiting
                    android.util.Log.e("TrackingFlow", "Socket accepted...");

                    CONTINUE_READ_WRITE = true;

                    is = socket.getInputStream();
                    os = new OutputStreamWriter(socket.getOutputStream());
                    new Thread(writter).start();

                    int bufferSize = 1024;
                    int bytesRead = -1;
                    int ptr=0;
                    byte[] buffer = new byte[bufferSize];
                    //Keep reading the messages while connection is open...
                    while (CONTINUE_READ_WRITE) {
                        // incoming messages from controller app
                        final StringBuilder sb = new StringBuilder();
                        bytesRead = is.read(buffer);
                        ptr=0;
                        if (bytesRead != -1) {
                            // parse the message here
                            while (ptr<bytesRead)
                            {
                                byte cmd=buffer[ptr++];
                                if (cmd==COMMAND_CODE)
                                {
                                    // get next byte, which is command
                                      if (ptr!=bytesRead)
                                      {
                                          cmd=buffer[ptr++];
                                          switch (cmd)
                                          {
                                              // these are commands received from the Arm controller phone
                                              // need to pass these to the imaging app via some API
                                              // for now, we just incrementing a counter, and updating display
                                              // via the handler
                                              case ((int) CMD_START):
                                                  cntStart++;
                                                  mHandler.obtainMessage((int) CMD_START)
                                                          .sendToTarget();
                                                  break;
                                              case ((int) CMD_ABORT):
                                                  cntAbort++;
                                                  mHandler.obtainMessage((int) CMD_ABORT)
                                                          .sendToTarget();
                                                  break;
                                              case ((int) CMD_DONE):
                                                  cntDone++;
                                                  mHandler.obtainMessage((int) CMD_DONE)
                                                          .sendToTarget();
                                                  break;

                                          }
                                          // now ack it
                                          cmd|=128; // set the high bit (makes it negative in Java
                                          addCmd(cmd); // and send it out
                                      }
                                    else
                                          cntErr++; // we had a command code with no command
                                }
                                    else if (cmd<0) {
                                    // received an Ack.  The server doesnt really care
                                        ;
                                    }
                                    else {
                                    cntIdle++; // this should be an idle code
                                }
                            }
                            // update the message timestamp

                        }
                    }
                } catch (IOException e) {
                    e.printStackTrace();
                    // close sockets go back to waiting on accept
                    try{
                        is.close();
                        os.close();
                        socket.close();
                        serverSocket.close();
                    }catch(Exception e1){}
                    CONTINUE_READ_WRITE = false; // this will cause the writer thread to close
                }
            }
            // the socket closed.
        }
    };

    void addCmd(byte cmd)
    {
        // add a command into the buffer
        // is it full?
        if (((cmdTail-cmdHead)==1)|| (cmdHead==0 && cmdTail==CMD_BUF_SIZE))
            cmdDropped++; //full
        else {
            // enque the command
            cmdBuf[cmdHead++]=cmd;
            if (cmdHead==CMD_BUF_SIZE) // did we hit the end?
                cmdHead=0; // wrap the ptr
        }
    }

    private Runnable writter = new Runnable() {

        @Override
        public void run() {
            // separate thread to write messages over BT stack
            byte cmd,cnt=0;

            while(CONTINUE_READ_WRITE){
                try {
                    // this thread provides outgoing messages to controller app
                    if (cmdHead==cmdTail)
                    {
                        // Cmd Que empty.
                        // just send a hello message to the other controller to let him know we are alive
                        if (cnt++ >=KEEPALIVE_TIMER)
                        {
                            os.write((HELLO_CODE)); // sends over BT stack
                            cnt=0;
                        }
                    }
                    else
                    {
                        // there is a command to send.  Do it
                        os.write((COMMAND_CODE));// sends over BT stack
                        cmd=cmdBuf[cmdTail++];
                        if (cmdTail==CMD_BUF_SIZE) // at end?
                            cmdTail=0; // wrap it
                        os.write((cmd));// sends over BT stack
                        // and reset the keepalive counter
                        cnt=0;
                    }
                    os.flush();
                    Thread.sleep(WRITER_SLEEP_MS);

                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
            android.util.Log.e("TrackingFlow", "Closing writer thread");
        }
    };

    private final Handler mHandler = new Handler() {
        // this is a message handler to allow the android imaging app and BT thread to talk to each other.
        // it will atomically send a command out to the arm controller
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case CMD_GO_HORIZONTAL:
                    addCmd(CMD_GO_HORIZONTAL);
                    cntHor++;
                    tv1.setText("# Hor: "+String.format("%d",cntHor));
                    break;
                case CMD_GO_UP:
                    cntUp++;
                    addCmd(CMD_GO_UP);
                    tv2.setText("# Up: "+String.format("%d",cntUp));
                    break;
                case CMD_GO_DOWN:
                    cntDown++;
                    addCmd(CMD_GO_DOWN);
                    tv3.setText("# Down: "+String.format("%d",cntDown));
                    break;
                case CMD_SCULPT:
                    cntSculpt++;
                    addCmd(CMD_SCULPT);
                    tv4.setText("# Sculpt: "+String.format("%d",cntSculpt));
                    break;
                case CMD_GO_FACE:
                    cntFace++;
                    addCmd(CMD_GO_FACE);
                    tv5.setText("# Face: "+String.format("%d",cntFace));
                    break;
                case CMD_TOO_CLOSE:
                    cntTooClose++;
                    addCmd(CMD_TOO_CLOSE);
                    tv6.setText("# Close: "+String.format("%d",cntTooClose));
                    break;
                case CMD_TOO_SHAKY:
                    cntTooShaky++;
                    addCmd(CMD_TOO_SHAKY);
                    tv7.setText("# Shaky: "+String.format("%d",cntTooShaky));
                    break;
                case CMD_TOO_DARK:
                    cntTooDark++;
                    addCmd(CMD_TOO_DARK);
                    tv8.setText("# Dark: "+String.format("%d",cntTooDark));
                    break;
                case CMD_TOO_FAST:
                    cntTooFast++;
                    addCmd(CMD_TOO_FAST);
                    tv9.setText("# Fast: "+String.format("%d",cntTooFast));
                    break;

                // These are simply for display.  Commands received from arm controller
                case CMD_START:
                    tv10.setText("# Start: "+String.format("%d",cntStart));
                    break;
                case CMD_ABORT:
                    tv11.setText("# Abort: "+String.format("%d",cntAbort));
                    break;
                case CMD_DONE:
                    tv12.setText("# Done: "+String.format("%d",cntDone));
                    break;

            }
        }
    };
}
