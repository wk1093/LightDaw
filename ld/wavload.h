#pragma once

#include "filetools.h"

#include <utility>
#include "audio.h"


// C++17 basic WAV file loader

struct WavChunk {
    uint32_t id{};
    uint32_t size{};
    ByteBuffer data;
};

std::vector<WavChunk> loadChunksFromBytes(const ByteBuffer& buffer, size_t pos = 0) {
    std::vector<WavChunk> chunks;
    while (pos < buffer.size()) {
        WavChunk chunk;
        // chunkID is 4 bytes big endian
        // chunkSize is 4 bytes little endian
        chunk.id = loadBytes32Big(buffer, pos);
        pos += 4;
        chunk.size = loadBytes32Little(buffer, pos);
        pos += 4;
        chunk.data.resize(chunk.size);
        for (size_t i = 0; i < chunk.size; i++) {
            chunk.data[i] = buffer[pos + i];
        }
        pos += chunk.size;
        chunks.push_back(chunk);
        if (pos + 8 > buffer.size()) {
            break;
        }
    }
    return chunks;
}

struct RiffChunk {
    uint32_t id{};
    uint32_t size{};
    uint32_t format{};

    RiffChunk() = default;
    RiffChunk(uint32_t id, uint32_t size, uint32_t format) : id(id), size(size), format(format) {}
};

struct FmtChunk {
    uint32_t id{};
    uint32_t size{};
    uint16_t audioFormat{};
    uint16_t numChannels{};
    uint32_t sampleRate{};
    uint32_t byteRate{};
    uint16_t blockAlign{};
    uint16_t bitsPerSample{};
    std::vector<uint8_t> extraData;

    FmtChunk() = default;
    FmtChunk(uint32_t id, uint32_t size, uint16_t audioFormat, uint16_t numChannels, uint32_t sampleRate, uint32_t byteRate, uint16_t blockAlign, uint16_t bitsPerSample, const std::vector<uint8_t>& extraData) : id(id), size(size), audioFormat(audioFormat), numChannels(numChannels), sampleRate(sampleRate), byteRate(byteRate), blockAlign(blockAlign), bitsPerSample(bitsPerSample), extraData(extraData) {}
    explicit FmtChunk(const WavChunk& chunk) : id(chunk.id), size(chunk.size) {
        if (chunk.size < 16) {
            audioFormat = 0;
            numChannels = 0;
            sampleRate = 0;
            byteRate = 0;
            blockAlign = 0;
            bitsPerSample = 0;
            extraData.clear();
            std::cerr << "Error: FmtChunk constructor called with too small chunk" << std::endl;
            return;
        }
        if (chunk.id == 0x666d7420) {
            audioFormat = loadBytes32Little(chunk.data, 0);
            numChannels = loadBytes32Little(chunk.data, 2);
            sampleRate = loadBytes32Little(chunk.data, 4);
            byteRate = loadBytes32Little(chunk.data, 8);
            blockAlign = loadBytes32Little(chunk.data, 12);
            bitsPerSample = loadBytes32Little(chunk.data, 14);
            if (chunk.size > 16) {
                extraData.resize(chunk.size - 16);
                for (size_t i = 0; i < chunk.size - 16; i++) {
                    extraData[i] = chunk.data[16 + i];
                }
            }
        } else {
            audioFormat = 0;
            numChannels = 0;
            sampleRate = 0;
            byteRate = 0;
            blockAlign = 0;
            bitsPerSample = 0;
            extraData.clear();
            std::cerr << "Error: FmtChunk constructor called with non-FMT chunk" << std::endl;
        }
    }

    static bool isGood(const WavChunk& chunk) {
        return chunk.id == 0x666d7420 && chunk.size >= 16;
    }
};

struct DataChunk {
    uint32_t id{};
    uint32_t size{};
    ByteBuffer data;

    DataChunk() = default;
    DataChunk(uint32_t id, uint32_t size, const std::vector<uint8_t>& data) : id(id), size(size), data(data) {}
    explicit DataChunk(const WavChunk& chunk) : id(chunk.id), size(chunk.size), data(chunk.data) {
        if (chunk.id != 0x64617461) {
            data.clear();
            std::cerr << "Error: DataChunk constructor called with non-DATA chunk" << std::endl;
        }
    }

    static bool isGood(const WavChunk& chunk) {
        return chunk.id == 0x64617461;
    }
};


struct WavFile {
    RiffChunk riff;
    FmtChunk fmt;
    DataChunk data;
    std::vector<WavChunk> otherChunks;

    WavFile() = default;

