#import <Foundation/Foundation.h>
#import <CoreLocation/CoreLocation.h>
#import <TargetConditionals.h>

#import "MGLTypes.h"

NS_ASSUME_NONNULL_BEGIN

/**
 The `MGLAnnotation` protocol is used to provide annotation-related information to a map view. To use this protocol, you adopt it in any custom objects that store or represent annotation data. Each object then serves as the source of information about a single map annotation and provides critical information, such as the annotation’s location on the map. Annotation objects do not provide the visual representation of the annotation but typically coordinate (in conjunction with the map view’s delegate) the creation of an appropriate objects to handle the display.
 
 An object that adopts this protocol must implement the `coordinate` property. The other methods of this protocol are optional.
 */
@protocol MGLAnnotation <NSObject>

#pragma mark Position Attributes

/** The center point (specified as a map coordinate) of the annotation. (required) (read-only) */
@property (nonatomic, readonly) CLLocationCoordinate2D coordinate;

@optional

/** The value indicating the z-order of the annotation. (optional) (read-only)
 Must be contained between 1 and UINT32_MAX.
 A value of 0 is considered as not set ; it is the default value.
 A value superior to UINT32_MAX will lead to an undefined behaviour.
 The greater the value, the most forefront the annotation will stand ; i.e. an annitation with a zOrder of UINT32_MAX will be displayed above any other annotation.
 If 2 annotations have the same zOrder, the default rule will apply (i.e. lower latitude above greater latitude).
 */
@property (nonatomic, readonly) NSUInteger zOrder;

#pragma mark Title Attributes

/**
 The string containing the annotation’s title.
 
 Although this property is optional, if you support the selection of annotations in your map view, you are expected to provide this property. This string is displayed in the callout for the associated annotation.
 */
@property (nonatomic, readonly, copy, nullable) NSString *title;

/**
 The string containing the annotation’s subtitle.
 
 This string is displayed in the callout for the associated annotation.
 */
@property (nonatomic, readonly, copy, nullable) NSString *subtitle;

#if !TARGET_OS_IPHONE

/** The string containing the annotation’s tooltip. */
@property (nonatomic, readonly, copy, nullable) NSString *toolTip;

#endif

@end

NS_ASSUME_NONNULL_END
