# Photo Frame GS02 (Grayscale E-ink)

E-ink digital picture frame with remote image updates via FTP and configuration settings adjustable through Telnet. Features deep sleep mode with RTC backup for extended battery life.

> **📌 Note:** This project is designed for the newer **LilyGo T5 4.7" E-Paper Plus** (ESP32-S3). For the older **LilyGo T5 4.7" E-Paper** (WROVER-E) version, check out [PhotoFrameGS01](https://github.com/tokosattila/PhotoFrameGS01.git).

## 📸 Gallery

| <img src="docs/images/pic01.jpg" width="240px" alt="Photo Frame Display" /> | <img src="docs/images/pic02.jpg" width="240px" alt="Photo Frame Hardware" /> | <img src="docs/images/pic03.jpg" width="240px" alt="Photo Frame Back Side" /> |
|:---:|:---:|:---:|
| *Photo Frame with Image* | *Hardware backside* | *Backside covered* |

| <img src="docs/images/pic04.jpg" width="370px" alt="Telnet" /> | <img src="docs/images/pic05.jpg" width="370px" alt="FTP" /> |
|:---:|:---:|
| *Telnet* | *FTP* |

## 🔧 Hardware

| Component | Specification |
|-----------|--------------|
| **Board** | LilyGo T5 4.7" E-Paper Plus |
| **MCU** | ESP32-S3 |
| **Display** | ED047TC1 (960×540, 16 grayscale) |
| **Flash** | 16MB |
| **PSRAM** | 8MB |
| **RTC** | PCF8563 (I2C, battery backup) |
| **Storage** | SD Card (SPI) + LittleFS (internal) |
| **Battery** | Li-Ion 18650 (optional) |

<img src="docs/LilyGoT54.7E-PaperPlus-Pins.png" width="370px" alt="PIN" />

## 🔄 Operating Modes

| Mode | Description |
|------|-------------|
| **Photo Frame Mode** | JPEG slideshow from SD Card or LittleFS with grayscale rendering, deep sleep between wake intervals (10sec to monthly) |
| **Maintenance Mode** | Button-triggered mode for configuration & remote management |
| **Low Battery Mode** | Auto shutdown with battery icon display |

## ✨ Features

- **Dual Storage Support** — SD Card (primary) + LittleFS (internal flash)
- **Smart Storage Fallback** — Auto-switch to secondary storage if images folder is empty
- **RTC Backup** — PCF8563 maintains time during deep sleep
- **WiFi Connectivity** — AP mode for setup, STA mode for network access
- **NTP Time Sync** — Automatic time synchronization with RTC backup
- **FTP Server** — Upload/manage images wirelessly (SD Card or LittleFS)
- **Telnet Console** — Remote monitoring, configuration & commands
- **Firmware OTA (Dual Slot)** — Update firmware via `/update/` (ota_0/ota_1) with boot-slot control
- **Battery Monitoring** — Auto low-power mode with voltage display
- **Grayscale Rendering** — 16-level dithering with brightness/contrast/gamma control
- **Deep Sleep Wake-up** — Timer-based or button-triggered (EXT1)
- **mDNS Support** — Access device via hostname.local

## 📁 Project Structure

```
src/
├── Main.cpp                    # Application entry point
├── App/
│   ├── Button.cpp/h            # Debounced button handling
│   ├── Configuration.cpp/h     # INI config management
│   ├── Connection.cpp/h        # WiFi management (AP/STA)
│   ├── Display.cpp/h           # E-Paper driver wrapper
│   ├── Firmware.cpp/h          # Firmware manager
│   ├── FTP.cpp/h               # FTP server
│   ├── Global.h                # Global definitions & macros
│   ├── LittleFS.cpp/h          # LittleFS operations
│   ├── NTP.cpp/h               # NTP time sync
│   ├── RTCTime.cpp/h           # PCF8563 RTC driver
│   ├── SDCard.cpp/h            # SD Card operations (SPI)
│   ├── Storage.cpp/h           # Storage manager with fallback
│   ├── Telnet.cpp/h            # Telnet console
│   ├── Telnet/
│   │   ├── Command.h           # Base command interface
│   │   └── Commands/           # Telnet command implementations
│   │       ├── BatInfoCommand.h
│   │       ├── BootPartitionCommand.h
│   │       ├── CallBackCommand.h
│   │       ├── CatCommand.h
│   │       ├── ClearCommand.h
│   │       ├── ConfigCommand.h
│   │       ├── DateCommand.h    # System & RTC date/time
│   │       ├── ExitCommand.h
│   │       ├── FetchCommand.h
│   │       ├── FileSystemInfoCommand.h
│   │       ├── FirmwareUpdateCommand.h
│   │       ├── HelpCommand.h
│   │       ├── ListCommand.h
│   │       ├── LogoutCommand.h
│   │       ├── MemInfoCommand.h
│   │       ├── NotFoundCommand.h
│   │       ├── NetInfoCommand.h
│   │       ├── NvsInfoCommand.h
│   │       ├── RebootCommand.h
│   │       ├── ResetCommand.h
│   │       ├── SketchInfoCommand.h
│   │       └── TimeStampCommand.h
│   └── Utils.cpp/h             # System utilities
├── Fonts/                      # OpenSans bitmap fonts (6-26pt)
│   └── opensans*.h             # 26 font variants
└── Images/
    └── DefaultImage.h          # Default fallback image

lib/
├── ArduinoHttpClient/          # HTTP client for image fetch
├── JPEGDEC/                    # JPEG decoder
├── LilyGoEPD47/                # E-Paper driver
├── SimpleFTPServer/            # FTP server (SD + LittleFS)
└── Unity/                      # Unit testing framework

test/
├── mocks/                      # Mock classes for testing
│   ├── MockString.h
│   └── MockWiFiClient.h
├── test_Button/                # Button unit tests
├── test_ConfigCommand/         # Config command parsing tests
├── test_Configuration/         # Configuration parser tests
├── test_DateCommand/           # Date/RTC command parsing tests
├── test_ESP32/                 # Hardware-specific ESP32 tests
├── test_FetchCommand/          # Fetch command tests
├── test_NTP/                   # NTP time utility tests
├── test_RTCTime/               # RTC time functions tests
├── test_SDCard/                # SD Card path/file utilities
├── test_Storage/               # Storage fallback logic tests
├── test_Telnet/                # Telnet command tests
├── test_Utils/                 # Utility function tests
└── test_Wrappers/              # Type wrapper tests
```

## 🛠️ Build

### Requirements
- [PlatformIO](https://platformio.org/)
- ESP32-S3 toolchain (arduino-esp32 >= 2.0.3)

### Commands
```bash
# Build
pio run

# Upload firmware
pio run -t upload

# Upload filesystem (LittleFS)
pio run -t uploadfs

# Run tests (native)
pio test -e native

# Monitor serial
pio device monitor
```

## ⚙️ Configuration

Place `config.ini` in SD Card or LittleFS root (`/config.ini`):

```ini
[device]
appname = PHOTO FRAME GS02
version = v1.0

[display]
jpg_brightness = 25         ; 0-100%
jpg_contrast = 75           ; 0-100%
jpg_gamma = 125             ; gamma correction
image_file =                ; current image file

[ntp]
ntp_server = pool.ntp.org
ntp_port = 123
ntp_gmt_offset = 7200       ;GMT+2 in seconds
ntp_update = 60000          ; update interval ms

[ap mode]
ap_enable = true
ap_ssid = PhotoFrameGS02
ap_password = 123456789
ap_ip = 192.168.4.1
ap_gateway = 192.168.4.1
ap_subnet = 255.255.255.0

[sta mode]
sta_ssid = YourNetwork
sta_password = YourPassword

[static ip]
sta_enable = false
sta_ip = 192.168.0.83
sta_gateway = 192.168.0.1
sta_subnet = 255.255.255.0
sta_dns1 = 192.168.0.1
sta_dns2 = 8.8.8.8

[mdns]
mdns_enable = false
mdns_hostname = photoframegs02

[timer]
wake_up = 5                 ; 1=10sec, 2=1min, 3=1hour, 4=12hour, 5=Daily, 6=Weekly, 7=Monthly

[telnet]
telnet_enable = true
telnet_port = 23
telnet_username = admin
telnet_password = 123456789
telnet_session = 3600000    ; session timeout ms

[ftp]
ftp_enable = true
ftp_port = 21
ftp_username = admin
ftp_password = 123456789

[storage]
default_file_system = sdcard ; sdcard | littlefs
fallback_enabled = true      ; smart fallback if images empty
```

## 📡 Telnet Commands

| Command | Description |
|---------|-------------|
| `help` | Show available commands |
| `clear` | Clear terminal screen |
| `list [path]` | List directories and files |
| `cat <filename>` | Show file content |
| `date` | Show system date and time |
| `date rtc` | Show RTC date and time |
| `date rtc set YYYY.MM.DD HH:MM:SS` | Set RTC date and time |
| `date rtc sync-from-ntp` | Sync RTC from NTP server |
| `date rtc sync-to-system` | Sync system time from RTC |
| `timestamp` | Show current Unix timestamp |
| `nvsinfo` | Show NVS usage info |
| `meminfo` | Show memory usage (heap, PSRAM) |
| `sketchinfo` | Show sketch/firmware info |
| `fsinfo` | Show filesystem usage (SD + LittleFS) |
| `netinfo` | Show network info (IP, MAC) |
| `batinfo` | Show battery voltage and percentage |
| `config <key> [value]` | Get or set config value |
| `fetch <url> [filename]` | Download image (max. 400kB, type: *.jpg, *.jpeg) |
| `fwupdate [status\|verify\|run]` | Verify/apply firmware update from `/update/` |
| `fwupdate` | Show update status |
| `fwupdate verify` | Verify firmware.bin/firmware.sha256 |
| `fwupdate run` | Perform update (asks y/n) |
| `bootpart [status\|ota0\|ota1]` | Show or set active OTA boot slot |
| `reset config` | Factory reset configuration |
| `reboot` | Restart device |
| `logout` | Logout telnet session |
| `exit` | Exit telnet connection |

## 🧩 Firmware (OTA)

This project uses a dual-slot OTA layout (`ota_0` + `ota_1`) controlled by the `otadata` partition.

- **Applying update**: upload `firmware.bin` and `firmware.sha256` into `/update/` on the active storage, then run `fwupdate verify` and `fwupdate run`.
- **Which slot is running**: use `bootpart status` (shows Running/Boot partitions).
- **Force boot slot**: use `bootpart ota0` or `bootpart ota1`, then `reboot`.
- **USB upload note**: a plain USB upload typically writes the firmware at `0x10000` (often `ota_0`). If the device still boots the other slot, set it explicitly with `bootpart`.

## 🔌 Pin Configuration

| Pin | Function | Description |
|-----|----------|-------------|
| GPIO21 | Button 1 | Wake from deep sleep, Enter maintenance mode |
| GPIO48 | Button 2 | Factory reset (hold 30 sec) |
| GPIO14 | Battery ADC | Battery voltage measurement |
| GPIO18 | RTC SDA (I2C) | PCF8563 real-time clock data line |
| GPIO17 | RTC SCL (I2C) | PCF8563 real-time clock clock line |
| GPIO16 | SD MISO | SD Card data out (Master In Slave Out) |
| GPIO15 | SD MOSI | SD Card data in (Master Out Slave In) |
| GPIO11 | SD SCK | SD Card serial clock |
| GPIO42 | SD CS | SD Card chip select |

## 📦 Dependencies

- [LilyGoEPD47](https://github.com/Xinyuan-LilyGO/LilyGo-EPD47) — E-Paper driver
- [JPEGDEC](https://github.com/bitbank2/JPEGDEC) — JPEG decoder
- [SimpleFTPServer](https://github.com/xreef/SimpleFTPServer) — FTP server
- [ArduinoHttpClient](https://github.com/arduino-libraries/ArduinoHttpClient) — HTTP client
- [Unity](https://github.com/ThrowTheSwitch/Unity) — Unit testing

## 🔋 Power Management

- **Photo Frame Mode**: Display image → deep sleep → wake by timer or button
- **Deep Sleep Current**: ~10µA (with RTC backup)
- **Wake-up Sources**: 
  - Timer (configurable: 10sec to monthly)
  - Button press (GPIO21, EXT1 wakeup)
- **RTC Backup**: PCF8563 maintains accurate time during sleep
- **Low Battery**: Auto-shutdown at configurable voltage threshold

## 📄 License

MIT License

Copyright (c) 2025-2026 Szeklerman
