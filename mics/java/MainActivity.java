package com.infotex.vipnet.vipnet_client_prototype;

import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.TextView;

import com.infotex.vipnet.vipnet_client_prototype.R;

import java.io.IOException;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.util.Date;

import com.infotex.vipnet.vipnet_client_prototype.LastLoadDate;

public class MainActivity extends AppCompatActivity {

    protected void showFile() {
        TextView tw = (TextView)findViewById(R.id.textView);
        try {
            FileInputStream fi = this.openFileInput(getString(R.string.dst_filename));
            byte[] b = new byte[fi.available()];
            fi.read(b);
            Date lastLoadDate = LastLoadDate.getInstance().lastLoadDate;
            if (lastLoadDate == null)
                tw.setText(String.format("%s%n%s", getString(R.string.file_was_loaded), new String(b)));
            else
                tw.setText(String.format("%s at %Tc:%n%s", getString(R.string.file_was_loaded), LastLoadDate.getInstance().lastLoadDate, new String(b)));
        } catch (FileNotFoundException e) {
            tw.setText(getString(R.string.file_not_found));
        } catch (IOException e) {
            tw.setText(getString(R.string.file_not_found));
        }
    }

    @Override
    public void onStart() {
        super.onStart();
        showFile();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        TextView tlw = (TextView)findViewById(R.id.textViewLogfile);
        tlw.setText(String.format("%s %s/%s", getString(R.string.logheader), Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS), getString(R.string.logname)));
        showFile();
    }

    public void onClickDelete(View v) {
        this.deleteFile(getString(R.string.dst_filename));
        TextView tw = (TextView)findViewById(R.id.textView);
        showFile();
    }
}

