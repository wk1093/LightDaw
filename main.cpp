#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include <imgui_internal.h>
#include <imgui-knobs.h>
#include <imgui_stdlib.h>
#include <vector>
#include <iostream>
#include "ld/audio.h"
#include "ld/wavload.h"
#include "ld/ldp.h"
#include "ld/synth.h"
#include "ld/string.h"
#include "ld/instrument.h"
#include "tinyfiledialogs.h"
#include <MidiFile.h>
#include <map>
#include <unordered_map>

void SetupImGuiStyle1() {
    ImGuiStyle *style = &ImGui::GetStyle();

    style->WindowPadding = ImVec2(15, 15);
    style->WindowRounding = 5.0f;
    style->FramePadding = ImVec2(5, 5);
    style->FrameRounding = 4.0f;
    style->ItemSpacing = ImVec2(12, 8);
    style->ItemInnerSpacing = ImVec2(8, 6);
    style->IndentSpacing = 25.0f;
    style->ScrollbarSize = 15.0f;
    style->ScrollbarRounding = 9.0f;
    style->GrabMinSize = 5.0f;
    style->GrabRounding = 3.0f;

    style->Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
    style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style->Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_ChildBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
    style->Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
    style->Colors[ImGuiCol_Border] = ImVec4(0.80f, 0.80f, 0.83f, 0.88f);
    style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
    style->Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
    style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
    style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
    style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
    style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
    style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
    style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
}

void AudioWaveform(ImVec2 size, AudioPlayer *player, double smoothDeltaTime) {
    ImVec2 position = ImGui::GetCursorScreenPos();

    ImDrawList *drawList = ImGui::GetWindowDrawList();
    // get theme colors
    ImGuiStyle *style = &ImGui::GetStyle();
    ImColor backgroundColor = style->Colors[ImGuiCol_FrameBg];
    if (ImGui::IsMouseHoveringRect(position, ImVec2(position.x + size.x, position.y + size.y))) {
        backgroundColor = style->Colors[ImGuiCol_FrameBgHovered];
    }
    ImColor waveColor = style->Colors[ImGuiCol_Text];


    drawList->AddRectFilled(position, ImVec2(position.x + size.x, position.y + size.y), backgroundColor);

    // as a test, we will just get the audio data and display as text
    AudioBuffer audiodata;
    if (player != nullptr) {
        // we will get all the samples from the last dt
        // we will display a waveform of the samples
        size_t numSamples = SAMPLE_RATE * smoothDeltaTime;
//            size_t numSamples = SAMPLE_RATE / 60;

        const AudioBuffer& samples = player->getBuffer();
        static const size_t limit = 100; // if we go over this, do every other sample (or every 3, or whatever is needed to stay under)
        // this is basically the quality of the waveform, lower is low quality, but better performance, higher makes it super smooth but slow
        int step = 1;
        if (numSamples > limit) {
            step = numSamples / limit;
        }
        for (int i = 0; i < numSamples; i += step) {
            float dat = 0.0f;
            if (i + player->getPosition() < samples.size()) {
                dat = samples[i+player->getPosition()];
            }
            audiodata.push_back(dat);
        }
    } else {
        audiodata = AudioBuffer(100);
    }

    // normalize the audio data by finding the farthest from 0 (max(abs(max), abs(min))) and dividing by that
    float max = 0.0f;
    for (float f: audiodata) {
        if (std::abs(f) > max) {
            max = std::abs(f);
        }
    }
    if (max == 0.0f) {
        max = 1.0f;
    }
    if (max < 0.5f) {
        max = 0.5f;
    }
    for (float &f: audiodata) {
        f /= max;
    }

    for (int i = 0; i < audiodata.size() - 1; i++) {
        float x1 = position.x + i * size.x / audiodata.size();
        float y1 = position.y + size.y / 2 - audiodata[i] * size.y / 2;
        float x2 = position.x + (i + 1) * size.x / audiodata.size();
        float y2 = position.y + size.y / 2 - audiodata[i+1] * size.y / 2;
        drawList->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), waveColor);
    }
}

struct MidiToBuffer {
    Instrument *synth;
    smf::MidiFile file;

    explicit MidiToBuffer(smf::MidiFile file, Instrument *synth1) : file(std::move(file)) {
        synth = synth1;
    }

    AudioBuffer toSound() {
        AudioStream stream{};

        file.doTimeAnalysis();
        file.linkNotePairs();
        double x = file.getFileDurationInSeconds();
        if (x < 0) {
            std::cerr << "Error: File duration is negative" << std::endl;
            return stream.buffer;
        }
        for (int i = 0; i < file.getTrackCount(); i++) {
            smf::MidiEventList &track = file[i];
            for (int j = 0; j < track.size(); j++) {
                smf::MidiEvent &event = track[j];
                if (event.isNoteOn()) {
                    double start = file.getTimeInSeconds(event.tick);

                    double duration = event.getDurationInSeconds();

                    double freq = 440 * std::pow(2, (event.getKeyNumber() - 69) / 12.0);
                    int velocity = event.getVelocity();
                    double vol = velocity / 127.0 * 0.5;
                    AudioBuffer buf = synth->generateSeconds(duration, freq, vol);
                    stream.write(buf, AudioOffset::fromSeconds(start));
                }
            }
        }
        return stream.buffer;
    }

