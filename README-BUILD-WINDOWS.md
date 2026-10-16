# OpenKO Client — Windows Build Instructions (for opencode agent)

This folder is a complete, ready-to-build checkout of the OpenKO **client** (`KnightOnline` / `WarFare`).
It was prepared on a Mac and copied to this Windows PC. Your job (the opencode agent running here):
**check/install the prerequisites, build the client, assemble the runnable folder, and verify it.**
Follow the steps below in order. Ask the user before installing large software if it is unclear.

The backend server is **already running** on another machine and does not need to be built here.

---

## 1. Goal (definition of done)

- `assets\Client\KnightOnLine.exe` exists (built from this source).
- Running it from `assets\Client\` opens the game client and can reach the login server.
- Nothing in `assets\Client\` (the game data) is modified except `Server.ini`, which already has the correct server IP.

---

## 2. Server connection info (already configured)

- Login / VersionManager: `192.168.0.114:15100` (TCP)
- Game server (Ebenezer): `192.168.0.114:15001` (TCP), AI internal 10020, UDP 8888
- `assets\Client\Server.ini` already contains `[Server] Count=1` / `IP0=192.168.0.114`.
- The client loads the game-server IP dynamically from the login server (it must be launched from `assets\Client\`).

> If the server IP ever changes, edit `assets\Client\Server.ini` (`IP0=`). Do not edit anything else there.

---

## 3. Prerequisites — check then install what is missing

The client is **Windows-only (MSVC / MSBuild, Visual Studio 2022)**. Required:

| Tool | Why | Check |
|---|---|---|
| **Visual Studio 2022** (17.13+) with workload **"Desktop development with C++"** | Compiler (MSVC v143) + Windows SDK + MSBuild. `.slnx` needs 17.13+. | `vswhere -latest -find MSBuild\**\Bin\MSBuild.exe` |
| **Git for Windows** (on PATH) | The build system syncs git submodules during the build | `git --version` |
| **DirectX End-User Runtime (June 2010)** | Runtime `d3dx9_*.dll` if the client fails to start with a DirectX DLL error | after first run |

### Check commands (PowerShell)

```powershell
& "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe
git --version
```

### Install if missing (winget)

```powershell
winget install --id Git.Git -e
winget install --id Microsoft.VisualStudio.2022.Community -e --override "--quiet --wait --add Microsoft.VisualStudio.Workload.NativeDesktop --includeRecommended"
```

After installing VS, close and reopen the terminal so MSBuild is discoverable.

> MFC (the "C++ MFC for latest v143 build tools" component) is **not** needed for the client
> (`Client.slnx` / `WarFare`). It is only needed for the optional Launcher/Option tools (`ClientTools.slnx`).

---

## 4. Submodules

This was cloned with `--recursive`, so `deps\*` and `assets\Client\` (the game data, `ko-client-assets` v1.298.0)
should already be present. Verify:

```powershell
git submodule status --recursive
```

If any are missing/empty, fetch them (needs internet):

```powershell
git submodule update --init --recursive
```

---

## 5. Build the client

Preferred: run the provided script (it locates MSBuild and copies the exe into place):

```powershell
.\build_client_Release_Win32.bat
```

Or do it manually. From a **"Developer PowerShell/Command Prompt for VS 2022"** at the repo root:

```powershell
msbuild /m /p:Configuration=Release /p:Platform=Win32 Client.slnx
```

- Config/Platform: **Release / Win32** (the client is 32-bit).
- This also builds the dependency submodules first (boost, dx9sdk, libjpeg, mpg123, openal-soft, spdlog, …)
  via the fetch wrappers — the first build can take a while.
- Do **not** pass `/p:NO_CLIENT_ASSETS=1` — we want the client data present.
- Build output: `bin\Release-Win32\KnightOnLine.exe` (target name is `KnightOnLine`).

You can also just open `Client.slnx` in Visual Studio 2022, pick `Release`/`Win32`, and Build.

---

## 6. Assemble the runnable folder

The client uses its current working directory as the data root, so the exe must run **from
`assets\Client\`**, alongside the data files (`Chr`, `Data`, `Item`, `Zones`, `Snd`, `UI`, `mss32.dll`, …).

`build_client_Release_Win32.bat` already copies `bin\Release-Win32\KnightOnLine.exe` into `assets\Client\`.
If you built manually, run:

```powershell
copy /Y "bin\Release-Win32\KnightOnLine.exe" "assets\Client\KnightOnLine.exe"
```

---

## 7. Run & verify

```powershell
.\run_client.bat
```

or manually:

```powershell
cd assets\Client
.\KnightOnLine.exe
```

Verify connectivity to the server first:

```powershell
Test-NetConnection 192.168.0.114 -Port 15100
Test-NetConnection 192.168.0.114 -Port 15001
```

Expected: the client starts, shows the login screen, and can select the server list entry.

---

## 8. Troubleshooting

- **`msbuild` not found** — rerun from the VS 2022 Developer Prompt, or close/reopen the terminal after installing VS.
- **`.slnx` not recognized** — update Visual Studio to 17.13+. As a fallback, open the individual project
  `src\Client\WarFare\WarFare.vcxproj` in VS and build it (Release/Win32).
- **Missing `d3dx9_*.dll` / DirectX error on launch** — install the DirectX End-User Runtime (June 2010).
- **Build fails fetching a submodule** — ensure internet access, then `git submodule update --init --recursive`.
- **Blank/black screen or missing data** — make sure you launched the exe from inside `assets\Client\`,
  not from `bin\Release-Win32\`.
- Version: assets are **1.298/1.299**; the server reports latest version 1299 — they match.

---

## 9. Notes

- This is academic/learning-only software; the client is intentionally close to the official 1.298 client.
- Do not change game assets or the network protocol.
- Only edit `assets\Client\Server.ini` for the server address; leave everything else in `assets\Client\` untouched.
