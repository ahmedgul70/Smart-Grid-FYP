package com.example.smartgrid;

import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import androidx.cardview.widget.CardView;

import android.annotation.SuppressLint;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.google.firebase.database.DataSnapshot;
import com.google.firebase.database.DatabaseError;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;
import com.google.firebase.database.ValueEventListener;

public class MainActivity extends AppCompatActivity {
    TextView mainread;
    public SharedPreferences.Editor editor;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        SharedPreferences pref = getApplicationContext().getSharedPreferences("MyPref", 0); // 0 - for private mode
        editor = pref.edit();

        mainread = (TextView) findViewById(R.id.tagline);
        CardView dashboard = findViewById(R.id.cardview1);
        CardView loadA = findViewById(R.id.cardview2);
        CardView loadB = findViewById(R.id.cardview3);
        CardView loadC = findViewById(R.id.cardview4);

        dashboard.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Intent i = new Intent(MainActivity.this, Dashboard.class);
                startActivity(i);
//                finish();
            }
        });
        loadA.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Intent i = new Intent(MainActivity.this, Load1.class);
                startActivity(i);
//                finish();
            }
        });

        loadB.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Intent i = new Intent(MainActivity.this, Load2.class);
                startActivity(i);
//                finish();
            }
        });

        loadC.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Intent i = new Intent(MainActivity.this, Load3.class);
                startActivity(i);
//                finish();
            }
        });



        FirebaseDatabase.getInstance().getReference(".info/connected").addValueEventListener(new ValueEventListener() {
            public void onDataChange(DataSnapshot snapshot) {
                if (((Boolean) snapshot.getValue(Boolean.class)).booleanValue()) {
                    MainActivity.this.mainread.setText("INTERNET CONNECTED ");
                } else {
                    MainActivity.this.mainread.setText("INTERNET CONNECTION FAILED");
                }
            }

            public void onCancelled(DatabaseError error) {
            }
        });
    }

    @SuppressLint("MissingSuperCall")
    @Override
    public void onBackPressed() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("LOG OUT");
        builder.setMessage("Do you want to Log Out ?")
                .setCancelable(false)
                .setPositiveButton("Yes", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        editor.clear();
                        editor.commit(); // commit changes
                        finish();
                        startActivity(new Intent(MainActivity.this, Login.class));
                    }
                })
                .setNegativeButton("No", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        dialog.cancel();
                    }
                });
        AlertDialog alert = builder.create();
        alert.show();
    }
}