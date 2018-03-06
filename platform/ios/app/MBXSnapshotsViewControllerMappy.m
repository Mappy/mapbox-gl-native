#import "MBXSnapshotsViewControllerMappy.h"

#import <Mapbox/Mapbox.h>

@interface MBXSnapshotsViewControllerMappy ()

@property (weak, nonatomic) IBOutlet UIImageView *snapshotImageViewFS;

@end

@implementation MBXSnapshotsViewControllerMappy {
    MGLMapSnapshotter* snapshotter;
	NSTimer* timer;
}

- (void)dealloc
{
}

- (void)viewDidLoad {
    [super viewDidLoad];

	snapshotter = [self startSnapshotterForImageView:_snapshotImageViewFS coordinates:CLLocationCoordinate2DMake(46.875, 1.5) completion:^(){
		NSLog(@"Snapshot done!");
		timer = [NSTimer scheduledTimerWithTimeInterval:3 repeats:NO block:^(NSTimer * _Nonnull _timer) {
			NSLog(@"Deallocating snapshoter");
			snapshotter = nil;
			timer = nil;
		}];
	}];
}

- (MGLMapSnapshotter*) startSnapshotterForImageView:(UIImageView*) imageView coordinates:(CLLocationCoordinate2D) coordinates completion:(void (^)(void))completion {
    // Create snapshot options
    MGLMapCamera* mapCamera = [[MGLMapCamera alloc] init];
    mapCamera.pitch = 0;
    mapCamera.centerCoordinate = coordinates;
    MGLMapSnapshotOptions* options = [[MGLMapSnapshotOptions alloc]
									  initWithStyleURL:[NSURL URLWithString:@"https://map.mappy.net/map/1.0/vector/standard.json"] //[MGLStyle satelliteStreetsStyleURL]
									  camera:mapCamera
									  size:CGSizeMake(imageView.frame.size.width, imageView.frame.size.height)];
    options.zoomLevel = 10;
    
    // Create and start the snapshotter
    __weak UIImageView *weakImageView = imageView;
    MGLMapSnapshotter* snapshotter = [[MGLMapSnapshotter alloc] initWithOptions:options];
    [snapshotter startWithCompletionHandler: ^(MGLMapSnapshot* snapshot, NSError *error) {
        if (error) {
            NSLog(@"Could not load snapshot: %@", [error localizedDescription]);
			completion();
        } else {
            weakImageView.image = snapshot.image;
			completion();
        }
    }];
    
    return snapshotter;
}


@end
