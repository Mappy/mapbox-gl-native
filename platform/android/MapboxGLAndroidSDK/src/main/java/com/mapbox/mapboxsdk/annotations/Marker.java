package com.mapbox.mapboxsdk.annotations;

import android.graphics.Bitmap;
import android.graphics.RectF;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.view.View;

import com.mapbox.mapboxsdk.R;
import com.mapbox.mapboxsdk.geometry.LatLng;
import com.mapbox.mapboxsdk.maps.MapView;
import com.mapbox.mapboxsdk.maps.MapboxMap;

/**
 * Marker is an annotation that shows an icon image at a geographical location.
 * </p>
 * An {@link InfoWindow} can be shown when a Marker is pressed
 * <p/>
 */
public class Marker extends Annotation {

    private static float sScreenDensity;

    private LatLng position;
    private String snippet;
    private Icon icon;
    private String title;
    private InfoWindow infoWindow = null;
    private boolean infoWindowShown = false;
    private int topOffsetPixels;
    private int zOrder = 0;

    @Nullable
    private RectF bounds;

    /**
     * Constructor
     */
    Marker() {
        super();
    }

    public Marker(BaseMarkerOptions baseMarkerOptions) {
        position = baseMarkerOptions.position;
        snippet = baseMarkerOptions.snippet;
        icon = baseMarkerOptions.icon;
        title = baseMarkerOptions.title;
        zOrder = baseMarkerOptions.zOrder;
    }

    public static void setScreenDensity(float screenDensity) {
        sScreenDensity = screenDensity;
    }

    public LatLng getPosition() {
        return position;
    }

    public String getSnippet() {
        return snippet;
    }

    public String getTitle() {
        return title;
    }

    /**
     * Do not use this method. Used internally by the SDK.
     */
    public void hideInfoWindow() {
        if (infoWindow != null) {
            infoWindow.close();
        }
        infoWindowShown = false;
    }

    /**
     * Do not use this method. Used internally by the SDK.
     */
    public boolean isInfoWindowShown() {
        return infoWindowShown;
    }

    /**
     * Sets the position.
     *
     * @param position new position
     */
    public void setPosition(LatLng position) {
        this.position = position;
        MapboxMap map = getMapboxMap();
        if (map != null) {
            map.updateMarker(this);
        }
    }

    void setSnippet(String snippet) {
        this.snippet = snippet;
    }

    /**
     * Sets the icon.
     *
     * @param icon The icon to be used as Marker image
     */
    public void setIcon(@Nullable Icon icon) {
        this.icon = icon;
        if (icon != null) {
            Bitmap bitmap = icon.getBitmap();
            int halfWidth = bitmap.getWidth() / 2;
            int halfHeight = bitmap.getHeight() / 2;
            bounds = new RectF(-halfWidth, -halfHeight, halfWidth, halfHeight);
            bounds.offset(icon.getOffsetX() * sScreenDensity, icon.getOffsetY() * sScreenDensity);
        } else {
            bounds = null;
        }

        MapboxMap map = getMapboxMap();
        if (map != null) {
            map.updateMarker(this);
        }
    }

    public Icon getIcon() {
        return icon;
    }

    /**
     * Get the RectF corresponding to the Icon Bitmap bounds.
     *
     * @return a copy of the precomputed RectF that can be manipulated safely or null if no Bitmap has been set.
     */
    @Nullable
    public RectF getBounds() {
        if (bounds != null) {
            return new RectF(bounds);
        }
        return null;
    }

    void setTitle(String title) {
        this.title = title;
    }

    void setZOrder(int zOrder) {
        this.zOrder = zOrder;
    }

    public int getZOrder() {
        return this.zOrder;
    }

    /**
     * Do not use this method. Used internally by the SDK.
     */
    public InfoWindow showInfoWindow(@NonNull MapboxMap mapboxMap, @NonNull MapView mapView) {
        setMapboxMap(mapboxMap);
        MapboxMap.InfoWindowAdapter infoWindowAdapter = getMapboxMap().getInfoWindowAdapter();
        if (infoWindowAdapter != null) {
            // end developer is using a custom InfoWindowAdapter
            View content = infoWindowAdapter.getInfoWindow(this);
            if (content != null) {
                infoWindow = new InfoWindow(content, mapboxMap);
                showInfoWindow(infoWindow, mapView);
                return infoWindow;
            }
        }

        InfoWindow infoWindow = getInfoWindow(mapView);
        if (mapView.getContext() != null) {
            infoWindow.adaptDefaultMarker(this, mapboxMap, mapView);
        }
        return showInfoWindow(infoWindow, mapView);
    }

    private InfoWindow showInfoWindow(InfoWindow iw, MapView mapView) {
        int offsetX = Math.round(icon.getOffsetX());
        int offsetY = Math.round(icon.getOffsetY());

        iw.open(mapView, this, getPosition(), offsetX, topOffsetPixels + offsetY);
        infoWindowShown = true;
        return iw;
    }

    private InfoWindow getInfoWindow(@NonNull MapView mapView) {
        if (infoWindow == null && mapView.getContext() != null) {
            infoWindow = new InfoWindow(mapView, R.layout.infowindow_view, getMapboxMap());
        }
        return infoWindow;
    }

    /**
     * Do not use this method. Used internally by the SDK.
     */
    public void setTopOffsetPixels(int topOffsetPixels) {
        this.topOffsetPixels = topOffsetPixels;
    }

    @Override
    public int compareTo(@NonNull Annotation annotation) {
        if (annotation instanceof Marker) {
            Marker marker = (Marker)annotation;
            // Sort by reverse z-order so that tapped marker is the upper one
            if (zOrder < marker.zOrder) {
                return 1;
            } else if (zOrder > marker.zOrder) {
                return -1;
            }
        }
        return super.compareTo(annotation);
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) {
            return true;
        }
        if (o == null || getClass() != o.getClass()) {
            return false;
        }
        if (!super.equals(o)) {
            return false;
        }

        Marker marker = (Marker) o;
        return !(getPosition() != null ? !getPosition().equals(marker.getPosition()) : marker.getPosition() != null);
    }

    @Override
    public int hashCode() {
        int result = super.hashCode();
        result = 31 * result + (getPosition() != null ? getPosition().hashCode() : 0);
        return result;
    }

    @Override
    public String toString() {
        return "Marker [position[" + getPosition() + "]]";
    }

}
