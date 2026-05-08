#include "gps_reader.h"
#include "config.h"
#include <HardwareSerial.h>
#include <string.h>
#include <stdlib.h>

static HardwareSerial gpsSerial(2);  // UART2

void GPSReader::begin() {
    gpsSerial.begin(GPS_BAUD, SERIAL_8N1, PIN_GPS_RX, PIN_GPS_TX);
    Serial.println("[GPS] UART2 started");
}

void GPSReader::poll() {
    while (gpsSerial.available()) {
        char c = (char)gpsSerial.read();
        if (c == '\r') continue;
        if (c == '\n') {
            _buf[_pos] = '\0';
            if (_pos > 0) _parseLine(_buf);
            _pos = 0;
        } else if (_pos < sizeof(_buf) - 1) {
            _buf[_pos++] = c;
        }
    }
}

void GPSReader::_parseLine(const char* line) {
    // Only parse $GPRMC (recommended minimum sentence – has speed + position)
    if (strncmp(line, "$GPRMC", 6) == 0) {
        _parseGPRMC(line);
    }
}

// $GPRMC,hhmmss.ss,A,lat,N/S,lon,E/W,speed_knots,track,date,mag,E/W*checksum
bool GPSReader::_parseGPRMC(const char* line) {
    char copy[96];
    strncpy(copy, line, sizeof(copy) - 1);
    copy[sizeof(copy) - 1] = '\0';

    // Tokenise by comma
    const char* fields[13] = {};
    uint8_t fc = 0;
    char* tok = strtok(copy, ",");
    while (tok && fc < 13) {
        fields[fc++] = tok;
        tok = strtok(nullptr, ",");
    }
    if (fc < 8) return false;

    // Field 2: status ('A' = active/valid, 'V' = void)
    if (!fields[2] || fields[2][0] != 'A') {
        _data.valid = false;
        return false;
    }

    // Field 7: speed over ground in knots
    float knots = fields[7] ? atof(fields[7]) : 0.f;
    _data.speedKph = knots * 1.852f;

    // Fields 3-6: lat/lon (simplified – not required for alert logic)
    if (fields[3] && fields[4] && fields[5] && fields[6]) {
        float rawLat = atof(fields[3]);
        float rawLon = atof(fields[5]);
        // Convert NMEA ddmm.mmmm → decimal degrees
        int latDeg = (int)(rawLat / 100);
        _data.lat = latDeg + (rawLat - latDeg * 100) / 60.f;
        if (fields[4][0] == 'S') _data.lat = -_data.lat;

        int lonDeg = (int)(rawLon / 100);
        _data.lng = lonDeg + (rawLon - lonDeg * 100) / 60.f;
        if (fields[6][0] == 'W') _data.lng = -_data.lng;
    }

    _data.valid = true;
    return true;
}
