#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <cstdint>

enum DeserializeResult {
    Success,
    Failure
};

typedef std::vector<uint8_t> ByteBuffer;

uint8_t loadByte(const ByteBuffer& buffer, size_t pos) {
    return buffer[pos];
}
uint16_t loadBytes16Big(const ByteBuffer& buffer, size_t pos) {
    return (buffer[pos] << 8) | buffer[pos + 1];
}
uint16_t loadBytes16Little(const ByteBuffer& buffer, size_t pos) {
    return buffer[pos] | (buffer[pos + 1] << 8);
}
uint32_t loadBytes32Big(const ByteBuffer& buffer, size_t pos) {
    return (buffer[pos] << 24) | (buffer[pos + 1] << 16) | (buffer[pos + 2] << 8) | buffer[pos + 3];
}
uint32_t loadBytes32Little(const ByteBuffer& buffer, size_t pos) {
    return (buffer[pos]) | (buffer[pos + 1] << 8) | (buffer[pos + 2] << 16) | (buffer[pos + 3] << 24);
}
uint64_t loadBytes64Big(const ByteBuffer& buffer, size_t pos) {
    return (static_cast<uint64_t>(buffer[pos]) << 56) | (static_cast<uint64_t>(buffer[pos + 1]) << 48) | (static_cast<uint64_t>(buffer[pos + 2]) << 40) | (static_cast<uint64_t>(buffer[pos + 3]) << 32) | (static_cast<uint64_t>(buffer[pos + 4]) << 24) | (static_cast<uint64_t>(buffer[pos + 5]) << 16) | (static_cast<uint64_t>(buffer[pos + 6]) << 8) | static_cast<uint64_t>(buffer[pos + 7]);
}
uint64_t loadBytes64Little(const ByteBuffer& buffer, size_t pos) {
    return (static_cast<uint64_t>(buffer[pos]) | (static_cast<uint64_t>(buffer[pos + 1]) << 8) | (static_cast<uint64_t>(buffer[pos + 2]) << 16) | (static_cast<uint64_t>(buffer[pos + 3]) << 24) | (static_cast<uint64_t>(buffer[pos + 4]) << 32) | (static_cast<uint64_t>(buffer[pos + 5]) << 40) | (static_cast<uint64_t>(buffer[pos + 6]) << 48) | (static_cast<uint64_t>(buffer[pos + 7]) << 56));
}

uint32_t loadBytes24Big(const ByteBuffer& buffer, size_t pos) {
    return (buffer[pos] << 16) | (buffer[pos + 1] << 8) | buffer[pos + 2];
}
uint32_t loadBytes24Little(const ByteBuffer& buffer, size_t pos) {
    return buffer[pos] | (buffer[pos + 1] << 8) | (buffer[pos + 2] << 16);
}

