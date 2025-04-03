package com.example.smartgrid;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.widget.CompoundButton;
import android.widget.ImageView;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import com.google.firebase.database.DataSnapshot;
import com.google.firebase.database.DatabaseError;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;
import com.google.firebase.database.ValueEventListener;

public class Load3 extends AppCompatActivity {
    TextView mainread;
    FirebaseDatabase db = FirebaseDatabase.getInstance();
    DatabaseReference node,node2,ampsnode,powernode,voltnode,dcnode,loadnode;
    private ImageView img;
    private Switch load3SW;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_load3);

        TextView amps = (TextView) findViewById(R.id.amps);
        mainread = (TextView) findViewById(R.id.tagline);
        TextView power = (TextView) findViewById(R.id.power);
        TextView volt = (TextView) findViewById(R.id.acvolt3);
        TextView dcvolt = (TextView) findViewById(R.id.dcvolt);
        img = (ImageView) findViewById(R.id.imageView3);
        load3SW = (Switch) findViewById(R.id.switch3);

        node2 = db.getReference("Buttons");
        node = db.getReference("Data");
        ampsnode = node.child("amps3");
        powernode = node.child("pow3");
        voltnode = node.child("voltage");
        dcnode = node.child("syncVolt");
        loadnode = node.child("load3state");

        ampsnode.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(DataSnapshot dataSnapshot) {
                String value = dataSnapshot.getValue(String.class);

                amps.setText(value+" A");
            }
            @Override
            public void onCancelled(DatabaseError error) {
                Toast.makeText(Load3.this, "Connection Error", Toast.LENGTH_LONG).show();
            }
        });
        voltnode.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(DataSnapshot dataSnapshot) {
                String value = dataSnapshot.getValue(String.class);
                volt.setText(value+" V");
            }
            @Override
            public void onCancelled(DatabaseError error) {
                Toast.makeText(Load3.this, "Connection Error", Toast.LENGTH_LONG).show();
            }
        });
        powernode.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(DataSnapshot dataSnapshot) {
                String value = dataSnapshot.getValue(String.class);

                power.setText(value+" W");
            }
            @Override
            public void onCancelled(DatabaseError error) {
                Toast.makeText(Load3.this, "Connection Error", Toast.LENGTH_LONG).show();
            }
        });
        dcnode.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(DataSnapshot dataSnapshot) {
                String value = dataSnapshot.getValue(String.class);
                dcvolt.setText(value+" V");
            }
            @Override
            public void onCancelled(DatabaseError error) {
                Toast.makeText(Load3.this, "Connection Error", Toast.LENGTH_LONG).show();
            }
        });

        loadnode.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(DataSnapshot dataSnapshot) {
                String value = dataSnapshot.getValue(String.class); //load status
                if(value.equals("ON")) {
                    img.setImageResource(R.drawable.on);
//                    load1SW.setChecked(true);
                }
                else if(value.equals("OFF")){
                    img.setImageResource(R.drawable.off);
//                    load1SW.setChecked(false);
                }
            }
            @Override
            public void onCancelled(DatabaseError error) {
                Log.e("Error: ", String.valueOf(error));
            }
        });
        load3SW.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked) {
                    node2.child("btn3").setValue("ON");
                } else {
                    node2.child("btn3").setValue("OFF");
                }
            }
        });

        FirebaseDatabase.getInstance().getReference(".info/connected").addValueEventListener(new ValueEventListener() {
            public void onDataChange(DataSnapshot snapshot) {
                if (((Boolean) snapshot.getValue(Boolean.class)).booleanValue()) {
                    Load3.this.mainread.setText("INTERNET CONNECTED ");
                } else {
                    Load3.this.mainread.setText("INTERNET CONNECTION FAILED");
                }
            }

            public void onCancelled(DatabaseError error) {
            }
        });
    }

    @Override
    public void onBackPressed() {
        super.onBackPressed();
        finish();
    }
}