    AudioBuffer toSound(double offset, double length) {
        // offset and length are in seconds
        AudioStream stream{};
        file.doTimeAnalysis();
        file.linkNotePairs();
        double x = file.getFileDurationInSeconds();
        if (x < 0) {
            std::cerr << "Error: File duration is negative" << std::endl;
            return stream.buffer;
        }
        for (int i = 0; i < file.getTrackCount(); i++) {
            smf::MidiEventList &track = file[i];
            for (int j = 0; j < track.size(); j++) {
                smf::MidiEvent &event = track[j];
                if (event.isNoteOn()) {
                    double start = file.getTimeInSeconds(event.tick);
                    if (start < offset) {
                        continue;
                    }
                    if (start > offset + length) {
                        break;
                    }
                    double duration = event.getDurationInSeconds();
                    double freq = 440 * std::pow(2, (event.getKeyNumber() - 69) / 12.0);
                    int velocity = event.getVelocity();
                    double vol = velocity / 127.0 * 0.5;
                    synth->noteOn(freq, vol);
                    AudioBuffer buf = synth->generateSeconds(duration, freq, vol);
                    synth->noteOff(freq);
                    stream.write(buf, AudioOffset::fromSeconds(start - offset));
                }
            }
        }
        return stream.buffer;
    }
};

struct LightDawState {
    std::unordered_map<uint64_t, LdifFile> instruments{};
    std::unordered_map<uint64_t, Instrument*> realInstruments{}; // should be one for each instrument
    std::vector<LdpfFile> patterns{};
    std::unordered_map<uint64_t, smf::MidiFile> midis{};
    size_t selectedPattern = 0;
    size_t selectedInstrument = 0;

    LdipFile project{};

    AudioPlayer *player{};
    AudioStream stream{};

    std::string filename;

    std::vector<std::string> error_queue; // when an error happens, it will be added to this queue
    // in the main loop, it will be displayed with a modal popup.
    // if multiple, they will be displayed in a list, but most of the time it will just be one error.

    enum AudioState {
        PLAYING,
        PAUSED,
        STOPPED
    } audioState = AudioState::STOPPED;

    bool patternMode = false;
    float progress = 0.0f;

    LightDawState() = default;

    static LightDawState newProj() {
        LightDawState state;
        state.project = LdipFile("New Project", "Unknown", "No description", "1.0");
        state.patterns.push_back(LdpfFile("New Pattern", {}));
        state.addInstrument(LdifFile("Square", 2, SquareSynth::id));
        state.createRealInstruments();
        return state;
    }

    uint64_t addInstrument(const LdifFile &instrument) {
        FileID id = FileID(instrument.toBytes());
        instruments[id.id] = instrument;
        return id.id;
    }

    static LightDawState fromArchive(const ArchiveFile &archive, const std::string &filename) {
        LightDawState state;
        state.filename = filename;
        for (const auto &key: archive.getFileNames()) {
            if (ends_with(key, ".ldif")) {
                ByteBuffer buffer = archive.getFile(key).data;
                LdifFile instrument = LdifFile::fromBytes(buffer);
                state.instruments[FileID::fromFilename(key).id] = instrument;
            } else if (ends_with(key, ".ldpf")) {
                ByteBuffer buffer = archive.getFile(key).data;
                LdpfFile pattern = LdpfFile::fromBytes(buffer);
                state.patterns.push_back(pattern);
            } else if (ends_with(key, ".mid")) {
                ByteBuffer buffer = archive.getFile(key).data;
                // midiFile uses a stream, so we need to copy the buffer into a stream
                std::stringstream stream;
                for (uint8_t byte: buffer) {
                    stream << byte;
                }
                smf::MidiFile midi;
                midi.read(stream);

                state.midis[FileID::fromFilename(key).id] = midi;
            } else if (ends_with(key, ".ldip")) {
                ByteBuffer buffer = archive.getFile(key).data;
                state.project = LdipFile::fromBytes(buffer);
            } else {
                std::cerr << "Error: Unknown file type: " << key << std::endl;
                state.error_queue.push_back("Failed to load project:\nUnknown file type: " + key);
            }
        }
        state.createRealInstruments();
        return state;
    }

    void save() {
        std::vector<ArchiveContainedFile> files;

        for (auto &[id, instrument]: instruments) {
            instrument.instrumentData = realInstruments[id]->serializeParams();
            files.emplace_back(FileID(id).toFilename("instrument", ".ldif"), instrument.toBytes());
        }

        for (const auto &pattern: patterns) {
            ByteBuffer bytes = pattern.toBytes();
            files.emplace_back(FileID(bytes).toFilename("pattern", ".ldpf"), bytes);
        }

        for (auto &[id, midi]: midis) {
            std::stringstream ss;
            midi.write(ss);

            ByteBuffer buffer;
            for (char c: ss.str()) {
                buffer.push_back(c);
            }
            files.emplace_back(FileID(id).toFilename("midi", ".mid"), buffer);
        }

        files.emplace_back("main.ldip", project.toBytes());

        ArchiveFile af(files);
        ByteBuffer buffer = af.toBytes();
        writeFile(filename, buffer);
    }

    void destroy() {
        if (player != nullptr) {
            if (player->isPlaying()) {
                player->stop();
            }
            delete player;
        }
        player = nullptr;
    }

