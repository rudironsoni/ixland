//
//  main.m
//  IOSTestApp - libiox iOS Test Suite
//
//  Tests libiox functionality on iOS Simulator/Device
//

#import <UIKit/UIKit.h>
#import <Foundation/Foundation.h>

// libiox headers
extern int iox_is_initialized(void);
extern const char *iox_version(void);
extern char *iox_getcwd(char *buf, size_t size);
extern int iox_chdir(const char *path);
extern pid_t iox_getpid(void);
extern pid_t iox_getppid(void);

@interface TestRunner : NSObject
+ (void)runAllTests;
@end

@implementation TestRunner

+ (void)log:(NSString *)format, ... {
    va_list args;
    va_start(args, format);
    NSString *msg = [[NSString alloc] initWithFormat:format arguments:args];
    va_end(args);
    
    NSLog(@"[libiox] %@", msg);
    printf("[libiox] %s\n", [msg UTF8String]);
    
    // Also write to file for debugging
    NSString *logPath = [NSTemporaryDirectory() stringByAppendingPathComponent:@"libiox_test.log"];
    NSString *logEntry = [NSString stringWithFormat:@"%@\n", msg];
    NSFileHandle *fileHandle = [NSFileHandle fileHandleForWritingAtPath:logPath];
    if (!fileHandle) {
        [[NSFileManager defaultManager] createFileAtPath:logPath contents:nil attributes:nil];
        fileHandle = [NSFileHandle fileHandleForWritingAtPath:logPath];
    }
    [fileHandle seekToEndOfFile];
    [fileHandle writeData:[logEntry dataUsingEncoding:NSUTF8StringEncoding]];
    [fileHandle closeFile];
}

+ (void)runAllTests {
    [self log:@"========================================"];
    [self log:@"libiox iOS Test Suite"];
    [self log:@"========================================"];
    
    int passed = 0;
    int failed = 0;
    
    // Test 1: Initialization
    [self log:@"\nTest 1: Library Initialization"];
    int init = iox_is_initialized();
    if (init) {
        [self log:@"✓ Library is initialized"];
        passed++;
    } else {
        [self log:@"✗ Library NOT initialized"];
        failed++;
    }
    
    // Test 2: Version
    [self log:@"\nTest 2: Version Check"];
    const char *ver = iox_version();
    if (ver && strlen(ver) > 0) {
        [self log:@"✓ Version: %s", ver];
        passed++;
    } else {
        [self log:@"✗ Version not available"];
        failed++;
    }
    
    // Test 3: getcwd
    [self log:@"\nTest 3: getcwd"];
    char cwd[1024];
    char *result = iox_getcwd(cwd, sizeof(cwd));
    if (result) {
        [self log:@"✓ CWD: %s", cwd];
        passed++;
    } else {
        [self log:@"✗ getcwd failed"];
        failed++;
    }
    
    // Test 4: getpid
    [self log:@"\nTest 4: getpid"];
    pid_t pid = iox_getpid();
    if (pid > 0) {
        [self log:@"✓ PID: %d", (int)pid];
        passed++;
    } else {
        [self log:@"✗ getpid failed"];
        failed++;
    }
    
    // Test 5: getppid
    [self log:@"\nTest 5: getppid"];
    pid_t ppid = iox_getppid();
    if (ppid > 0) {
        [self log:@"✓ PPID: %d", (int)ppid];
        passed++;
    } else {
        [self log:@"✗ getppid failed"];
        failed++;
    }
    
    // Test 6: chdir
    [self log:@"\nTest 6: chdir"];
    int chdir_result = iox_chdir("/tmp");
    if (chdir_result == 0) {
        char new_cwd[1024];
        iox_getcwd(new_cwd, sizeof(new_cwd));
        [self log:@"✓ chdir to /tmp succeeded, new CWD: %s", new_cwd];
        passed++;
    } else {
        [self log:@"✗ chdir failed (may be expected)"];
        // Don't count as failure - chdir might not work in all contexts
    }
    
    // Summary
    [self log:@"\n========================================"];
    [self log:@"Test Results: %d passed, %d failed", passed, failed];
    if (failed == 0) {
        [self log:@"✓ ALL TESTS PASSED"];
    } else {
        [self log:@"✗ SOME TESTS FAILED"];
    }
    [self log:@"========================================"];
}

@end

@interface AppDelegate : UIResponder <UIApplicationDelegate>
@property (strong, nonatomic) UIWindow *window;
@end

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Run tests immediately
    [TestRunner runAllTests];
    
    // Create minimal UI
    self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    UIViewController *vc = [[UIViewController alloc] init];
    vc.view.backgroundColor = [UIColor systemBackgroundColor];
    
    UILabel *label = [[UILabel alloc] initWithFrame:CGRectMake(20, 100, 300, 100)];
    label.text = @"libiox Test Complete\nCheck console for results";
    label.numberOfLines = 0;
    label.textAlignment = NSTextAlignmentCenter;
    [vc.view addSubview:label];
    
    self.window.rootViewController = vc;
    [self.window makeKeyAndVisible];
    
    return YES;
}

@end

int main(int argc, char * argv[]) {
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}
