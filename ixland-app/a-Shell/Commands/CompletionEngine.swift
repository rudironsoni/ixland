// CompletionEngine.swift
// Tab completion system for commands, filenames, and variables
// Part of M4-I3: Implement history and completion

import Foundation
import ios_system

/// Provides tab completion for the terminal
public actor CompletionEngine {

    // MARK: - Types

    /// Types of completions
    public enum CompletionType {
        case command      // First word - complete command name
        case filename     // File/directory path
        case variable     // Environment variable ($VAR)
        case history      // From command history
        case builtin      // Built-in command options
    }

    /// A completion suggestion
    public struct Completion: Identifiable, Equatable {
        public let id = UUID()
        public let text: String
        public let type: CompletionType
        public let description: String?
        public let icon: String?

        public init(text: String, type: CompletionType, description: String? = nil, icon: String? = nil) {
            self.text = text
            self.type = type
            self.description = description
            self.icon = icon
        }
    }

    /// Context for completion
    public struct Context {
        public let text: String           // Full input text
        public let cursorPosition: Int    // Cursor position in text
        public let sessionId: String      // Current session

        public var wordAtCursor: String {
            guard cursorPosition > 0 else { return "" }
            let prefix = String(text.prefix(cursorPosition))
            let words = prefix.components(separatedBy: .whitespacesAndNewlines)
            return words.last ?? ""
        }

        public var isFirstWord: Bool {
            let prefix = String(text.prefix(cursorPosition))
            return !prefix.contains(where: { $0.isWhitespace })
        }
    }

    // MARK: - Properties

    private var commandRegistry: Set<String> = []
    private var builtinCommands: Set<String> = []
    private var pathDirectories: [String] = []
    private var fileManager: FileManager

    // MARK: - Initialization

    public init() {
        self.fileManager = FileManager.default
        Task {
            await refreshCommandRegistry()
        }
    }

    // MARK: - Public API

    /// Get completions for the given context
    public func completions(for context: Context) -> [Completion] {
        let word = context.wordAtCursor

        // Determine completion type based on context
        if word.hasPrefix("$") {
            return completeVariable(word: word)
        } else if context.isFirstWord {
            return completeCommand(word: word)
        } else {
            return completeFilename(word: word, sessionId: context.sessionId)
        }
    }

    /// Get single-tab completion (complete as much as possible)
    public func completeSingle(context: Context) -> String? {
        let completions = completions(for: context)
        guard !completions.isEmpty else { return nil }

        // Find common prefix
        let texts = completions.map { $0.text }
        guard let commonPrefix = longestCommonPrefix(texts) else {
            return nil
        }

        return commonPrefix
    }

    /// Get next completion in cycle (for double-tap tab)
    public func completeNext(context: Context, previous: String? = nil) -> Completion? {
        let completions = completions(for: context)
        guard !completions.isEmpty else { return nil }

        // Sort completions
        let sorted = completions.sorted { $0.text < $1.text }

        // If no previous, return first
        guard let previous = previous else {
            return sorted.first
        }

        // Find next after previous
        if let index = sorted.firstIndex(where: { $0.text == previous }),
           index + 1 < sorted.count {
            return sorted[index + 1]
        }

        // Wrap around
        return sorted.first
    }

    // MARK: - Completion Types

    private func completeCommand(word: String) -> [Completion] {
        var results: [Completion] = []

        // Match against command registry
        for command in commandRegistry {
            if command.hasPrefix(word.lowercased()) {
                results.append(Completion(
                    text: command,
                    type: .command,
                    icon: "⌘"
                ))
            }
        }

        // Match against built-ins
        for builtin in builtinCommands {
            if builtin.hasPrefix(word.lowercased()) {
                results.append(Completion(
                    text: builtin,
                    type: .builtin,
                    description: "Built-in",
                    icon: "⚙️"
                ))
            }
        }

        return results
    }

    private func completeFilename(word: String, sessionId: String) -> [Completion] {
        var results: [Completion] = []

        // Expand tilde
        let expandedWord = expandTilde(word, sessionId: sessionId)

        // Determine directory to search
        let (searchDir, prefix) = parsePath(expandedWord)

        // List directory contents
        guard let contents = try? fileManager.contentsOfDirectory(
            atPath: searchDir
        ) else {
            return results
        }

        // Filter and create completions
        for item in contents {
            if item.hasPrefix(prefix) {
                let fullPath = (searchDir as NSString).appendingPathComponent(item)
                let isDirectory = isDirectory(at: fullPath)
                let suffix = isDirectory ? "/" : ""

                // Collapse path back to original form (with tilde if needed)
                let displayText = collapseTilde(fullPath + suffix, sessionId: sessionId)

                results.append(Completion(
                    text: displayText,
                    type: .filename,
                    description: isDirectory ? "Directory" : "File",
                    icon: isDirectory ? "📁" : "📄"
                ))
            }
        }

        return results
    }

    private func completeVariable(word: String) -> [Completion] {
        var results: [Completion] = []
        let varName = String(word.dropFirst()) // Remove $

        // Get environment variables
        var environPtr = environ
        while let envPtr = environPtr?.pointee {
            let env = String(cString: envPtr)
            if let equalsIndex = env.firstIndex(of: "=") {
                let key = String(env[..<equalsIndex])
                let value = String(env[env.index(after: equalsIndex)...])

                if key.hasPrefix(varName) {
                    results.append(Completion(
                        text: "$" + key,
                        type: .variable,
                        description: value,
                        icon: "🔤"
                    ))
                }
            }
            environPtr = environPtr?.advanced(by: 1)
        }

        return results
    }

    // MARK: - Path Helpers

    private func parsePath(_ path: String) -> (directory: String, prefix: String) {
        if path.isEmpty || path.hasSuffix("/") {
            return (path.isEmpty ? "." : path, "")
        }

        let nsPath = path as NSString
        let dir = nsPath.deletingLastPathComponent
        let prefix = nsPath.lastPathComponent

        return (dir.isEmpty ? "." : dir, prefix)
    }

    private func expandTilde(_ path: String, sessionId: String) -> String {
        guard path.hasPrefix("~") else { return path }

        // Get home directory
        let home = String(cString: ios_getenv("HOME") ?? "/".cString(using: .utf8)!)

        if path.hasPrefix("~/") {
            return home + String(path.dropFirst(1))
        } else if path == "~" {
            return home
        }

        // Handle ~username (if needed)
        return path
    }

    private func collapseTilde(_ path: String, sessionId: String) -> String {
        let home = String(cString: ios_getenv("HOME") ?? "/".cString(using: .utf8)!)

        if path.hasPrefix(home) {
            return "~" + String(path.dropFirst(home.count))
        }

        return path
    }

    private func isDirectory(at path: String) -> Bool {
        var isDir: ObjCBool = false
        return FileManager.default.fileExists(atPath: path, isDirectory: &isDir) && isDir.boolValue
    }

    // MARK: - Command Registry

    public func refreshCommandRegistry() {
        // Clear existing
        commandRegistry.removeAll()

        // Add commands from PATH directories
        let pathEnv = String(cString: ios_getenv("PATH") ?? "/usr/bin:/bin".cString(using: .utf8)!)
        let paths = pathEnv.components(separatedBy: ":")

        for path in paths {
            guard let contents = try? fileManager.contentsOfDirectory(atPath: path) else {
                continue
            }
            commandRegistry.formUnion(contents)
        }

        // Add built-in commands
        builtinCommands = [
            "cd", "pwd", "echo", "exit", "export", "unset", "alias", "unalias",
            "source", ".", "help", "history", "clear", "jobs", "fg", "bg"
        ]
    }

    public func registerCommand(_ name: String) {
        commandRegistry.insert(name.lowercased())
    }

    public func unregisterCommand(_ name: String) {
        commandRegistry.remove(name.lowercased())
    }

    // MARK: - Utility

    private func longestCommonPrefix(_ strings: [String]) -> String? {
        guard let first = strings.first else { return nil }
        guard strings.count > 1 else { return first }

        var prefix = ""
        for (index, char) in first.enumerated() {
            for string in strings.dropFirst() {
                if index >= string.count || string[string.index(string.startIndex, offsetBy: index)] != char {
                    return prefix
                }
            }
            prefix.append(char)
        }

        return prefix
    }

    // MARK: - Hints

    /// Get context-aware hints (ghost text)
    public func hint(for context: Context) -> String? {
        let word = context.wordAtCursor

        // Don't hint if word is empty
        guard !word.isEmpty else { return nil }

        // Get completions
        let completions = completions(for: context)
        guard let first = completions.first else { return nil }

        // Return suffix of best match
        if first.text.hasPrefix(word) && first.text.count > word.count {
            let start = first.text.index(first.text.startIndex, offsetBy: word.count)
            return String(first.text[start...])
        }

        return nil
    }
}

