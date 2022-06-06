package com.infotex.vipnet.vipnet_client_prototype;

import android.content.ContentProvider;
import android.content.ContentValues;
import android.database.Cursor;
import android.net.Uri;
import android.content.UriMatcher;
import android.content.Context;
import android.os.Environment;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.FileOutputStream;
import java.io.FileNotFoundException;
import java.util.Calendar;
import java.util.Date;

import com.infotex.vipnet.vipnet_client_prototype.LastLoadDate;

public class DstProvider extends ContentProvider {

    static final String CONTENT_TYPE_DST = "vnd.android.cursor.item/vnd.vipnet.dst";
    static final String AUTHORITY = "localhost.vipnet";

    static final int URI_GETDST = 1;

    private static final UriMatcher uriMatcher;
    static { uriMatcher = new UriMatcher(UriMatcher.NO_MATCH);
        uriMatcher.addURI(AUTHORITY, "getdst", URI_GETDST);
    }

    public boolean onCreate() {
        return true;
    }

    public Cursor query(Uri uri, String[] projection, String selection, String[] selectionArgs, String sortOrder) {
        throw new RuntimeException("The select operation is not allowed!");
    }

    public Uri insert(Uri uri, ContentValues values) {
        switch (uriMatcher.match(uri)) {
            case URI_GETDST:
                if (!values.containsKey("dst"))
                  break;
                byte[] dst = values.getAsByteArray("dst");
                Context ctx = getContext();
                try {
                    FileOutputStream fo = ctx.openFileOutput(ctx.getString(R.string.dst_filename), ctx.MODE_PRIVATE);
                    fo.write(dst);
                    logInsert(dst);
                }
                catch (FileNotFoundException e) {
                }
                catch (IOException e) {
                }
                break;
            default:
                break;
        }
        return uri;
    }

    public int delete(Uri uri, String selection, String[] selectionArgs) {
        throw new RuntimeException("The delete operation is not allowed!");
    }

    public int update(Uri uri, ContentValues values, String selection, String[] selectionArgs) {
        throw new RuntimeException("The update operation is not allowed!");
    }

    public String getType(Uri uri) {
        return CONTENT_TYPE_DST;
    }


    public void logInsert(byte[] dst) {
        Context ctx = getContext();
        if (!Environment.MEDIA_MOUNTED.equals(Environment.getExternalStorageState()))
            return;
        File f = new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS), ctx.getString(R.string.logname));
        try {
            FileOutputStream stream = new FileOutputStream(f, true);
            Date lastLoadDate = Calendar.getInstance().getTime();
            LastLoadDate.getInstance().lastLoadDate = lastLoadDate;
            stream.write(String.format("%n [%Tc]: ", lastLoadDate).getBytes());
            stream.write(dst);
        }
        catch (FileNotFoundException e) {
        }
        catch (IOException e) {
        }
    }

}