void writeByte(ByteBuffer& buffer, size_t pos, uint8_t value) {
    buffer[pos] = value;
}
void writeByte(ByteBuffer& buffer, size_t* pos, uint8_t value) {
    buffer[*pos] = value;
    (*pos)++;
}
void writeBytes16Big(ByteBuffer& buffer, size_t pos, uint16_t value) {
    buffer[pos] = value >> 8;
    buffer[pos + 1] = value & 0xFF;
}
void writeBytes16Big(ByteBuffer& buffer, size_t* pos, uint16_t value) {
    buffer[*pos] = value >> 8;
    buffer[*pos + 1] = value & 0xFF;
    *pos += 2;
}
void writeBytes16Little(ByteBuffer& buffer, size_t pos, uint16_t value) {
    buffer[pos] = value & 0xFF;
    buffer[pos + 1] = value >> 8;
}
void writeBytes16Little(ByteBuffer& buffer, size_t* pos, uint16_t value) {
    buffer[*pos] = value & 0xFF;
    buffer[*pos + 1] = value >> 8;
    *pos += 2;
}
void writeBytes32Big(ByteBuffer& buffer, size_t pos, uint32_t value) {
    buffer[pos] = value >> 24;
    buffer[pos + 1] = (value >> 16) & 0xFF;
    buffer[pos + 2] = (value >> 8) & 0xFF;
    buffer[pos + 3] = value & 0xFF;
}
void writeBytes32Big(ByteBuffer& buffer, size_t* pos, uint32_t value) {
    buffer[*pos] = value >> 24;
    buffer[*pos + 1] = (value >> 16) & 0xFF;
    buffer[*pos + 2] = (value >> 8) & 0xFF;
    buffer[*pos + 3] = value & 0xFF;
    *pos += 4;
}
void writeBytes32Little(ByteBuffer& buffer, size_t pos, uint32_t value) {
    buffer[pos] = value & 0xFF;
    buffer[pos + 1] = (value >> 8) & 0xFF;
    buffer[pos + 2] = (value >> 16) & 0xFF;
    buffer[pos + 3] = value >> 24;
}
void writeBytes32Little(ByteBuffer& buffer, size_t* pos, uint32_t value) {
    buffer[*pos] = value & 0xFF;
    buffer[*pos + 1] = (value >> 8) & 0xFF;
    buffer[*pos + 2] = (value >> 16) & 0xFF;
    buffer[*pos + 3] = value >> 24;
    *pos += 4;
}
void writeBytes64Big(ByteBuffer& buffer, size_t pos, uint64_t value) {
    buffer[pos] = value >> 56;
    buffer[pos + 1] = (value >> 48) & 0xFF;
    buffer[pos + 2] = (value >> 40) & 0xFF;
    buffer[pos + 3] = (value >> 32) & 0xFF;
    buffer[pos + 4] = (value >> 24) & 0xFF;
    buffer[pos + 5] = (value >> 16) & 0xFF;
    buffer[pos + 6] = (value >> 8) & 0xFF;
    buffer[pos + 7] = value & 0xFF;
}
void writeBytes64Big(ByteBuffer& buffer, size_t* pos, uint64_t value) {
    buffer[*pos] = value >> 56;
    buffer[*pos + 1] = (value >> 48) & 0xFF;
    buffer[*pos + 2] = (value >> 40) & 0xFF;
    buffer[*pos + 3] = (value >> 32) & 0xFF;
    buffer[*pos + 4] = (value >> 24) & 0xFF;
    buffer[*pos + 5] = (value >> 16) & 0xFF;
    buffer[*pos + 6] = (value >> 8) & 0xFF;
    buffer[*pos + 7] = value & 0xFF;
    *pos += 8;
}
void writeBytes64Little(ByteBuffer& buffer, size_t pos, uint64_t value) {
    buffer[pos] = value & 0xFF;
    buffer[pos + 1] = (value >> 8) & 0xFF;
    buffer[pos + 2] = (value >> 16) & 0xFF;
    buffer[pos + 3] = (value >> 24) & 0xFF;
    buffer[pos + 4] = (value >> 32) & 0xFF;
    buffer[pos + 5] = (value >> 40) & 0xFF;
    buffer[pos + 6] = (value >> 48) & 0xFF;
    buffer[pos + 7] = value >> 56;
}
void writeBytes64Little(ByteBuffer& buffer, size_t* pos, uint64_t value) {
    buffer[*pos] = value & 0xFF;
    buffer[*pos + 1] = (value >> 8) & 0xFF;
    buffer[*pos + 2] = (value >> 16) & 0xFF;
    buffer[*pos + 3] = (value >> 24) & 0xFF;
    buffer[*pos + 4] = (value >> 32) & 0xFF;
    buffer[*pos + 5] = (value >> 40) & 0xFF;
    buffer[*pos + 6] = (value >> 48) & 0xFF;
    buffer[*pos + 7] = value >> 56;
    *pos += 8;
}

