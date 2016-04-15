package com.mapbox.mapboxsdk.location;

import android.Manifest;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.location.Location;
import android.location.LocationManager;
import android.support.annotation.NonNull;
import android.support.v4.content.ContextCompat;
import android.util.Log;

import com.mapbox.mapboxsdk.telemetry.TelemetryLocationReceiver;
import com.mapzen.android.lost.api.LocationRequest;
import com.mapzen.android.lost.api.LostApiClient;

import java.util.ArrayList;
import java.util.List;

public class ExternalLocationServices extends LocationServices implements ExternalLocationListener{

    private static final String TAG = "ExternalLocServices";

    /**
     * Private constructor for singleton LocationServices
     */
    private ExternalLocationServices(Context context) {
        super(context);
    }

    /**
     * Primary (singleton) access method for LocationServices
     *
     * @param context Context
     * @return LocationServices
     */
    public static void initExternalLocationServices(@NonNull final Context context) {
        if(instance == null || !(instance instanceof ExternalLocationServices)) {
            instance = new ExternalLocationServices(context.getApplicationContext());
        }
    }

    /**
     * Primary (singleton) access method for LocationServices
     *
     * @param context Context
     * @return LocationServices
     */
    public static LocationServices getLocationServices(@NonNull final Context context) {
        if (instance == null) {
            initExternalLocationServices(context);
        }
        return instance;
    }

    @Override
    public void toggleGPS(boolean enableGPS) {
        if ((ContextCompat.checkSelfPermission(context, Manifest.permission.ACCESS_COARSE_LOCATION) != PackageManager.PERMISSION_GRANTED) &&
                (ContextCompat.checkSelfPermission(context, Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED)) {
            Log.w(TAG, "Location Permissions Not Granted Yet.  Try again after requesting.");
            isGPSEnabled = false;
            return;
        }

        isGPSEnabled = enableGPS;
    }

    @Override
    public void onLocationChanged(Location location) {
        Log.i(TAG, "!!!! onLocationChanged()..." + location + ", GPS enabled ?" + isGPSEnabled);
        if (isGPSEnabled) {
            super.onLocationChanged(location);
        }
    }

    public static void releaseExternalLocationServices() {
        instance = null;
    }
}
