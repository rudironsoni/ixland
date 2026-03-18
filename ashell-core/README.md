# ashell-core

Core command implementations for a-Shell. This module contains the Unix commands that users interact with, implemented in Swift and C.

## Structure

```
ashell-core/
├── Sources/
│   ├── Commands/           # Swift-implemented commands
│   │   └── pkg.swift       # Package manager command
│   └── PackageManager/     # Runtime package management
│       ├── PackageManager.swift
│       └── ArchiveExtractor.swift
├── Tests/                  # Unit tests
├── include/                # Public C headers
└── README.md
```

## Commands

### pkg

Package manager for installing/removing XCFramework-based packages at runtime.

**Subcommands:**
- `pkg install <name>` - Install a package
- `pkg remove <name>` - Remove a package
- `pkg list` - List installed packages
- `pkg search <query>` - Search catalog
- `pkg info <name>` - Show package info
- `pkg update` - Update catalog

## PackageManager

Actor-based package manager:

```swift
let manager = PackageManager.shared
let result = await manager.install(package: "hello")
```

## Dependencies

- ios_system (ashell-system) - Platform layer
- Foundation - Swift standard library
- CryptoKit - Checksum verification

## Notes

- This is for **runtime** package management
- For **build-time** package creation, see `ashell-packages/`
