#import <Carbon/Carbon.h>
#import <Cocoa/Cocoa.h>
#import <IOKit/ps/IOPowerSources.h>
#import <IOKit/ps/IOPSKeys.h>

// Force-included before the actual dispatcher, every wrapper, and unchanged SDK.
@interface OwnershipURL : NSObject
- (id)initWithString:(NSString *)value;
- (NSString *)absoluteString;
@end
@interface OwnershipImage : NSObject
- (id)initWithCGImage:(CGImageRef)image size:(NSSize)size;
- (NSData *)TIFFRepresentation;
@end
@interface OwnershipCenter : NSObject
- (void)addObserver:(id)observer selector:(SEL)selector name:(NSString *)name object:(id)object;
- (void)removeObserver:(id)observer;
@end
@interface OwnershipWorkspace : NSObject
+ (OwnershipWorkspace *)sharedWorkspace;
- (OwnershipCenter *)notificationCenter;
@end
@interface OwnershipUUID : NSObject
+ (id)UUID;
- (NSString *)UUIDString;
@end
CFURLRef OwnershipCreateURL(CFAllocatorRef, CFStringRef, CFURLPathStyle, Boolean);
CFStringRef OwnershipCopyPath(CFURLRef, CFURLPathStyle);
IOPSLowBatteryWarningLevel OwnershipBatteryWarning();
CFTypeRef OwnershipPowerInfo();
CFArrayRef OwnershipPowerList(CFTypeRef);
CFDictionaryRef OwnershipPowerDescription(CFTypeRef, CFTypeRef);
CFDictionaryRef OwnershipAdapter();
CFPropertyListRef OwnershipPropertyCopy(CFAllocatorRef, CFPropertyListRef, CFOptionFlags);
CFDataRef OwnershipPropertyData(CFAllocatorRef, CFPropertyListRef, CFPropertyListFormat,
                               CFOptionFlags, CFErrorRef *);
#define NSURL OwnershipURL
#define NSImage OwnershipImage
#define NSWorkspace OwnershipWorkspace
#define NSUUID OwnershipUUID
#define NSNotificationCenter OwnershipCenter
#define NSWorkspaceWillSleepNotification @"synthetic-will-sleep"
#define NSWorkspaceDidWakeNotification @"synthetic-did-wake"
#define NSWorkspaceScreensDidSleepNotification @"synthetic-screens-sleep"
#define NSWorkspaceScreensDidWakeNotification @"synthetic-screens-wake"
#define NSWorkspaceWillPowerOffNotification @"synthetic-power-off"
#define CFURLCreateWithFileSystemPath OwnershipCreateURL
#define CFURLCopyFileSystemPath OwnershipCopyPath
#define IOPSGetBatteryWarningLevel OwnershipBatteryWarning
#define IOPSCopyPowerSourcesInfo OwnershipPowerInfo
#define IOPSCopyPowerSourcesList OwnershipPowerList
#define IOPSGetPowerSourceDescription OwnershipPowerDescription
#define IOPSCopyExternalPowerAdapterDetails OwnershipAdapter
#define CFPropertyListCreateDeepCopy OwnershipPropertyCopy
#define CFPropertyListCreateData OwnershipPropertyData
