//
//  IOXEnvironmentTests.mm
//  Environment syscall tests - 5 syscalls
//

#import <XCTest/XCTest.h>
#import "IOXTestHelpers.h"

@interface IOXEnvironmentTests : XCTestCase
@property (nonatomic, strong) IOXTestEnvironment *env;
@end

@implementation IOXEnvironmentTests

- (void)setUp {
    [super setUp];
    self.env = [[IOXTestEnvironment alloc] init];
    [IOXTestHelpers resetLibraryState];
}

- (void)tearDown {
    [self.env cleanup];
    self.env = nil;
    [super tearDown];
}

#pragma mark - getenv (1 syscall)

- (void)testGetenv_ExistingVariable_ReturnsValue {
    // Set a variable first
    setenv("IOX_TEST_VAR", "test_value", 1);
    
    char *value = getenv("IOX_TEST_VAR");
    XCTAssertNotNil([NSString stringWithUTF8String:value]);
    XCTAssertEqual(strcmp(value, "test_value"), 0);
    
    unsetenv("IOX_TEST_VAR");
}

- (void)testGetenv_Nonexistent_ReturnsNull {
    char *value = getenv("IOX_NONEXISTENT_VAR_12345");
    XCTAssertEqual(value, NULL);
}

- (void)testGetenv_Path_ReturnsValue {
    char *path = getenv("PATH");
    XCTAssertNotEqual(path, NULL);
    XCTAssertGreaterThan(strlen(path), 0);
}

#pragma mark - setenv (1 syscall)

- (void)testSetenv_CreatesNewVariable {
    int result = setenv("IOX_SETENV_TEST", "new_value", 0);
    XCTAssertEqual(result, 0);
    
    char *value = getenv("IOX_SETENV_TEST");
    XCTAssertEqual(strcmp(value, "new_value"), 0);
    
    unsetenv("IOX_SETENV_TEST");
}

- (void)testSetenv_OverwritesExisting {
    setenv("IOX_OVERWRITE_TEST", "original", 1);
    
    int result = setenv("IOX_OVERWRITE_TEST", "new_value", 1);
    XCTAssertEqual(result, 0);
    
    char *value = getenv("IOX_OVERWRITE_TEST");
    XCTAssertEqual(strcmp(value, "new_value"), 0);
    
    unsetenv("IOX_OVERWRITE_TEST");
}

- (void)testSetenv_NoOverwrite_KeepsOriginal {
    setenv("IOX_NOOVERWRITE_TEST", "original", 1);
    
    int result = setenv("IOX_NOOVERWRITE_TEST", "new_value", 0);
    XCTAssertEqual(result, 0);
    
    char *value = getenv("IOX_NOOVERWRITE_TEST");
    XCTAssertEqual(strcmp(value, "original"));
    
    unsetenv("IOX_NOOVERWRITE_TEST");
}

- (void)testSetenv_EmptyValue_SetsEmpty {
    int result = setenv("IOX_EMPTY_TEST", "", 1);
    XCTAssertEqual(result, 0);
    
    char *value = getenv("IOX_EMPTY_TEST");
    XCTAssertEqual(strlen(value), 0);
    
    unsetenv("IOX_EMPTY_TEST");
}

#pragma mark - unsetenv (1 syscall)

- (void)testUnsetenv_RemovesVariable {
    setenv("IOX_UNSET_TEST", "value", 1);
    XCTAssertNotEqual(getenv("IOX_UNSET_TEST"), NULL);
    
    int result = unsetenv("IOX_UNSET_TEST");
    XCTAssertEqual(result, 0);
    
    XCTAssertEqual(getenv("IOX_UNSET_TEST"), NULL);
}

- (void)testUnsetenv_Nonexistent_NoError {
    int result = unsetenv("IOX_NONEXISTENT_UNSET");
    XCTAssertEqual(result, 0);
}

- (void)testUnsetenv_AfterGetenv {
    setenv("IOX_GET_THEN_UNSET", "temp", 1);
    char *value = getenv("IOX_GET_THEN_UNSET");
    XCTAssertEqual(strcmp(value, "temp"), 0);
    
    unsetenv("IOX_GET_THEN_UNSET");
    XCTAssertEqual(getenv("IOX_GET_THEN_UNSET"), NULL);
}

#pragma mark - clearenv (1 syscall)

- (void)testClearenv_RemovesAllVariables {
    // Set some test variables
    setenv("IOX_CLEAR_1", "value1", 1);
    setenv("IOX_CLEAR_2", "value2", 1);
    
    XCTAssertNotEqual(getenv("IOX_CLEAR_1"), NULL);
    XCTAssertNotEqual(getenv("IOX_CLEAR_2"), NULL);
    
    int result = clearenv();
    XCTAssertEqual(result, 0);
    
    // Variables should be gone
    XCTAssertEqual(getenv("IOX_CLEAR_1"), NULL);
    XCTAssertEqual(getenv("IOX_CLEAR_2"), NULL);
    
    // PATH might also be gone or set to minimal
    // Note: clearenv behavior can vary by system
}

- (void)testClearenv_CanSetNewVariablesAfter {
    clearenv();
    
    int result = setenv("IOX_AFTER_CLEAR", "works", 1);
    XCTAssertEqual(result, 0);
    
    char *value = getenv("IOX_AFTER_CLEAR");
    XCTAssertEqual(strcmp(value, "works"), 0);
    
    unsetenv("IOX_AFTER_CLEAR");
}

#pragma mark - Integration Tests

- (void)testEnvironmentVariablePersistence {
    // Set a variable
    setenv("IOX_PERSIST_TEST", "persistent", 1);
    
    // Get it multiple times
    for (int i = 0; i < 100; i++) {
        char *value = getenv("IOX_PERSIST_TEST");
        XCTAssertEqual(strcmp(value, "persistent"), 0);
    }
    
    unsetenv("IOX_PERSIST_TEST");
}

- (void)testEnvironmentStress_100Variables {
    NSUInteger baseline = [IOXMemoryTracker currentMemoryUsage];
    
    // Create 100 environment variables
    for (int i = 0; i < 100; i++) {
        NSString *name = [NSString stringWithFormat:@"IOX_STRESS_%d", i];
        NSString *value = [NSString stringWithFormat:@"value_%d_data", i];
        setenv([name UTF8String], [value UTF8String], 1);
    }
    
    // Verify all exist
    for (int i = 0; i < 100; i++) {
        NSString *name = [NSString stringWithFormat:@"IOX_STRESS_%d", i];
        char *value = getenv([name UTF8String]);
        XCTAssertNotEqual(value, NULL);
    }
    
    // Clean up
    for (int i = 0; i < 100; i++) {
        NSString *name = [NSString stringWithFormat:@"IOX_STRESS_%d", i];
        unsetenv([name UTF8String]);
    }
    
    // Check memory
    IOXAssertNoMemoryLeak(baseline, 4096);
}

@end
