
# Tortoise-WoW / TurtleWoW Local Server Setup Guide (Windows 11 + WSL Ubuntu)

## Overview

This guide documents the complete setup and troubleshooting process used to successfully compile and run the `Penqle/tortoise-wow` fork locally using:

- Windows 11
- WSL Ubuntu
- MariaDB/MySQL
- TurtleWoW 1.18.1 client
- Custom Turtle patches and extracted map data

It also documents the major issues encountered and how they were fixed.

---

# 1. Environment Setup

## Install WSL Ubuntu

Open PowerShell as Administrator:

```powershell
wsl --install
```

Restart the PC if required.

---

## Install Required Packages

Inside Ubuntu:

```bash
sudo apt update

sudo apt install -y \
git cmake build-essential clang \
libssl-dev libbz2-dev libreadline-dev \
libncurses-dev libboost-all-dev \
libmysqlclient-dev mariadb-server \
ace-dev
```

---

# 2. Clone the Repository

```bash
mkdir -p ~/twow
cd ~/twow

git clone https://github.com/Penqle/tortoise-wow.git
cd tortoise-wow
```

---

# 3. IMPORTANT: Correct CMake Flags

This was one of the MOST IMPORTANT fixes.

The server MUST be compiled with:

```bash
-DALLOW_TURTLE_ADDONS=ON
```

Otherwise the client crashes with:

```text
Your game interface files are corrupt.
Please remove your InterfaceFrameXML folder.
```

---

## Correct Build Commands

```bash
cd ~/twow/tortoise-wow

rm -rf build
mkdir build
cd build

cmake .. \
-DCMAKE_INSTALL_PREFIX=~/twow/tortoise-wow/server \
-DBUILD_EXTRACTORS=ON \
-DBUILD_TOOLS=ON \
-DBUILD_AHBOT=ON \
-DUSE_ANTICHEAT=OFF \
-DALLOW_TURTLE_ADDONS=ON
```

Compile:

```bash
make -j$(nproc)
```

Install:

```bash
make install
```

---

# 4. Database Setup

## Create Databases

Inside MySQL:

```sql
CREATE DATABASE tw_world;
CREATE DATABASE tw_logon;
CREATE DATABASE tw_chars;
CREATE DATABASE tw_logs;
```

Create user:

```sql
CREATE USER 'mangos'@'localhost' IDENTIFIED BY 'mangos';

GRANT ALL PRIVILEGES ON *.* TO 'mangos'@'localhost';

FLUSH PRIVILEGES;
```

---

# 5. Import SQL Files

Import the provided SQL base files into:

- tw_world
- tw_logon
- tw_chars
- tw_logs

Verify table counts:

```bash
mysql -u mangos -pmangos -e "
SELECT table_schema, COUNT(*) tables
FROM information_schema.tables
WHERE table_schema IN ('tw_logon','tw_world','tw_chars','tw_logs')
GROUP BY table_schema;
"
```

Expected example:

```text
tw_char   ~93
tw_logon  ~41
tw_world  ~280
```

---

# 6. Configure realmd.conf and mangosd.conf

Example:

```ini
LoginDatabase.Info     = "127.0.0.1;3306;mangos;mangos;tw_logon"
WorldDatabase.Info     = "127.0.0.1;3306;mangos;mangos;tw_world"
CharacterDatabase.Info = "127.0.0.1;3306;mangos;mangos;tw_chars"
LogsDatabase.Info      = "127.0.0.1;3306;mangos;mangos;tw_logs"
```

---

# 7. Fix: No Valid Realms Specified

If realmd says:

```text
No valid realms specified.
```

Insert a realm manually:

```sql
INSERT INTO realmlist
(id,name,address,port,icon,realmflags,timezone,allowedSecurityLevel,population)
VALUES
(1,'Turtle WoW Local','127.0.0.1',8085,1,0,1,0,1);
```

Verify:

```sql
SELECT * FROM realmlist;
```

---

# 8. Extract TurtleWoW Data

VERY IMPORTANT.

The server requires Turtle client extracted data:

- dbc
- maps
- vmaps
- mmaps

Copy these folders into:

```text
~/twow/tortoise-wow/server/data
```

---

# 9. Fix: Transport / Taxi Crashes

A segmentation fault occurred during:

```text
Loading transport templates...
```

Cause:

- missing taxi data
- incomplete world database

This was resolved after importing the correct Turtle world database and extracted data.

---

# 10. Warden / Anticheat Crash Fixes

## Original Crash

```text
WardenModuleMgr.cpp:89:
Assertion in GetWindowsModule failed:
!_winModules.empty()
```

Then:

```text
CLIENT_PLATFORM_X86
```

---

## Fix

Disable anticheat during compilation:

```bash
-DUSE_ANTICHEAT=OFF
```

This bypasses incompatible Windows Warden module checks under WSL/Linux.

---

# 11. Fix: InterfaceFrameXML / Corrupt Interface

## Error

```text
Your game interface files are corrupt.
Please remove your InterfaceFrameXML folder.
```

Deleting Interface/WTF/Cache alone DID NOT fix it.

The REAL fix was:

```bash
-DALLOW_TURTLE_ADDONS=ON
```

during compilation.

This is mandatory for TurtleWoW compatibility.

---

# 12. Turtle Client Patch Files

Required client files included:

```text
hotfix.mpq
twpatch.mpq
twpatch_7070.mpq
twpatch_7100.mpq
```

These must exist in the TurtleWoW client.

---

# 13. Client Cleanup

Delete:

```text
WTF
Cache
Interface
```

before testing after major changes.

---

# 14. Launching the Server

Terminal 1:

```bash
cd ~/twow/tortoise-wow/server/bin
./realmd
```

Terminal 2:

```bash
cd ~/twow/tortoise-wow/server/bin
./mangosd
```

---

# 15. Success Indicators

You should now be able to:

- Login successfully
- See the realm
- Create a character
- Enter the world
- Load maps properly

---

# 16. Useful Startup Script

```bash
#!/bin/bash

cd ~/twow/tortoise-wow/server/bin

./realmd &

sleep 2

./mangosd
```

---

# 17. Troubleshooting Summary

| Problem | Cause | Fix |
|---|---|---|
| No valid realms specified | Empty realmlist table | Insert realm manually |
| Segmentation fault on transports | Missing taxi/map data | Import proper Turtle DB + extracted maps |
| WardenModuleMgr.cpp assertion | Linux/WSL anticheat incompatibility | `-DUSE_ANTICHEAT=OFF` |
| InterfaceFrameXML corrupt | Turtle addons disabled | `-DALLOW_TURTLE_ADDONS=ON` |
| Realm visible but cannot enter world | Patch mismatch/client mismatch | Use proper Turtle client + patches |
| Taxi tables empty | Incomplete world DB | Re-import correct world SQL |

---

# Final Notes

This setup process was significantly more complex than a standard CMaNGOS install due to:

- TurtleWoW custom patches
- Warden/anticheat integration
- Turtle-specific addon support
- Client patch dependencies
- WSL/Linux compatibility

Once correctly configured, the server successfully:

- boots
- accepts logins
- loads characters
- loads world maps
- supports Turtle addons

Enjoy your local TurtleWoW server.
