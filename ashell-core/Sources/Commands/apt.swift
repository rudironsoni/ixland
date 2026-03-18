//
//  apt.swift
//  ashell-core: Debian-style package manager command
//
//  This command provides runtime package installation/removal.
//  Packages are built using ashell-packages and distributed as XCFrameworks.
//

import Foundation
import ios_system

// Import PackageManager from sibling module
import PackageManager

@_cdecl("apt")
public func apt(argc: Int32, argv: UnsafeMutablePointer<UnsafeMutablePointer<Int8>?>?) -> Int32 {
    let args = convertCArguments(argc: argc, argv: argv)

    let usage = """
    usage: apt <command> [arguments]

    Package manager for a-Shell native packages (Debian-style).

    Commands:
        install <package>     Install a package
        remove <package>      Remove an installed package
        update                Update package catalog
        search <query>          Search for packages
        list                    List installed packages
        info <package>          Show package information

    Examples:
        apt install hello       # Install the hello package
        apt list                # Show installed packages
        apt search util         # Search for utility packages

    """

    guard args.count > 1 else {
        fputs(usage, thread_stdout)
        return 0
    }

    let command = args[1]

    switch command {
    case "install":
        guard args.count > 2 else {
            fputs("apt install: package name required\n", thread_stderr)
            fputs("usage: apt install <package>\n", thread_stderr)
            return 1
        }
        let packageName = args[2]

        fputs("Installing \(packageName)...\n", thread_stdout)

        let semaphore = DispatchSemaphore(value: 0)
        var result: Int32 = 1

        Task {
            let manager = PackageManager.shared
            let installResult = await manager.install(package: packageName)

            switch installResult {
            case .success:
                fputs("\(packageName) installed successfully.\n", thread_stdout)
                result = 0
            case .alreadyInstalled:
                fputs("\(packageName) is already installed.\n", thread_stdout)
                result = 0
            case .downloadFailed(let error):
                fputs("Error: \(error)\n", thread_stderr)
            case .verificationFailed(let error):
                fputs("Error: \(error)\n", thread_stderr)
            case .extractionFailed(let error):
                fputs("Error: \(error)\n", thread_stderr)
            case .registrationFailed(let error):
                fputs("Error: \(error)\n", thread_stderr)
            }

            semaphore.signal()
        }

        semaphore.wait()
        return result

    case "remove", "uninstall":
        guard args.count > 2 else {
            fputs("apt remove: package name required\n", thread_stderr)
            fputs("usage: apt remove <package>\n", thread_stderr)
            return 1
        }
        let packageName = args[2]

        fputs("Removing \(packageName)...\n", thread_stdout)

        let semaphore = DispatchSemaphore(value: 0)
        var result: Int32 = 1

        Task {
            let manager = PackageManager.shared
            let success = await manager.remove(package: packageName)

            if success {
                fputs("\(packageName) removed successfully.\n", thread_stdout)
                result = 0
            } else {
                fputs("Error: Failed to remove \(packageName)\n", thread_stderr)
            }

            semaphore.signal()
        }

        semaphore.wait()
        return result

    case "update":
        fputs("Updating package catalog...\n", thread_stdout)

        let semaphore = DispatchSemaphore(value: 0)
        var result: Int32 = 1

        Task {
            let manager = PackageManager.shared
            let success = await manager.updateCatalog()

            if success {
                fputs("Package catalog updated.\n", thread_stdout)
                result = 0
            } else {
                fputs("Error: Failed to update catalog\n", thread_stderr)
            }

            semaphore.signal()
        }

        semaphore.wait()
        return result

    case "list":
        let semaphore = DispatchSemaphore(value: 0)

        Task {
            let manager = PackageManager.shared
            let packages = await manager.listInstalled()

            if packages.isEmpty {
                fputs("No packages installed.\n", thread_stdout)
            } else {
                fputs("Installed packages:\n", thread_stdout)
                fputs("\(String(repeating: "-", count: 50))\n", thread_stdout)
                fputs(String(format: "%-20s %-10s %s\n", "Name", "Version", "Commands"), thread_stdout)
                fputs("\(String(repeating: "-", count: 50))\n", thread_stdout)

                for pkg in packages {
                    let commands = pkg.commands.joined(separator: ", ")
                    fputs(String(format: "%-20s %-10s %s\n", pkg.name, pkg.version, commands), thread_stdout)
                }
            }

            semaphore.signal()
        }

        semaphore.wait()
        return 0

    case "search":
        guard args.count > 2 else {
            fputs("apt search: query required\n", thread_stderr)
            fputs("usage: apt search <query>\n", thread_stderr)
            return 1
        }
        let query = args[2]

        fputs("Searching for '\(query)'...\n", thread_stdout)

        let semaphore = DispatchSemaphore(value: 0)

        Task {
            let manager = PackageManager.shared
            let results = await manager.search(query: query)

            if results.isEmpty {
                fputs("No packages found matching '\(query)'.\n", thread_stdout)
            } else {
                fputs("Found \(results.count) package(s):\n", thread_stdout)
                fputs("\(String(repeating: "-", count: 60))\n", thread_stdout)

                for pkg in results {
                    fputs("\(pkg.name)@\(pkg.version)\n", thread_stdout)
                    fputs("  \(pkg.description)\n", thread_stdout)
                    if let homepage = pkg.homepage {
                        fputs("  Homepage: \(homepage)\n", thread_stdout)
                    }
                    if !pkg.dependencies.isEmpty {
                        fputs("  Dependencies: \(pkg.dependencies.joined(separator: ", "))\n", thread_stdout)
                    }
                    fputs("\n", thread_stdout)
                }
            }

            semaphore.signal()
        }

        semaphore.wait()
        return 0

    case "info":
        guard args.count > 2 else {
            fputs("apt info: package name required\n", thread_stderr)
            fputs("usage: apt info <package>\n", thread_stderr)
            return 1
        }
        let packageName = args[2]

        let semaphore = DispatchSemaphore(value: 0)

        Task {
            let manager = PackageManager.shared
            let (installed, metadata) = await manager.info(package: packageName)

            if let meta = metadata {
                fputs("Package: \(meta.name)\n", thread_stdout)
                fputs("Version: \(meta.version)\n", thread_stdout)
                fputs("Description: \(meta.description)\n", thread_stdout)
                if let homepage = meta.homepage {
                    fputs("Homepage: \(homepage)\n", thread_stdout)
                }
                if !meta.dependencies.isEmpty {
                    fputs("Dependencies: \(meta.dependencies.joined(separator: ", "))\n", thread_stdout)
                }
                fputs("Commands: \(meta.commands.joined(separator: ", "))\n", thread_stdout)
            } else {
                fputs("Package '\(packageName)' not found in catalog.\n", thread_stdout)
            }

            if let inst = installed {
                fputs("\nInstalled version: \(inst.version)\n", thread_stdout)
                fputs("Install date: \(inst.installDate)\n", thread_stdout)
                fputs("Framework: \(inst.frameworkPath)\n", thread_stdout)
            } else if metadata != nil {
                fputs("\nNot currently installed.\n", thread_stdout)
            }

            semaphore.signal()
        }

        semaphore.wait()
        return 0

    case "-h", "--help", "help":
        fputs(usage, thread_stdout)
        return 0

    default:
        fputs("Unknown command: \(command)\n", thread_stderr)
        fputs(usage, thread_stderr)
        return 1
    }
}

/// Convert C arguments to Swift array
private func convertCArguments(argc: Int32, argv: UnsafeMutablePointer<UnsafeMutablePointer<Int8>?>) -> [String] {
    var args: [String] = []
    for i in 0..<Int(argc) {
        if let arg = argv[i] {
            args.append(String(cString: arg))
        }
    }
    return args
}
