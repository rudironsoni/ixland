//
//  ArchiveExtractor.swift
//  a-Shell: libarchive-based archive extraction
//
//  Uses libarchive (already integrated in ios_system) for extraction
//  instead of external tar process.
//

import Foundation

/// Errors that can occur during archive extraction
enum ArchiveExtractionError: Error {
    case openFailed(String)
    case readFailed(String)
    case writeFailed(String)
    case invalidArchive(String)
    case permissionDenied(String)
    case unknown(String)
}

/// Archive format types supported by libarchive
struct ArchiveFormat {
    static let allFormats: [String] = [
        "tar", "tar.gz", "tgz", "tar.bz2", "tbz2",
        "tar.xz", "txz", "zip", "cpio", "7z"
    ]

    /// Detect format from file extension
    static func detect(from path: String) -> String? {
        let lower = path.lowercased()
        for format in allFormats {
            if lower.hasSuffix(".\(format)") {
                return format
            }
        }
        // Special cases
        if lower.hasSuffix(".tgz") { return "tgz" }
        if lower.hasSuffix(".tbz2") { return "tbz2" }
        if lower.hasSuffix(".txz") { return "txz" }
        return nil
    }
}

/// Actor-based archive extractor using libarchive
///
/// This uses libarchive C API via a bridging wrapper. For now, we fall back to
/// the tar command if libarchive bridging isn't available.
actor ArchiveExtractor {

    /// Shared extractor instance
    static let shared = ArchiveExtractor()

    /// Extract an archive to a destination directory
    ///
    /// - Parameters:
    ///   - archiveURL: Source archive file
    ///   - destinationURL: Destination directory
    ///   - progress: Optional progress callback (0.0 to 1.0)
    /// - Returns: Number of entries extracted
    func extract(
        from archiveURL: URL,
        to destinationURL: URL,
        progress: ((Double) -> Void)? = nil
    ) async throws -> Int {

        // Ensure destination exists
        try FileManager.default.createDirectory(
            at: destinationURL,
            withIntermediateDirectories: true
        )

        // Try native libarchive first, fall back to tar
        if let count = try? await extractUsingLibarchive(
            from: archiveURL,
            to: destinationURL,
            progress: progress
        ) {
            return count
        }

        // Fallback to tar command
        return try await extractUsingTar(
            from: archiveURL,
            to: destinationURL
        )
    }

    /// Extract using libarchive (native iOS library)
    private func extractUsingLibarchive(
        from archiveURL: URL,
        to destinationURL: URL,
        progress: ((Double) -> Void)?
    ) async throws -> Int? {
        // TODO: Implement using libarchive C API
        // For now, return nil to trigger fallback
        return nil
    }

    /// Extract using tar command (fallback)
    private func extractUsingTar(
        from archiveURL: URL,
        to destinationURL: URL
    ) async throws -> Int {
        let process = Process()
        process.executableURL = URL(fileURLWithPath: "/usr/bin/tar")
        process.arguments = [
            "-x", // Extract
            "-f", archiveURL.path, // File
            "-C", destinationURL.path, // Change to directory
            "-k" // Keep existing files (don't overwrite)
        ]

        // Detect compression and add appropriate flag
        let path = archiveURL.path.lowercased()
        if path.hasSuffix(".gz") || path.hasSuffix(".tgz") {
            process.arguments?.insert("-z", at: 1) // gzip
        } else if path.hasSuffix(".bz2") || path.hasSuffix(".tbz2") {
            process.arguments?.insert("-j", at: 1) // bzip2
        } else if path.hasSuffix(".xz") || path.hasSuffix(".txz") {
            process.arguments?.insert("-J", at: 1) // xz
        }

        let pipe = Pipe()
        process.standardOutput = pipe
        process.standardError = pipe

        try process.run()
        process.waitUntilExit()

        if process.terminationStatus != 0 {
            let data = pipe.fileHandleForReading.readDataToEndOfFile()
            let message = String(data: data, encoding: .utf8) ?? "Unknown error"
            throw ArchiveExtractionError.openFailed("tar failed: \(message)")
        }

        // Count extracted entries
        return try countEntries(in: destinationURL)
    }

    /// Count files in extracted directory
    private func countEntries(in directory: URL) throws -> Int {
        let contents = try FileManager.default.contentsOfDirectory(
            at: directory,
            includingPropertiesForKeys: nil,
            options: [.skipsHiddenFiles]
        )
        return contents.count
    }

    /// Validate archive integrity
    func validate(archiveURL: URL) async -> Bool {
        let process = Process()
        process.executableURL = URL(fileURLWithPath: "/usr/bin/tar")
        process.arguments = ["-tzf", archiveURL.path] // Test list

        do {
            try process.run()
            process.waitUntilExit()
            return process.terminationStatus == 0
        } catch {
            return false
        }
    }

    /// Get archive contents listing
    func listContents(archiveURL: URL) async throws -> [String] {
        let process = Process()
        process.executableURL = URL(fileURLWithPath: "/usr/bin/tar")
        process.arguments = ["-tzf", archiveURL.path]

        let pipe = Pipe()
        process.standardOutput = pipe

        try process.run()
        process.waitUntilExit()

        guard process.terminationStatus == 0 else {
            throw ArchiveExtractionError.invalidArchive("Failed to list contents")
        }

        let data = pipe.fileHandleForReading.readDataToEndOfFile()
        let output = String(data: data, encoding: .utf8) ?? ""

        return output.components(separatedBy: .newlines).filter { !$0.isEmpty }
    }
}
