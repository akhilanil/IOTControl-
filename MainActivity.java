    package com.example.akhil.iotcontrol;

    import android.content.Context;
    import android.content.SharedPreferences;
    import android.graphics.Color;
    import android.os.AsyncTask;
    import android.app.AlertDialog;
    import android.support.v7.app.AppCompatActivity;
    import android.os.Bundle;
    import android.view.View;
    import android.widget.Button;
    import android.widget.EditText;
    import android.widget.Toast;

    import java.io.BufferedInputStream;
    import java.io.IOException;
    import java.io.InputStream;
    import java.net.HttpURLConnection;
    import java.net.URL;

    public class MainActivity extends AppCompatActivity implements View.OnClickListener {

        public final static String PREF_IP = "PREF_IP_ADDRESS";
        public final static String PREF_PORT = "PREF_PORT_NUMBER";

        private boolean WS1Status = false;
        private boolean WS2Status = false;
        private boolean WS3Status = false;

        private String button = "";

        private Button WS1,WS2,WS3;
        private EditText ip, port, data;

        private int skey;
        private int publicKey;

        SharedPreferences.Editor editor;
        SharedPreferences sharedPreferences;


        @Override
        protected void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            setContentView(R.layout.activity_main);

            skey = 79;
            publicKey = 3337;

            sharedPreferences = getSharedPreferences("HTTP_HELPER_PREFS", Context.MODE_PRIVATE);
            editor = sharedPreferences.edit();

            WS1 = (Button)findViewById(R.id.WS1);
            WS2 = (Button)findViewById(R.id.WS2);
            WS3 = (Button)findViewById(R.id.WS3);

            data = (EditText)findViewById(R.id.data);
            ip   = (EditText)findViewById(R.id.ip);
            port = (EditText)findViewById(R.id.port);


            WS1.setOnClickListener(this);
            WS2.setOnClickListener(this);
            WS3.setOnClickListener(this);

            ip.setText(sharedPreferences.getString(PREF_IP,""));
            port.setText(sharedPreferences.getString(PREF_PORT,""));

            data.setFocusable(false);

        }

        String messageEncrypt(int data) {

            int val = 0;
            int power;
            int i, temp, mod, num;

            power = skey;
            mod = publicKey;
            val = data;
            num = data;

            for(i = 1; i<power; i++) {
                temp = num * val;
                val = temp % mod;
            }

            return String.valueOf(val);
        }

        @Override
        public void onClick(View view) {


            String parameterValue = "";

            String ipAddress = ip.getText().toString().trim();

            String portNumber = port.getText().toString().trim();

            editor.putString(PREF_IP, ipAddress);
            editor.putString(PREF_PORT, portNumber);
            editor.commit();

            if (view.getId() == WS1.getId()) {
                parameterValue = "13";

                button = "13";
            }
            else if (view.getId() == WS2.getId()) {
                parameterValue = "12";
                button = "12";
            }
            else {
                parameterValue = "11";
                button = "11";
            }

            data.setText(parameterValue);
            parameterValue = messageEncrypt(Integer.parseInt(parameterValue));
            data.setText(data.getText().toString().trim() + " : "+parameterValue);

            if(!ipAddress.equals("") && !portNumber.equals("")) {
                new HttpRequestAsyncTask(
                        view.getContext(), parameterValue, ipAddress, portNumber
                ).execute();





            }
        }


        public String sendRequest(String parameterValue, String ipAddress, Context context,
                                  String portNumber, String parameterName, InputStream inputStream) {


            String serverResponse = "ERROR";
            String link = "http://"+ipAddress+":"+portNumber+"/?"+parameterName+"="+parameterValue+
                          "&key="+publicKey;

            try {
                URL url = new URL(link);
                HttpURLConnection conn = (HttpURLConnection) url.openConnection();
                HttpURLConnection.setFollowRedirects(false);
                conn.setRequestMethod("GET");
                conn.setConnectTimeout(5000);
                conn.setReadTimeout(5000);
                System.out.println("WAITING FOR RESPONSE..!!!");
                inputStream = new BufferedInputStream(conn.getInputStream());
                serverResponse = org.apache.commons.io.IOUtils.toString(inputStream, "UTF-8");
                inputStream.close();

            }catch (IOException e) {
                e.printStackTrace();
            }


            return serverResponse;
        }


        private class HttpRequestAsyncTask extends AsyncTask<Void, Void ,Void> {


            private String requestReply,ipAddress, portNumber;
            private Context context;

            private AlertDialog alertDialog;
            private String parameter = "pin";
            private String parameterValue;


            public HttpRequestAsyncTask(Context context, String parameterValue, String ipAddress,
                                        String portNumber ) {

                this.context = context;
                this.ipAddress = ipAddress;
                this.parameterValue = parameterValue;
                this.portNumber = portNumber;

                alertDialog = new AlertDialog.Builder(this.context)
                        .setTitle("HTTP Response From IP Address:")
                        .create();


            }


            @Override
            protected Void doInBackground(Void... voids) {

               InputStream inputStream = null;

                alertDialog.setMessage("Data sent, waiting for reply from server...");
                alertDialog.setCancelable(false);
                if(!alertDialog.isShowing()) {
                    alertDialog.show();
                }
                this.requestReply = sendRequest(parameterValue, ipAddress, context,
                                                portNumber, parameter, inputStream);
                return null;
            }

            @Override
            protected void onPostExecute(Void aVoid) {

                alertDialog.setMessage(requestReply);
                alertDialog.setCancelable(true);
                if(!alertDialog.isShowing()) {
                    alertDialog.show(); // show dialog
                }
                if(!requestReply.equals("ERROR")){
                    if(button.equals("13")) {
                        if(!WS1Status) {
                                WS1.setBackgroundColor(Color.rgb(84, 138, 245));
                                WS1.setTextColor(Color.rgb(255, 255, 255));
                            }
                            else {
                                WS1.setBackgroundColor(Color.rgb(170, 170, 170));
                                WS1.setTextColor(Color.rgb(0, 0, 0));
                            }
                            WS1Status = !WS1Status;
                        }
                        else if(button.equals("12")) {
                            if(!WS2Status) {
                                WS2.setBackgroundColor(Color.rgb(84, 138, 245));
                                WS2.setTextColor(Color.rgb(255, 255, 255));
                            }
                            else {
                                WS2.setBackgroundColor(Color.rgb(170, 170, 170));
                                WS2.setTextColor(Color.rgb(0, 0, 0));
                            }
                            WS2Status = !WS2Status;
                        }
                        else {
                            if(!WS3Status) {
                                WS3.setBackgroundColor(Color.rgb(84, 138, 245));
                                WS3.setTextColor(Color.rgb(255, 255, 255));
                            }
                            else {
                                WS3.setBackgroundColor(Color.rgb(170, 170, 170));
                                WS3.setTextColor(Color.rgb(0, 0, 0));
                            }
                            WS3Status = !WS3Status;
                        }
                }

            }

            @Override
            protected void onPreExecute() {
                alertDialog.setMessage("Sending data to server, please wait...");
                alertDialog.setCancelable(false);
                if(!alertDialog.isShowing()) {
                    alertDialog.show();
                }
            }


        }



    }