    explicit WavFile(const AudioBuffer& buf, AudioFormat format=AudioFormat::UInt8, uint32_t sampleRate=44100, uint16_t numChannels=1) {
        // Create a suitable WAV file from an audio buffer
        riff = RiffChunk(0x52494646, 0, 0x57415645);
        if (format == AudioFormat::Float32) {
            std::cerr << "Error: WavFile constructor called with Float32 format, but it is a byte buffer" << std::endl;
            return;
        }
        fmt = FmtChunk(0x666d7420, 16, 1, numChannels, sampleRate, sampleRate * numChannels * (format == AudioFormat::UInt8 ? 1 : format == AudioFormat::Int16 ? 2 : 4), numChannels * (format == AudioFormat::UInt8 ? 1 : format == AudioFormat::Int16 ? 2 : 4), format == AudioFormat::UInt8 ? 8 : format == AudioFormat::Int16 ? 16 : 32, {});
        ByteBuffer buffer;
        buffer.resize(buf.size() * (format == AudioFormat::UInt8 ? 1 : format == AudioFormat::Int16 ? 2 : 4) * numChannels);
        for (size_t i = 0; i < buf.size(); i++) {
            if (format == AudioFormat::UInt8) {
                buffer[i] = static_cast<uint8_t>(buf[i] * 127.0f + 128.0f);
            } else if (format == AudioFormat::Int16) {
                writeBytes16Little(buffer, i * 2, static_cast<int16_t>(buf[i] * 32767.0f));
            } else if (format == AudioFormat::Int32) {
                writeBytes32Little(buffer, i * 4, static_cast<int32_t>(buf[i] * 2147483647.0f));
            }
        }
        data = DataChunk(0x64617461, buffer.size(), buffer);
    }

    static WavFile loadFromFile(const std::string& filename) {
        return loadFromBytes(loadFile(filename));
    }

    static WavFile loadFromBytes(const std::vector<uint8_t>& buffer) {
        WavFile wavFile;
        // we can't load riff with loadChunksFromBytes because it's not a chunk
        if (buffer.size() < 12) {
            std::cerr << "Error: File too small to be a WAV file" << std::endl;
            return wavFile;
        }
        if (loadBytes32Big(buffer, 0) != 0x52494646) {
            std::cerr << "Error: File does not start with RIFF" << std::endl;
            return wavFile;
        }
        wavFile.riff = RiffChunk(0x52494646, loadBytes32Little(buffer, 4), loadBytes32Big(buffer, 8));
        if (wavFile.riff.format != 0x57415645) {
            std::cerr << "Error: File does not start with WAVE" << std::endl;
            return wavFile;
        }
        std::vector<WavChunk> chunks = loadChunksFromBytes(buffer, 12);
        bool fmtFound = false;
        bool dataFound = false;
        for (const WavChunk& chunk : chunks) {
            if (FmtChunk::isGood(chunk)) {
                if (fmtFound) {
                    std::cerr << "Error: Multiple FMT chunks found" << std::endl;
                    continue;
                }
                wavFile.fmt = FmtChunk(chunk);
                fmtFound = true;
            } else if (DataChunk::isGood(chunk)) {
                if (dataFound) {
                    std::cerr << "Error: Multiple DATA chunks found" << std::endl;
                    continue;
                }
                wavFile.data = DataChunk(chunk);
                dataFound = true;
            } else {
                wavFile.otherChunks.push_back(chunk);
            }
        }
        if (!fmtFound) {
            std::cerr << "Error: No FMT chunk found" << std::endl;
        }
        if (!dataFound) {
            std::cerr << "Error: No DATA chunk found" << std::endl;
        }
        return wavFile;
    }

    ByteBuffer& getAudioData() {
        return data.data;
    }

    [[nodiscard]] AudioFormat getAudioFormat() const {
        if (fmt.bitsPerSample == 8) {
            return AudioFormat::UInt8;
        } else if (fmt.bitsPerSample == 16) {
            return AudioFormat::Int16;
        } else if (fmt.bitsPerSample == 32) {
            return AudioFormat::Int32;
        } else {
            std::cerr << "Error: Unknown bits per sample" << std::endl;
            return AudioFormat::UInt8;
        }
    }

    ByteBuffer toBytes() {
        ByteBuffer buffer;
        buffer.resize((4*3) + (4*8) + fmt.extraData.size() + 4 + 4 + data.data.size());
        size_t pos = 0;
        writeBytes32Big(buffer, &pos, riff.id);
        writeBytes32Little(buffer, &pos, riff.size);
        writeBytes32Big(buffer, &pos, riff.format);

        writeBytes32Big(buffer, &pos, fmt.id);
        writeBytes32Little(buffer, &pos, fmt.size);

        writeBytes16Little(buffer, &pos, fmt.audioFormat);
        writeBytes16Little(buffer, &pos, fmt.numChannels);
        writeBytes32Little(buffer, &pos, fmt.sampleRate);
        writeBytes32Little(buffer, &pos, fmt.byteRate);
        writeBytes16Little(buffer, &pos, fmt.blockAlign);
        writeBytes16Little(buffer, &pos, fmt.bitsPerSample);
        for (uint8_t byte : fmt.extraData) {
            buffer.push_back(byte);
        }

        writeBytes32Big(buffer, &pos, data.id);
        writeBytes32Little(buffer, &pos, data.size);
        if (data.data.size() != data.size) {
            std::cerr << "Error: Data size does not match" << std::endl;
            std::cerr << "Data size: " << data.data.size() << ", chunk size: " << data.size << std::endl;
        }
        for (uint8_t byte : data.data) {
            writeByte(buffer, &pos, byte);
        }
        return buffer;
    }

    void saveToFile(const std::string& filename) {
        writeFile(filename, toBytes());
    }
};