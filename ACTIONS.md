# Release workflow

This document describes the CI/CD pipeline for building and distributing skald-godot.

## Overview

```
cog bump --auto
    |
    v
cocogitto creates tag (v0.1.0), pushes commit + tag
    |
    v
GitHub Actions triggers on v* tag
    |
    ├── Build macOS (universal dylib, debug + release)
    ├── Build Linux (x86_64 .so, debug + release)
    └── Build Windows (x86_64 .dll, debug + release)
    |
    v
Package job:
    ├── Creates GitHub Release with skald-godot.zip
    └── Pushes built binaries to `release` branch
```

## Triggering a release

1. Make sure all changes are committed on `main`.
2. Run `cog bump --auto` (or `--major` / `--minor` / `--patch`).
3. Cocogitto creates a bump commit, generates the changelog, tags the release, and pushes everything (via `post_bump_hooks` in `cog.toml`).
4. The tag push triggers the GitHub Actions workflow automatically.

## What the workflow does

### Build matrix

The workflow (`.github/workflows/build.yml`) builds on three runners in parallel:

| Platform | Runner | Architecture | Output |
|---|---|---|---|
| macOS | `macos-latest` | universal (arm64 + x86_64) | `.dylib` |
| Linux | `ubuntu-22.04` | x86_64 | `.so` |
| Windows | `windows-latest` | x86_64 | `.dll` |

Each platform builds both `template_debug` and `template_release` targets, producing two binaries per platform (six total).

### Package & release

After all three builds succeed:

1. **GitHub Release** — A `skald-godot.zip` containing `addons/skald/` (gdextension descriptor + all platform binaries) is attached to the release. Users download and extract into their project root.

2. **`release` branch** — The built binaries are force-pushed to a `release` branch with binaries committed. This branch exists for Godot AssetLib compatibility, since AssetLib pulls directly from a branch rather than from release artifacts.

### Manual trigger

The workflow can also be triggered manually via `workflow_dispatch` from the Actions tab, without creating a tag. This is useful for testing the build pipeline. In this case, only the build artifacts are uploaded (no GitHub Release or branch update).

## Distribution channels

### GitHub Releases (primary)

Users download `skald-godot.zip` from the Releases page and extract it into their Godot project root. The zip contains `addons/skald/` with the gdextension descriptor and all platform binaries — ready to use.

### Godot AssetLib (future)

Once the first release is published, the addon can be submitted to the [Godot Asset Library](https://godotengine.org/asset-library/). The submission should point at the `release` branch, which contains committed binaries. Users will be able to install directly from within the Godot editor.

## Updating the skald core library

When the upstream skald repo has changes to pull in:

```bash
cd skald
git pull origin master
cd ..
git add skald
git commit -m "feat: update skald to vX.Y.Z"
cog bump --auto
```

The tag push triggers the full build-and-release pipeline.
