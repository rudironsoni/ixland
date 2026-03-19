// a_shell_cmd_tests.m
// Unit tests for command management

#import <XCTest/XCTest.h>
#import "a_shell_system/a_shell_system.h"

@interface AShellCmdTests : XCTestCase
@end

@implementation AShellCmdTests

- (void)setUp {
    [super setUp];
    ashell_env_initialize();
}

- (void)tearDown {
    [super tearDown];
}

// Test command registration
- (void)testCommandRegistration {
    const char* cmdName = "test_cmd";
    
    int result = ashell_register_command(cmdName, ASHELL_CMD_MAIN, "test", "", "");
    XCTAssertEqual(result, 0, @"Command registration should succeed");
    
    // Verify command exists
    bool exists = ashell_command_exists(cmdName);
    XCTAssertTrue(exists, @"Command should exist after registration");
    
    // Cleanup
    ashell_unregister_command(cmdName);
}

// Test command lookup
- (void)testCommandLookup {
    const char* cmdName = "lookup_test";
    ashell_register_command(cmdName, ASHELL_CMD_MAIN, "test", "", "");
    
    bool exists = ashell_command_exists(cmdName);
    XCTAssertTrue(exists, @"Should find registered command");
    
    // Look up non-existent
    bool notFound = ashell_command_exists("does_not_exist");
    XCTAssertFalse(notFound, @"Should not find non-existent command");
    
    ashell_unregister_command(cmdName);
}

// Test command unregistration
- (void)testCommandUnregistration {
    const char* cmdName = "removable";
    ashell_register_command(cmdName, ASHELL_CMD_MAIN, "test", "", "");
    
    int result = ashell_unregister_command(cmdName);
    XCTAssertEqual(result, 0, @"Command unregistration should succeed");
    
    bool exists = ashell_command_exists(cmdName);
    XCTAssertFalse(exists, @"Unregistered command should not exist");
}

// Test command info retrieval
- (void)testCommandInfo {
    const char* cmdName = "info_test";
    ashell_register_command(cmdName, ASHELL_CMD_MAIN, "test", "abc:", "file");
    
    ashell_command_info_t info;
    memset(&info, 0, sizeof(info));
    
    int result = ashell_get_command_info(cmdName, &info);
    XCTAssertEqual(result, 0, @"Getting command info should succeed");
    
    ashell_unregister_command(cmdName);
}

// Test duplicate registration
- (void)testDuplicateRegistration {
    const char* cmdName = "duplicate";
    
    // Register first time
    int result = ashell_register_command(cmdName, ASHELL_CMD_MAIN, "test1", "", "");
    XCTAssertEqual(result, 0, @"First registration should succeed");
    
    // Register again should succeed (replace)
    result = ashell_register_command(cmdName, ASHELL_CMD_MAIN, "test2", "", "");
    XCTAssertEqual(result, 0, @"Re-registration should succeed");
    
    ashell_unregister_command(cmdName);
}

// Test command listing
- (void)testCommandListing {
    // Register several commands
    ashell_register_command("cmd_a", ASHELL_CMD_MAIN, "test", "", "");
    ashell_register_command("cmd_b", ASHELL_CMD_MAIN, "test", "", "");
    ashell_register_command("cmd_c", ASHELL_CMD_MAIN, "test", "", "");
    
    // Get list of commands
    char* names[100];
    int count = ashell_list_commands(names, 100);
    
    XCTAssertGreaterThan(count, 0, @"Should have at least one command");
    
    // Cleanup
    ashell_unregister_command("cmd_a");
    ashell_unregister_command("cmd_b");
    ashell_unregister_command("cmd_c");
}

// Test commands as string
- (void)testCommandsAsString {
    NSString* commands = commandsAsString();
    XCTAssertNotNil(commands, @"Should get commands as string");
    XCTAssertTrue(commands.length > 0, @"Should have at least one command");
}

// Test commands as array
- (void)testCommandsAsArray {
    NSArray* commands = commandsAsArray();
    XCTAssertNotNil(commands, @"Should get commands as array");
    XCTAssertGreaterThan(commands.count, 0, @"Should have at least one command");
}

// Test getopt string retrieval
- (void)testGetoptString {
    const char* cmdName = "getopt_test";
    ashell_register_command(cmdName, ASHELL_CMD_MAIN, "test", "abc:", "file");
    
    NSString* opts = getoptString(@(cmdName));
    XCTAssertNotNil(opts, @"Should get getopt string");
    
    ashell_unregister_command(cmdName);
}

// Test operates on retrieval
- (void)testOperatesOn {
    const char* cmdName = "operates_test";
    ashell_register_command(cmdName, ASHELL_CMD_MAIN, "test", "", "file");
    
    NSString* operates = operatesOn(@(cmdName));
    XCTAssertNotNil(operates, @"Should get operates on string");
    
    ashell_unregister_command(cmdName);
}

// Test replace command
- (void)testReplaceCommand {
    const char* original = "orig_cmd";
    const char* replacement = "repl_cmd";
    
    ashell_register_command(original, ASHELL_CMD_MAIN, "test", "", "");
    ashell_register_command(replacement, ASHELL_CMD_MAIN, "test", "", "");
    
    // Replace original with replacement
    ashell_replace_command(original, replacement, false);
    
    ashell_unregister_command(original);
    ashell_unregister_command(replacement);
}

// Test command execution (basic)
- (void)testCommandExecution {
    // Test with a simple built-in command
    char* args[] = {"pwd"};
    int result = ashell_execute_command("pwd", 1, args);
    XCTAssertEqual(result, 0, @"pwd should execute successfully");
}

// Test command with arguments
- (void)testCommandWithArguments {
    // Test echo
    char* args[] = {"echo", "hello", "world"};
    int result = ashell_execute_command("echo", 3, args);
    XCTAssertEqual(result, 0, @"echo should execute successfully");
}

@end
