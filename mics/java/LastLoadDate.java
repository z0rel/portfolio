package com.infotex.vipnet.vipnet_client_prototype;

import java.util.Date;

public class LastLoadDate {
    private static LastLoadDate mInstance= null;

    public Date lastLoadDate;

    protected LastLoadDate() {}

    public static synchronized LastLoadDate getInstance(){
        if(null == mInstance){
            mInstance = new LastLoadDate();
        }
        return mInstance;
    }
}
