# AGENTS.md — OpenKO client build task

When opencode is started in this folder on the Windows PC, read and follow
**`README-BUILD-WINDOWS.md`** in this same directory and carry out the task described there.

Summary of the task:
1. Check prerequisites (Visual Studio 2022 17.13+ with "Desktop development with C++", Git). Install what is missing (ask the user before large installs).
2. Ensure git submodules are present (`git submodule status --recursive`; fetch if needed).
3. Build the client: `Release`/`Win32` via `Client.slnx`
   (or run `build_client_Release_Win32.bat`).
4. Copy `bin\Release-Win32\KnightOnLine.exe` into `assets\Client\`.
5. Verify and launch with `run_client.bat` (working dir must be `assets\Client\`).

The game server is already running remotely at `192.168.0.114` (login 15100, game 15001);
`assets\Client\Server.ini` already points at it. Do not modify anything else under `assets\Client\`.

Work autonomously, report progress, and stop to ask if a step is blocked or a large download/install is required.
