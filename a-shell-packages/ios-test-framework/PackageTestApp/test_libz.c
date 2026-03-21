//
//  test_libz.c
//  libz test suite for iOS
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <zlib.h>

#define TEST_BUFFER_SIZE 1024
#define COMPRESSED_SIZE 2048

// Test 1: Basic compression and decompression
int test_libz_compression(void) {
    const char *original = "Hello, World! This is a test string for zlib compression.";
    size_t original_len = strlen(original) + 1;
    
    unsigned char compressed[COMPRESSED_SIZE];
    unsigned long compressed_len = COMPRESSED_SIZE;
    
    // Compress
    int ret = compress(compressed, &compressed_len, (const unsigned char *)original, original_len);
    if (ret != Z_OK) {
        fprintf(stderr, "Compression failed: %d\n", ret);
        return 1;
    }
    
    // Verify compression happened (compressed should be smaller or equal)
    if (compressed_len > original_len * 2) {
        fprintf(stderr, "Compression ineffective: %lu -> %lu\n", original_len, compressed_len);
        // Not a failure, just warning
    }
    
    // Decompress
    unsigned char decompressed[TEST_BUFFER_SIZE];
    unsigned long decompressed_len = TEST_BUFFER_SIZE;
    
    ret = uncompress(decompressed, &decompressed_len, compressed, compressed_len);
    if (ret != Z_OK) {
        fprintf(stderr, "Decompression failed: %d\n", ret);
        return 1;
    }
    
    // Verify
    if (decompressed_len != original_len) {
        fprintf(stderr, "Size mismatch: expected %zu, got %lu\n", original_len, decompressed_len);
        return 1;
    }
    
    if (strcmp((const char *)decompressed, original) != 0) {
        fprintf(stderr, "Content mismatch after decompression\n");
        return 1;
    }
    
    printf("  Compression: %zu -> %lu bytes (%.1f%%)\n", 
           original_len, compressed_len, 
           100.0 * compressed_len / original_len);
    
    return 0;
}

// Test 2: Large data compression
int test_libz_decompression(void) {
    // Create larger test data
    size_t large_size = 10000;
    unsigned char *original = malloc(large_size);
    if (!original) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Fill with pattern
    for (size_t i = 0; i < large_size; i++) {
        original[i] = (unsigned char)(i % 256);
    }
    
    unsigned char *compressed = malloc(large_size * 2);
    if (!compressed) {
        free(original);
        return 1;
    }
    
    unsigned long compressed_len = large_size * 2;
    int ret = compress(compressed, &compressed_len, original, large_size);
    if (ret != Z_OK) {
        fprintf(stderr, "Large data compression failed: %d\n", ret);
        free(original);
        free(compressed);
        return 1;
    }
    
    unsigned char *decompressed = malloc(large_size);
    if (!decompressed) {
        free(original);
        free(compressed);
        return 1;
    }
    
    unsigned long decompressed_len = large_size;
    ret = uncompress(decompressed, &decompressed_len, compressed, compressed_len);
    if (ret != Z_OK) {
        fprintf(stderr, "Large data decompression failed: %d\n", ret);
        free(original);
        free(compressed);
        free(decompressed);
        return 1;
    }
    
    if (decompressed_len != large_size || memcmp(original, decompressed, large_size) != 0) {
        fprintf(stderr, "Large data verification failed\n");
        free(original);
        free(compressed);
        free(decompressed);
        return 1;
    }
    
    printf("  Large data: %zu -> %lu bytes (%.1f%%)\n",
           large_size, compressed_len,
           100.0 * compressed_len / large_size);
    
    free(original);
    free(compressed);
    free(decompressed);
    
    return 0;
}

// Test 3: Edge cases
int test_libz_edge_cases(void) {
    // Test empty data
    unsigned char empty[] = "";
    unsigned char compressed[256];
    unsigned long compressed_len = 256;
    
    int ret = compress(compressed, &compressed_len, empty, 1);
    if (ret != Z_OK) {
        fprintf(stderr, "Empty data compression failed: %d\n", ret);
        return 1;
    }
    
    unsigned char decompressed[256];
    unsigned long decompressed_len = 256;
    ret = uncompress(decompressed, &decompressed_len, compressed, compressed_len);
    if (ret != Z_OK || decompressed_len != 1 || decompressed[0] != '\0') {
        fprintf(stderr, "Empty data decompression failed\n");
        return 1;
    }
    
    // Test highly compressible data
    unsigned char repeating[1000];
    memset(repeating, 'A', sizeof(repeating));
    
    compressed_len = sizeof(compressed);
    ret = compress(compressed, &compressed_len, repeating, sizeof(repeating));
    if (ret != Z_OK) {
        fprintf(stderr, "Repeating data compression failed: %d\n", ret);
        return 1;
    }
    
    // Should compress very well
    if (compressed_len > sizeof(repeating) / 10) {
        fprintf(stderr, "Warning: Low compression ratio for repeating data\n");
    }
    
    printf("  Edge cases: empty data, repeating data (ratio: %.1f:1)\n",
           1.0 * sizeof(repeating) / compressed_len);
    
    return 0;
}
