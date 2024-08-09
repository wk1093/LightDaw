#pragma once

#include "filetools.h"

#include "audio.h"
#include <cmath>

#define GLOBAL_VOLUME 0.5f // this lets us mix without clipping

// C++17 basic LightDaw Synthesizer

struct Synth { // abstract class
    float sampleRate = 44100;
    float frequency = 440;
    float amplitude = 0.5;
    float volume = 1.0;
    bool open = false;

    virtual AudioBuffer generateSamples(size_t sampleCount) = 0;
    virtual AudioBuffer generateSeconds(double seconds) {
        return generateSamples(static_cast<size_t>(seconds * sampleRate));
    }

    virtual void update(double time, double bpm) {} // time in seconds

    virtual ~Synth() = default;

    inline static const uint64_t id = 0;

    // this function lets the gui of what parameters the synth has, and how to change them
    // this function is not required to be implemented, but if it isn't the synth has no GUI/parameters

    virtual void drawGui() {
        if (ImGui::Begin("Base Synth", &open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking)) {
            ImGui::Text("Base Synth");
            ImGui::End();
        }
    }

    virtual ByteBuffer serializeParams() {
        return {};
    }

    virtual DeserializeResult deserializeParams(const ByteBuffer& data) {
        return Success;
    }
};

struct SineSynth : public Synth {

    AudioBuffer generateSamples(size_t sampleCount) override {
        AudioBuffer buffer(sampleCount);
        for (size_t i = 0; i < sampleCount; i++) {
            float t = static_cast<float>(i) / sampleRate;
            float value = amplitude * std::sin(2.0f * (float)M_PI * frequency * t);
            buffer[i] = value;
        }
        return buffer;
    }

    inline static const uint64_t id = 1;

    void drawGui() override {
        if (ImGui::Begin("Sine Synth", &open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking)) {
            ImGui::Text("Sine Synth");
            ImGui::End();
        }
    }
};

struct TriangleSynth : public Synth {

    AudioBuffer generateSamples(size_t sampleCount) override {
        AudioBuffer buffer(sampleCount);
        for (size_t i = 0; i < sampleCount; i++) {
            float t = static_cast<float>(i) / sampleRate;
            float value = amplitude * 2.0f * std::abs(2.0f * (frequency * t - std::floor(frequency * t + 0.5f))) - 1.0f;
            buffer[i] = value;
        }
        return buffer;
    }

    inline static const uint64_t id = 2;

    void drawGui() override {
        if (ImGui::Begin("Triangle Synth", &open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking)) {
            ImGui::Text("Triangle Synth");
            ImGui::End();
        }
    }
};

struct SquareSynth : public Synth {
    AudioBuffer generateSamples(size_t sampleCount) override {
        AudioBuffer buffer(sampleCount);
        for (size_t i = 0; i < sampleCount; i++) {
            float t = static_cast<float>(i) / sampleRate;
            float value = amplitude * (std::sin(2.0f * (float)M_PI * frequency * t) > 0.0f ? 1.0f : -1.0f);
            buffer[i] = value;
        }
        return buffer;
    }

    inline static const uint64_t id = 3;

    void drawGui() override {
        if (ImGui::Begin("Square Synth", &open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking)) {
            ImGui::Text("Square Synth");
            ImGui::End();
        }
    }
};

struct SawSynth : public Synth {
    AudioBuffer generateSamples(size_t sampleCount) override {
        AudioBuffer buffer(sampleCount);
        for (size_t i = 0; i < sampleCount; i++) {
            float t = static_cast<float>(i) / sampleRate;
            float value = amplitude * 2.0f * (frequency * t - std::floor(frequency * t + 0.5f));
            buffer[i] = value;
        }
        return buffer;
    }

    inline static const uint64_t id = 4;

    void drawGui() override {
        if (ImGui::Begin("Saw Synth", &open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking)) {
            ImGui::Text("Saw Synth");
            ImGui::End();
        }
    }
};

struct EnvelopeSynth : public Synth {
    float attack = 0.1f;
    float decay = 0.1f;
    float sustain = 0.5f;
    float release = 0.1f;
    float attackVol = 1.0f;
    float decayVol = 0.5f;
    float releaseVol = 0.0f;
    float currentVol = 0.0f;
    // what to use (sine, triangle, square, saw)
    enum class WaveType {
        Sine,
        Triangle,
        Square,
        Saw
    } waveType = WaveType::Sine;

