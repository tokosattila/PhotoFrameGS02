#include "Firmware.h"
#include <Update.h>
#include <mbedtls/sha256.h>

#define BUF_SIZE 1024

Firmware_::Firmware_(fs::FS &filesystem, const String &path)
  : _fs(filesystem), _path(path) {}

bool Firmware_::updateAvailable() {
    File f = _fs.open(_path, FILE_READ);
    if(!f) return false;
    bool ok = (f.size() > 0);
    f.close();
    return ok;
}

void Firmware_::setError(const String &err) {
    _lastError = err;
}

String Firmware_::getLastError() const {
    return _lastError;
}

bool Firmware_::verifySha256(const String &shaPath) {
    File bin = _fs.open(_path, FILE_READ);
    if(!bin) { setError("firmware file not found"); return false; }
    File shaFile = _fs.open(shaPath, FILE_READ);
    if(!shaFile) { bin.close(); setError("sha file not found"); return false; }

    String expected = "";
    while(shaFile.available()) expected += (char)shaFile.read();
    expected.trim();
    shaFile.close();

    mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);
    if(mbedtls_sha256_starts_ret(&ctx, 0) != 0) {
        bin.close();
        setError("sha init failed");
        return false;
    }

    uint8_t buf[BUF_SIZE];
    while(bin.available()) {
        size_t r = bin.read(buf, BUF_SIZE);
        if(mbedtls_sha256_update_ret(&ctx, buf, r) != 0) {
            bin.close();
            setError("sha update failed");
            mbedtls_sha256_free(&ctx);
            return false;
        }
    }

    uint8_t out[32];
    if(mbedtls_sha256_finish_ret(&ctx, out) != 0) {
        bin.close();
        setError("sha finish failed");
        mbedtls_sha256_free(&ctx);
        return false;
    }
    mbedtls_sha256_free(&ctx);
    bin.close();

    char hex[65];
    for(int i=0;i<32;i++) sprintf(hex + i*2, "%02x", out[i]);
    hex[64] = 0;
    String got(hex);
    if(got.equalsIgnoreCase(expected)) return true;
    setError("sha mismatch");
    return false;
}

bool Firmware_::performUpdate(Stream *logStream) {
    File bin = _fs.open(_path, FILE_READ);
    if(!bin) { setError("firmware file not found"); return false; }
    size_t size = bin.size();
    if(size == 0) { bin.close(); setError("empty firmware file"); return false; }

    if(logStream) {
        logStream->print("Starting update, size=");
        logStream->println(size);
    }

    if(!Update.begin(size)) {
        bin.close();
        setError("Update.begin failed");
        return false;
    }

    uint8_t buf[BUF_SIZE];
    size_t written = 0;
    while(bin.available()) {
        size_t r = bin.read(buf, BUF_SIZE);
        size_t w = Update.write(buf, r);
        if(w != r) {
            setError("write failed");
            bin.close();
            return false;
        }
        written += w;
        if(logStream && size > 0) {
            int pct = (written * 100) / size;
            logStream->print("Progress: ");
            logStream->print(pct);
            logStream->println(" %");
        }
    }

    bool ok = Update.end(true);
    if(!ok || !Update.isFinished()) {
        setError("Update failed or not finished");
        bin.close();
        return false;
    }

    bin.close();
    if(!_fs.remove(_path)) {
        if(logStream) logStream->println("Warning: failed to remove firmware file");
    }
    if(logStream) logStream->println("Update successful, rebooting...");
    return true;
}
