# G0-G2 implementation report

## Scope

- G0 donor freeze recorded.
- G1 linux compatibility lane scaffolding added.
- G2 host-boundary enforcement wired via runtime handle validation.

## Files and directories added

- ixland-system/runtime/linux_host
- ixland-system/compatibility/linux/{abi,loader,runtime,distro,projection,signal_translation,fs_translation,proc_views,dev_views,emulation/aarch64,emulation/common,tcti,tests}
- ixland-system/observability
- ixland-app/{Workspace,SessionUI,Terminal,FileAccess,Credentials,Remote,Compatibility/LinuxSessionAdapter}

## Build integration

- CMake targets added in ixland-system/CMakeLists.txt:
  - ixland_linux_compat_host
  - ixland_linux_compat_abi
  - ixland_linux_compat_loader
  - ixland_linux_compat_emu_common
  - ixland_linux_compat_emu_aarch64
  - ixland_linux_compat_tcti
  - ixland_linux_compat_runtime
  - ixland_linux_compat_tests

## Host boundary enforcement

- ixland_linux_runtime_bootstrap now rejects null handles, invalid magic, or inactive handles.
- runtime handle issued only by ixland_linux_compatibility_host_get_runtime_handle.

## Tests

- compatibility/linux/tests/test_linux_compat_main.c
- compatibility/linux/tests/test_linux_compat_abi.c
- compatibility/linux/tests/test_linux_host_boundary.c
- iOS execution path wired in CMake via `xcrun simctl spawn booted` for `linux-compat-tests`
- `ixland-core-tests` binary builds and runs via simulator spawn; existing core test failures remain (`exec_native_happy_path`, `exec_cloexec_behavior`) and long-running signal suite execution requires follow-up.

## Rename gate proof

- Added `docs/compatibility-linux/iox-to-ixland-rename-report.md`.
- Tracked path legacy hits: 0.
- Tracked content legacy hits (non-exception): 0.
- Naming check `legacy name eradication checks`: pass (with documented exceptions).

## Pending for G3+

- No canonical semantic ownership uplifts performed.
- No session/workspace integration.
- No shell readiness exposure.