    void createRealInstruments() {
        for (auto &[id, instrument]: instruments) {
            if (realInstruments.find(id) == realInstruments.end()) {
                if (instrument.flags == LdifFile::FLAGS_SYNTH) { // 0x25c2230 sqr  0x25cc9a0 sin
                    realInstruments[id] = new SynthInstrument(createSynth(instrument.id.id));
                    if (realInstruments[id] == nullptr) {
                        std::cerr << "Error: Failed to create synth" << std::endl;
                        error_queue.emplace_back("Error: Failed to create synth");
                    } else if (dynamic_cast<SynthInstrument *>(realInstruments[id]) == nullptr) {
                        std::cerr << "Error: Failed to create synth" << std::endl;
                        error_queue.emplace_back("Error: Failed to create synth");
                    } else if (dynamic_cast<SynthInstrument *>(realInstruments[id])->synth == nullptr) {
                        std::cerr << "Error: Failed to create synth" << std::endl;
                        error_queue.emplace_back("Error: Failed to create synth");
                    }
                    DeserializeResult x = realInstruments[id]->deserializeParams(instrument.instrumentData);
                    if (x == DeserializeResult::Failure) {
                        // we assume it's our fault (the data we gave was wrong)
                        // so we will recreate the synth (to get the default values) and serialize it
                        // then we will save the new data
                        std::cerr << "Error: Failed to deserialize synth parameters" << std::endl;
                        delete realInstruments[id];
                        realInstruments[id] = new SynthInstrument(createSynth(instrument.id.id));
                        instrument.instrumentData = realInstruments[id]->serializeParams();

                    }
                } else {
                    std::cerr << "Error: Instrument is not a synth" << std::endl;
                    error_queue.emplace_back("Error: Instrument is not a synth:\nExternal instruments are not supported in this version!");
                    // TODO
                }
            }
        }
    }

    void play() {
        stream.clear();
        // if pattern mode, play selected pattern, else play all patterns (todo: playlist)
        if (patternMode) {
            if (selectedPattern >= patterns.size()) {
                std::cerr << "Error: Selected pattern out of bounds" << std::endl;
                error_queue.emplace_back("Error: Selected pattern out of bounds");
                return;
            }
            LdpfFile pattern = patterns[selectedPattern];
            for (const auto &[midifileID, instrumentfileID]: pattern.pairs) {
                if (midis.find(midifileID.id) == midis.end()) {
                    std::cerr << "Error: Midi file not found" << std::endl;
                    error_queue.emplace_back("Error: Midi not found!");
                    continue;
                }
                if (instruments.find(instrumentfileID.id) == instruments.end()) {
                    std::cerr << "Error: Instrument file not found" << std::endl;
                    error_queue.emplace_back("Error: Instrument not found!");
                    continue;
                }
                LdifFile instrument = instruments[instrumentfileID.id];
                if (instrument.flags == LdifFile::FLAGS_SYNTH) {
                    Synth *synth = createSynth(instrument.id.id);

                    Instrument *instr = realInstruments[instrumentfileID.id];
                    MidiToBuffer midiToBuffer(midis[midifileID.id], instr);
                    stream.write(midiToBuffer.toSound());
                } else {
                    std::cerr << "Error: Instrument is not a synth" << std::endl;
                    error_queue.emplace_back("Error: Instrument is not a synth:\nExternal instruments are not supported in this version!");
                    // TODO
                }
            }
        } else {
            for (const auto &pattern: patterns) {
                for (const auto &[midifileID, instrumentfileID]: pattern.pairs) {
                    if (midis.find(midifileID.id) == midis.end()) {
                        std::cerr << "Error: Midi file not found" << std::endl;
                        error_queue.emplace_back("Error: Midi not found!");
                        continue;
                    }
                    if (instruments.find(instrumentfileID.id) == instruments.end()) {
                        std::cerr << "Error: Instrument file not found" << std::endl;
                        error_queue.emplace_back("Error: Instrument not found!");
                        continue;
                    }
                    LdifFile instrument = instruments[instrumentfileID.id];
                    if (instrument.flags == LdifFile::FLAGS_SYNTH) {
                        Synth *synth = createSynth(instrument.id.id);

                        Instrument *instr = realInstruments[instrumentfileID.id];
                        MidiToBuffer midiToBuffer(midis[midifileID.id], instr);
                        stream.write(midiToBuffer.toSound());
                    } else {
                        std::cerr << "Error: Instrument is not a synth" << std::endl;
                        error_queue.emplace_back("Error: Instrument is not a synth:\nExternal instruments are not supported in this version!");
                        // TODO
                    }
                }
            }
        }
        if (player != nullptr) {
            if (player->isPlaying()) {
                player->stop();
            }
            delete player;
        }
        player = new AudioPlayer(stream.buffer);
        player->seek(progress);
        player->play();
        audioState = PLAYING; // TODO: add some prints to the main loop, check where the crash happens
    }
};

void sanitizeMidi(smf::MidiFile &file) {
    // remove all bogus events that are not note on, note off, or a timing/tempo event (things like messages and meta events)
    for (int i = 0; i < file.getTrackCount(); i++) {
        smf::MidiEventList &track = file[i];
        for (int j = track.size() - 1; j >= 0; j--) {
            smf::MidiEvent &event = track[j];
            if (event.isCopyright() || event.isMarkerText() || event.isLyricText()) {
                event.clear();
            }
        }
    }
    file.removeEmpties(); // remove all cleared events.
}

