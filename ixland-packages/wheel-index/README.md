# a-Shell Python Wheel Index

Custom wheel index for iOS-compatible Python packages.

## Structure

```
wheel-index/
├── index.html              # Package index
├── packages/               # Wheel files
│   ├── numpy/
│   │   └── numpy-1.26.0-ios_13_0_arm64.whl
│   ├── pillow/
│   └── ...
└── metadata/               # Package metadata
    └── numpy-1.26.0.json
```

## Platform Tags

a-Shell uses custom platform tags for iOS:
- `ios_13_0_arm64` - iOS 13.0+, arm64 devices
- `ios_13_0_universal2` - Universal binary (future)

## Adding Packages

1. Build wheel for iOS using mobile-forge
2. Run post-processor:
   ```bash
   ../scripts/post-process-wheel.sh numpy-1.26.0-cp312-cp312-ios_13_0_arm64.whl
   ```
3. Copy processed wheel to `packages/`
4. Update `index.html`

## Using with pip

Configure pip to use this index:
```bash
pip config set global.index-url https://ashell.example.com/wheel-index/
pip install numpy
```

## Available Packages

| Package | Version | Status |
|---------|---------|--------|
| numpy | 1.26.0 | ✅ Available |
| pillow | 10.0.0 | 🔄 Building |
| ... | ... | ... |