// MARK: - Integration with HistoryManager

extension CompletionEngine {

    /// Add history-based completions
    public func completeFromHistory(_ word: String, historyManager: HistoryManager) async -> [Completion] {
        let entries = await historyManager.entries(matching: word)
        let uniqueCommands = Set(entries.map { $0.command })

        return uniqueCommands.prefix(5).map { command in
            Completion(
                text: command,
                type: .history,
                description: "From history",
                icon: "🕐"
            )
        }
    }
}

// MARK: - Completion UI State

/// Tracks completion state for UI
public class CompletionState: ObservableObject {
    @Published public var completions: [CompletionEngine.Completion] = []
    @Published public var selectedIndex: Int = 0
    @Published public var isVisible: Bool = false

    private let engine: CompletionEngine

    public init(engine: CompletionEngine) {
        self.engine = engine
    }

    public func update(for context: CompletionEngine.Context) async {
        let newCompletions = await engine.completions(for: context)

        await MainActor.run {
            self.completions = newCompletions
            self.selectedIndex = 0
            self.isVisible = !newCompletions.isEmpty
        }
    }

    public func selectNext() {
        guard !completions.isEmpty else { return }
        selectedIndex = (selectedIndex + 1) % completions.count
    }

    public func selectPrevious() {
        guard !completions.isEmpty else { return }
        selectedIndex = (selectedIndex - 1 + completions.count) % completions.count
    }

    public var selectedCompletion: CompletionEngine.Completion? {
        guard selectedIndex < completions.count else { return nil }
        return completions[selectedIndex]
    }

    public func hide() {
        isVisible = false
    }
}