int main() {
    // https://github.com/ocornut/imgui/issues/5115

    // Setup window
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(1280, 720, "LightDaw Studio", NULL, NULL);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    initAudio();

//    std::vector<ArchiveContainedFile> files;
//
//    // split test.mid into new file for each track (except track 1 which will be copied for each other track)
//    // track 1 is the info track (tempo, other stuff)
//    smf::MidiFile baseTest("assets/test.mid");
//    std::vector<FileID> midis;
//    for (int i = 1; i < baseTest.getTrackCount(); i++) {
//        smf::MidiFile test = baseTest;
//        sanitizeMidi(test);
//        // remove all tracks except the first and the i-th
//        for (int j = baseTest.getTrackCount() - 1; j >= 0; j--) {
//            if (j != 0 && j != i) {
//                test.deleteTrack(j);
//            }
//        }
//        std::stringstream stream;
//        test.write(stream);
//        ByteBuffer buffer;
//        for (char c : stream.str()) {
//            buffer.push_back(c);
//        }
//        FileID id = FileID(buffer);
//        files.emplace_back(id.toFilename("midi", ".mid"), buffer);
//        midis.push_back(id);
//    }
//
//    LdipFile proj = LdipFile("Test Project", "Test Author", "Test Description", "1.0");
//    ByteBuffer buffer1 = proj.toBytes();
//    files.emplace_back("main.ldip", buffer1);
//
//    LdifFile instrument = LdifFile("Advanced Square", 2, AdvancedSquare::id);
//    FileID squareID = FileID(instrument.toBytes());
//    files.emplace_back(squareID.toFilename("instrument", ".ldif"), instrument.toBytes());
//
//    LdifFile instrument2 = LdifFile("Advanced Sine", 2, AdvancedSine::id);
//    FileID sineID = FileID(instrument2.toBytes());
//    files.emplace_back(sineID.toFilename("instrument", ".ldif"), instrument2.toBytes());
//
//    // make a new pattern for each midi file
//    // first uses square, the rest use sine
//    for (int i = 0; i < midis.size(); i++) {
//        FileID synthID = i == 0 ? squareID : sineID;
//        LdpfFile pattern = LdpfFile("Test Pattern " + std::to_string(i), {{midis[i], synthID}});
//        ByteBuffer patternbytes = pattern.toBytes();
//        auto patternid = FileID(patternbytes);
//        files.emplace_back(patternid.toFilename("pattern", ".ldpf"), patternbytes);
//    }
//
//
//    ArchiveFile af1(files);
//    ByteBuffer buffer2 = af1.toBytes();
//    writeFile("test.ldpa", buffer2);

    ArchiveFile af = ArchiveFile::fromFile("test.ldpa");

    LightDawState state = LightDawState::fromArchive(af, "test.ldpa");


    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Setup ImGui Style
    SetupImGuiStyle1();

    // Setup ImGui Fonts
    ImGuiIO &io = ImGui::GetIO();
    // use Inter font

    ImFont* mainFont = io.Fonts->AddFontFromFileTTF("./assets/fonts/inter.ttf", 16.0f);
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    std::vector<uint8_t> open(10, 1);
    std::vector<std::string> names(10);
    int n = 0;
    bool first = true;

    // load fontawesome
    ImWchar icons_ranges[] = { 0x0020, 0x00FF,
                               0xf000, 0xf3ff,
                               0};
    ImFont* faFont = io.Fonts->AddFontFromFileTTF("./assets/fonts/fontawesome-webfont.ttf", 16.0f, nullptr, icons_ranges);

    ImFont* largeFont = io.Fonts->AddFontFromFileTTF("./assets/fonts/inter.ttf", 20.0f);

    std::cout << "Starting main loop\n";
    bool running = true;

    // timing
    double lastTime = glfwGetTime();
    double deltaTime = 0.0;
    double currentTime = 0.0;
    double smoothDeltaTime = 0.0;
    std::vector<float> frameTimes(100, 0.0f);
    int frameTimeIndex = 0;

    bool enablestyleeditor = false;

    while (running) {
        n = 0;
        glfwPollEvents();
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        currentTime = glfwGetTime();
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        smoothDeltaTime = 0.0;
        for (int i = 0; i < frameTimes.size(); i++) {
            smoothDeltaTime += frameTimes[i];
        }
        smoothDeltaTime /= frameTimes.size();
        frameTimes[frameTimeIndex] = deltaTime;
        frameTimeIndex = (frameTimeIndex + 1) % frameTimes.size();


        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::DockSpaceOverViewport();
        // menu bar at the top
        ImVec2 menuBarSize;
        ImVec2 menuBarPos;
        if (ImGui::BeginMainMenuBar()) {
            menuBarSize = ImGui::GetWindowSize();
            menuBarPos = ImGui::GetWindowPos();
            bool opennewpopup = false;
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New")) {
                    opennewpopup = true;

                }
                if (ImGui::MenuItem("Open")) {
                    const char *filters[] = {"*.ldpa"};
                    const char *result = tinyfd_openFileDialog("Open Project", "", 1, filters, "LightDaw Project", 0);
                    if (result != nullptr) {
                        std::cout << "Opening " << result << std::endl;
                        ArchiveFile af = ArchiveFile::fromFile(result);
                        state.destroy();
                        state = LightDawState::fromArchive(af, result);
                    }
                }
                if (ImGui::MenuItem("Save")) {
                    if (!state.filename.empty()) {
                        state.save();
                    } else {
                        const char *filters[] = {"*.ldpa"};
                        const char *result = tinyfd_saveFileDialog("Save Project", "", 1, filters, "LightDaw Project");
                        if (result != nullptr) {
                            std::cout << "Saving to " << result << std::endl;
                            state.filename = result;
                            state.save();
                        }
                    }
                }
                if (ImGui::MenuItem("Save As")) {
                    const char *filters[] = {"*.ldpa"};
                    const char *result = tinyfd_saveFileDialog("Save Project", "", 1, filters, "LightDaw Project");
                    if (result != nullptr) {
                        std::cout << "Saving to " << result << std::endl;
                        state.filename = result;
                        state.save();
                    }
                }
                if (ImGui::MenuItem("Close")) {
                    glfwSetWindowShouldClose(window, true);
                }
                ImGui::EndMenu();
            }
            if (opennewpopup) {
                ImGui::OpenPopup("New Project");
            }
            if (ImGui::BeginPopupModal("New Project", nullptr,
                                       ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize |
                                       ImGuiWindowFlags_NoMove)) {
                ImGui::Text("Do you want to save the current project?");
                if (ImGui::Button("Yes")) {
                    if (!state.filename.empty()) {
                        state.save();
                    } else {
                        const char *filters[] = {"*.ldpa"};
                        const char *result = tinyfd_saveFileDialog("Save Project", "", 1, filters, "LightDaw Project");
                        if (result != nullptr) {
                            std::cout << "Saving to " << result << std::endl;
                            state.filename = result;
                            state.save();
                        }
                    }
                    state.destroy();
                    state = LightDawState::newProj();

                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::Button("No")) {
                    // don't save the project
                    state.destroy();
                    state = LightDawState::newProj();
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::Button("Cancel")) {
                    // don't do anything
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
            if (ImGui::BeginMenu("Edit")) {
                // theme selector
                if (ImGui::BeginMenu("Theming")) {
                    // 3 buttons: save, load, and customize
                    if (ImGui::MenuItem("Save Theme")) {
                        // save the current theme
                    }
                    if (ImGui::MenuItem("Load Theme")) {
                        // load a theme
                    }
                    if (ImGui::MenuItem("Customize Theme")) {
                        // doesn't ImGui have a built-in theme editor?
                        enablestyleeditor = true;
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View")) {
                for (int i = 0; i < 10; i++) {
                    if (names[i].empty()) {
                        break;
                    }
                    if (ImGui::MenuItem(names[i].c_str(), nullptr, (bool *) &open[i])) {}
                }
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        ImGui::SetNextWindowPos(ImVec2(menuBarPos.x, menuBarPos.y + menuBarSize.y));
        ImGui::SetNextWindowSize(ImVec2(menuBarSize.x, menuBarSize.y*3));
        ImGui::Begin("Transport", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoDecoration);

        if (state.audioState == LightDawState::PLAYING) {
            if (state.player == nullptr) {
                state.audioState = LightDawState::STOPPED;
            } else {
                if (!state.player->isPlaying()) {
                    state.audioState = LightDawState::STOPPED;
                }
            }
        }
        if (state.audioState == LightDawState::STOPPED) {
            ImGui::PushFont(faFont);
            bool playbutton = ImGui::Button("\uf04b");
            ImGui::SameLine(0, 0);
            bool stopbutton = ImGui::Button("\uf04d");
            ImGui::PopFont();
            if (playbutton) {
                state.play();
            }
            if (stopbutton) {
                if (state.player != nullptr) {
                    state.player->stop();
                    delete state.player;
                    state.player = nullptr;
                } else {
                    if (state.audioState == LightDawState::STOPPED)
                        state.progress = 0.0f;
                }

                state.audioState = LightDawState::STOPPED;
            }
        } else if (state.audioState == LightDawState::PLAYING) {
            ImGui::PushFont(faFont);
            bool pausebutton = ImGui::Button("\uf04c");
            ImGui::SameLine(0, 0);
            bool stopbutton = ImGui::Button("\uf04d");
            ImGui::PopFont();

            if (pausebutton) {
                state.player->pause();
                state.audioState = LightDawState::PAUSED;
            }
            if (stopbutton) {
                state.player->stop();
                state.audioState = LightDawState::STOPPED;
                delete state.player;
                state.player = nullptr;
            }
        } else if (state.audioState == LightDawState::PAUSED) {
            ImGui::PushFont(faFont);
            bool resumebutton = ImGui::Button("\uf04b");
            ImGui::SameLine(0, 0);
            bool stopbutton = ImGui::Button("\uf04d");
            ImGui::PopFont();
            if (resumebutton) {
                state.player->resume();
                state.audioState = LightDawState::PLAYING;
            }
            if (stopbutton) {
                state.player->stop();
                state.audioState = LightDawState::STOPPED;
                delete state.player;
                state.player = nullptr;
            }
        } else {
            std::cerr << "Error: Invalid audio state" << std::endl;
            state.error_queue.emplace_back("Error: Invalid audio state");
        }

        ImGui::SameLine(0, 0);
        // mode selector (pattern or playlist)
//        ImGui::Checkbox("Pattern Mode", &state.patternMode);
        // Selector will be a custom widget
        // it will have the text "Pattern" and "Song" and pattern will be on top and song on the bottom, when clicked it will switch
        // basically a vertical switch
        // we want it to be sorta rounded
        // first we make the bckground
        ImVec2 size = ImVec2(100, 40);
        ImVec2 pos = ImGui::GetCursorScreenPos();
        pos.y -= (size.y - ImGui::GetTextLineHeightWithSpacing()) / 2;
        bool res = ImGui::InvisibleButton("Mode Selector", size);
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        ImVec2 p0 = ImVec2(pos.x, pos.y);
        ImVec2 p1 = ImVec2(pos.x + size.x, pos.y + size.y);
        ImVec2 center = ImVec2((p0.x + p1.x) / 2, (p0.y + p1.y) / 2);
        ImColor backgroundColor;
        ImColor selectedColor;
        // get colors from theme
        ImGuiStyle *style = &ImGui::GetStyle();
        if (ImGui::IsItemHovered()) {
            backgroundColor = style->Colors[ImGuiCol_FrameBgHovered];
            selectedColor = style->Colors[ImGuiCol_ButtonActive];
        } else {
            backgroundColor = style->Colors[ImGuiCol_FrameBg];
            selectedColor = style->Colors[ImGuiCol_ButtonActive];
        }
        if (ImGui::IsItemActive()) {
            backgroundColor = style->Colors[ImGuiCol_FrameBgActive];
        }
//        draw_list->AddRectFilled(p0, p1, backgroundColor);
        ImGui::RenderFrame(p0, p1, backgroundColor, true, style->FrameRounding);
        // we will make the top half white and the bottom half black (if pattern mode is true)
        ImVec2 p2 = ImVec2(p0.x, center.y);
        ImVec2 p3 = ImVec2(p1.x, center.y);
        float radius = size.y / 4;
        ImVec2 topTextCenter = ImVec2(center.x, center.y - size.y / 4);
        ImVec2 bottomTextCenter = ImVec2(center.x, center.y + size.y / 4);
        if (state.patternMode) {
            draw_list->AddRectFilled(p0, p3, selectedColor, radius);
        } else {
            draw_list->AddRectFilled(p2, p1, selectedColor, radius);
        }
        ImVec2 textSize = ImGui::CalcTextSize("Pattern");
        ImVec2 textPos = ImVec2(topTextCenter.x - textSize.x / 2, topTextCenter.y - textSize.y / 2);
        draw_list->AddText(textPos, ImGui::GetColorU32(ImGuiCol_Text), "Pattern");
        textSize = ImGui::CalcTextSize("Song");
        textPos = ImVec2(bottomTextCenter.x - textSize.x / 2, bottomTextCenter.y - textSize.y / 2);
        draw_list->AddText(textPos, ImGui::GetColorU32(ImGuiCol_Text), "Song");
        if (res) {
            state.patternMode = !state.patternMode;
        }

        // pattern selection and editor menu
        // it shows the current selected pattern name, when you click it it shows a list of patterns, if you click one, it selects it
        // if you right click, you can edit it (change name, delete) or create a new pattern

        ImGui::SameLine(0, 0);

        static size_t patternToRename = 999999;
        static bool doRename = false;

        if (ImGui::BeginCombo("##Pattern", state.patterns[state.selectedPattern].name.c_str(), ImGuiComboFlags_WidthFitPreview)) {
            for (int i = 0; i < state.patterns.size(); i++) {
                bool selected = state.selectedPattern == i;
                if (ImGui::Selectable(state.patterns[i].name.c_str(), selected)) {
                    state.selectedPattern = i;
                }
                // right click
                if (ImGui::BeginPopupContextItem(NULL, ImGuiPopupFlags_MouseButtonRight)) {
                    ImGui::PushFont(largeFont);
                    ImGui::Text("Edit Pattern");
                    ImGui::PopFont();
                    ImGui::Separator();
                    if (ImGui::MenuItem("Rename")) {
                        // open a popup to rename the pattern
                        patternToRename = i;
                        doRename = true;
                    }
                    if (ImGui::MenuItem("Delete")) {
                        // delete the pattern
                        state.patterns.erase(state.patterns.begin() + i);
                        if (state.selectedPattern >= state.patterns.size()) {
                            state.selectedPattern = state.patterns.size() - 1;
                        }
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::MenuItem("New")) {
                        // create a new pattern
                        state.patterns.push_back(LdpfFile("Pattern " + std::to_string(state.patterns.size()), {}));
                    }
                    ImGui::EndPopup();
                }

                if (selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        if (state.player != nullptr) {
            state.progress = state.player->progress();
        }


        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 10);
        float width = ImGui::GetWindowWidth() / 2;
        ImGui::PushItemWidth(width);
        // make it less wide/tall
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        // make the slider handle itself bigger than the slider bar
        ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 5);
        ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 10);
        ImGui::SliderFloat("##Progress", &state.progress, 0.0f, 1.0f, "");
        ImGui::PopStyleVar(4);
        ImGui::PopItemWidth();



        // slider calculations
        if (state.audioState == LightDawState::PLAYING) {
            if (state.player != nullptr) {
                state.player->seek(state.progress);
            }
        }

        // a basic waveform display
        // we want it to take up the whole vertical space, and 100 pixels horizontally
        ImGui::SameLine(0, 0);
        ImGui::SetCursorScreenPos(ImVec2(ImGui::GetCursorScreenPos().x, menuBarPos.y + menuBarSize.y));
        AudioWaveform(ImVec2(180, menuBarSize.y * 3), state.player, smoothDeltaTime);



        ImGui::End();

        static std::string renaming;
        if (doRename) {
            if (patternToRename >= state.patterns.size()) {
                std::cerr << "Error: Pattern to rename out of bounds" << std::endl;
                state.error_queue.emplace_back("Error: Pattern to rename out of bounds");
            }
            ImGui::OpenPopup("Rename Pattern");
            renaming = state.patterns[patternToRename].name;
            doRename = false;
        }

        if (ImGui::BeginPopup("Rename Pattern")) {
            ImGui::InputText("Name", &renaming);
            if (ImGui::Button("Ok")) {
                state.patterns[patternToRename].name = renaming;
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();
            }


            ImGui::EndPopup();
        }

        if (enablestyleeditor) {
            ImGui::Begin("Theme Editor", &enablestyleeditor);
            ImGui::ShowStyleEditor();
            ImGui::End();
        }

        for (auto& instr : state.realInstruments) {
            if (instr.second != nullptr) {
                instr.second->update(deltaTime, 120); // TODO: get bpm from project
                instr.second->updateGui();
            }

        } //



#define WINDOW_START(name, ...) if (first) names[n] = name; if (open[n]) { if (ImGui::Begin(name, (bool*)&open[n], ImGuiWindowFlags_NoCollapse __VA_ARGS__)) {
#define WINDOW_END() } ImGui::End(); } n++;

        static bool renameinstrument = false;
        static size_t instrumentToRename = 999999;
        static std::string renamingInstrument = "";
        static uint64_t toDelete = 0;
        static bool deleteinstrument = false;
        static bool newinstrument = false;
        static uint64_t oldreplaceID = 0;
        static bool replacenew = false;
        WINDOW_START("Instruments")
                for (const auto &[id, instrument]: state.instruments) {
                    if (ImGuiKnobs::Knob(("##Volume"+instrument.name).c_str(), &state.realInstruments[id]->volume, 0.0f, 1.0f,
                                         0.01f, "Volume: %.2f", ImGuiKnobVariant_Wiper, 25, ImGuiKnobFlags_DragHorizontal | ImGuiKnobFlags_NoTitle | ImGuiKnobFlags_NoInput | ImGuiKnobFlags_ValueTooltip, 1000)) {
                        // volume changed
                    }
                    ImGui::SameLine();
                    if (ImGui::Selectable(instrument.name.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick)) {
                        // open the instrument GUI
                        state.realInstruments[id]->toggleGui();
                    }
                    if (ImGui::BeginPopupContextItem(NULL, ImGuiPopupFlags_MouseButtonRight)) {
                        ImGui::PushFont(largeFont);
                        ImGui::Text("New Instrument");
                        ImGui::PopFont();
                        ImGui::Separator();
                        if (ImGui::MenuItem("New")) {
                            newinstrument = true;
                            replacenew = false;
                            ImGui::CloseCurrentPopup();
                        }
                        if (ImGui::MenuItem("Replace")) {
                            oldreplaceID = id;
                            replacenew = true;
                            newinstrument = true;
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::Separator();
                        ImGui::PushFont(largeFont);
                        ImGui::Text("Edit Instrument");
                        ImGui::PopFont();
                        ImGui::Separator();
                        if (ImGui::MenuItem("Rename")) {
                            renameinstrument = true;
                            instrumentToRename = id;
                        }
                        if (ImGui::MenuItem("Delete")) {
                            toDelete = id;
                            deleteinstrument = true;
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::EndPopup();
                    }
                }
        WINDOW_END()

        if (newinstrument) {
            ImGui::OpenPopup("New Instrument");
            newinstrument = false;
        }
        bool newselected = false;
        LdifFile newinstrumentfile;
        if (ImGui::BeginPopup("New Instrument")) {
            ImGui::PushFont(largeFont);
            ImGui::Text("Built-in Synths");
            ImGui::PopFont();
            ImGui::Separator();
            for (const auto &[id, name] : getSynthNames()) {
                if (ImGui::Selectable(name.c_str())) {
                    newinstrumentfile = LdifFile(name, 2, id);
                    newselected = true;
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::Separator();
            ImGui::PushFont(largeFont);
            ImGui::Text("Other");
            ImGui::PopFont();

            ImGui::EndPopup();
        }

        if (newselected) {
            FileID newid = FileID(newinstrumentfile.toBytes());
            // to replace is a bit complicated, we need to replace all references to the old instrument with the new one
            if (replacenew) {
                for (auto &pattern: state.patterns) {
                    for (auto &pair: pattern.pairs) {
                        if (pair.synthFileID.id == oldreplaceID) {
                            pair.synthFileID = newid;
                        }
                    }
                }
                state.instruments.erase(oldreplaceID);
            }
            state.instruments[newid.id] = newinstrumentfile;
            state.createRealInstruments();
            // now we need to add the extra data
            state.instruments[newid.id].instrumentData = state.realInstruments[newid.id]->serializeParams();
        }

        if (deleteinstrument) {
                // check if instrument is used, if it is, ask the user if they want to delete it anyway (if so, all references to it will be removed)
            bool used = false;
            for (const auto &pattern: state.patterns) {
                for (const auto &[midifileID, instrumentfileID]: pattern.pairs) {
                    if (instrumentfileID.id == toDelete) {
                        used = true;
                        break;
                    }
                }
                if (used) {
                    break;
                }
            }
            if (used) {
                ImGui::OpenPopup("Delete Instrument");
            } else {
                state.instruments.erase(toDelete);
                state.createRealInstruments();
            }
            deleteinstrument = false;
        }

        if (ImGui::BeginPopupModal("Delete Instrument")) {
            ImGui::Text("The instrument is used in a pattern, do you want to delete it anyway?");
            if (ImGui::Button("Yes")) {
                state.instruments.erase(toDelete);
                state.createRealInstruments();
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::Button("No")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        if (renameinstrument) {
            ImGui::OpenPopup("Rename Instrument");
            renamingInstrument = state.instruments[instrumentToRename].name;
            renameinstrument = false;
        }
        if (ImGui::BeginPopupModal("Rename Instrument")) {
            ImGui::InputText("Name", &renamingInstrument);
            if (ImGui::Button("Ok")) {
                state.instruments[instrumentToRename].name = renamingInstrument;
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }


        WINDOW_START("Piano", | ImGuiWindowFlags_MenuBar)

                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Edit")) {
                        if (ImGui::MenuItem("TODO")) {
                            // TODO
                        }
                        ImGui::EndMenu();
                    }
                    std::string selectedInstrument = "Instruments";
                    if (state.selectedInstrument != 0) {
                        selectedInstrument = state.instruments[state.selectedInstrument].name;
                    }
                    if (ImGui::BeginMenu(selectedInstrument.c_str())) {
                        for (const auto &[id, instrument]: state.instruments) {
                            if (ImGui::MenuItem(instrument.name.c_str(), nullptr, state.selectedInstrument == id)) {
                                state.selectedInstrument = id;
                            }
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();

                    // piano roll
                    // we will have a piano roll on the left, and the notes in the middle
                    // the piano roll will be a grid of keys, with the keys being the notes
                    // the notes will be displayed as rectangles

                    ImVec4 whitenote = ImColor(255, 255, 255);
                    ImVec4 blacknote = ImColor(0, 0, 0);
                    ImVec4 background = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
                    ImVec4 line = ImGui::GetStyleColorVec4(ImGuiCol_Border);

                    static float scroll = 0.0f;
                    static float noteHeight = 20.0f;
                    ImGui::SetNextWindowScroll(ImVec2(0, scroll));
                    if (ImGui::BeginChild("LeftNotes", ImVec2(100, 0), ImGuiChildFlags_ResizeX, ImGuiWindowFlags_NoScrollbar)) {
                        for (int i = 0; i < 128; i++) {
                            if (i % 12 == 1 || i % 12 == 3 || i % 12 == 6 || i % 12 == 8 || i % 12 == 10) {
                                ImGui::PushStyleColor(ImGuiCol_Button, blacknote);
                            } else {
                                ImGui::PushStyleColor(ImGuiCol_Button, whitenote);
                            }
                            // we want NO spacing between the buttons
                            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
                            ImGui::Button(std::to_string(i).c_str(), ImVec2(100, noteHeight));
                            ImGui::PopStyleVar();
                            ImGui::PopStyleColor();


                        }
                        ImGui::EndChild();
                    }
                    ImGui::SameLine(0, 0);
                    if (ImGui::BeginChild("MiddleGrid")) {
                        // make a grid for length is just window length
                        // height is 128 * noteHeight
                        // we will have a grid of 128 * 16
                        // each cell will be a note
                        // we want each
                        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
                        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 0));
                        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
                        ImGui::BeginTable("Notes", 16, ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit);
                        for (int i = 0; i < 128; i++) {
                            ImGui::TableNextRow();
                            for (int j = 0; j < 16; j++) {
                                ImGui::TableNextColumn();
                                ImGui::Button(std::to_string(i).c_str(), ImVec2(50, noteHeight));
                            }
                        }
                        ImGui::EndTable();
                        ImGui::PopStyleVar(3);
                        scroll = ImGui::GetScrollY();
                        ImGui::EndChild();

                    }



                }
        WINDOW_END()

        ImGui::ShowDemoWindow();

        if (!state.error_queue.empty()) {
            ImGui::OpenPopup("Error");
            // if there is any errors, we should always stop playback
            if (state.player != nullptr) {
                state.player->stop();
                delete state.player;
                state.player = nullptr;
            }
        }
        if (ImGui::BeginPopupModal("Error", nullptr,
                                   ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize |
                                   ImGuiWindowFlags_NoMove)) {
            if (state.error_queue.size() == 1) {
                ImGui::Text("An error occurred:");
                ImGui::Text(state.error_queue[0].c_str());
            } else {
                ImGui::Text("Multiple errors occurred:");
                for (const std::string &error: state.error_queue) {
                    ImGui::Text(error.c_str());
                }
            }
            if (ImGui::Button("Ok")) {
                state.error_queue.clear();
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::Button("Exit Without Saving")) {
                state.error_queue.clear();
                running = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        if (glfwWindowShouldClose(window)) {
            // check if the user wants to save first
            ImGui::OpenPopup("Close LightDAW");
        }
        if (ImGui::BeginPopupModal("Close LightDAW", nullptr,
                                   ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize |
                                   ImGuiWindowFlags_NoMove)) {
            ImGui::Text("Do you want to save the current project?");
            if (ImGui::Button("Yes")) {
                if (!state.filename.empty()) {
                    state.save();
                } else {
                    const char *filters[] = {"*.ldpa"};
                    const char *result = tinyfd_saveFileDialog("Save Project", "", 1, filters, "LightDaw Project");
                    if (result != nullptr) {
                        std::cout << "Saving to " << result << std::endl;
                        state.filename = result;
                        state.save();
                    }
                }
                ImGui::CloseCurrentPopup();
                running = false;
            }
            if (ImGui::Button("No")) {
                // don't save the project
                ImGui::CloseCurrentPopup();
                running = false;
            }
            if (ImGui::Button("Cancel")) {
                // don't do anything
                glfwSetWindowShouldClose(window, false);
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow *backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(window);
        first = false;
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();


    glfwTerminate();

    terminateAudio();

    return 0;
}