    AudioBuffer generateSamples(size_t sampleCount) override {
        AudioBuffer buffer(sampleCount);
        float time = 0.0f;
        for (size_t i = 0; i < sampleCount; i++) {
            float t = static_cast<float>(i) / sampleRate;
            if (time < attack) {
                currentVol = time / attack * (attackVol - 0.0f) + 0.0f;
            } else if (time < attack + decay) {
                currentVol = (time - attack) / decay * (decayVol - attackVol) + attackVol;
            } else if (time < attack + decay + sustain) {
                currentVol = sustain;
            } else if (time < attack + decay + sustain + release) {
                currentVol = (time - attack - decay - sustain) / release * (releaseVol - sustain) + sustain;
            } else {
                currentVol = 0.0f;
            }
            float value = 0.0f;
            float vol = amplitude * currentVol * GLOBAL_VOLUME;
            switch (waveType) {
                case WaveType::Sine:
                    value = vol * std::sin(2.0f * (float)M_PI * frequency * t);
                    break;
                case WaveType::Triangle:
                    value = vol * 2.0f * std::abs(2.0f * (frequency * t - std::floor(frequency * t + 0.5f))) - 1.0f;
                    break;
                case WaveType::Square:
                    value = vol * (std::sin(2.0f * (float)M_PI * frequency * t) > 0.0f ? 1.0f : -1.0f);
                    break;
                case WaveType::Saw:
                    value = vol * 2.0f * (frequency * t - std::floor(frequency * t + 0.5f));
                    break;
            }
            buffer[i] = value;
            time += 1.0f / sampleRate;
        }
        return buffer;
    }

    inline static const uint64_t id = 5;

    void drawGui() override {
        if (ImGui::Begin("Envelope Synth", &open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking)) {
            ImGui::Text("Envelope Synth");
            // basic combo
            if (ImGui::BeginCombo("Wave Type", waveType == WaveType::Sine ? "Sine" : waveType == WaveType::Triangle ? "Triangle" : waveType == WaveType::Square ? "Square" : "Saw")) {
                if (ImGui::Selectable("Sine", waveType == WaveType::Sine)) waveType = WaveType::Sine;
                if (ImGui::Selectable("Triangle", waveType == WaveType::Triangle)) waveType = WaveType::Triangle;
                if (ImGui::Selectable("Square", waveType == WaveType::Square)) waveType = WaveType::Square;
                if (ImGui::Selectable("Saw", waveType == WaveType::Saw)) waveType = WaveType::Saw;
                ImGui::EndCombo();
            }
            ImGui::SliderFloat("Attack", &attack, 0.0f, 1.0f);
            ImGui::SliderFloat("Decay", &decay, 0.0f, 1.0f);
            ImGui::SliderFloat("Sustain", &sustain, 0.0f, 1.0f);
            ImGui::SliderFloat("Release", &release, 0.0f, 1.0f);
            ImGui::SliderFloat("Attack Volume", &attackVol, 0.0f, 1.0f);
            ImGui::SliderFloat("Decay Volume", &decayVol, 0.0f, 1.0f);
            ImGui::SliderFloat("Release Volume", &releaseVol, 0.0f, 1.0f);
            ImGui::End();
        }
    }

    ByteBuffer serializeParams() override {
        ByteBuffer buffer;
        Writer w(buffer);
        w.writeFloat32(attack);
        w.writeFloat32(decay);
        w.writeFloat32(sustain);
        w.writeFloat32(release);
        w.writeFloat32(attackVol);
        w.writeFloat32(decayVol);
        w.writeFloat32(releaseVol);
        auto waveTypeInt = static_cast<uint8_t>(waveType);
        w.write8(waveTypeInt);

        return buffer;
    }

    DeserializeResult deserializeParams(const ByteBuffer& data) override {
        if (data.size() != 29) return Failure;
        Reader r(data);
        attack = r.readFloat32();
        decay = r.readFloat32();
        sustain = r.readFloat32();
        release = r.readFloat32();
        attackVol = r.readFloat32();
        decayVol = r.readFloat32();
        releaseVol = r.readFloat32();
        auto waveTypeInt = r.read8();
        waveType = static_cast<WaveType>(waveTypeInt);
        return Success;
    }
};


Synth* createSynth(uint64_t id) {
    switch (id) {
        case SineSynth::id:
            return new SineSynth();
        case TriangleSynth::id:
            return new TriangleSynth();
        case SquareSynth::id:
            return new SquareSynth();
        case SawSynth::id:
            return new SawSynth();
        case EnvelopeSynth::id:
            return new EnvelopeSynth();
        default:
            std::cerr << "Error: Unknown synth id " << id << std::endl;
            return nullptr;
    }
}

// get all synth ids and names for them, this allows us to add more to the add menu by only changing this file
std::vector<std::pair<uint64_t, std::string>> getSynthNames() {
    return {
        {SineSynth::id, "Sine"},
        {TriangleSynth::id, "Triangle"},
        {SquareSynth::id, "Square"},
        {SawSynth::id, "Saw"},
        {EnvelopeSynth::id, "Envelope"}
    };
}