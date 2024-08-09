#pragma once

#include <iomanip>
#include <utility>
#include <map>
#include "filetools.h"

// C++17 basic LightDaw Project file loader/writer

// implement custom archive format to contain multiple files in one.
// This will be used to store all the project files in one file.

struct FileID {
    // many file that we have multiple of (midi, instruments, etc) will have an ID for the filename
    // this allows us to not have to store the filename in the archive, and just use the ID (a 64-bit integer)
    // these IDsi are hashes of the file contents so they are unique, but the same for the same file
    // this way we can easily check if a file is already in the archive

    uint64_t id{};

    FileID() = default;
    FileID(uint64_t id) : id(id) {}
    explicit FileID(const ByteBuffer& contents) {
        id = hash64(contents);
    }

    [[nodiscard]] std::string toFilename() const {
        // example id: 128: filename: 'ld0000000000000080'
        std::stringstream ss;
        ss << std::hex << std::setw(16) << std::setfill('0') << id;
        return "ld" + ss.str();
    }

    [[nodiscard]] std::string toFilename(const std::string& ext) const {
        return toFilename() + ext;
    }

    [[nodiscard]] std::string toFilename(const std::string& dir, const std::string& ext) const {
        return dir + "/" + toFilename(ext);
    }

    static FileID fromFilename(const std::string& filename) {
        if (filename.size() < 18) throw std::runtime_error("Invalid filename");
        // sometimes it will start with a directory, so we just remove it
        std::string ff = filename.substr(filename.find_last_of('/') + 1, 18);
        if (ff.substr(0, 2) != "ld") throw std::runtime_error("Invalid filename");
        // sometimes the filename will have an added extension, so we just ignore it
        std::string hex = ff.substr(2, 16);
        uint64_t id = std::stoull(hex, nullptr, 16);
        return {id};
    }
};

struct ArchiveEntry {
    std::string filename; // 2 bytes for len
    uint64_t offset{};
    uint64_t size{};
    uint64_t checksum{};
};

struct ArchiveContainedFile {
    std::string filename;
    std::vector<uint8_t> data;

    ArchiveContainedFile() = default;
    ArchiveContainedFile(std::string filename, const std::vector<uint8_t>& data) : filename(std::move(filename)), data(data) {}

};

struct ArchiveFile {
    uint32_t identifier{}; // 'LDAR' (reversed because of little-endian)
    uint32_t version{};
    uint64_t entryCount{};
    uint64_t dataSectionSize{};
    uint64_t dataChecksum{};
    std::vector<ArchiveEntry> entries; // number of entries = entryCount, all entry bytes together = entrySectionSize
    std::vector<uint8_t> data; // dataSectionSize bytes, final checksum = dataChecksum

    ArchiveFile() = default;
    ArchiveFile(uint32_t identifier, uint32_t version, uint64_t entryCount, uint64_t dataSectionSize, uint64_t dataChecksum, std::vector<ArchiveEntry> entries, std::vector<uint8_t> data) : identifier(identifier), version(version), entryCount(entryCount), dataSectionSize(dataSectionSize), dataChecksum(dataChecksum), entries(std::move(entries)), data(std::move(data)) {}
    explicit ArchiveFile(const std::vector<ArchiveContainedFile>& files) {
        identifier = 0x4C444152;
        version = 1;
        entryCount = files.size();
        for (const auto& file : files) {
            ArchiveEntry entry;
            entry.filename = file.filename;
            entry.offset = data.size();
            entry.size = file.data.size();
            entry.checksum = checksum64(file.data);
            entries.push_back(entry);
            data.insert(data.end(), file.data.begin(), file.data.end());
        }
        dataSectionSize = data.size();
        dataChecksum = checksum64(data);
    }

    [[nodiscard]] ArchiveContainedFile getFile(const std::string& filename) const {
        for (const auto& entry : entries) {
            if (entry.filename == filename) {
                return {filename, std::vector<uint8_t>(data.begin() + (int64_t)entry.offset, data.begin() + (int64_t)entry.offset + (int64_t)entry.size)};
            }
        }
        throw std::runtime_error("File not found in archive");
    }

    [[nodiscard]] ArchiveContainedFile getFile(uint64_t index) const {
        if (index >= entries.size()) throw std::runtime_error("Index out of bounds");
        const auto& entry = entries[index];
        return {entry.filename, std::vector<uint8_t>(data.begin() + (int64_t)entry.offset, data.begin() + (int64_t)entry.offset + (int64_t)entry.size)};
    }

    [[nodiscard]] std::vector<std::string> getFileNames() const {
        std::vector<std::string> names;
        for (const auto& entry : entries) {
            names.push_back(entry.filename);
        }
        return names;
    }

    [[nodiscard]] std::map<std::string, ConstByteBufferView> getFiles() const {
        std::map<std::string, ConstByteBufferView> files;
        for (const auto& entry : entries) {
            files[entry.filename] = ConstByteBufferView(data.data() + entry.offset, entry.size);
        }
        return files;
    }

