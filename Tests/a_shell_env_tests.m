// a_shell_env_tests.m
// Unit tests for environment variable management

#import <XCTest/XCTest.h>
#import "a_shell_system/a_shell_system.h"

@interface AShellEnvTests : XCTestCase
@end

@implementation AShellEnvTests

- (void)setUp {
    [super setUp];
    ashell_env_initialize();
}

- (void)tearDown {
    [super tearDown];
}

// Test environment initialization
- (void)testEnvironmentInitialization {
    XCTAssertTrue(ashell_env_is_initialized(), @"Environment should be initialized");
}

// Test PATH manipulation - append
- (void)testPathAppend {
    const char* testDir = "/usr/local/test/bin";
    
    int result = ashell_env_path_append(testDir);
    XCTAssertEqual(result, 0, @"Appending to PATH should succeed");
    
    // Get PATH and verify it contains our directory
    const char* path = ios_getenv("PATH");
    XCTAssertNotNil(path, @"PATH should exist");
    XCTAssertTrue(strstr(path, testDir) != NULL, @"PATH should contain appended directory");
}

// Test PATH manipulation - prepend
- (void)testPathPrepend {
    const char* testDir = "/priority/bin";
    
    int result = ashell_env_path_prepend(testDir);
    XCTAssertEqual(result, 0, @"Prepending to PATH should succeed");
    
    const char* path = ios_getenv("PATH");
    XCTAssertNotNil(path, @"PATH should exist");
    // Should be at the beginning
    XCTAssertTrue(strstr(path, testDir) != NULL, @"PATH should contain prepended directory");
}

// Test PATH manipulation - remove
- (void)testPathRemove {
    const char* testDir = "/tmp/remove_test";
    
    // First add it
    ashell_env_path_append(testDir);
    
    // Then remove it
    int result = ashell_env_path_remove(testDir);
    XCTAssertEqual(result, 0, @"Removing from PATH should succeed");
    
    const char* path = ios_getenv("PATH");
    XCTAssertTrue(strstr(path, testDir) == NULL, @"PATH should not contain removed directory");
}

// Test environment variable set and get
- (void)testEnvironmentVariableSetGet {
    const char* result = ios_setenv("TEST_VAR", "test_value", 1);
    // ios_setenv returns pointer to value on success
    XCTAssertNotNil(result, @"Setting environment variable should succeed");
    
    const char* value = ios_getenv("TEST_VAR");
    XCTAssertNotNil(value, @"Should be able to get TEST_VAR");
    XCTAssertEqualObjects(@(value), @"test_value", @"Value should match");
}

// Test environment variable unset
- (void)testEnvironmentVariableUnset {
    ios_setenv("UNSET_ME", "value", 1);
    
    int result = ios_unsetenv("UNSET_ME");
    XCTAssertEqual(result, 0, @"Unsetting should succeed");
    
    const char* value = ios_getenv("UNSET_ME");
    XCTAssertNil(value, @"Unset variable should be NULL");
}

// Test HOME directory
- (void)testHomeDirectory {
    const char* home = ios_getenv("HOME");
    XCTAssertNotNil(home, @"HOME should be set");
    XCTAssertTrue(strlen(home) > 0, @"HOME should not be empty");
}

// Test PATH array operations
- (void)testPathArrayOperations {
    int count = 0;
    char** paths = ashell_env_get_path_array(&count);
    
    XCTAssertNotNil(paths, @"Should get path array");
    XCTAssertGreaterThan(count, 0, @"Should have at least one path entry");
    
    // Free the array
    ashell_env_free_path_array(paths);
}

// Test environment print (should not crash)
- (void)testEnvironmentPrint {
    XCTAssertNoThrow(ashell_env_print(), @"Printing environment should not crash");
}

// Test PATH array with modifications
- (void)testPathArrayAfterModification {
    const char* newPath = "/test/array/path";
    ashell_env_path_append(newPath);
    
    int count = 0;
    char** paths = ashell_env_get_path_array(&count);
    
    BOOL found = NO;
    for (int i = 0; i < count; i++) {
        if (strcmp(paths[i], newPath) == 0) {
            found = YES;
            break;
        }
    }
    
    XCTAssertTrue(found, @"Path array should contain new path");
    ashell_env_free_path_array(paths);
}

@end
