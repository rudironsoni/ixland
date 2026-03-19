# LineEditor Integration Guide (M4-I2)

## Overview

This guide documents how to integrate [LineEditor](https://github.com/holzschu/LineEditor) into a-Shell for interactive input with history and completion.

## Status

**Phase:** Implementation Ready
**Priority:** P2
**Blocked by:** None
**Blocks:** M4-I3 (history and completion)

## Prerequisites

- Xcode 15.0+
- iOS 16.0+ deployment target
- a-Shell Xcode project

## Integration Steps

### 1. Add LineEditor Dependency

LineEditor is available as a Swift Package. Add it to the a-Shell Xcode project:

```
File → Add Package Dependencies
URL: https://github.com/holzschu/LineEditor
```

### 2. Create LineEditor Bridge

Create `a-Shell/Terminal/LineEditorBridge.swift`:

```swift
import Foundation
import LineEditor
import ios_system

/// Bridge between LineEditor and ios_system for interactive input
public class LineEditorBridge {
    static let shared = LineEditorBridge()

    private var history: [String] = []
    private let historyFile: URL

    init() {
        // Store history in ASHELL_CONFIG
        let configDir = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask).first!
            .appendingPathComponent(".ashell")
        historyFile = configDir.appendingPathComponent("history")
        loadHistory()
    }

    /// Get interactive input with history and completion
    func readLine(prompt: String, completion: @escaping (String?) -> Void) {
        // Only use LineEditor for TTY
        guard isatty(STDIN_FILENO) != 0 else {
            // Fallback to basic input for pipes
            var input = readLine()
            completion(input)
            return
        }

        let config = LineEditor.Configuration(
            prompt: prompt,
            history: history,
            completionHandler: { [weak self] line, range in
                return self?.completions(for: line, in: range) ?? []
            }
        )

        LineEditor.readLine(configuration: config) { result in
            if let line = result {
                self.addToHistory(line)
            }
            completion(result)
        }
    }

    /// Provide completions for current input
    private func completions(for line: String, in range: NSRange) -> [String] {
        // TODO: Implement command and filename completion
        // See M4-I3 for full completion implementation
        return []
    }

    /// Add command to history
    private func addToHistory(_ command: String) {
        guard !command.isEmpty else { return }
        history.append(command)
        // Limit history size
        if history.count > 1000 {
            history.removeFirst(history.count - 1000)
        }
        saveHistory()
    }

    /// Load history from disk
    private func loadHistory() {
        guard let data = try? Data(contentsOf: historyFile),
              let strings = try? JSONDecoder().decode([String].self, from: data) else {
            return
        }
        history = strings
    }

    /// Save history to disk
    private func saveHistory() {
        try? FileManager.default.createDirectory(at: historyFile.deletingLastPathComponent(),
                                                  withIntermediateDirectories: true)
        if let data = try? JSONEncoder().encode(history) {
            try? data.write(to: historyFile)
        }
    }
}
```

### 3. Integrate with ios_system

Add to `ios_system/ios_system.h`:

```c
// LineEditor integration (M4-I2)
// Returns: user input string (caller must free), or NULL on EOF/error
extern char* ios_readline(const char* prompt);
```

### 4. Hook into Interactive Commands

Modify interactive commands (sh, python, etc.) to use `ios_readline()` instead of `fgets()` when `isatty()` returns true.

## Testing

1. Build a-Shell with LineEditor
2. Run `sh` command
3. Verify:
   - Up/down arrows navigate history
   - Tab triggers completion (once M4-I3 is done)
   - History persists after app restart

## Dependencies

- LineEditor Swift package
- M4-I1 (sysinfo API) - COMPLETED
- M4-I3 (completion) - Should be done together or after

## References

- LineEditor: https://github.com/holzschu/LineEditor
- Original a-Shell LineEditor usage: Check SwiftTerm2 branch
