package us.fischnet.myapplication;

import android.app.Activity;
import android.content.Intent;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.widget.Button;
import android.widget.TextView;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.Toast;

import com.google.android.gms.appindexing.Action;
import com.google.android.gms.appindexing.AppIndex;
import com.google.android.gms.common.api.GoogleApiClient;

import java.util.ArrayList;
import java.util.Set;

import us.fischnet.bt.btTerm;
import us.fischnet.bt.spCnct;

public class MainActivity extends Activity {
    Button b1, b2, b3, b4, b5,b6;
    private BluetoothAdapter mBtAdapter;
    private BluetoothAdapter BA;
    private Set<BluetoothDevice> pairedDevices;
    ListView lv;
    String address; // target device
    BluetoothDevice targetBTDevice;
    spCnct sphinxBTDevice;

    public static final byte CMD_GO_HORIZONTAL = 0; // move horizontally
    public static final byte CMD_GO_UP= CMD_GO_HORIZONTAL+1; // go up
    public static final byte CMD_GO_DOWN= CMD_GO_UP+1; // go back to horizontal
    public static final byte CMD_SCULPT= CMD_GO_DOWN+1; // sculpt
    public static final byte CMD_GO_FACE= CMD_SCULPT+1; // return to face

    // corrective messages from imaging app
    public static final byte CMD_TOO_CLOSE= 16;
    public static final byte CMD_TOO_SHAKY= CMD_TOO_CLOSE+1;
    public static final byte CMD_TOO_DARK= CMD_TOO_SHAKY+1;
    public static final byte CMD_TOO_FAST= CMD_TOO_DARK+1;

    // incoming control messages from arm controller
    public static final byte CMD_START = 64;
    public static final byte CMD_ABORT = CMD_START+1;
    public static final byte CMD_DONE = CMD_ABORT+1;

    /**
     * ATTENTION: This was auto-generated to implement the App Indexing API.
     * See https://g.co/AppIndexing/AndroidStudio for more information.
     */
    private GoogleApiClient client;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        b1 = (Button) findViewById(R.id.button);
        b2 = (Button) findViewById(R.id.button2);
        b3 = (Button) findViewById(R.id.button3);
        b4 = (Button) findViewById(R.id.button4);
        b5 = (Button) findViewById(R.id.button5);
        b6 = (Button) findViewById(R.id.button6);

        BA = BluetoothAdapter.getDefaultAdapter();
        lv = (ListView) findViewById(R.id.listView);
        // ATTENTION: This was auto-generated to implement the App Indexing API.
        // See https://g.co/AppIndexing/AndroidStudio for more information.
        client = new GoogleApiClient.Builder(this).addApi(AppIndex.API).build();
    }

    public void on(View v) {
        if (!BA.isEnabled()) {
            Intent turnOn = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(turnOn, 0);
            Toast.makeText(getApplicationContext(), "Turned on", Toast.LENGTH_LONG).show();
        } else {
            Toast.makeText(getApplicationContext(), "Already on", Toast.LENGTH_LONG).show();
        }
    }

    public void off(View v) {
        //BA.disable();
        //Toast.makeText(getApplicationContext(), "Turned off", Toast.LENGTH_LONG).show();
        sphinxBTDevice.mSpHandler.obtainMessage((int) CMD_START).sendToTarget();
    }

    public void visible(View v) {
        Intent getVisible = new Intent(BluetoothAdapter.ACTION_REQUEST_DISCOVERABLE);
        startActivityForResult(getVisible, 0);
    }

    public void connect(View v) {
        Intent intent = new Intent(getApplicationContext(), btTerm.class);
        intent.putExtra("TARGET_BT_DEVICE",address);
        startActivityForResult(intent,0);
    }

    public void spCnct(View v) {
        // this initiates a BT connection to the Sphinx application

        //Intent intent = new Intent(getApplicationContext(), spCnct.class);
        //intent.putExtra("TARGET_BT_DEVICE",address);
        //startActivityForResult(intent,0);
        sphinxBTDevice=new spCnct(address);

    }

    public void list(View v) {
        pairedDevices = BA.getBondedDevices();
        ArrayList list = new ArrayList();
        Intent intent = null;

        for (BluetoothDevice bt : pairedDevices)
            list.add(bt.getName() + " " + bt.getAddress());
        Toast.makeText(getApplicationContext(), "Showing Paired Devices", Toast.LENGTH_SHORT).show();

        final ArrayAdapter adapter = new ArrayAdapter(this, android.R.layout.simple_list_item_1, list);
        lv.setAdapter(adapter);

        lv.setOnItemClickListener(new OnItemClickListener() {
            public void onItemClick(AdapterView<?> parent, View view,
                                    int position, long id) {
//                Toast.makeText(getApplicationContext(), "clicked device", 2).show();
                String info = ((TextView) view).getText().toString();
                address = info.substring(info.length() - 17);
                for (int i=0;i<=position; i++)
                    targetBTDevice= pairedDevices.iterator().next();
            }
        });
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.

        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }
        return super.onOptionsItemSelected(item);
    }
}
