// KernelTests.m
// Unit tests for a-shell-kernel syscall implementations

#import <XCTest/XCTest.h>
#import "a_shell_system.h"

@interface KernelTests : XCTestCase
@end

@implementation KernelTests

- (void)setUp {
    [super setUp];
    // Initialize kernel before each test
}

- (void)tearDown {
    // Cleanup after each test
    [super tearDown];
}

// MARK: - Process Tests

- (void)testForkReturnsENOSYS {
    // fork() should return ENOSYS on iOS (not implemented)
    pid_t result = a_shell_fork();
    XCTAssertEqual(result, -1, "fork should return -1");
    XCTAssertEqual(errno, ENOSYS, "fork should set errno to ENOSYS");
}

- (void)testGetpidReturnsValidPid {
    pid_t pid = getpid();
    XCTAssertGreaterThan(pid, 0, "getpid should return positive PID");
}

- (void)testGetppidReturnsValidPid {
    pid_t ppid = getppid();
    XCTAssertGreaterThan(ppid, 0, "getppid should return positive PPID");
}

// MARK: - Signal Tests

- (void)testSignalInstallation {
    sighandler_t oldHandler = signal(SIGUSR1, SIG_IGN);
    XCTAssertNotEqual(oldHandler, SIG_ERR, "signal should not return SIG_ERR");
    
    // Restore original handler
    signal(SIGUSR1, oldHandler);
}

- (void)testKillInvalidPid {
    int result = kill(-1, SIGTERM);
    XCTAssertEqual(result, -1, "kill with invalid pid should return -1");
    XCTAssertEqual(errno, ESRCH, "kill should set errno to ESRCH");
}

// MARK: - File I/O Tests

- (void)testOpenClose {
    const char *testFile = "/tmp/test_open_close.txt";
    int fd = open(testFile, O_CREAT | O_RDWR, 0644);
    XCTAssertGreaterThanOrEqual(fd, 0, "open should return valid fd");
    
    int result = close(fd);
    XCTAssertEqual(result, 0, "close should return 0");
    
    unlink(testFile);
}

- (void)testReadWrite {
    const char *testFile = "/tmp/test_read_write.txt";
    const char *testData = "Hello, a-Shell!";
    char buffer[256];
    
    int fd = open(testFile, O_CREAT | O_RDWR, 0644);
    XCTAssertGreaterThanOrEqual(fd, 0, "open should succeed");
    
    ssize_t written = write(fd, testData, strlen(testData));
    XCTAssertEqual(written, strlen(testData), "write should write all bytes");
    
    lseek(fd, 0, SEEK_SET);
    ssize_t readBytes = read(fd, buffer, sizeof(buffer));
    XCTAssertEqual(readBytes, strlen(testData), "read should read all bytes");
    
    buffer[readBytes] = '\0';
    XCTAssertEqualObjects(@(buffer), @(testData), "read data should match written data");
    
    close(fd);
    unlink(testFile);
}

// MARK: - Memory Tests

- (void)testMmapAnonymous {
    size_t size = 4096;
    void *addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
    XCTAssertNotEqual(addr, MAP_FAILED, "mmap should succeed");
    
    // Write and read
    char *ptr = (char *)addr;
    ptr[0] = 'X';
    XCTAssertEqual(ptr[0], 'X', "mmap'd memory should be writable and readable");
    
    int result = munmap(addr, size);
    XCTAssertEqual(result, 0, "munmap should return 0");
}

// MARK: - Time Tests

- (void)testAlarm {
    unsigned int old = alarm(0);
    XCTAssertEqual(old, 0, "alarm(0) should return 0 initially");
}

- (void)testNanosleep {
    struct timespec req = {0, 10000000}; // 10ms
    struct timespec rem;
    
    int result = nanosleep(&req, &rem);
    XCTAssertEqual(result, 0, "nanosleep should return 0");
}

@end