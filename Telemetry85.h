#ifndef TELEMETRY85_H
#define TELEMETRY85_H

#include <stdint.h>
#include <string.h>
#include <math.h>

class Telemetry85 {
private:
    static constexpr const char* b85 = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!#$%&()*+,-./:;<=>?@[]^_`{|}~";

public:
    static void encode(uint16_t value, char* buffer) {
        if (value > 7224) value = 7224; 
        
        buffer[0] = b85[value / 85];
        buffer[1] = b85[value % 85];
        buffer[2] = '\0';
    }

    static void encodeFloat(float value, float precision, char* buffer) {
        uint16_t scaledValue = (uint16_t)round(value * precision);
        encode(scaledValue, buffer);
    }

    static uint16_t decode(const char* encoded) {
        if (!encoded || encoded[0] == '\0' || encoded[1] == '\0') {
            return 0; 
        }

        const char* p1 = strchr(b85, encoded[0]);
        const char* p2 = strchr(b85, encoded[1]);

        if (!p1 || !p2) {
            return 0; 
        }

        uint16_t val1 = p1 - b85;
        uint16_t val2 = p2 - b85;

        return (val1 * 85) + val2;
    }

    static float decodeFloat(const char* encoded, float precision) {
        uint16_t raw = decode(encoded);
        return (float)raw / precision;
    }
};

#endif