    [[nodiscard]] ByteBuffer toBytes() const {
        if (identifier != 0x4C444152) throw std::runtime_error("Invalid LDAR identifier");
        if (version != 1) throw std::runtime_error("Invalid LDAR version");
        if (entries.size() != entryCount) throw std::runtime_error("Invalid entry count");
        if (data.size() != dataSectionSize) throw std::runtime_error("Invalid data size");
        ByteBuffer buffer;
        Writer writer(buffer);

        writer.write32(identifier);
        writer.write32(version);
        writer.write64(entryCount);
        writer.write64(dataSectionSize);
        if (dataChecksum != checksum64(data)) throw std::runtime_error("Invalid data checksum on write");
        writer.write64(dataChecksum);
        for (const auto& entry : entries) {
            writer.writeStr16(entry.filename);
            writer.write64(entry.offset);
            writer.write64(entry.size);
            // verify checksum
            if (entry.checksum != checksum64(std::vector<uint8_t>(data.begin() + (int64_t)entry.offset, data.begin() + (int64_t)entry.offset + (int64_t)entry.size))) throw std::runtime_error("Invalid entry checksum on write");
            writer.write64(entry.checksum);
        }
        writer.write(data);

        return buffer;
    }

    static ArchiveFile fromBytes(ByteBuffer& buffer) {
        Reader reader(buffer);

        ArchiveFile header;
        header.identifier = reader.read32();
        if (header.identifier != 0x4C444152) throw std::runtime_error("Invalid LDAR identifier");
        header.version = reader.read32();
        if (header.version != 1) throw std::runtime_error("Invalid LDAR version");
        header.entryCount = reader.read64();
        header.dataSectionSize = reader.read64();
        header.dataChecksum = reader.read64();
        for (uint64_t i = 0; i < header.entryCount; i++) {
            ArchiveEntry entry;
            entry.filename = reader.readStr16();
            entry.offset = reader.read64();
            entry.size = reader.read64();
            entry.checksum = reader.read64();
            // verify
            if (entry.offset + entry.size > header.dataSectionSize) throw std::runtime_error("Invalid entry offset/size");
            header.entries.push_back(entry);
        }
        header.data = reader.read(header.dataSectionSize);
        // verify
        if (header.dataChecksum != checksum64(header.data)) throw std::runtime_error("Invalid data checksum");
        for (const auto& entry : header.entries) {
            if (entry.checksum != checksum64(std::vector<uint8_t>(header.data.begin() + (int64_t)entry.offset, header.data.begin() + (int64_t)entry.offset + (int64_t)entry.size))) throw std::runtime_error("Invalid entry checksum");
        }

        return header;
    }
    static ArchiveFile fromFile(const std::string& filename) {
        ByteBuffer b = loadFile(filename);
        return fromBytes(b);
    }
};

struct LdipFile { // LightDaw Project file
    uint32_t identifier{}; // 'LDIP' (reversed because of little-endian)
    uint32_t version{};
    std::string name; // 2 bytes for len of all these
    std::string author;
    std::string description;
    std::string projVersion;
    uint64_t creationTime{};
    uint64_t lastModifiedTime{};

    LdipFile() = default;
    LdipFile(uint32_t identifier, uint32_t version, std::string name, std::string author, std::string description, std::string projVersion, uint64_t creationTime, uint64_t lastModifiedTime) : identifier(identifier), version(version), name(std::move(name)), author(std::move(author)), description(std::move(description)), projVersion(std::move(projVersion)), creationTime(creationTime), lastModifiedTime(lastModifiedTime) {}
    LdipFile(std::string name, std::string author, std::string description, std::string projVersion) : identifier(0x4C444950), version(1), name(std::move(name)), author(std::move(author)), description(std::move(description)), projVersion(std::move(projVersion)), creationTime(std::time(nullptr)), lastModifiedTime(std::time(nullptr)) {}
    [[nodiscard]] ByteBuffer toBytes() const {
        if (identifier != 0x4C444950) throw std::runtime_error("Invalid LDIP identifier");
        if (version != 1) throw std::runtime_error("Invalid LDIP version");
        ByteBuffer buffer;
        Writer writer(buffer);

        writer.write32(identifier);
        writer.write32(version);
        writer.writeStr16(name);
        writer.writeStr16(author);
        writer.writeStr16(description);
        writer.writeStr16(projVersion);
        writer.write64(creationTime);
        writer.write64(lastModifiedTime);

        return buffer;
    }

    static LdipFile fromBytes(ByteBuffer& buffer) {
        Reader reader(buffer);

        LdipFile header;
        header.identifier = reader.read32();
        if (header.identifier != 0x4C444950) throw std::runtime_error("Invalid LDIP identifier");
        header.version = reader.read32();
        if (header.version != 1) throw std::runtime_error("Invalid LDIP version");
        header.name = reader.readStr16();
        header.author = reader.readStr16();
        header.description = reader.readStr16();
        header.projVersion = reader.readStr16();
        header.creationTime = reader.read64();
        header.lastModifiedTime = reader.read64();

        return header;
    }
};

