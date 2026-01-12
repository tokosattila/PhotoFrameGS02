// src/App/Firmware.h
// Simple firmware updater class for single-partition ESP32

#ifndef FIRMWARE_H
#define FIRMWARE_H

#include <Arduino.h>
#include <FS.h>

class Firmware_ {
public:
    Firmware_(fs::FS &filesystem, const String &path = "/update/firmware.bin");

    // Check if firmware file exists and non-zero
    bool updateAvailable();

    // Verify SHA256 hex file located at shaPath (e.g. /update/firmware.sha256)
    bool verifySha256(const String &shaPath);

    // Perform firmware update from binary; optional log stream for progress
    bool performUpdate(Stream *logStream = nullptr);

    // Human-readable last error
    String getLastError() const;

private:
    fs::FS &_fs;
    String _path;
    String _lastError;
    void setError(const String &err);
};

#endif // FIRMWARE_H
