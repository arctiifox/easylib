#include <iostream>
#include <windows.h>
#include <mciapi.h>
#include <string>
#include <thread>
#include <chrono>
#include <filesystem>

#pragma comment(lib, "Winmm.lib") // Link with Winmm.lib for mciSendString

/*
    GUIDE:

    To open *.mp3:
    mciSendString("open \"*.mp3\" type mpegvideo alias mp3", NULL, 0, NULL);

    To play *.mp3:
    mciSendString("play mp3", NULL, 0, NULL);

    To play and wait until the *.mp3 has finished playing:
    mciSendString("play mp3 wait", NULL, 0, NULL);

    To replay (play again from start) the *.mp3:
    mciSendString("play mp3 from 0", NULL, 0, NULL);

    To replay and wait until the *.mp3 has finished playing:
    mciSendString("play mp3 from 0 wait", NULL, 0, NULL);

    To play the *.mp3 and replay it every time it ends like a loop:
    mciSendString("play mp3 repeat", NULL, 0, NULL);

    To pause the *.mp3 in middle:
    mciSendString("pause mp3", NULL, 0, NULL);

    and to resume it:
    mciSendString("resume mp3", NULL, 0, NULL);

    To stop it in middle:
    mciSendString("stop mp3", NULL, 0, NULL);
*/

class audioPlayer {
public:
    bool playing = false;
    bool paused = false;
    bool skipSignal = false;
    int remainingTime = 0;

    void playMusic(const std::string& path) {
        if (playing) {
            endMusic();
        }

        // Open the mp3 file using ANSI version of mciSendString
        std::string command = "open \"" + path + "\" type mpegvideo alias mp3";

        mciSendStringA(command.c_str(), NULL, 0, NULL);
        mciSendStringA("play mp3", NULL, 0, NULL);

        playing = true;

        int length = audioTime(path);
        if (length == 0) return;

        std::thread t([this, length]() {
            this->remainingTime = length;
            int seconds = 0;

            while (seconds < length) {
                std::this_thread::sleep_for(std::chrono::seconds(1));

                if (!paused) ++seconds; --remainingTime;
                if (!playing || skipSignal) return;
            }

            playing = false;
        });
        t.detach();
    }

    void pauseMusic() {
        if (!paused) {
            mciSendStringA("pause mp3", NULL, 0, NULL);
            paused = true;
        }
    }

    void unpauseMusic() {
        if (paused) {
            mciSendStringA("resume mp3", NULL, 0, NULL);
            paused = false;
        }
    }

    void endMusic() {
        if (playing) {
            mciSendStringA("stop mp3", NULL, 0, NULL);
            mciSendStringA("close mp3", NULL, 0, NULL);

            playing = false;
        }
    }

private:
    int audioTime(const std::string& audioName) {
        std::string exePath = std::filesystem::current_path().string();
        std::string command = "\"" + exePath + "\\ffmpeg\\ffprobe.exe\" -v error -show_entries format=duration -of default=noprint_wrappers=1:nokey=1 \"" + audioName + "\"";
        std::cout << std::endl << command;
        FILE* pipe = _popen(command.c_str(), "r");
        if (!pipe) {
            std::cerr << "Failed to run command.\n";
            return 0;
        }

        char buffer[128];
        std::string duration = "";
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            duration = buffer;
        }
        _pclose(pipe);

        duration.erase(duration.find_last_not_of(" \n\r\t") + 1); // Trim newline and other trailing whitespace
        return static_cast<int>(std::stoi(duration));
    }
};