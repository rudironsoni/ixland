// HistoryManager.swift
// Command history persistence and management
// Part of M4-I3: Implement history and completion

import Foundation
import Combine

/// Manages command history persistence and navigation
public actor HistoryManager {

    // MARK: - Types

    /// Represents a history entry with metadata
    public struct HistoryEntry: Codable, Identifiable {
        public let id: UUID
        public let command: String
        public let timestamp: Date
        public let sessionId: String?

        public init(command: String, sessionId: String? = nil) {
            self.id = UUID()
            self.command = command.trimmingCharacters(in: .whitespacesAndNewlines)
            self.timestamp = Date()
            self.sessionId = sessionId
        }
    }

    /// Configuration for history behavior
    public struct Configuration {
        let maxEntries: Int
        let saveToDisk: Bool
        let ignoreDuplicates: Bool
        let ignorePrefixes: [String]

        public static let `default` = Configuration(
            maxEntries: 1000,
            saveToDisk: true,
            ignoreDuplicates: true,
            ignorePrefixes: [" ", "#", "pwd", "ls"]
        )
    }

    // MARK: - Properties

    private var entries: [HistoryEntry] = []
    private var currentIndex: Int = -1
    private var searchIndex: Int = -1
    private var searchQuery: String = ""
    private let configuration: Configuration
    private let historyFileURL: URL

    // MARK: - Initialization

    public init(configuration: Configuration = .default) {
        self.configuration = configuration

        // Use ASHELL_CONFIG directory for history file
        let configDir = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask)
            .first?
            .appendingPathComponent(".ashell", isDirectory: true) ??
            FileManager.default.temporaryDirectory

        self.historyFileURL = configDir.appendingPathComponent("history")
    }

    // MARK: - Public API

    /// Add a command to history
    public func addCommand(_ command: String, sessionId: String? = nil) {
        let trimmed = command.trimmingCharacters(in: .whitespacesAndNewlines)

        // Skip empty commands
        guard !trimmed.isEmpty else { return }

        // Skip commands matching ignore prefixes
        for prefix in configuration.ignorePrefixes {
            if trimmed.hasPrefix(prefix) { return }
        }

        // Skip duplicates if configured
        if configuration.ignoreDuplicates,
           let last = entries.last,
           last.command == trimmed {
            return
        }

        // Add entry
        let entry = HistoryEntry(command: trimmed, sessionId: sessionId)
        entries.append(entry)

        // Trim to max size
        if entries.count > configuration.maxEntries {
            entries.removeFirst(entries.count - configuration.maxEntries)
        }

        // Reset navigation index
        currentIndex = entries.count

        // Save to disk asynchronously
        if configuration.saveToDisk {
            Task {
                await save()
            }
        }
    }

    /// Get previous command (up arrow)
    public func previousCommand() -> String? {
        guard !entries.isEmpty else { return nil }
        guard currentIndex > 0 else { return entries.first?.command }

        currentIndex -= 1
        return entries[currentIndex].command
    }

    /// Get next command (down arrow)
    public func nextCommand() -> String? {
        guard currentIndex < entries.count - 1 else {
            currentIndex = entries.count
            return nil // Empty line at end
        }

        currentIndex += 1
        return entries[currentIndex].command
    }

    /// Reset navigation to end (after executing a command)
    public func resetNavigation() {
        currentIndex = entries.count
        searchIndex = -1
        searchQuery = ""
    }

    // MARK: - Search (Ctrl-R)

    /// Start a reverse search with initial query
    public func startSearch(query: String = "") {
        searchQuery = query
        searchIndex = entries.count - 1
    }

    /// Find previous match in reverse search
    public func searchPrevious() -> HistoryEntry? {
        guard !entries.isEmpty, !searchQuery.isEmpty else { return nil }

        while searchIndex >= 0 {
            let entry = entries[searchIndex]
            searchIndex -= 1

            if entry.command.localizedCaseInsensitiveContains(searchQuery) {
                return entry
            }
        }

        return nil
    }

    /// Find next match in reverse search
    public func searchNext() -> HistoryEntry? {
        guard !entries.isEmpty, !searchQuery.isEmpty else { return nil }

        while searchIndex < entries.count - 1 {
            searchIndex += 1
            let entry = entries[searchIndex]

            if entry.command.localizedCaseInsensitiveContains(searchQuery) {
                return entry
            }
        }

        return nil
    }

    /// Cancel current search
    public func cancelSearch() {
        searchIndex = -1
        searchQuery = ""
    }

    // MARK: - Persistence

    /// Save history to disk
    public func save() async {
        guard configuration.saveToDisk else { return }

        do {
            // Ensure directory exists
            let directory = historyFileURL.deletingLastPathComponent()
            try FileManager.default.createDirectory(
                at: directory,
                withIntermediateDirectories: true,
                attributes: nil
            )

            // Encode and save
            let encoder = JSONEncoder()
            encoder.dateEncodingStrategy = .iso8601
            let data = try encoder.encode(entries)
            try data.write(to: historyFileURL, options: .atomic)
        } catch {
            print("HistoryManager: Failed to save history - \(error)")
        }
    }

    /// Load history from disk
    public func load() async {
        guard configuration.saveToDisk else { return }
        guard FileManager.default.fileExists(atPath: historyFileURL.path) else {
            return
        }

        do {
            let data = try Data(contentsOf: historyFileURL)
            let decoder = JSONDecoder()
            decoder.dateDecodingStrategy = .iso8601
            entries = try decoder.decode([HistoryEntry].self, from: data)
            currentIndex = entries.count
        } catch {
            print("HistoryManager: Failed to load history - \(error)")
            entries = []
            currentIndex = 0
        }
    }

    /// Clear all history
    public func clear() {
        entries.removeAll()
        currentIndex = 0
        searchIndex = -1

        // Delete file
        if configuration.saveToDisk {
            try? FileManager.default.removeItem(at: historyFileURL)
        }
    }

    // MARK: - Queries

    /// Get all entries (for UI display)
    public func allEntries() -> [HistoryEntry] {
        return entries
    }

    /// Get entries matching filter
    public func entries(matching query: String) -> [HistoryEntry] {
        return entries.filter { entry in
            entry.command.localizedCaseInsensitiveContains(query)
        }
    }

    /// Get entries for specific session
    public func entries(forSession sessionId: String) -> [HistoryEntry] {
        return entries.filter { $0.sessionId == sessionId }
    }

    /// Get most recent N entries
    public func recentEntries(count: Int) -> [HistoryEntry] {
        let start = max(0, entries.count - count)
        return Array(entries[start...])
    }

    /// Get history size
    public var count: Int {
        return entries.count
    }

    /// Get history file size for monitoring
    public func fileSize() -> UInt64 {
        guard FileManager.default.fileExists(atPath: historyFileURL.path) else {
            return 0
        }

        do {
            let attributes = try FileManager.default.attributesOfItem(atPath: historyFileURL.path)
            return attributes[.size] as? UInt64 ?? 0
        } catch {
            return 0
        }
    }

    /// Get formatted history for display
    public func formattedHistory() -> String {
        return entries.enumerated().map { index, entry in
            let number = String(format: "%5d", index + 1)
            let timestamp = ISO8601DateFormatter().string(from: entry.timestamp)
            return "\(number)  \(timestamp)  \(entry.command)"
        }.joined(separator: "\n")
    }
}

// MARK: - HISTSIZE Support

extension HistoryManager {

    /// Update configuration from environment variables
    public func updateFromEnvironment() {
        // Check HISTSIZE
        if let histSizeStr = getenv("HISTSIZE"),
           let histSize = Int(String(cString: histSizeStr)),
           histSize > 0 {
            // Re-initialize with new max (not implemented for simplicity)
            // In production, would need to resize entries array
            print("HistoryManager: HISTSIZE=\(histSize) - resizing not yet implemented")
        }

        // Check HISTFILE
        // Could support custom history file location
    }
}

// MARK: - Integration Helpers

extension HistoryManager {

    /// Import history from a file (e.g., .bash_history)
    public func importFromFile(_ url: URL) async throws {
        let content = try String(contentsOf: url, encoding: .utf8)
        let commands = content.components(separatedBy: .newlines)

        for command in commands {
            addCommand(command)
        }

        await save()
    }

    /// Export history to a file
    public func exportToFile(_ url: URL) async throws {
        let lines = entries.map { $0.command }
        let content = lines.joined(separator: "\n")
        try content.write(to: url, atomically: true, encoding: .utf8)
    }
}
