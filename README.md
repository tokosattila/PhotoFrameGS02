# PhotoFrameGS02

E-ink digital picture frame with remote image updates via FTP and configuration settings adjustable through Telnet. Features deep sleep mode with RTC backup for extended battery life.

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
| **Button** | GPIO21 (wake-up & settings) |

### Pin Configuration (ESP32-S3)

| Pin | Function |
|-----|----------|
| GPIO21 | Button (wake-up / settings) |
| GPIO48 | Reset button |
| GPIO14 | Battery ADC |
| GPIO18 | RTC SDA (I2C) |
| GPIO17 | RTC SCL (I2C) |
| GPIO16 | SD MISO |
| GPIO15 | SD MOSI |
| GPIO11 | SD SCK |
| GPIO42 | SD CS |

## 🔄 Operating Modes

| Mode | Description |
|------|-------------|
| **Photo Frame Mode** | JPEG slideshow from SD Card or LittleFS with grayscale rendering |
| **Deep Sleep Mode** | Low-power state with RTC timekeeping, configurable wake intervals |
| **Maintenance Mode** | Button-triggered mode for WiFi config, FTP & Telnet access |
| **Low Battery Mode** | Auto shutdown with battery icon display |

## ✨ Features

- **Dual Storage Support** — SD Card (primary) + LittleFS (internal flash)
- **Smart Storage Fallback** — Auto-switch to secondary storage if images folder is empty
- **RTC Backup** — PCF8563 maintains time during deep sleep
- **WiFi Connectivity** — AP mode for setup, STA mode for network access
- **NTP Time Sync** — Automatic time synchronization with RTC backup
- **FTP Server** — Upload/manage images wirelessly (SD Card or LittleFS)
- **Telnet Console** — Remote monitoring, configuration & commands
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
│   ├── FileSystem.cpp/h        # Abstract filesystem interface
│   ├── LittleFS.cpp/h          # LittleFS operations
│   ├── SDCard.cpp/h            # SD Card operations (SPI)
│   ├── Storage.cpp/h           # Storage manager with fallback
│   ├── FTP.cpp/h               # FTP server
│   ├── Global.h                # Global definitions & macros
│   ├── NTP.cpp/h               # NTP time sync
│   ├── RTCTime.cpp/h           # PCF8563 RTC driver
│   ├── Telnet.cpp/h            # Telnet console
│   ├── Telnet/
│   │   ├── Command.h           # Base command interface
│   │   └── Commands/           # Telnet command implementations
│   │       ├── BatInfoCommand.h
│   │       ├── CatCommand.h
│   │       ├── ClearCommand.h
│   │       ├── ConfigCommand.h
│   │       ├── DateCommand.h    # System & RTC date/time
│   │       ├── ExitCommand.h
│   │       ├── FetchCommand.h
│   │       ├── FileSystemInfoCommand.h
│   │       ├── HelpCommand.h
│   │       ├── ListCommand.h
│   │       ├── LogoutCommand.h
│   │       ├── MemInfoCommand.h
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

Place `config.ini` in LittleFS root (`/config.ini`):

```ini
[device]
appname = PHOTO FRAME GS02
version = v2.0

[display]
jpg_brightness = 30      ; 0-100%
jpg_contrast = 35        ; 0-100%
jpg_gamma = 135          ; gamma correction
image_file =             ; current image file
images_dir = images      ; images directory
image_ext = *.jpg        ; image file extension filter

[storage]
default_fs = 2           ; 1=LittleFS, 2=SDCard
fallback_enable = true   ; smart fallback if images empty

[ntp]
ntp_server = pool.ntp.org
ntp_port = 123
ntp_gmt_offset = 1       ; GMT offset in hours
ntp_update = 60000       ; update interval ms

[connection]
ap_enable = false        ; AP mode (true) or STA mode (false)

[ap mode]
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
mdns_enable = true
mdns_hostname = photoframe

[timer]
wake_up = 4              ; 1=10sec, 2=1min, 3=1hour, 4=12hour, 5=Daily, 6=Weekly, 7=Monthly

[telnet]
telnet_enable = true
telnet_port = 23
telnet_username = admin
telnet_password = 123456789
telnet_session = 3600000 ; session timeout ms

[ftp]
ftp_enable = true
ftp_port = 21
ftp_username = admin
ftp_password = 123456789
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
| `netinfo` | Show network info (IP, MAC, RSSI) |
| `batinfo` | Show battery voltage and percentage |
| `config <key> [value]` | Get or set config value |
| `fetch <url> [filename]` | Download image (max. 200kB, *.jpg) |
| `reset config` | Factory reset configuration |
| `reboot` | Restart device |
| `logout` | Logout telnet session |
| `exit` | Exit telnet connection |

## 📦 Dependencies

- [LilyGoEPD47](https://github.com/Xinyuan-LilyGO/LilyGo-EPD47) — E-Paper driver for ESP32-S3
- [JPEGDEC](https://github.com/bitbank2/JPEGDEC) — Fast JPEG decoder
- [SimpleFTPServer](https://github.com/xreef/SimpleFTPServer) — FTP server (modified for dual storage)
- [ArduinoHttpClient](https://github.com/arduino-libraries/ArduinoHttpClient) — HTTP client
- [Unity](https://github.com/ThrowTheSwitch/Unity) — Unit testing framework

## 🔋 Power Management

- **Photo Frame Mode**: Display image → deep sleep → wake by timer or button
- **Deep Sleep Current**: ~10µA (with RTC backup)
- **Wake-up Sources**: 
  - Timer (configurable: 10sec to monthly)
  - Button press (GPIO21, EXT1 wakeup)
- **RTC Backup**: PCF8563 maintains accurate time during sleep
- **Low Battery**: Auto-shutdown at configurable voltage threshold

## 📄 License

MIT

---

**Author:** Szeklerman  
**Hardware:** LilyGo T5 4.7" E-Paper Plus (ESP32-S3)
