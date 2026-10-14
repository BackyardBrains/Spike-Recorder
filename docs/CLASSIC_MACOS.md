# Spike Recorder Classic for macOS

This update preserves the legacy C++ application and its historical Mac App
Store identity, `com.backyardbrains.SpikeRecorder` (app 972173310). It changes the
Mac application, menu, and window names to **Spike Recorder Classic**, and bumps
the public version from 1.9.38 to **1.9.39**, build **87877549**.

The build number also sorts after the 4.0.1 TestFlight builds already uploaded to
this same record. Recording paths, formats, DSP, and the bundle identifier are
unchanged. Windows and Linux application names and versions are unchanged.

## Build

The bundled native libraries are Intel-only. Both Xcode configurations explicitly
build `x86_64`. This update requires **macOS 12.0 or later**, the minimum supported
by the installed Xcode 27 toolchain. Apple silicon machines require Rosetta to run
this Intel application; this is not a native Apple silicon port.

From the repository root, with Xcode selected:

```sh
xcodebuild -project SpikeRecorder.xcodeproj \
  -scheme SpikeRecorder -configuration Release \
  -destination 'generic/platform=macOS' \
  -derivedDataPath build/ClassicDerivedData \
  CODE_SIGNING_ALLOWED=NO build
```

The compiled app is `build/ClassicDerivedData/Build/Products/Release/Spike Recorder Classic.app`.

For an unsigned archive suitable for inspection and later signing:

```sh
xcodebuild -project SpikeRecorder.xcodeproj \
  -scheme SpikeRecorder -configuration Release \
  -destination 'generic/platform=macOS' \
  -derivedDataPath build/ClassicDerivedData \
  -archivePath 'build/Spike Recorder Classic.xcarchive' \
  CODE_SIGNING_ALLOWED=NO archive
```

These commands produce **unsigned** outputs, not an App Store submission or a
notarized download. Distribution requires a working Apple signing key, export,
Apple validation, and runtime checks. The existing macOS signing identities in
the project can be overridden for the intended signing/export environment.

## Verification on 2026-10-14

- Xcode 27.0 (27A266a), macOS SDK 27.0: Release build and unsigned archive passed.
- Built bundle: Classic name, version 1.9.39, build 87877549, historical bundle ID,
  minimum macOS 12.0, Intel architecture.
- Required non-system dynamic libraries resolve within the bundle. Legacy optional
  weak dependencies `for_mac_ftn_alloc.dylib` and `webp.framework` are absent.
- The legacy compiler warnings remain; this update does not claim a warning-free build.
- Signed archive was attempted but the Mac login keychain returned
  `errSecInternalComponent`. No signed distribution package or Apple upload was produced.
- Runtime/device testing is pending: the Apple silicon build machine has no
  Rosetta installation. No recording or hardware behavior is claimed as tested.

The public App Store listing has not been renamed by these source changes.
