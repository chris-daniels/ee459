package com.example.arduinosensors;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.UUID;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Context;
import android.content.Intent;
import android.location.Criteria;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.Bundle;
import android.os.Handler;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

public class MainActivity extends Activity {

	public final static String EXTRA_MESSAGE = "com.example.arduinosensors.MESSAGE";
	Button btnOn, btnOff, btnSend, btnGPS;
	TextView txtArduino, txtString, txtStringLength;
	TextView txtSpeed, txtLatitude, txtLongitude, txtAltitude;
	private char userInput ;
	Handler bluetoothIn;

	final int handlerState = 0;                        //used to identify handler message
	private BluetoothAdapter btAdapter = null;
	private BluetoothSocket inSocket = null;
	private BluetoothSocket outSocket = null;
	
	public LocationManager locManager;
	public LocationListener locListener;
	
	private double latitude, longitude, altitude;
	private float speed;
	
	private StringBuilder recDataString = new StringBuilder();

	private ConnectedThread inConnectedThread;
	private ConnectedThread outConnectedThread;
	
	// SPP UUID service - this should work for most devices
	private static final UUID BTMODULEUUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

	// String for MAC address
	private static String inputAddress;
	private static String outputAddress;
	
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		setContentView(R.layout.activity_main);

		//Link the buttons and textViews to respective views
		btnOn = (Button) findViewById(R.id.buttonOn);
		btnOff = (Button) findViewById(R.id.buttonOff);
		btnSend = (Button) findViewById(R.id.send_input);
		btnGPS = (Button) findViewById(R.id.getGPS);
		txtString = (TextView) findViewById(R.id.txtString);
		txtStringLength = (TextView) findViewById(R.id.testView1);
		txtSpeed = (TextView) findViewById(R.id.txtSpeed);
		txtLatitude = (TextView) findViewById(R.id.txtLatitude);
		txtLongitude = (TextView) findViewById(R.id.txtLongitude);
		txtAltitude = (TextView) findViewById(R.id.txtAltitude);
		
		
//		//set up initial gps stuff
		locManager = (LocationManager) this.getSystemService(Context.LOCATION_SERVICE);
		Location location = locManager.getLastKnownLocation(LocationManager.NETWORK_PROVIDER);
//		Location location = locManager.getLastKnownLocation(LocationManager.GPS_PROVIDER);
		longitude = location.getLongitude();
		txtLongitude.setText("Longitude: " + longitude);
		latitude = location.getLatitude();
		txtLatitude.setText("Latitude: " + latitude);
		altitude = location.getAltitude();
		txtAltitude.setText("Elevation: " + altitude);
		speed = location.getSpeed();
		txtSpeed.setText(speed + "m/s");

		locListener = new LocationListener() {
			
			@Override
		    public void onLocationChanged(Location location) {
				longitude = location.getLongitude();
				txtLongitude.setText("Longitude: " + longitude);
				latitude = location.getLatitude();
				txtLatitude.setText("Latitude: " + latitude);
				altitude = location.getAltitude();
				txtAltitude.setText("Elevation: " + altitude);
				speed = location.getSpeed();
				txtSpeed.setText(speed + "m/s");
				
				//outConnectedThread.write("%Speed: " + speed + "m/s\n" + "Longitude: " + longitude + "\n" + "Latitude: " + latitude + "\n" + "Elevation: " + altitude + "\n");
		    }

			@Override
		    public void onStatusChanged(String provider, int status, Bundle extras) {}

			@Override
		    public void onProviderEnabled(String provider) {}

			@Override
		    public void onProviderDisabled(String provider) {}
		  };
//		  
//		Criteria crit = new Criteria();
//		crit.setAccuracy(Criteria.ACCURACY_FINE);
//		String best = locManager.getBestProvider(crit, false);
		locManager.requestLocationUpdates(LocationManager.NETWORK_PROVIDER, 1000, 1, locListener);
//		locManager.requestLocationUpdates(LocationManager.GPS_PROVIDER, 1000, 1, locListener);

		
		//bluetooth input handler
		bluetoothIn = new Handler() {
			public void handleMessage(android.os.Message msg) {
				if (msg.what == handlerState) {                                     //if message is what we want
					
					String readMessage = (String) msg.obj;                          // msg.arg1 = bytes from connect thread
					
					byte b = readMessage.getBytes()[0];
					int i = b & 0xFF;
					readMessage = Integer.toHexString(i);
					userInput = readMessage.charAt(0);
					
					//sensorView0.setText(readMessage);
					
					if(b != 0)
						sendToHelmet(b);
				}
			}
		};

		btAdapter = BluetoothAdapter.getDefaultAdapter();       // get Bluetooth adapter
		checkBTState(); 

		// Set up onClick listeners for buttons to send 1 or 0 to turn on/off LED
		btnOff.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				outConnectedThread.write("1");    // Send "0" via Bluetooth
				Toast.makeText(getBaseContext(), "Turn off LED", Toast.LENGTH_SHORT).show();
			}
		});

		btnOn.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				outConnectedThread.write("2");    // Send "1" via Bluetooth
				Toast.makeText(getBaseContext(), "Turn on LED", Toast.LENGTH_SHORT).show();
			}
		});
		
		btnSend.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				EditText editText = (EditText) findViewById(R.id.input_text);
				String message = editText.getText().toString();
				outConnectedThread.write(message + "\n");    // Send "1" via Bluetooth
			}
		});
		
		btnGPS.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				//set up initial gps stuff
				Location location = locManager.getLastKnownLocation(LocationManager.NETWORK_PROVIDER);
