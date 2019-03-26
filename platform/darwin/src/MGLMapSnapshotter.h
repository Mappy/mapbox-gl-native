#import <Foundation/Foundation.h>
#import "MGLTypes.h"
#import "MGLGeometry.h"
#import "MGLMapCamera.h"

#if TARGET_OS_IPHONE
	#import <UIKit/UIGeometry.h>
#endif

NS_ASSUME_NONNULL_BEGIN

/**
 The options to use when creating images with the `MGLMapSnapshotter`.
 */
MGL_EXPORT
@interface MGLMapSnapshotOptions : NSObject

/**
 Creates a set of options with the minimum required information.
 
 @param styleURL URL of the map style to snapshot. The URL may be a full HTTP or
    HTTPS URL, a Mapbox URL indicating the style’s map ID
    (`mapbox://styles/{user}/{style}`), or a path to a local file relative to
    the application’s resource path. Specify `nil` for the default style.
 @param size The image size.
 */
- (instancetype)initWithStyleURL:(nullable NSURL *)styleURL camera:(MGLMapCamera *)camera size:(CGSize)size;

#pragma mark - Configuring the Map

/**
 URL of the map style to snapshot.
 */
@property (nonatomic, readonly) NSURL *styleURL;

/**
 The zoom level.
 
 The default zoom level is 0. If this property is non-zero and the camera
 property is non-nil, the camera’s altitude is ignored in favor of this
 property’s value.
 */
@property (nonatomic) double zoomLevel;

/**
 A camera representing the viewport visible in the snapshot.
 
 If this property is non-nil and the `coordinateBounds` property is set to a
 non-empty coordinate bounds, the camera’s center coordinate and altitude are
 ignored in favor of the `coordinateBounds` property.
 */
@property (nonatomic) MGLMapCamera *camera;

/**
 The coordinate rectangle that encompasses the bounds to capture.
 
 If this property is non-empty and the camera property is non-nil, the camera’s
 center coordinate and altitude are ignored in favor of this property’s value.
 */
@property (nonatomic) MGLCoordinateBounds coordinateBounds;

/**
 The insets to apply to coordinates bounds if present
 */
@property (nonatomic) UIEdgeInsets insets;

#pragma mark - Configuring the Image

/**
 The size of the output image, measured in points.
 
 */
@property (nonatomic, readonly) CGSize size;

/**
 The scale of the output image. Defaults to the main screen scale.
 
 The minimum scale is 1.
 */
@property (nonatomic) CGFloat scale;

@end

/**
 An image generated by a snapshotter object.
 */
MGL_EXPORT
@interface MGLMapSnapshot : NSObject

#if TARGET_OS_IPHONE
/**
 Converts the specified map coordinate to a point in the coordinate space of the
 image.
 */
- (CGPoint)pointForCoordinate:(CLLocationCoordinate2D)coordinate;

/**
 Converts the specified image point to a map coordinate.
 */
- (CLLocationCoordinate2D)coordinateForPoint:(CGPoint)point;

/**
 The image of the map’s content.
 */
@property (nonatomic, readonly) UIImage *image;
#else
/**
 Converts the specified map coordinate to a point in the coordinate space of the
 image.
 */
- (NSPoint)pointForCoordinate:(CLLocationCoordinate2D)coordinate;

/**
 Converts the specified image point to a map coordinate.
 */
- (CLLocationCoordinate2D)coordinateForPoint:(NSPoint)point;


/**
 The image of the map’s content.
 */
@property (nonatomic, readonly) NSImage *image;
#endif

@end

/**
 A block to processes the result or error of a snapshot request.
 
 @param snapshot The `MGLMapSnapshot` that was generated or `nil` if an error
    occurred.
 @param error The error that occured or `nil` when successful.
 */
typedef void (^MGLMapSnapshotCompletionHandler)(MGLMapSnapshot* _Nullable snapshot, NSError* _Nullable error);

/**
 An `MGLMapSnapshotter` generates static raster images of the map. Each snapshot
 image depicts a portion of a map defined by an `MGLMapSnapshotOptions` object
 you provide. The snapshotter generates an `MGLMapSnapshot` object
 asynchronously, passing it into a completion handler once tiles and other
 resources needed for the snapshot are finished loading.
 
 You can change the snapshotter’s options at any time and reuse the snapshotter
 for multiple distinct snapshots; however, the snapshotter can only generate one
 snapshot at a time. If you need to generate multiple snapshots concurrently,
 create multiple snapshotter objects.
 
 For an interactive map, use the `MGLMapView` class. Both `MGLMapSnapshotter`
 and `MGLMapView` are compatible with offline packs managed by the
 `MGLOfflineStorage` class.
 
 From a snapshot, you can obtain an image and convert geographic coordinates to
 the image’s coordinate space in order to superimpose markers and overlays. If
 you do not need offline map functionality, you can use the `Snapshot` class in
 [MapboxStatic.swift](https://github.com/mapbox/MapboxStatic.swift/) to generate
 static map images with overlays.
 
 ### Example
 
 ```swift
 let camera = MGLMapCamera(lookingAtCenter: CLLocationCoordinate2D(latitude: 37.7184, longitude: -122.4365), altitude: 100, pitch: 20, heading: 0)
 
 let options = MGLMapSnapshotOptions(styleURL: MGLStyle.satelliteStreetsStyleURL, camera: camera, size: CGSize(width: 320, height: 480))
 options.zoomLevel = 10
 
 let snapshotter = MGLMapSnapshotter(options: options)
 snapshotter.start { (snapshot, error) in
     if let error = error {
         fatalError(error.localizedDescription)
     }
     
     image = snapshot?.image
 }
 ```
 
 #### Related examples
 See the <a href="https://docs.mapbox.com/ios/maps/examples/map-snapshotter/">
 Create a static map snapshot</a> example to learn how to use the
 `MGLMapSnapshotter` to generate a static image based on an `MGLMapView`
 object's style, camera, and view bounds.
 */
MGL_EXPORT
@interface MGLMapSnapshotter : NSObject

- (instancetype)init NS_UNAVAILABLE;

/**
 Initializes and returns a map snapshotter object that produces snapshots
 according to the given options.
 
 @param options The options to use when generating a map snapshot.
 @return An initialized map snapshotter.
 */
- (instancetype)initWithOptions:(MGLMapSnapshotOptions *)options NS_DESIGNATED_INITIALIZER;

/**
 Starts the snapshot creation and executes the specified block with the result.
 
 @param completionHandler The block to handle the result in.
 */
- (void)startWithCompletionHandler:(MGLMapSnapshotCompletionHandler)completionHandler;

/**
 Starts the snapshot creation and executes the specified block with the result
 on the specified queue.
 
 @param queue The queue to handle the result on.
 @param completionHandler The block to handle the result in.
 */
- (void)startWithQueue:(dispatch_queue_t)queue completionHandler:(MGLMapSnapshotCompletionHandler)completionHandler;

/**
 Cancels the snapshot creation request, if any.
 
 Once you call this method, you cannot resume the snapshot. In order to obtain
 the snapshot, create a new `MGLMapSnapshotter` object.
 */
- (void)cancel;

/**
 The options to use when generating a map snapshot.
 */
@property (nonatomic) MGLMapSnapshotOptions *options;

/**
 Indicates whether a snapshot is currently being generated.
 */
@property (nonatomic, readonly, getter=isLoading) BOOL loading;

@end

NS_ASSUME_NONNULL_END
