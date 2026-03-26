//
//  IOXTestHelpers.mm
//  IOXTestSuite
//
//  Test utilities implementation
//

#import "IOXTestHelpers.h"
#import <sys/stat.h>
#import <unistd.h>

// C function declarations for library reset
extern "C" {
    void iox_library_deinit(void);
    void iox_library_init(void);
    extern _Atomic(int) iox_initialized;
}

#pragma mark - IOXMemoryTracker

@implementation IOXMemoryTracker

+ (NSUInteger)currentResidentSize {
    struct task_basic_info info;
    mach_msg_type_number_t size = TASK_BASIC_INFO_COUNT;
    kern_return_t kerr = task_info(mach_task_self(),
                                   TASK_BASIC_INFO,
                                   (task_info_t)&info,
                                   &size);
    if (kerr != KERN_SUCCESS) {
        return 0;
    }
    return info.resident_size;
}

+ (NSUInteger)currentVirtualSize {
    struct task_basic_info info;
    mach_msg_type_number_t size = TASK_BASIC_INFO_COUNT;
    kern_return_t kerr = task_info(mach_task_self(),
                                   TASK_BASIC_INFO,
                                   (task_info_t)&info,
                                   &size);
    if (kerr != KERN_SUCCESS) {
        return 0;
    }
    return info.virtual_size;
}

+ (NSDictionary *)memoryReport {
    return @{
        @"resident": @([self currentResidentSize]),
        @"virtual": @([self currentVirtualSize]),
        @"resident_mb": @([self currentResidentSize] / (1024.0 * 1024.0))
    };
}

@end

#pragma mark - IOXTestEnvironment

@interface IOXTestEnvironment ()
@property (readwrite, nonatomic) NSString *testDirectory;
@property (readwrite, nonatomic) NSString *tempDirectory;
@end

@implementation IOXTestEnvironment

+ (instancetype)sharedEnvironment {
    static IOXTestEnvironment *shared = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        shared = [[self alloc] init];
    });
    return shared;
}

- (instancetype)init {
    self = [super init];
    if (self) {
        self.tempDirectory = NSTemporaryDirectory();
        self.testDirectory = [self.tempDirectory stringByAppendingPathComponent:@"iox_tests"];
    }
    return self;
}

- (void)setup {
    // Ensure test directory exists
    NSFileManager *fm = [NSFileManager defaultManager];
    NSError *error = nil;
    
    // Clean up any existing test directory
    [self cleanup];
    
    // Create fresh test directory
    [fm createDirectoryAtPath:self.testDirectory
  withIntermediateDirectories:YES
                   attributes:nil
                        error:&error];
    
    if (error) {
        NSLog(@"Failed to create test directory: %@", error);
    }
}

- (void)cleanup {
    NSFileManager *fm = [NSFileManager defaultManager];
    NSError *error = nil;
    
    if ([fm fileExistsAtPath:self.testDirectory]) {
        [fm removeItemAtPath:self.testDirectory error:&error];
        if (error) {
            NSLog(@"Failed to cleanup test directory: %@", error);
        }
    }
}

- (NSString *)pathForFile:(NSString *)filename {
    return [self.testDirectory stringByAppendingPathComponent:filename];
}

- (void)resetLibraryState {
    // Deinitialize library
    iox_library_deinit();
    
    // Reset initialization flag
    atomic_store(&iox_initialized, 0);
    
    // Reinitialize
    iox_library_init();
}

@end

#pragma mark - IOXPerformanceTimer

@interface IOXPerformanceTimer ()
@property (readwrite, nonatomic) NSTimeInterval elapsedTime;
@property (readwrite, nonatomic) NSUInteger iterations;
@property (nonatomic) NSDate *startTime;
@end

@implementation IOXPerformanceTimer

- (void)start {
    self.startTime = [NSDate date];
}

- (void)stop {
    if (self.startTime) {
        self.elapsedTime = [[NSDate date] timeIntervalSinceDate:self.startTime];
    }
}

- (void)measureBlock:(void (^)(void))block iterations:(NSUInteger)count {
    self.iterations = count;
    
    // Warmup
    block();
    
    // Measure
    [self start];
    for (NSUInteger i = 0; i < count; i++) {
        block();
    }
    [self stop];
}

- (NSString *)report {
    if (self.iterations == 0) {
        return @"No iterations performed";
    }
    
    double perIteration = self.elapsedTime / self.iterations * 1000.0; // ms
    return [NSString stringWithFormat:@"%.0f iterations in %.3f sec (%.3f ms/op)",
            (double)self.iterations, self.elapsedTime, perIteration];
}

@end