//				Location location = locManager.getLastKnownLocation(LocationManager.GPS_PROVIDER);
				longitude = location.getLongitude();
				txtLongitude.setText("Longitude: " + longitude);
				latitude = location.getLatitude();
				txtLatitude.setText("Latitude: " + latitude);
				altitude = location.getAltitude();
				txtAltitude.setText("Elevation: " + altitude);
				speed = location.getSpeed();
				txtSpeed.setText(speed + "m/s");
				
				outConnectedThread.write("%Speed: " + speed + "m/s\n" + "Longitude: " + longitude + "\n" + "Latitude: " + latitude + "\n" + "Elevation: " + altitude + "\n");

			}
		});
	}

	private BluetoothSocket createBluetoothSocket(BluetoothDevice device) throws IOException {

		return  device.createRfcommSocketToServiceRecord(BTMODULEUUID);
		//creates secure outgoing connecetion with BT device using UUID
	}

	@Override
	public void onResume() {
		super.onResume();

		//Get MAC address from DeviceListActivity via intent
		Intent intent = getIntent();

		//Get the MAC address from the DeviceListActivty via EXTRA
		inputAddress = intent.getStringExtra(DeviceListActivity.EXTRA_DEVICE_INPUT_ADDRESS);
		outputAddress = intent.getStringExtra(DeviceListActivity.EXTRA_DEVICE_OUTPUT_ADDRESS);
		
		//create device and set the MAC address
		BluetoothDevice inputDevice = btAdapter.getRemoteDevice(inputAddress);
		BluetoothDevice outputDevice = btAdapter.getRemoteDevice(outputAddress);
		
		
		//input socket
		try {
			inSocket = createBluetoothSocket(inputDevice);
		} catch (IOException e) {
			Toast.makeText(getBaseContext(), "Socket creation failed", Toast.LENGTH_LONG).show();
		}
		// Establish the Bluetooth socket connection.
		try
		{
			inSocket.connect();
		} catch (IOException e) {
			try
			{
				inSocket.close();
			} catch (IOException e2)
			{
				//insert code to deal with this
			}
		}
		
		//output socket
		try {
			outSocket = createBluetoothSocket(outputDevice);
		} catch (IOException e) {
			Toast.makeText(getBaseContext(), "Socket creation failed", Toast.LENGTH_LONG).show();
		}
		// Establish the Bluetooth socket connection.
		try
		{
			outSocket.connect();
		} catch (IOException e) {
			try
			{
				outSocket.close();
			} catch (IOException e2)
			{
				//insert code to deal with this
			}
		}
		
		inConnectedThread = new ConnectedThread(inSocket);
		inConnectedThread.start();
		outConnectedThread = new ConnectedThread(outSocket);
		outConnectedThread.start();
		
		//I send a character when resuming.beginning transmission to check device is connected
		//If it is not an exception will be thrown in the write method and finish() will be called
		//outConnectedThread.write("x");
	}

	@Override
	public void onPause()
	{
		super.onPause();
		/*try
		{
			//Don't leave Bluetooth sockets open when leaving activity
			inSocket.close();
			outSocket.close();
		} catch (IOException e2) {
			//insert code to deal with this
		}
		*/
	}

	//Checks that the Android device Bluetooth is available and prompts to be turned on if off
	private void checkBTState() {

		if(btAdapter==null) {
			Toast.makeText(getBaseContext(), "Device does not support bluetooth", Toast.LENGTH_LONG).show();
		} else {
			if (btAdapter.isEnabled()) {
			} else {
				Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
				startActivityForResult(enableBtIntent, 1);
			}
		}
	}

	//Called when user clicks send button
	public void sendMessage(View view) {

	}
	
	public void sendToHelmet(byte b) {
		outConnectedThread.write("#");    // Send "0" via Bluetooth
		outConnectedThread.write_byte(b);
	}
	
	//create new class for connect thread
	private class ConnectedThread extends Thread {
		private final InputStream mmInStream;
		private final OutputStream mmOutStream;

		//creation of the connect thread
		public ConnectedThread(BluetoothSocket socket) {
			InputStream tmpIn = null;
			OutputStream tmpOut = null;

			try {
				//Create I/O streams for connection
				tmpIn = socket.getInputStream();
				tmpOut = socket.getOutputStream();
			} catch (IOException e) { }

			mmInStream = tmpIn;
			mmOutStream = tmpOut;
		}

		public void run() {
			byte[] buffer = new byte[256];
			int bytes; 

			// Keep looping to listen for received messages
			while (true) {
				try {
					bytes = mmInStream.read(buffer);            //read bytes from input buffer
					String readMessage = new String(buffer, 0, bytes);
					// Send the obtained bytes to the UI Activity via handler
					bluetoothIn.obtainMessage(handlerState, bytes, -1, readMessage).sendToTarget();
				} catch (IOException e) {
					break;
				}
			}
		}
		//write method
		public void write(String input) {
			byte[] msgBuffer = input.getBytes();           //converts entered String into bytes
			try {
				
				mmOutStream.write(msgBuffer);                //write bytes over BT connection via outstream
			} catch (IOException e) {
			//	if you cannot write, close the application
				//Toast.makeText(getBaseContext(), "Connection Failure", Toast.LENGTH_LONG).show();
				Toast.makeText(getBaseContext(), e.getMessage(), Toast.LENGTH_LONG);
				finish();
			}
		}
		public void write_byte(byte input) {
			try {
				
				mmOutStream.write(input);                //write bytes over BT connection via outstream
			} catch (IOException e) {
			//	if you cannot write, close the application
				//Toast.makeText(getBaseContext(), "Connection Failure", Toast.LENGTH_LONG).show();
				Toast.makeText(getBaseContext(), e.getMessage(), Toast.LENGTH_LONG);
				finish();
			}
		}
	}
}