struct LdpfFile { // LightDaw Pattern file (a pattern just links a midi file to a synth, or multiple synths)
    uint32_t identifier{}; // 'LDPF' (reversed because of little-endian)
    uint32_t version{};
    std::string name; // 2 bytes for len
    uint16_t pairCount{};
    struct Pair {
        FileID midiFileID;
        FileID synthFileID;
    };
    std::vector<Pair> pairs;

    LdpfFile() = default;
    LdpfFile(uint32_t identifier, uint32_t version, std::string name, uint16_t pairCount, const std::vector<Pair>& pairs) : identifier(identifier), version(version), name(std::move(name)), pairCount(pairCount), pairs(pairs) {
        if (pairs.size() != pairCount) throw std::runtime_error("Invalid pair count");
    }
    LdpfFile(std::string name, const std::vector<Pair>& pairs) : identifier(0x4C445046), version(1), name(std::move(name)), pairCount(pairs.size()), pairs(pairs) {}

    [[nodiscard]] ByteBuffer toBytes() const {
        if (identifier != 0x4C445046) throw std::runtime_error("Invalid LDPF identifier");
        if (version != 1) throw std::runtime_error("Invalid LDPF version");
        if (pairs.size() != pairCount) throw std::runtime_error("Invalid pair count");
        ByteBuffer buffer;
        Writer writer(buffer);

        writer.write32(identifier);
        writer.write32(version);
        writer.writeStr16(name);
        writer.write16(pairCount);
        for (const auto& pair : pairs) {
            writer.write64(pair.midiFileID.id);
            writer.write64(pair.synthFileID.id);
        }

        return buffer;
    }

    static LdpfFile fromBytes(ByteBuffer& buffer) {
        Reader reader(buffer);

        LdpfFile header;
        header.identifier = reader.read32();
        if (header.identifier != 0x4C445046) throw std::runtime_error("Invalid LDPF identifier");
        header.version = reader.read32();
        if (header.version != 1) throw std::runtime_error("Invalid LDPF version");
        header.name = reader.readStr16();
        header.pairCount = reader.read16();
        for (uint16_t i = 0; i < header.pairCount; i++) {
            header.pairs.push_back({FileID(reader.read64()), FileID(reader.read64())});
        }

        return header;
    }
};

struct LdifFile {
    uint32_t identifier{}; // 'LDIF' (reversed because of little-endian)
    uint32_t version{};
    std::string name; // 2 bytes for len
    uint8_t flags{}; // right now it's just 0, 1, or 2
    // 0: This instrument is a VST plugin (NOT IMPLEMENTED)
    // 1: This is a sample-based instrument, using an audio file (NOT IMPLEMENTED)
    // 2: This is a built-in synth
    FileID id; // used for 1 or 2 (1: this points to the audio file, 2: this points to the synth id (there is no synth file, the id's are hardcoded))
    // for 0: we don't really know what to do yet, maybe store the VSTs plugin file, or just it's name?

    ByteBuffer instrumentData; // for synths, this is the parameters, for other instruments, it might be some struct serialized

    LdifFile() = default;

    static const uint8_t FLAGS_VST = 0;
    static const uint8_t FLAGS_SAMPLE = 1;
    static const uint8_t FLAGS_SYNTH = 2;

    LdifFile(uint32_t identifier, uint32_t version, std::string name, uint8_t flags, const FileID& id) : identifier(identifier), version(version), name(std::move(name)), flags(flags), id(id) {}
    LdifFile(std::string name, uint8_t flags, const FileID& id) : identifier(0x4C444946), version(1), name(std::move(name)), flags(flags), id(id) {}
    [[nodiscard]] ByteBuffer toBytes() const {
        if (identifier != 0x4C444946) throw std::runtime_error("Invalid LDIF identifier");
        if (version != 1) throw std::runtime_error("Invalid LDIF version");
        ByteBuffer buffer;
        Writer writer(buffer);

        writer.write32(identifier);
        writer.write32(version);
        writer.writeStr16(name);
        if (flags > 2) throw std::runtime_error("Invalid flags");
        if (flags == 0) throw std::runtime_error("VST plugin instruments are not implemented yet");
        writer.write8(flags);
        writer.write64(id.id);
        writer.write(instrumentData);

        return buffer;
    }

    static LdifFile fromBytes(ByteBuffer& buffer) {
        Reader reader(buffer);

        LdifFile header;
        header.identifier = reader.read32();
        if (header.identifier != 0x4C444946) throw std::runtime_error("Invalid LDIF identifier");
        header.version = reader.read32();
        if (header.version != 1) throw std::runtime_error("Invalid LDIF version");
        header.name = reader.readStr16();
        header.flags = reader.read8();
        if (header.flags > 2) throw std::runtime_error("Invalid flags");
        if (header.flags == 0) throw std::runtime_error("VST plugin instruments are not implemented yet");
        header.id = FileID(reader.read64());

        header.instrumentData = reader.readRemaining();

        return header;
    }
};

// TODO: other formats



