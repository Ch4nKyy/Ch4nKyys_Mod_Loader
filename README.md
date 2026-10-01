# Ch4nKyy's Mod Loader

For lack of a better name.

DLL mod loader for Windows x64 using a version.dll proxy.

Find it on [Nexusmods](https://www.nexusmods.com/baldursgate3/mods/TODO).

Usage:

- Put version.dll next to your game's exe.
- Create a directory called NativeMods next to the exe and put your DLL mods in there.

## Notes

Instead of forwarding the exports to the system version.dll, it forwards them to kernelbase.dll.
Loading the system version.dll by name is complicated, when the proxy version.dll is already loaded.
kernelbase.dll offers all the needed exports anyway.

Mods are loaded synchronously from a hook on the executable's \_initterm_e import. This keeps
LoadLibrary calls out of the mod loader's DllMain while still ensuring that mods are initialized
before the game continues past \_initterm_e. It requires the executable to import \_initterm_e from
api-ms-win-crt-runtime-l1-1-0.dll.

Loaded mods may optionally export void Init(). If present, the loader calls it once immediately
after successfully loading the DLL. Using Init() is recommended over performing substantial
initialization inside DllMain or deferring initialization to a worker thread, as it keeps
initialization synchronous and outside the loader lock.

The mods are loaded in case-insensitive, lexicographic order.

## BG3 Notes

This loader was originally created for Baldur’s Gate 3. The popular NativeModLoader works by
replacing the game’s bink2w64.dll, which can be replaced when verifying the game files or during
game updates. Then the mods stop working. This loader instead uses a version.dll proxy. Since BG3
loads this DLL but does not ship its own copy, this avoids the issue.

It can be used alongside NativeModLoader. It automatically detects when NativeModLoader is present
and, in that case, does not load any mods itself. This avoids both loaders hooking \_initterm_e and
calling Init() on the same mods.

## Building

```
git clone https://github.com/Ch4nKyy/Ch4nKyys_Mod_Loader.git
cd Ch4nKyys_Mod_Loader
.\build-release.ps1
```

Requires [CMake](https://cmake.org/) on your PATH.

publish.ps1 requires [7zip](https://www.7-zip.org/) installed to the default directory.
