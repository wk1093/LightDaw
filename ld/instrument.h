#pragma once

#include "filetools.h"
#include "synth.h"
#include "audio.h"

class Instrument {
public:
    virtual AudioBuffer generateSamples(size_t sampleCount, double freq, double vol) = 0;
    virtual AudioBuffer generateSeconds(double seconds, double freq, double vol) { // freq in Hz, vol in [0, 1]
        return generateSamples(static_cast<size_t>(seconds * SAMPLE_RATE), freq, vol);
    }

    // update will be called every frame, time is in seconds
    // it will also let the synth know the midi bpm
    virtual void update(double time, double bpm) {}
    // these functions are to let the instrument know when a note is played, this allows for more complex instruments
    // these functions are not required to be implemented
    // the generatesamples functions will also be called when a note is played.
    // noteon will be called right before generatesamples is called
    virtual void noteOn(double freq, double vol) {}
    virtual void noteOff(double freq) {}

    virtual ~Instrument() = default;

    virtual void openGui() {}
    virtual void closeGui() {}
    virtual void updateGui() {}
    virtual void toggleGui() {}

    virtual ByteBuffer serializeParams() {
        return {};
    }

    virtual DeserializeResult deserializeParams(const ByteBuffer& data) {
        return Success;
    }
};

class SynthInstrument : public Instrument {
public:
    Synth* synth = nullptr;

    explicit SynthInstrument(Synth* synth) : synth(synth) {}
    SynthInstrument() = default;

    ~SynthInstrument() override {
        if (synth != nullptr) {
            delete synth;
        }
    }

    AudioBuffer generateSamples(size_t sampleCount, double freq, double vol) override {
        if (synth == nullptr) {
            std::cerr << "Error: SynthInstrument::generateSamples called with no synth" << std::endl;
            return AudioBuffer(sampleCount);
        }
        synth->frequency = (float)freq;
        synth->amplitude = (float)vol / 2.0f;
        return synth->generateSamples(sampleCount);
    }

    void update(double time, double bpm) override {
        if (synth != nullptr) {
            synth->update(time, bpm);
        }
    }

    void openGui() override {
        if (synth != nullptr) {
            synth->open = true;
        }
    }

    void closeGui() override {
        if (synth != nullptr) {
            synth->open = false;
        }
    }

    void updateGui() override {
        if (synth != nullptr) {
            if (synth->open)
                synth->drawGui();
        }
    }

    void toggleGui() override {
        if (synth != nullptr) {
            synth->open = !synth->open;
        }
    }

    ByteBuffer serializeParams() override {
        if (synth != nullptr) {
            return synth->serializeParams();
        }
        return {};
    }

    DeserializeResult deserializeParams(const ByteBuffer& data) override {
        if (synth != nullptr) {
            return synth->deserializeParams(data);
        }
        return Failure;
    }
};