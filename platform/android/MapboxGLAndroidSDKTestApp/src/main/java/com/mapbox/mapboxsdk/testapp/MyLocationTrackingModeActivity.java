package com.mapbox.mapboxsdk.testapp;

import android.location.Location;
import android.os.Bundle;
import android.support.annotation.DrawableRes;
import android.support.annotation.Nullable;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.Snackbar;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.ActionBar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Spinner;
import android.widget.Toast;

import com.mapbox.mapboxsdk.constants.MyBearingTracking;
import com.mapbox.mapboxsdk.constants.MyLocationTracking;
import com.mapbox.mapboxsdk.geometry.LatLng;
import com.mapbox.mapboxsdk.utils.ApiAccess;
import com.mapbox.mapboxsdk.views.MapView;

public class MyLocationTrackingModeActivity extends AppCompatActivity implements MapView.OnMyLocationChangeListener, AdapterView.OnItemSelectedListener {

    private MapView mMapView;
    private Spinner mLocationSpinner, mBearingSpinner;
    private FloatingActionButton mMarkerDisplayFAB;
    private Location mLocation;

    private boolean mMarkerDefaultDisplay = true;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_my_location_tracking);

        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        ActionBar actionBar = getSupportActionBar();
        if (actionBar != null) {
            actionBar.setDisplayShowTitleEnabled(false);
            actionBar.setDisplayHomeAsUpEnabled(true);
            actionBar.setDisplayShowHomeEnabled(true);
        }

        ArrayAdapter<CharSequence> locationTrackingAdapter = ArrayAdapter.createFromResource(actionBar.getThemedContext(), R.array.user_tracking_mode, android.R.layout.simple_spinner_item);
        locationTrackingAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mLocationSpinner = (Spinner) findViewById(R.id.spinner_location);
        mLocationSpinner.setAdapter(locationTrackingAdapter);
        mLocationSpinner.setOnItemSelectedListener(this);

        ArrayAdapter<CharSequence> bearingTrackingAdapter = ArrayAdapter.createFromResource(actionBar.getThemedContext(), R.array.user_bearing_mode, android.R.layout.simple_spinner_item);
        bearingTrackingAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mBearingSpinner = (Spinner) findViewById(R.id.spinner_bearing);
        mBearingSpinner.setAdapter(bearingTrackingAdapter);
        mBearingSpinner.setOnItemSelectedListener(this);

        mMarkerDisplayFAB = (FloatingActionButton) findViewById(R.id.my_location_tracking_marker_display);

        mMapView = (MapView) findViewById(R.id.mapView);
        mMapView.setAccessToken(ApiAccess.getToken(this));
        mMapView.onCreate(savedInstanceState);
        mMapView.setOnMyLocationChangeListener(this);

        try {
            mMapView.setMyLocationEnabled(true);
        } catch (SecurityException e) {
            //should not occur, permission was checked in MainActivity
            Toast.makeText(this, "Location permission is not availlable", Toast.LENGTH_SHORT).show();
            finish();
        }

        mMapView.setOnMyLocationTrackingModeChangeListener(new MapView.OnMyLocationTrackingModeChangeListener() {
            @Override
            public void onMyLocationTrackingModeChange(@MyLocationTracking.Mode int myLocationTrackingMode) {
                if (MyLocationTracking.TRACKING_NONE == myLocationTrackingMode) {
                    mLocationSpinner.setOnItemSelectedListener(null);
                    mLocationSpinner.setSelection(0);
                    mLocationSpinner.setOnItemSelectedListener(MyLocationTrackingModeActivity.this);
                }
                changeMarkerDisplay();
            }
        });

        mMapView.setOnMyBearingTrackingModeChangeListener(new MapView.OnMyBearingTrackingModeChangeListener() {
            @Override
            public void onMyBearingTrackingModeChange(@MyBearingTracking.Mode int myBearingTrackingMode) {
                if (MyBearingTracking.NONE == myBearingTrackingMode) {
                    mBearingSpinner.setOnItemSelectedListener(null);
                    mBearingSpinner.setSelection(0);
                    mBearingSpinner.setOnItemSelectedListener(MyLocationTrackingModeActivity.this);
                }
                changeMarkerDisplay();
            }
        });


        mMarkerDisplayFAB.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                toggleMarkerDisplay();
            }
        });
    }

    private void changeMarkerDisplay() {
        @DrawableRes int defaultId;
        @DrawableRes int bearingId;
        @DrawableRes int displayMarkerButtonId;
        int accuracyColorId;
        if (mMarkerDefaultDisplay) {
            defaultId = R.drawable.my_location;
            bearingId = R.drawable.my_location_bearing;
            accuracyColorId = R.color.my_location_ring;
            if (mMapView.getMyBearingTrackingMode() == MyBearingTracking.COMPASS) {
                displayMarkerButtonId = R.drawable.custom_location_bearing_marker;
            } else {
                displayMarkerButtonId = R.drawable.custom_location_marker;
            }
        } else {
            defaultId = R.drawable.custom_location_marker;
            bearingId = R.drawable.custom_location_bearing_marker;
            accuracyColorId = R.color.custom_location_color;
            if (mMapView.getMyBearingTrackingMode() == MyBearingTracking.COMPASS) {
                displayMarkerButtonId = R.drawable.my_location_bearing;
            } else {
                displayMarkerButtonId = R.drawable.my_location;

            }
        }

        int accuracyColor = ContextCompat.getColor(mMapView.getContext(), accuracyColorId);

        mMapView.setUserLocationDrawable(defaultId);
        mMapView.setUserLocationBearingDrawable(bearingId);
        mMapView.setUserLocationAccuracyColor(accuracyColor, accuracyColor);
        mMarkerDisplayFAB.setImageResource(displayMarkerButtonId);
    }

    private void toggleMarkerDisplay() {
        mMarkerDefaultDisplay = !mMarkerDefaultDisplay;
        changeMarkerDisplay();
    }

    @Override
    public void onMyLocationChange(@Nullable Location location) {
        if (location != null) {
            if (mLocation == null) {
                // initial location to reposition map
                mMapView.setLatLng(new LatLng(location.getLatitude(), location.getLongitude()));
                mLocationSpinner.setEnabled(true);
                mBearingSpinner.setEnabled(true);
            }
            mLocation = location;
            showSnackBar();
        }
    }

    private void showSnackBar() {
        String desc = "Loc Chg: ";
        boolean noInfo = true;
        if (mLocation.hasSpeed()) {
            desc += String.format("Spd = %.1f km/h ", mLocation.getSpeed() * 3.6f);
            noInfo = false;
        }
        if (mLocation.hasAltitude()) {
            desc += String.format("Alt = %.0f m ", mLocation.getAltitude());
            noInfo = false;
        }
        if (noInfo) {
            desc += "No extra info";
        }
        Snackbar.make(findViewById(android.R.id.content), desc, Snackbar.LENGTH_SHORT).show();
    }

    @Override
    public void onItemSelected(AdapterView<?> parent, View view, int position, long id) throws SecurityException {
        if (parent.getId() == R.id.spinner_location) {
            switch (position) {
                case 0:
                    mMapView.setMyLocationTrackingMode(MyLocationTracking.TRACKING_NONE);
                    break;

                case 1:
                    mMapView.setMyLocationTrackingMode(MyLocationTracking.TRACKING_FOLLOW);
                    break;
            }
        } else if (parent.getId() == R.id.spinner_bearing) {
            switch (position) {
                case 0:
                    mMapView.setMyBearingTrackingMode(MyBearingTracking.NONE);
                    break;

                case 1:
                    mMapView.setMyBearingTrackingMode(MyBearingTracking.GPS);
                    break;

                case 2:
                    mMapView.setMyBearingTrackingMode(MyBearingTracking.COMPASS);
                    break;
            }
        }
    }

    @Override
    public void onNothingSelected(AdapterView<?> parent) {

    }

    @Override
    protected void onStart() {
        super.onStart();
        mMapView.onStart();
    }

    @Override
    public void onResume() {
        super.onResume();
        mMapView.onResume();
    }

    @Override
    public void onPause() {
        super.onPause();
        mMapView.onPause();
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        mMapView.onSaveInstanceState(outState);
    }

    @Override
    protected void onStop() {
        super.onStop();
        mMapView.onStop();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mMapView.onDestroy();
    }

    @Override
    public void onLowMemory() {
        super.onLowMemory();
        mMapView.onLowMemory();
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home:
                onBackPressed();
                return true;
            default:
                return super.onOptionsItemSelected(item);
        }
    }

}