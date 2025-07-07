#pragma once

#include "mcli.h"
/**
 * Arduino Serial adapter - works with Serial, Serial1, etc.
 */

class ArduinoSerialIo : public mcli::CliIoInterface {
    public:
        ArduinoSerialIo(Stream& stream = Serial) : stream_(stream) {}

        // Required abstract methods from CliIoInterface
        void put_byte(char c) override {
            stream_.write(c);
        }
        char get_byte() override {
            while (!byte_available()) {}
            return stream_.read();
        }
        bool byte_available() override {
            return stream_.available() > 0;
        }

        // Optional bulk methods
        void put_bytes(const char* data, size_t len) {
            stream_.write(data, len);
        }

        size_t get_bytes(char* buffer, size_t max_len) override {
            size_t count = 0;
            while (count < max_len && stream_.available()) {
                buffer[count++] = stream_.read();
            }
            return count;
        }

        void flush() override {
            stream_.flush();
        }

    private:
        Stream& stream_;
};