package us.fischnet.myapplication;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.google.android.gms.appindexing.AppIndex;
import com.google.android.gms.common.api.GoogleApiClient;

import java.util.ArrayList;
import java.util.Set;

import us.fischnet.bt.btTerm;

import static android.content.Intent.FLAG_ACTIVITY_REORDER_TO_FRONT;

public class MainActivity extends Activity {
    Button b1, b2, b3, b4, b5,b6;
    private BluetoothAdapter mBtAdapter;
    private BluetoothAdapter BA;
    private Set<BluetoothDevice> pairedDevices;
    Intent btIntent;
    ListView lv;
    String address,rpiAddress,spAddress; // target device
    int listPos,listSPPos,listRPIPos;
    BluetoothDevice targetBTDevice;

    public static final byte CMD_GO_HORIZONTAL = 0; // move horizontally
    public static final byte CMD_GO_UP= CMD_GO_HORIZONTAL+1; // go up
    public static final byte CMD_GO_DOWN= CMD_GO_UP+1; // go back to horizontal
    public static final byte CMD_GO_SCULPT= CMD_GO_DOWN+1; // sculpt
    public static final byte CMD_GO_FACE= CMD_GO_SCULPT+1; // return to face

    // corrective messages from imaging app
    public static final byte CMD_BASE_HINT= 16;
    public static final int     kHintMsgTooHot                       = CMD_BASE_HINT; // Phone is too hot
    public static final int     kHintMsgLowBattery                   = CMD_BASE_HINT+1; // Low battery level
    public static final int     kHintMsgLowStorage                   = CMD_BASE_HINT+2; // Low free disk space
    public static final int     kHintMsgLowMovementCalib             = CMD_BASE_HINT+3; // Too little movement
    public static final int     kHintMsgLowMovementSculpt            = CMD_BASE_HINT+4; // Too little movement
    public static final int     kHintMsgHighMovement                 = CMD_BASE_HINT+5; // Too much movement
    public static final int     kHintMsgCoverageReminder             = CMD_BASE_HINT+6; // Remind user to cover all parts
    public static final int     kHintMsgAlmostDone                   = CMD_BASE_HINT+7; // The scan is almost done
    public static final int     kHintMsgFrameSubject                 = CMD_BASE_HINT+8; // Frame head and shoulder (message shown at the very start)
    public static final int     kHintMsgOrbitAround                  = CMD_BASE_HINT+9; // Message shown before the calibration phase
    public static final int     kHintMsgResizeBlob                   = CMD_BASE_HINT+10; // Message shown after the calibration phase
    public static final int     kHintMsgPreSculpting                 = CMD_BASE_HINT+11; // Message shown during pre-sculpting
    public static final int     kHintMsgPreSculptingFace             = CMD_BASE_HINT+12; // Message shown during face(180) pre-sculpting
    public static final int     kHintMsgTrackingLost                 = CMD_BASE_HINT+13; // Tracking lost
    public static final int     kHintMsgTooClose                     = CMD_BASE_HINT+14; // User is scanning too close
    public static final int     kHintMsgFaceTutorialStart            = CMD_BASE_HINT+15; // Face tutorial intro message
    public static final int     kHintMsgFaceTutorialCalibrationStart = CMD_BASE_HINT+16; // Face tutorial calibration start message
    public static final int     kHintMsgHeadTutorialStart            = CMD_BASE_HINT+17; // Head tutorial intro message
    public static final int     kHintMsgHeadTutorialCalibrationStart = CMD_BASE_HINT+18; // Head tutorial calibration start message
    public static final int     kHintMsgInitialSculptingTips         = CMD_BASE_HINT+19; // Hint to the user when starting to sculpt


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

    /**
     * ATTENTION: This was auto-generated to implement the App Indexing API.
     * See https://g.co/AppIndexing/AndroidStudio for more information.
     */
    private GoogleApiClient client;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        b3 = (Button) findViewById(R.id.button3);
        b5 = (Button) findViewById(R.id.button5);
        b6 = (Button) findViewById(R.id.button6);

        BA = BluetoothAdapter.getDefaultAdapter();
        lv = (ListView) findViewById(R.id.listView);
        // ATTENTION: This was auto-generated to implement the App Indexing API.
        // See https://g.co/AppIndexing/AndroidStudio for more information.
        client = new GoogleApiClient.Builder(this).addApi(AppIndex.API).build();
    }



    public void connect(View v) {
        rpiAddress=address; // save the address for the RPI
        // should clear the background color of the old setting, if there was one
        listRPIPos=listPos; // save it
        lv.getChildAt(listRPIPos).setBackgroundColor(
                Color.parseColor("YELLOW"));

        if (btIntent!=null)
        {
            btIntent.setFlags(FLAG_ACTIVITY_REORDER_TO_FRONT);
        }
        else {
            btIntent = new Intent(getApplicationContext(), btTerm.class);
        }
        btIntent.putExtra("TARGET_BT_DEVICE", address);
        btIntent.putExtra("TARGET_SP_DEVICE", spAddress);
        startActivityForResult(btIntent, 0);

    }

    public void spCnct(View v) {
        // grabs the address for talking to the sphinx phone.
        spAddress=address; // save the sphinx address
        listSPPos=listPos;
        // should highlight this with a specific color

        lv.getChildAt(listSPPos).setBackgroundColor(
                Color.parseColor("CYAN"));
    //    Color.parseColor("#00743D"));
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
                String info = ((TextView) view).getText().toString();
                address = info.substring(info.length() - 17);
                listPos=position;

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