float loadFloat32Big(const ByteBuffer& buffer, size_t pos) {
    union {
        uint32_t i;
        float f;
    } value{};
    value.i = loadBytes32Big(buffer, pos);
    return value.f;
}
float loadFloat32Little(const ByteBuffer& buffer, size_t pos) {
    union {
        uint32_t i;
        float f;
    } value{};
    value.i = loadBytes32Little(buffer, pos);
    return value.f;
}

void writeFloat32Big(ByteBuffer& buffer, size_t pos, float value) {
    union {
        uint32_t i;
        float f;
    } data{};
    data.f = value;
    writeBytes32Big(buffer, pos, data.i);
}

void writeFloat32Little(ByteBuffer& buffer, size_t pos, float value) {
    union {
        uint32_t i;
        float f;
    } data{};
    data.f = value;
    writeBytes32Little(buffer, pos, data.i);
}

std::vector<uint8_t> loadFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << path << std::endl;
        return {};
    }
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> buffer(size);
    file.read(reinterpret_cast<char*>(buffer.data()), (std::streamsize)size);
    return buffer;
}

void writeFile(const std::string& path, const std::vector<uint8_t>& buffer) {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << path << std::endl;
        return;
    }
    file.write(reinterpret_cast<const char*>(buffer.data()), (std::streamsize)buffer.size());
}

struct Writer {
    ByteBuffer& buf;
    size_t pos;
    bool bigEndian = false;

    explicit Writer(ByteBuffer& buf) : buf(buf), pos(0) {}
    Writer(ByteBuffer& buf, size_t pos) : buf(buf), pos(pos) {}

    void write8(uint8_t value) {
        if (pos + 1 > buf.size()) {
            buf.resize(pos + 1);
        }
        buf[pos++] = value;
    }
    void write16(uint16_t value) {
        if (pos + 2 > buf.size()) {
            buf.resize(pos + 2);
        }
        if (bigEndian) {
            buf[pos++] = value >> 8;
            buf[pos++] = value & 0xFF;
        } else {
            buf[pos++] = value & 0xFF;
            buf[pos++] = value >> 8;
        }
    }
    void write32(uint32_t value) {
        if (pos + 4 > buf.size()) {
            buf.resize(pos + 4);
        }
        if (bigEndian) {
            buf[pos++] = value >> 24;
            buf[pos++] = (value >> 16) & 0xFF;
            buf[pos++] = (value >> 8) & 0xFF;
            buf[pos++] = value & 0xFF;
        } else {
            buf[pos++] = value & 0xFF;
            buf[pos++] = (value >> 8) & 0xFF;
            buf[pos++] = (value >> 16) & 0xFF;
            buf[pos++] = value >> 24;
        }
    }
    void write64(uint64_t value) {
        if (pos + 8 > buf.size()) {
            buf.resize(pos + 8);
        }
        if (bigEndian) {
            buf[pos++] = value >> 56;
            buf[pos++] = (value >> 48) & 0xFF;
            buf[pos++] = (value >> 40) & 0xFF;
            buf[pos++] = (value >> 32) & 0xFF;
            buf[pos++] = (value >> 24) & 0xFF;
            buf[pos++] = (value >> 16) & 0xFF;
            buf[pos++] = (value >> 8) & 0xFF;
            buf[pos++] = value & 0xFF;
        } else {
            buf[pos++] = value & 0xFF;
            buf[pos++] = (value >> 8) & 0xFF;
            buf[pos++] = (value >> 16) & 0xFF;
            buf[pos++] = (value >> 24) & 0xFF;
            buf[pos++] = (value >> 32) & 0xFF;
            buf[pos++] = (value >> 40) & 0xFF;
            buf[pos++] = (value >> 48) & 0xFF;
            buf[pos++] = value >> 56;
        }
    }

