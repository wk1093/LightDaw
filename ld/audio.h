#pragma once

#include    <iostream>
#include "filetools.h"
#include <portaudio.h>
#include <vector>
#include <algorithm>


typedef std::vector<float> AudioBuffer;

// LightDAW is wav-based, not a single audio library being used when playing audio, we just render into a wav, and play that wav
// so we need to do a lot of math to mix audio and be able to output it to a wav file

AudioBuffer mixBuffers(const AudioBuffer& buffer1, const AudioBuffer& buffer2, int64_t offset = 0) {
    // if offset is positive, buffer2 is delayed
    // if offset is negative, buffer1 is delayed
    size_t size;
    if (offset > 0) {
        size = std::max(buffer1.size(), buffer2.size() + offset);
    } else {
        size = std::max(buffer1.size() - offset, buffer2.size());
    }
    AudioBuffer mixedBuffer(size);

    AudioBuffer* a = offset > 0 ? (AudioBuffer*)&buffer2 : (AudioBuffer*)&buffer1;
    AudioBuffer* b = offset > 0 ? (AudioBuffer*)&buffer1 : (AudioBuffer*)&buffer2;

    for (size_t i = 0; i < size; i++) {
        if (i < a->size() && i - offset >= 0 && i - offset < b->size()) {
            mixedBuffer[i] = (*a)[i] + (*b)[i - offset];
        } else if (i < a->size()) {
            mixedBuffer[i] = (*a)[i];
        } else if (i - offset >= 0 && i - offset < b->size()) {
            mixedBuffer[i] = (*b)[i - offset];
        }
    }

    return mixedBuffer;
}

void modifyVolume(AudioBuffer& buffer, float volume) { // 0.0f mute, 1.0f no change, 2.0f double volume
    for (float & i : buffer) {
        i *= volume;
    }
}

AudioBuffer addToBuffer(AudioBuffer& main, const AudioBuffer& toAdd, size_t offset = 0, float volume = 1.0f) {
    for (size_t i = 0; i < toAdd.size(); i++) {
        if (i + offset < main.size()) {
            main[i + offset] += toAdd[i] * volume;
        }
    }
    return main;
}

// we will make a data structure that acts as if you were playing sound to a speaker, but instead writes it to a buffer
// this way we can mix multiple sounds together and output them to a wav file
// when writing you give a "time" in milliseconds, and it will write the sound to the buffer at that time
// it will store as an AudioBuffer (float32), but you can write in different formats, it will convert it to float32
#define SAMPLE_RATE 44100

enum class AudioFormat {
    // my format
    Float32, // audiobuffer
    // wav formats
    UInt8, // bytebuffer
    Int16, // bytebuffer
    Int32, // bytebuffer



};

struct AudioOffset {
    size_t samples = 0;

    AudioOffset() = default;

    static AudioOffset fromSamples(size_t samples) {
        AudioOffset offset;
        offset.samples = samples;
        return offset;
    }

    static AudioOffset fromSeconds(double seconds) {
        AudioOffset offset;
        offset.samples = static_cast<size_t>(seconds * SAMPLE_RATE);
        return offset;
    }

    static AudioOffset fromMilliseconds(int64_t milliseconds) {
        AudioOffset offset;
        offset.samples = static_cast<size_t>(milliseconds * SAMPLE_RATE / 1000);
        return offset;
    }

    AudioOffset operator+(const AudioOffset& other) const {
        AudioOffset offset;
        offset.samples = samples + other.samples;
        return offset;
    }

    AudioOffset operator-(const AudioOffset& other) const {
        AudioOffset offset;
        offset.samples = samples - other.samples;
        return offset;
    }
};

struct AudioStream {
    AudioBuffer buffer{};

    AudioStream() = default;

    void write(const AudioBuffer& buf, AudioOffset offset = AudioOffset::fromSamples(0)) {
        if (offset.samples + buf.size() > buffer.size()) {
            buffer.resize(offset.samples + buf.size());
        }
        for (size_t i = 0; i < buf.size(); i++) {
            // mix the audio
            buffer[i + offset.samples] += buf[i];
            // this get's loud very quickly, so we need to normalize it
            if (buffer[i + offset.samples] > 1.0f) {
                buffer[i + offset.samples] = 1.0f;
            } else if (buffer[i + offset.samples] < -1.0f) {
                buffer[i + offset.samples] = -1.0f;
            }
        }
    }

    void write(const ByteBuffer& buf, AudioFormat format = AudioFormat::UInt8, AudioOffset offset = AudioOffset::fromSamples(0)) {
        AudioBuffer convertedBuffer;
        if (format == AudioFormat::Float32) {
            std::cerr << "Error: AudioStream::write called with Float32 format, but it is a byte buffer" << std::endl;
        } else if (format == AudioFormat::UInt8) {
            for (size_t i = 0; i < buf.size(); i++) {
                convertedBuffer.push_back(static_cast<float>(buf[i]) / 128.0f - 1.0f);
            }
        } else if (format == AudioFormat::Int16) {
            for (size_t i = 0; i < buf.size(); i += 2) {
                int16_t value = (buf[i] << 8) | buf[i + 1];
                convertedBuffer.push_back(static_cast<float>(value) / 32768.0f);
            }
        } else if (format == AudioFormat::Int32) {
            for (size_t i = 0; i < buf.size(); i += 4) {
                int32_t value = (buf[i] << 24) | (buf[i + 1] << 16) | (buf[i + 2] << 8) | buf[i + 3];
                convertedBuffer.push_back(static_cast<float>(value) / 2147483648.0f);
            }
        }
        else {
            std::cerr << "Error: AudioStream::write called with unsupported format" << std::endl;
        }
        write(convertedBuffer, offset);
    }

