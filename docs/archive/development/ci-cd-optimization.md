# CI/CD Optimization for 2000 Minutes/Month

This document explains how the GitHub Actions workflow is optimized to stay within the free tier limit.

## Current Limits

- **Free tier**: 2000 macOS minutes/month
- **macOS runner cost**: 10x Linux minutes (1 macOS min = 10 Linux min)
- **Effective limit**: ~200 minutes of macOS build time

## Optimization Strategies

### 1. Trigger Only When Needed

```yaml
on:
  push:
    tags: ['v*', 'package-*']  # Only on release tags, NOT every push
  pull_request:
    paths: ['ashell-packages/**']  # Only if packages changed
```

**Savings**: No builds on documentation changes, config updates, etc.

### 2. Cancel Redundant Runs

```yaml
concurrency:
  group: ${{ github.workflow }}-${{ github.ref }}
  cancel-in-progress: true
```

**Savings**: If you push multiple times quickly, only the latest runs.

### 3. Detect Changes (Ubuntu = Free)

```yaml
detect-changes:
  runs-on: ubuntu-latest  # FREE
```

Only triggers macOS build if packages actually changed.

**Savings**: 0 macOS minutes for non-package changes.

### 4. Validate Before macOS (Ubuntu = Free)

```yaml
validate:
  runs-on: ubuntu-latest  # FREE
  needs: detect-changes
```

Catches syntax errors before spending macOS minutes.

**Savings**: No wasted minutes on broken builds.

### 5. Aggressive Caching

```yaml
- uses: actions/cache@v4
  with:
    path: ashell-packages/.build/downloads
```

Caches downloaded sources across builds.

**Savings**: ~2-5 minutes per build on cache hit.

### 6. Build Only Changed Packages

```yaml
# On PRs, only build packages that changed
if: git diff --name-only | grep "ashell-packages/<pkg>"
```

**Savings**: Build 1 package instead of all packages.

### 7. Limit Concurrent Jobs

```yaml
strategy:
  max-parallel: 2  # Only 2 macOS jobs at once
```

Prevents parallel job explosion.

### 8. Hard Timeouts

```yaml
timeout-minutes: 15  # Kill hanging builds
```

Prevents runaway builds from consuming all minutes.

### 9. Short Artifact Retention

```yaml
retention-days: 7  # Auto-delete after 7 days
```

Saves storage (though storage is separate from minutes).

## Estimated Usage

| Scenario | Time | Minutes |
|----------|------|---------|
| Hello package build (cached) | ~3 min | 30 |
| Hello package build (no cache) | ~8 min | 80 |
| 5 packages (cached, parallel=2) | ~8 min | 80 |
| 5 packages (no cache, parallel=2) | ~20 min | 200 |

**Monthly budget**: 2000 minutes = ~200 macOS minutes

**Safe usage pattern**:
- 10-20 package builds per month
- Or 2-3 full rebuilds of all packages

## Manual Trigger for Control

Use workflow dispatch for full control:

```bash
# Only build specific packages
curl -X POST \
  -H "Authorization: token $GITHUB_TOKEN" \
  -d '{"ref":"main","inputs":{"packages":"hello,coreutils-minimal"}}' \
  https://api.github.com/repos/rudironsoni/a-shell-next/actions/workflows/release.yml/dispatches
```

## Cost Monitoring

Monitor usage at:
https://github.com/settings/billing

Set up alerts when approaching limits.

## Future Optimizations

If minutes run low:

1. **Self-hosted Mac runner** (one-time hardware cost)
2. **Build only on demand** (manual triggers only)
3. **Batch releases** (weekly instead of per-merge)
4. **Selective builds** (only build stable packages)

## Recommendations

1. **Enable branch protection** - Require PR reviews before builds
2. **Squash merges** - Reduces number of commits that trigger builds
3. **Schedule builds** - Use cron to build weekly instead of per-push
4. **Use Linux for testing** - More script validation in Docker
