#include <mbgl/util/logging.hpp>
#include <mbgl/util/enum.hpp>

#import <Foundation/Foundation.h>

namespace mbgl {

void Log::platformRecord(EventSeverity severity, const std::string &msg) {
    NSString *message =
        [[NSString alloc] initWithBytes:msg.data() length:msg.size() encoding:NSUTF8StringEncoding];
    NSLog(@"[%s] %@", Enum<EventSeverity>::toString(severity), message);

#ifndef NDEBUG
	BOOL logInFile = [[NSUserDefaults standardUserDefaults] boolForKey:@"MGLMapboxMetricsDebugLoggingEnabled"];
	if (logInFile)
	{
		NSString *messageToWrite = [NSString stringWithFormat:@"%@: %@\r\n",[NSDate date], message];
		NSString *logFilePath = [[NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) firstObject] stringByAppendingPathComponent:[NSString stringWithFormat:@"mappy_log.txt"]];
		NSFileManager *fileManager = [[NSFileManager alloc] init];
		if ([fileManager fileExistsAtPath:logFilePath]) {
			NSFileHandle *fileHandle = [NSFileHandle fileHandleForWritingAtPath:logFilePath];
			[fileHandle seekToEndOfFile];
			[fileHandle writeData:[messageToWrite dataUsingEncoding:NSUTF8StringEncoding]];
		} else {
			NSLog(@"Mappy logging at %@", logFilePath);
			[fileManager createFileAtPath:logFilePath
								 contents:[messageToWrite dataUsingEncoding:NSUTF8StringEncoding]
							   attributes:@{ NSFileProtectionKey: NSFileProtectionCompleteUntilFirstUserAuthentication }];
		}
	}

#endif
}

}
