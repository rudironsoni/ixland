//
//  main.m
//  PackageTestApp
//
//  Test runner for iOS cross-compiled packages
//

#import <Foundation/Foundation.h>
#import <dlfcn.h>

// Test function prototypes
extern int test_libz_compression(void);
extern int test_libz_decompression(void);
extern int test_libz_edge_cases(void);

// Test result structure
@interface TestResult : NSObject
@property (nonatomic, strong) NSString *name;
@property (nonatomic, assign) BOOL passed;
@property (nonatomic, strong) NSString *errorMessage;
@end

@implementation TestResult
@end

// Test runner
@interface TestRunner : NSObject
@property (nonatomic, strong) NSMutableArray<TestResult *> *results;
- (void)runTest:(NSString *)name testBlock:(int (^)(void))testBlock;
- (void)printResults;
- (int)exitCode;
@end

@implementation TestRunner

- (instancetype)init {
    self = [super init];
    if (self) {
        _results = [NSMutableArray array];
    }
    return self;
}

- (void)runTest:(NSString *)name testBlock:(int (^)(void))testBlock {
    TestResult *result = [[TestResult alloc] init];
    result.name = name;
    
    int ret = testBlock();
    if (ret == 0) {
        result.passed = YES;
        NSLog(@"✓ %@ PASSED", name);
    } else {
        result.passed = NO;
        result.errorMessage = [NSString stringWithFormat:@"Exit code: %d", ret];
        NSLog(@"✗ %@ FAILED: %@", name, result.errorMessage);
    }
    
    [self.results addObject:result];
}

- (void)printResults {
    NSLog(@"\n========================================");
    NSLog(@"Test Results Summary");
    NSLog(@"========================================");
    
    int passed = 0;
    int failed = 0;
    
    for (TestResult *result in self.results) {
        if (result.passed) {
            passed++;
        } else {
            failed++;
        }
    }
    
    NSLog(@"Total: %lu", (unsigned long)self.results.count);
    NSLog(@"Passed: %d", passed);
    NSLog(@"Failed: %d", failed);
    
    if (failed > 0) {
        NSLog(@"\nFailed tests:");
        for (TestResult *result in self.results) {
            if (!result.passed) {
                NSLog(@"  - %@: %@", result.name, result.errorMessage);
            }
        }
    }
    
    NSLog(@"========================================\n");
}

- (int)exitCode {
    for (TestResult *result in self.results) {
        if (!result.passed) {
            return 1;
        }
    }
    return 0;
}

@end

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        NSLog(@"========================================");
        NSLog(@"iOS Package Test Framework");
        NSLog(@"========================================\n");
        
        TestRunner *runner = [[TestRunner alloc] init];
        
        // Run libz tests
        NSLog(@"Running libz tests...\n");
        
        [runner runTest:@"libz_compression" testBlock:^int {
            return test_libz_compression();
        }];
        
        [runner runTest:@"libz_decompression" testBlock:^int {
            return test_libz_decompression();
        }];
        
        [runner runTest:@"libz_edge_cases" testBlock:^int {
            return test_libz_edge_cases();
        }];
        
        [runner printResults];
        
        return [runner exitCode];
    }
}
