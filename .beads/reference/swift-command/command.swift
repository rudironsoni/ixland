// swift-command/command.swift
// Template for Swift-implemented commands in a-Shell
//
// Source: /home/rrj/.beads/reference/swift-command/command.swift
// Purpose: Reference implementation for new Swift commands
// Usage: Copy to ashell-core/Sources/Commands/{Name}Command.swift

import Foundation
import ios_system

// =============================================================================
// COMMAND IMPLEMENTATION
// =============================================================================

/// {CommandName} command implementation
///
/// This command demonstrates the pattern for implementing commands in Swift
/// that integrate with ios_system and a-Shell.
public struct {CommandName}Command {

    // MARK: - Configuration

    /// Command name used in terminal
    static let commandName = "{command}"

    /// Command description for help
    static let commandDescription = "Description of what this command does"

    /// Version string
    static let version = "1.0.0"

    // MARK: - Options

    struct Options {
        var verbose = false
        var help = false
        var version = false
        var outputPath: String?
        var arguments: [String] = []
    }

    // MARK: - Main Entry Point

    /// C-interop entry point for ios_system
    ///
    /// This function is called from C via the @_cdecl attribute.
    /// It bridges C-style arguments to Swift OptionParser.
    @discardableResult
    static func run(argc: Int32, argv: UnsafeMutablePointer<UnsafeMutablePointer<Int8>?>) -> Int32 {
        // Convert C arguments to Swift array
        var args: [String] = []
        for i in 0..<Int(argc) {
            if let arg = argv[i] {
                args.append(String(cString: arg))
            }
        }

        // Parse options
        let options: Options
        do {
            options = try parseArguments(args)
        } catch {
            printError("Error: \(error)")
            printUsage()
            return 1
        }

        // Handle help/version
        if options.help {
            printUsage()
            return 0
        }

        if options.version {
            printVersion()
            return 0
        }

        // Execute command
        do {
            try execute(options: options)
            return 0
        } catch {
            printError("Error: \(error)")
            return 1
        }
    }

    // MARK: - Argument Parsing

    private static func parseArguments(_ args: [String]) throws -> Options {
        var options = Options()
        var i = 1  // Skip program name

        while i < args.count {
            let arg = args[i]

            switch arg {
            case "-h", "--help":
                options.help = true

            case "-v", "--version":
                options.version = true

            case "-V", "--verbose":
                options.verbose = true

            case "-o", "--output":
                i += 1
                guard i < args.count else {
                    throw ArgumentError.missingValue("--output")
                }
                options.outputPath = args[i]

            case "--":
                // End of options
                i += 1
                options.arguments.append(contentsOf: args[i...])
                return options

            case _ where arg.hasPrefix("-"):
                throw ArgumentError.unknownOption(arg)

            default:
                options.arguments.append(arg)
            }

            i += 1
        }

        return options
    }

    // MARK: - Command Execution

    private static func execute(options: Options) throws {
        // Implement command logic here

        if options.verbose {
            printVerbose("Executing with options: \(options)")
        }

        // Example: Process files
        for argument in options.arguments {
            if options.verbose {
                printVerbose("Processing: \(argument)")
            }

            // Implement actual logic here
            let result = process(argument)

            if let outputPath = options.outputPath {
                try result.write(toFile: outputPath, atomically: true, encoding: .utf8)
            } else {
                print(result)
            }
        }
    }

    private static func process(_ input: String) -> String {
        // Implement processing logic
        return "Processed: \(input)"
    }

    // MARK: - Output Helpers

    private static func print(_ message: String) {
        // Use ios_system's stdout
        if let stdout = thread_stdout {
            fputs(message + "\n", stdout)
        } else {
            Swift.print(message)
        }
    }

    private static func printError(_ message: String) {
        // Use ios_system's stderr
        if let stderr = thread_stderr {
            fputs(message + "\n", stderr)
        } else {
            fputs(message + "\n", stderr)
        }
    }

    private static func printVerbose(_ message: String) {
        // Only print in verbose mode
        print("[verbose] \(message)")
    }

    private static func printUsage() {
        print("""
        Usage: \(commandName) [OPTIONS] [ARGUMENTS]

        \(commandDescription)

        Options:
          -h, --help       Show this help message
          -v, --version    Show version information
          -V, --verbose    Enable verbose output
          -o, --output     Write output to file

        Examples:
          \(commandName) file.txt           Process file.txt
          \(commandName) -o result.txt input.txt  Write to result.txt
          \(commandName) --verbose *.txt    Process with verbose output
        """)
    }

    private static func printVersion() {
        print("\(commandName) \(version)")
    }
}

// MARK: - Error Types

enum ArgumentError: Error {
    case unknownOption(String)
    case missingValue(String)
    case invalidValue(String, String)
}

// MARK: - C-Interop Export

/// C-callable entry point
///
/// Export this function for ios_system to call.
/// Register in commands.plist as: "command:command_main::no"
@_cdecl("{command}_main")
public func {command}Main(argc: Int32, argv: UnsafeMutablePointer<UnsafeMutablePointer<Int8>?>) -> Int32 {
    return {CommandName}Command.run(argc: argc, argv: argv)
}

// =============================================================================
// TESTING
// =============================================================================

#if DEBUG
// Unit tests for the command
import Testing

@Test
func testArgumentParsing() throws {
    // Test argument parsing logic
    // This runs during development/testing
}

@Test
func testCommandExecution() throws {
    // Test command execution
    // Mock ios_system I/O for testing
}
#endif

// =============================================================================
// INTEGRATION
// =============================================================================

/*
To integrate this command into a-Shell:

1. Copy this file to:
   ashell-core/Sources/Commands/{Name}Command.swift

2. Register in Package.swift:
   .target(name: "{Name}Command", dependencies: ["ios_system"])

3. Add to commands.plist:
   <key>{command}</key>
   <array>
       <string>{Name}Command.framework/{Name}Command</string>
       <string>{command}_main</string>
       <string>r0qndD...</string>
       <string>no</string>
   </array>

4. Build:
   swift build --target {Name}Command

5. Test:
   Run in a-Shell terminal: {command} --help
*/