    void writeStr8(const std::string& str) { // str with length stored in 8 bits
        if (buf.size() > pos + 1 + str.size()) {
            buf.resize(pos + 1 + str.size());
        }
        if (str.size() > 255u) {
            std::cerr << "writeStr8: String too long: " << str.size() << std::endl;
            return;
        }
        write8(str.size());
        for (char c : str) {
            write8(c);
        }
    }
    void writeStr16(const std::string& str) { // str with length stored in 16 bits
        if (buf.size() > pos + 2 + str.size()) {
            buf.resize(pos + 2 + str.size());
        }
        if (str.size() > 65535u) {
            std::cerr << "writeStr16: String too long: " << str.size() << std::endl;
            return;
        }
        write16(str.size());
        for (char c : str) {
            write8(c);
        }
    }
    void writeStr32(const std::string& str) { // str with length stored in 32 bits
        if (buf.size() > pos + 4 + str.size()) {
            buf.resize(pos + 4 + str.size());
        }
        if (str.size() > 4294967295ul) {
            std::cerr << "writeStr32: String too long: " << str.size() << std::endl;
            return;
        }
        write32(str.size());
        for (char c : str) {
            write8(c);
        }
    }
    void writeStr64(const std::string& str) { // str with length stored in 64 bits
        if (buf.size() > pos + 8 + str.size()) {
            buf.resize(pos + 8 + str.size());
        }
        if (str.size() > 18446744073709551615ull) {
            std::cerr << "writeStr64: String too long: " << str.size() << std::endl;
            return;
        }
        write64(str.size());
        for (char c : str) {
            write8(c);
        }
    }

    void write(const ByteBuffer& buffer) {
        if (buf.size() > pos + buffer.size()) {
            buf.resize(pos + buffer.size());
        }
        for (uint8_t byte : buffer) {
            write8(byte);
        }
    }

    void writeFloat32(float value) {
        union {
            uint32_t i;
            float f;
        } data{};
        data.f = value;
        write32(data.i);
    }

    void writeFloat64(double value) {
        union {
            uint64_t i;
            double f;
        } data{};
        data.f = value;
        write64(data.i);
    }
};

struct Reader {
    const ByteBuffer& buf;
    size_t pos;
    bool bigEndian = false;

    explicit Reader(const ByteBuffer& buf) : buf(buf), pos(0) {}
    Reader(const ByteBuffer& buf, size_t pos) : buf(buf), pos(pos) {}

    uint8_t read8() {
        return buf[pos++];
    }
    uint16_t read16() {
        if (bigEndian) {
            uint16_t value = (buf[pos] << 8) | buf[pos + 1];
            pos += 2;
            return value;
        } else {
            uint16_t value = buf[pos] | (buf[pos + 1] << 8);
            pos += 2;
            return value;
        }
    }
    uint32_t read32() {
        if (bigEndian) {
            uint32_t value = (buf[pos] << 24) | (buf[pos + 1] << 16) | (buf[pos + 2] << 8) | buf[pos + 3];
            pos += 4;
            return value;
        } else {
            uint32_t value = buf[pos] | (buf[pos + 1] << 8) | (buf[pos + 2] << 16) | (buf[pos + 3] << 24);
            pos += 4;
            return value;
        }
    }
    uint64_t read64() {
        if (bigEndian) {
            uint64_t value = (static_cast<uint64_t>(buf[pos]) << 56) | (static_cast<uint64_t>(buf[pos + 1]) << 48) | (static_cast<uint64_t>(buf[pos + 2]) << 40) | (static_cast<uint64_t>(buf[pos + 3]) << 32) | (static_cast<uint64_t>(buf[pos + 4]) << 24) | (static_cast<uint64_t>(buf[pos + 5]) << 16) | (static_cast<uint64_t>(buf[pos + 6]) << 8) | static_cast<uint64_t>(buf[pos + 7]);
            pos += 8;
            return value;
        } else {
            uint64_t value = (static_cast<uint64_t>(buf[pos]) | (static_cast<uint64_t>(buf[pos + 1]) << 8) | (static_cast<uint64_t>(buf[pos + 2]) << 16) | (static_cast<uint64_t>(buf[pos + 3]) << 24) | (static_cast<uint64_t>(buf[pos + 4]) << 32) | (static_cast<uint64_t>(buf[pos + 5]) << 40) | (static_cast<uint64_t>(buf[pos + 6]) << 48) | (static_cast<uint64_t>(buf[pos + 7]) << 56));
            pos += 8;
            return value;
        }
    }

