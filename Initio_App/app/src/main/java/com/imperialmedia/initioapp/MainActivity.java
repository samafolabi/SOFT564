package com.imperialmedia.initioapp;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;

import android.os.Bundle;
import android.text.method.ScrollingMovementMethod;
import android.view.View;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.TextView;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;

public class MainActivity extends AppCompatActivity {

    TextView messages;
    Button subscribe_button;

    private PrintWriter out;
    private BufferedReader in;
    private Socket s;
    final String IP = "192.168.43.21";
    final int port = 9000;
    boolean subscribed = false;
    char direction = 0; //0 - stop, 1 - fwd, 2 - bwd, 3 - lft, 4 - rgt

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Toolbar myToolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(myToolbar);

        subscribe_button = (Button) findViewById(R.id.subscribe);
        messages = (TextView) findViewById(R.id.messages);
        messages.setMovementMethod(new ScrollingMovementMethod());
    }

    public void subscribe(View v) {
        new Thread(new Connect()).start();
    }

    public void send(String msg) {
        if (subscribed) {
            messages.append(msg + '\n');
            new Thread(new Send("_S_" + msg + "_"));
        }
    }

    public void up(View v) {
        String dir = "FWD";
        if (direction == 1) {
            dir = "STP";
        }
        send(dir);
    }

    public void down(View v) {
        String dir = "BWD";
        if (direction == 2) {
            dir = "STP";
        }
        send(dir);
    }

    public void left(View v) {
        String dir = "LFT";
        if (direction == 3) {
            dir = "STP";
        }
        send(dir);
    }

    public void right(View v) {
        String dir = "RGT";
        if (direction == 4) {
            dir = "STP";
        }
        send(dir);
    }

    public void rotate_left(View v) {
        send("ROL");
    }

    public void rotate_right(View v) {
        send("ROR");
    }

    class Connect implements Runnable {
        public void run() {
            try {
                s = new Socket(IP, port);
                out = new PrintWriter(s.getOutputStream());
                in = new BufferedReader(new InputStreamReader(s.getInputStream()));
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        out.write("_S_SUBDR_");
                        messages.append("App: Subscribed\n");
                        subscribed = true;
                        subscribe_button.setEnabled(false);
                    }
                });
                new Thread(new Receive()).start();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    class Receive implements Runnable {
        @Override
        public void run() {
            while (true) {
                try {
                    final String message = in.readLine();
                    if (message != null) {
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                messages.append("Server: " + message + "\n");
                            }
                        });
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                    try {
                        in.close();
                        out.close();
                        s.close();
                    } catch (Exception ex) {
                        ex.printStackTrace();
                    }
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            messages.append("App: Unsuscribed\n");
                            subscribe_button.setEnabled(true);
                            subscribed = false;
                        }
                    });
                }
            }
        }
    }

    class Send implements Runnable {
        private String message;
        Send(String message) {
            this.message = message;
        }
        @Override
        public void run() {
            try {
                out.write(message);
                out.flush();
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        messages.append("App: " + message + "\n");
                    }
                });
            } catch (Exception e) {
                e.printStackTrace();
                try {
                    in.close();
                    out.close();
                    s.close();
                } catch (Exception ex) {
                    ex.printStackTrace();
                }
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        messages.append("App: Unsuscribed\n");
                        subscribe_button.setEnabled(true);
                        subscribed = false;
                    }
                });
            }
        }
    }
}