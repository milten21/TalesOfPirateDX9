# TalesOfPirateDX9

**About This Repository**
**This repository contains the source code for _Tales of Pirate_, updated to DirectX 9. Over the past few years, we collaborated with several freelancers to modernize the engine from its original DX8 foundation.**

**In response to certain groups within the community profiting from leaked versions of this code, we have decided to make the source public for the benefit of all developers. Please note that we do not offer support for future bugs or technical issues.**

While the core DX9 upgrade is functional, it originated from a separate development branch; you may encounter minor inconsistencies. This remains an excellent starting point for new projects. Finally, we are not responsible for any legacy bugs or exploits present in the original code, as these were inherited from previous owners.

### **Credits & Copyright**

**Ownership & Contributions** All rights to the engine upgrades and source modifications belong to **[SatisfyTeam](https://satisfy.live/)**, **Mothannkh**, and the private benefactors who funded the project’s development but chose to remain anonymous.

**Terms of Use** This source is provided to the community for development purposes. While we encourage innovation, we ask that users respect the work put in by the team and the contributors who made this progress possible.

### CURRENT SOURCE SPECIFICATIONS

**Core Engine & Networking**

- Upgraded from DirectX 8 to DirectX 9
- Client and Server built with VS 2026 (C++14)
- TopNet source integration
- RSA/AES connection encryption (Client/Server)
- Encrypted .clu files
- Core packets updated to use Smart Pointers
- Anti-WPE and Anti-DDoS protection
- Anti-Dupe system

**Gameplay Systems**

- Offline Stalls and Offline Mode
- Full Mount System
- Guild Bank with transaction logs
- AuthorizedGM system
- Reworked Jackpot system
- In-game Shop remade (Classic style, 9 items per page)

**User Interface & Features**

- Multi-resolution support
- 60 FPS toggle option
- HP and SP bars displayable as values or percentages
- World Map with Zoom In/Out
- Drop Info with integrated Drop Filter
- Right-click inventory menu options
- Friend/Enemy combat modes
- Visibility toggles for Effects, Apps, and Mounts

**Scripting & Structure**

- Restructured LUA folders for better organization
- Balanced Vanilla LUA setup
- Simplified file structure with intuitive variable and function names
- Modular design for easy code migration (copy/paste friendly)
  vs config file
  **[Here](https://drive.google.com/file/d/1GR8GdSNe-UocUNpkvChoHBaqZJaTcuuS/view)**

### Building the servers

The four servers (AccountServer, GroupServer, GateServer, GameServer), their shared libraries, Crypto++ and LuaJIT build from one CMake project. The client keeps its own solution, `sources/talesofpirates.sln`.

Requirements: Visual Studio 2026 with the C++ workload and ATL/MFC, CMake 4.2 or newer (the copy bundled with Visual Studio is fine), and a [vcpkg](https://github.com/microsoft/vcpkg) checkout whose path contains no spaces, pointed to by the `VCPKG_ROOT` environment variable. vcpkg builds ICU from `vcpkg.json` on the first configure; the preset installs it under `%VCPKG_ROOT%/installed-tales-servers`.

```
cmake --preset windows-x86
cmake --build --preset windows-x86-debug
cmake --build --preset windows-x86-release
```

The preset generates `build/windows-x86/TalesOfPirateServers.slnx` for Visual Studio, with GameServer as the startup project and `server/<Name>` as its debugging working directory. Executables land in `sources/Server/<Name>/Bin` (`<Name>.exe` for Release, `<Name>_D.exe` for Debug) (ICU is linked statically, so there are no runtime DLLs to copy), so `sources/symlinks.bat` keeps working.

### Running the servers on Linux with Docker

The servers also build with GCC for 32-bit Linux. The `Dockerfile` builds them on `i386/debian:bookworm-slim`, runs the tests, and produces a runtime image with the binaries, their locale bundles, unixODBC with the PostgreSQL driver, and nothing else: no game data and no credentials. All four servers run on Linux. Compose starts one GameServer, `game00`, by default, and all four with the `full` profile.

```
cp .env.example .env              # then set real passwords in .env
docker compose up --build         # PostgreSQL, seeded from postgresql/, plus account, group, gate and game00
docker compose --profile full up  # the same plus game01, game02 and game03
docker attach tales-gate-1        # a server console; detach with Ctrl+P Ctrl+Q
docker attach tales-game00-1      # GameServer console: `status`, or any line of Lua
docker compose stop               # graceful stop, the servers exit with code 0
```

GateServer publishes its client port (`GATE_PUBLISHED_PORT`, default 1973) and the ShowRange UDP port (`GATE_SHOWRANGE_PUBLISHED_PORT`, default 1976; set `GATE_SHOWRANGE=0` to turn the listener off). Point the client at the host's address and that port. PostgreSQL is published on `POSTGRES_PUBLISHED_IP:POSTGRES_PUBLISHED_PORT` (default `127.0.0.1:5432`), so database tools on the host can connect as `postgres` with `POSTGRES_PASSWORD`, or as `pko_account` and `pko_game`; set `POSTGRES_PUBLISHED_IP=0.0.0.0` to accept connections from other machines too. The other servers are reachable only inside the compose network. The servers find each other by compose service name and start in any order; each one retries its upstream link until it comes up. GroupServer logs in to AccountServer with `GROUP_NAME` and `GROUP_ACCOUNT_PASSWORD`, which both services read from the environment. GroupServer's Lua scripts and `ChaNameFilter.txt` are bind-mounted read-only from `server/GroupServer`.

Each GameServer service runs one instance configuration and logs in to GateServer by service name: `game00` runs `GameServer00.cfg` (garner, lonetower, teampk, PKmap), and the `full` profile adds `game01` (`GameServer01.cfg`: magicsea, jialebi), `game02` (`GameServer02.cfg`: darkblue, eastgoaf) and `game03` (`GameServer03.cfg`: abandonedcity, abandonedcity2, abandonedcity3, darkswamp, puzzleworld, puzzleworld2). The instances share the image, the bind mounts below and the stop grace period, and each has its own log volume and attachable console (`docker attach tales-game01-1`). To add an instance, add a `GameServerNN.cfg.template` under `docker/templates/GameServer/`, its `GAMENN_INFO_PASSWORD` to `.env` and the `x-game` environment in `compose.yaml`, a `gameNN` service with its `gameNN-log` volume, and the instance to `game_servers` in `docker/smoke-test.sh`. Every instance bind-mounts `server/GameServer/resource` and `server/GameServer/addons` read-write, because the scripts keep their own data files there, and `cmd.cfg` read-only. The servers run as the image's `tales` user (uid 999), so on a native Linux host those two folders must be writable by that uid; Docker Desktop does not enforce this. Edits to Lua scripts and tables apply on restart or through the in-game reload commands. Like the Windows server, GameServer rewrites the client `.bin` tables in `resource` from the `.txt` tables on every start; their table data is unchanged, but uninitialized padding differs from run to run, so git reports them as modified. `INFO_HOST`, `INFO_PORT` and `GAME00_INFO_PASSWORD` through `GAME03_INFO_PASSWORD` fill the InfoServer line of each instance's configuration; every GameServer container renders all four instance configurations, so each needs all four passwords; GameServer does not connect to an InfoServer today. `docker compose stop` gives GameServer 90 seconds: it notifies players, saves all of them (`End SaveAllPlayer` in `log/GameServerNN/enter_map.log`) and exits. Docker Desktop bind mounts ignore file-name case; on a Linux host the names in scripts, tables and code must match the files exactly.

Each server's configuration is rendered at container start from `docker/templates/<Name>/*.template` with values from the environment, so listen addresses, peer hosts, ports, database host and credentials come from `compose.yaml` and `.env`. `.env.example` lists every variable the templates and `compose.yaml` read, with the defaults compose uses. The file logs of each server go to a named volume mounted at `/var/log/tales/<Name>`. On a crash a Linux server writes a stack trace to the console and to `crash.log` in its log directory (`log/GameServer00/crash.log` for `game00`), then dumps core where the host's `core_pattern` says (on Docker Desktop for Windows that is `%LOCALAPPDATA%\Temp\wsl-crashes`).

`docker/smoke-test.sh` checks that `.env.example` covers every variable and that only `game00` runs without a profile, then builds the image and brings up a throwaway stack with the `full` profile. It checks that AccountServer is connected to the database, that GroupServer is logged in to AccountServer and GateServer to GroupServer, that `game00` through `game03` are registered with GateServer and that GateServer added the maps from their configurations, and that the published Gate port accepts a client. It then stops the stack and checks every server exited with code 0 and that every GameServer saved all players on the way out. If port 1973 is taken on the host, run it as `GATE_PUBLISHED_PORT=21973 docker/smoke-test.sh`.

For incremental Linux builds, `docker compose run --rm dev` configures, builds and tests the bind-mounted source tree in the toolchain image, keeping the build tree in the `linux-build` volume.