    std::string readStr8() { // str with length stored in 8 bits
        size_t len = read8();
        std::string str;
        str.reserve(len);
        for (size_t i = 0; i < len; ++i) {
            str.push_back((char)read8());
        }
        return str;
    }
    std::string readStr16() { // str with length stored in 16 bits
        size_t len = read16();
        std::string str;
        str.reserve(len);
        for (size_t i = 0; i < len; ++i) {
            str.push_back((char)read8());
        }
        return str;
    }
    std::string readStr32() { // str with length stored in 32 bits
        size_t len = read32();
        std::string str;
        str.reserve(len);
        for (size_t i = 0; i < len; ++i) {
            str.push_back((char)read8());
        }
        return str;
    }
    std::string readStr64() { // str with length stored in 64 bits
        size_t len = read64();
        std::string str;
        str.reserve(len);
        for (size_t i = 0; i < len; ++i) {
            str.push_back((char)read8());
        }
        return str;
    }

    ByteBuffer read(size_t len) {
        ByteBuffer buffer;
        buffer.reserve(len);
        for (size_t i = 0; i < len; ++i) {
            buffer.push_back(read8());
        }
        return buffer;
    }

    float readFloat32() {
        union {
            uint32_t i;
            float f;
        } data{};
        data.i = read32();
        return data.f;
    }

    double readFloat64() {
        union {
            uint64_t i;
            double f;
        } data{};
        data.i = read64();
        return data.f;
    }

    ByteBuffer readRemaining() {
        return read(buf.size() - pos);
    }
};

uint64_t checksum64(const ByteBuffer& buffer) {
    uint64_t checksum = 0;
    for (uint8_t byte : buffer) {
        checksum += byte;
    }
    return checksum;
}

uint64_t hash64(const ByteBuffer& buffer) {
    uint64_t hash = 0;
    for (uint8_t byte : buffer) {
        hash = (hash * 31) + byte;
    }
    return hash;
}

struct ByteBufferView {
    uint8_t* data;
    size_t size;

    ByteBufferView(uint8_t* data, size_t size) : data(data), size(size) {}
    ByteBufferView() : data(nullptr), size(0) {}

    uint8_t& operator[](size_t index) {
        if (index >= size) {
            std::cerr << "Error: Index out of bounds" << std::endl;
        }
        if (data == nullptr) {
            std::cerr << "Error: ByteBufferView is empty" << std::endl;
        }
        return data[index];
    }

    const uint8_t& operator[](size_t index) const {
        if (index >= size) {
            std::cerr << "Error: Index out of bounds" << std::endl;
        }
        if (data == nullptr) {
            std::cerr << "Error: ByteBufferView is empty" << std::endl;
        }
        return data[index];
    }

    [[nodiscard]] uint8_t* begin() const  {
        return data;
    }

    [[nodiscard]] uint8_t* end() const {
        return data + size;
    }
};

struct ConstByteBufferView {
    const uint8_t* data;
    size_t size;

    ConstByteBufferView(const uint8_t* data, size_t size) : data(data), size(size) {}
    ConstByteBufferView() : data(nullptr), size(0) {}

    const uint8_t& operator[](size_t index) const {
        if (index >= size) {
            std::cerr << "Error: Index out of bounds" << std::endl;
        }
        if (data == nullptr) {
            std::cerr << "Error: ConstByteBufferView is empty" << std::endl;
        }
        return data[index];
    }

    [[nodiscard]] const uint8_t* begin() const {
        return data;
    }

    [[nodiscard]] const uint8_t* end() const {
        return data + size;
    }
};