//
//  PackageManager.swift
//  a-Shell: Native package manager for XCFramework-based commands
//
//  Created for a-Shell Next modernization.
//

import Foundation
import ios_system
import CryptoKit

// MARK: - Types

/// Represents an installed package
struct InstalledPackage: Codable {
    let name: String
    let version: String
    let installDate: Date
    let frameworkPath: String
    let commands: [String]
}

/// Package metadata from catalog
struct PackageMetadata: Codable {
    let name: String
    let version: String
    let description: String
    let homepage: String?
    let sha256: String
    let size: Int
    let dependencies: [String]
    let commands: [String]
}

/// Result of package installation
enum InstallResult {
    case success
    case alreadyInstalled
    case downloadFailed(String)
    case verificationFailed(String)
    case extractionFailed(String)
    case registrationFailed(String)
}

// MARK: - Package Manager

/// Actor-based package manager for thread-safe package operations
public actor PackageManager {

    // MARK: - Singleton

    static let shared = PackageManager()

    // MARK: - Properties

    /// Package installation prefix
    let prefixURL: URL

    /// Configuration directory
    let configURL: URL

    /// Frameworks directory
    let frameworksURL: URL

    /// Package registry file
    let registryURL: URL

    /// Catalog URL for package metadata
    let catalogURL: URL

    /// URL session for downloads
    private let urlSession: URLSession

    // MARK: - Initialization

    init() {
        // Set up paths following the PREFIX model
        let documentsPath = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask).first!
        let libraryPath = FileManager.default.urls(for: .libraryDirectory, in: .userDomainMask).first!

        // PREFIX = ~/Library/ashell (binaries, frameworks)
        self.prefixURL = libraryPath.appendingPathComponent("ashell")
        self.frameworksURL = prefixURL.appendingPathComponent("Frameworks")

        // Config = ~/Documents/.ashell (configs, data)
        self.configURL = documentsPath.appendingPathComponent(".ashell")

        // Registry tracks installed packages
        self.registryURL = configURL.appendingPathComponent("packages.json")

        // Default catalog (can be overridden via environment)
        if let catalogEnv = getenv("ASHELL_CATALOG_URL"),
           let catalogString = String(cString: catalogEnv).addingPercentEncoding(withAllowedCharacters: .urlQueryAllowed),
           let catalog = URL(string: catalogString) {
            self.catalogURL = catalog
        } else {
            self.catalogURL = URL(string: "https://rudironsoni.github.io/a-shell-next/catalog")!
        }

        // Configure URL session with reasonable timeouts
        let config = URLSessionConfiguration.default
        config.timeoutIntervalForRequest = 60
        config.timeoutIntervalForResource = 300
        self.urlSession = URLSession(configuration: config)

        // Ensure directories exist
        try? FileManager.default.createDirectory(at: frameworksURL, withIntermediateDirectories: true)
        try? FileManager.default.createDirectory(at: configURL, withIntermediateDirectories: true)
    }

    // MARK: - Public API

    /// Install a package by name
    func install(package name: String, version: String? = nil) async -> InstallResult {
        ios_log("Installing package: \(name)")

        // Check if already installed
        if await isInstalled(name: name) {
            ios_log("Package \(name) is already installed")
            return .alreadyInstalled
        }

        // Fetch package metadata
        guard let metadata = await fetchPackageMetadata(name: name) else {
            return .downloadFailed("Package not found in catalog: \(name)")
        }

        // Check version if specified
        if let requestedVersion = version, requestedVersion != metadata.version {
            return .downloadFailed("Version \(requestedVersion) not available (latest: \(metadata.version))")
        }

        // Check dependencies
        for dependency in metadata.dependencies {
            if !await isInstalled(name: dependency) {
                ios_log("Installing dependency: \(dependency)")
                let depResult = await install(package: dependency)
                if case .downloadFailed(let error) = depResult {
                    return .downloadFailed("Failed to install dependency \(dependency): \(error)")
                }
            }
        }

        // Download package
        guard let packageURL = await downloadPackage(metadata: metadata) else {
            return .downloadFailed("Failed to download \(name)")
        }

        // Verify checksum
        if !await verifyPackage(packageURL: packageURL, expectedHash: metadata.sha256) {
            try? FileManager.default.removeItem(at: packageURL)
            return .verificationFailed("Checksum mismatch for \(name)")
        }

        // Extract and install
        let frameworkURL = frameworksURL.appendingPathComponent("\(name).framework")
        if !await extractPackage(packageURL: packageURL, to: frameworkURL) {
            try? FileManager.default.removeItem(at: packageURL)
            return .extractionFailed("Failed to extract \(name)")
        }

        // Register commands
        if !await registerCommands(for: metadata, frameworkURL: frameworkURL) {
            try? FileManager.default.removeItem(at: frameworkURL)
            return .registrationFailed("Failed to register commands for \(name)")
        }

        // Update registry
        let package = InstalledPackage(
            name: name,
            version: metadata.version,
            installDate: Date(),
            frameworkPath: frameworkURL.path,
            commands: metadata.commands
        )
        await addToRegistry(package: package)

        // Cleanup
        try? FileManager.default.removeItem(at: packageURL)

        ios_log("Successfully installed \(name) v\(metadata.version)")
        return .success
    }

    /// Remove an installed package
    func remove(package name: String) async -> Bool {
        ios_log("Removing package: \(name)")

        guard let package = await getPackage(name: name) else {
            ios_log("Package \(name) is not installed")
            return false
        }

        // Unregister commands
        await unregisterCommands(for: package)

        // Remove framework
        let frameworkURL = URL(fileURLWithPath: package.frameworkPath)
        try? FileManager.default.removeItem(at: frameworkURL)

        // Remove from registry
        await removeFromRegistry(name: name)

        ios_log("Successfully removed \(name)")
        return true
    }

    /// List installed packages
    func listInstalled() async -> [InstalledPackage] {
        return await loadRegistry()
    }

    /// Search for packages in catalog
    func search(query: String) async -> [PackageMetadata] {
        // For now, return all packages that match the query
        // In production, this would query a proper package index
        return await fetchAllPackages().filter { pkg in
            pkg.name.contains(query) || pkg.description.contains(query)
        }
    }

    /// Update package catalog
    func updateCatalog() async -> Bool {
        // Download latest catalog
        // For now, this is a placeholder
        ios_log("Updating package catalog from \(catalogURL)")
        return true
    }

    /// Get package info
    func info(package name: String) async -> (InstalledPackage?, PackageMetadata?) {
        let installed = await getPackage(name: name)
        let metadata = await fetchPackageMetadata(name: name)
        return (installed, metadata)
    }

    // MARK: - Private Methods

    private func isInstalled(name: String) async -> Bool {
        return await getPackage(name: name) != nil
    }

    private func getPackage(name: String) async -> InstalledPackage? {
        let registry = await loadRegistry()
        return registry.first { $0.name == name }
    }

    private func loadRegistry() async -> [InstalledPackage] {
        guard FileManager.default.fileExists(atPath: registryURL.path),
              let data = try? Data(contentsOf: registryURL),
              let registry = try? JSONDecoder().decode([InstalledPackage].self, from: data) else {
            return []
        }
        return registry
    }

    private func addToRegistry(package: InstalledPackage) async {
        var registry = await loadRegistry()
        registry.removeAll { $0.name == package.name }
        registry.append(package)
        saveRegistry(registry)
    }

    private func removeFromRegistry(name: String) async {
        var registry = await loadRegistry()
        registry.removeAll { $0.name == name }
        saveRegistry(registry)
    }

    private func saveRegistry(_ registry: [InstalledPackage]) {
        guard let data = try? JSONEncoder().encode(registry) else { return }
        try? data.write(to: registryURL)
    }

    private func fetchPackageMetadata(name: String) async -> PackageMetadata? {
        // For now, return a hardcoded set of packages
        // In production, this would fetch from the catalog URL
        let packages = await fetchAllPackages()
        return packages.first { $0.name == name }
    }

    private func fetchAllPackages() async -> [PackageMetadata] {
        // Built-in packages for the bootstrap set
        // In production, this would be fetched from catalog.json
        return [
            PackageMetadata(
                name: "hello",
                version: "1.0.0",
                description: "Simple greeting command",
                homepage: "https://github.com/rudironsoni/a-shell-next",
                sha256: "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
                size: 1024,
                dependencies: [],
                commands: ["hello"]
            ),
            PackageMetadata(
                name: "coreutils-minimal",
                version: "9.4",
                description: "Minimal GNU coreutils (basename, dirname, readlink, realpath, mktemp)",
                homepage: "https://www.gnu.org/software/coreutils/",
                sha256: "",
                size: 2048,
                dependencies: [],
                commands: ["basename", "dirname", "readlink", "realpath", "mktemp"]
            )
        ]
    }

    private func downloadPackage(metadata: PackageMetadata) async -> URL? {
        let downloadURL = catalogURL.appendingPathComponent("\(metadata.name)-\(metadata.version).tar.gz")
        let tempURL = FileManager.default.temporaryDirectory.appendingPathComponent(UUID().uuidString)

        do {
            let (data, response) = try await urlSession.data(from: downloadURL)

            guard let httpResponse = response as? HTTPURLResponse,
                  httpResponse.statusCode == 200 else {
                ios_log("Download failed: \(response)")
                return nil
            }

            try data.write(to: tempURL)
            return tempURL
        } catch {
            ios_log("Download error: \(error)")
            return nil
        }
    }

    private func verifyPackage(packageURL: URL, expectedHash: String) async -> Bool {
        guard let data = try? Data(contentsOf: packageURL) else { return false }

        // Compute SHA256
        let hash = SHA256.hash(data: data)
        let computedHash = hash.compactMap { String(format: "%02x", $0) }.joined()

        return computedHash == expectedHash
    }

    private func extractPackage(packageURL: URL, to frameworkURL: URL) async -> Bool {
        // Use ArchiveExtractor (libarchive-based with tar fallback)
        let extractor = ArchiveExtractor.shared

        do {
            // Remove existing framework
            try? FileManager.default.removeItem(at: frameworkURL)

            // Extract using ArchiveExtractor
            let _ = try await extractor.extract(
                from: packageURL,
                to: frameworkURL.deletingLastPathComponent()
            )

            return true
        } catch {
            ios_log("Extraction error: \(error)")
            return false
        }
    }

    private func registerCommands(for metadata: PackageMetadata, frameworkURL: URL) async -> Bool {
        // Load commands.plist from framework
        let plistURL = frameworkURL.appendingPathComponent("commands.plist")

        guard FileManager.default.fileExists(atPath: plistURL.path),
              let plistData = try? Data(contentsOf: plistURL),
              let plist = try? PropertyListSerialization.propertyList(from: plistData, format: nil) as? [String: [String]] else {
            ios_log("Failed to load commands.plist from \(plistURL)")
            return false
        }

        // Register with ios_system via addCommandList
        // This requires the C API from ios_system
        let plistPath = strdup(plistURL.path)
        addCommandList(plistPath)
        free(plistPath)

        return true
    }

    private func unregisterCommands(for package: InstalledPackage) async {
        // Remove commands from ios_system
        for command in package.commands {
            let commandName = strdup(command)
            replaceCommand(commandName, nil)
            free(commandName)
        }
    }
}

// MARK: - C Interop

/// C-compatible logging function
private func ios_log(_ message: String) {
    fputs("[pkg] \(message)\n", thread_stderr)
}

// MARK: - Helper Functions

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

/// Print to thread stdout
private func printToStdout(_ message: String) {
    fputs(message, thread_stdout)
}