    void write(const AudioStream& stream, AudioOffset offset = AudioOffset::fromSamples(0)) {
        write(stream.buffer, offset);
    }

    void clear() {
        buffer.clear();
    }

    void playtest() {
        // play the audio
        Pa_Initialize();
        PaStream* stream;
        Pa_OpenDefaultStream(&stream, 0, 1, paFloat32, SAMPLE_RATE, paFramesPerBufferUnspecified, nullptr, nullptr);
        Pa_StartStream(stream);
        Pa_WriteStream(stream, buffer.data(), buffer.size());
        Pa_StopStream(stream);
        Pa_CloseStream(stream);
        Pa_Terminate();
    }
};

struct AudioPlayer {
    struct AudioPlayerData {
        AudioBuffer buffer;
        size_t position = 0;
    } data;

    PaStream* stream{};

    static int callback(const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData) {
        auto* data = (AudioPlayerData*)userData;
        if (data->position >= data->buffer.size()) {
            return paComplete;
        }
        float* out = (float*)outputBuffer;
        for (unsigned long i = 0; i < framesPerBuffer; i++) {
            if (data->position < data->buffer.size()) {
                *out++ = data->buffer[data->position++];
            } else {
                *out++ = 0.0f;
            }
        }
        return paContinue;
    }

    AudioPlayer() {
        PaError e = Pa_OpenDefaultStream(&stream, 0, 1, paFloat32, SAMPLE_RATE, paFramesPerBufferUnspecified, callback, &data);
        if (e != paNoError) {
            std::cerr << "Error: PortAudio failed to open stream" << std::endl;
            return;
        }
    }

    explicit AudioPlayer(const AudioBuffer& buffer) : AudioPlayer() {
        data.buffer = buffer;
    }

    ~AudioPlayer() {
        Pa_CloseStream(stream);
    }

    void play() {
        // prevent errors
        if (data.buffer.empty()) {
            std::cerr << "Error: AudioPlayer::play called with empty buffer" << std::endl;
            return;
        }
        // make sure there is no corruption in the buffer
        if (data.position >= data.buffer.size()) {
            data.position = 0;
        }
        // if already playing, do nothing
        PaError active = Pa_IsStreamActive(stream);
        if (active == 1) {
            return;
        }
        PaError e = Pa_StartStream(stream);
        if (e != paNoError) {
            std::cerr << "Error: PortAudio failed to start stream" << std::endl;
            std::cerr << Pa_GetErrorText(e) << std::endl;
            return;
        }
    }

    void pause() const {
        PaError stopped = Pa_IsStreamStopped(stream);
        if (stopped == 1) {
            return;
        }
        PaError e = Pa_StopStream(stream);
        if (e != paNoError) {
            std::cerr << "Error: PortAudio failed to stop stream" << std::endl;
            std::cerr << Pa_GetErrorText(e) << std::endl;
            return;
        }
    }

    void resume() const {
        PaError stopped = Pa_IsStreamStopped(stream);
        if (stopped == 0) {
            return;
        }
        PaError e = Pa_StartStream(stream);
        if (e != paNoError) {
            std::cerr << "Error: PortAudio failed to start stream" << std::endl;
            std::cerr << Pa_GetErrorText(e) << std::endl;
            return;
        }
    }

    void stop() {
        PaError stopped = Pa_IsStreamStopped(stream);
        if (stopped == 1) {
            data.position = 0;
            return;
        }
        PaError e = Pa_AbortStream(stream);
        data.position = 0;
        if (e != paNoError) {
            std::cerr << "Error: PortAudio failed to stop stream" << std::endl;
            std::cerr << Pa_GetErrorText(e) << std::endl;
            return;
        }

    }

    [[nodiscard]] bool isPlaying() const {
        return Pa_IsStreamActive(stream) == 1;
    }

    float progress() const {
        return static_cast<float>(data.position) / data.buffer.size();
    }

    void seek(float progress) {
        data.position = static_cast<size_t>(progress * data.buffer.size());
    }

    const AudioBuffer& getBuffer() const {
        return data.buffer;
    }

    size_t getPosition() const {
        return data.position;
    }

};

void initAudio() {
    Pa_Initialize();
}

void terminateAudio() {
    Pa_Terminate();
}

std::vector<AudioPlayer> __audioPlaytestPlayers;

void playtest(const AudioBuffer& buffer) {
    __audioPlaytestPlayers.erase(std::remove_if(__audioPlaytestPlayers.begin(), __audioPlaytestPlayers.end(), [](const AudioPlayer& player) {
        return !player.isPlaying();
    }), __audioPlaytestPlayers.end());

    __audioPlaytestPlayers.emplace_back(buffer);
    __audioPlaytestPlayers.back().play();
}


