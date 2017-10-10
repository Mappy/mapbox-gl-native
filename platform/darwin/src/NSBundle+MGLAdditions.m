#import "NSBundle+MGLAdditions.h"

#import "MGLAccountManager.h"

@implementation NSBundle (MGLAdditions)

+ (instancetype)mgl_frameworkBundle {
    NSBundle *bundle = [self bundleForClass:[MGLAccountManager class]];

    if (![bundle.infoDictionary[@"CFBundlePackageType"] isEqualToString:@"FMWK"]) {
        // For static frameworks, the bundle is the containing application
        // bundle but the resources are in  app bundle.
        bundle = [NSBundle mainBundle];
    }

    return bundle;
}

+ (nullable NSString *)mgl_frameworkBundleIdentifier {
    return self.mgl_frameworkInfoDictionary[@"CFBundleIdentifier"];
}

+ (nullable NS_DICTIONARY_OF(NSString *, id) *)mgl_frameworkInfoDictionary {
    NSBundle *bundle = self.mgl_frameworkBundle;
    return bundle.infoDictionary;
}

+ (nullable NSString *)mgl_applicationBundleIdentifier {
    NSString *bundleIdentifier = [NSBundle mainBundle].bundleIdentifier;
    if (!bundleIdentifier) {
        // There’s no main bundle identifier when running in a unit test bundle.
        bundleIdentifier = [NSBundle bundleForClass:[MGLAccountManager class]].bundleIdentifier;
    }
    return bundleIdentifier;
}

@end
