#ifndef AI_HPP
#define AI_HPP

#include <windows.h>
#include <iostream>
#include <string>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

class Model {
public:
    // AI Settings
    const bool flash_attn = true;
    const double heat = 1;
    const size_t ctx_size = 4096;

    // Layers
    const int gpu_layers = 0;
    const int cpu_layers = 0;

    // Output settings
    bool show_extras = false;

    Model (const std::string& modelName, const size_t ctx_size2, const size_t gpu_layers2, const size_t cpu_layers2, const double heat2, const bool flash_attn2) : name(modelName), ctx_size(ctx_size2), gpu_layers(gpu_layers2), cpu_layers(cpu_layers2), heat(heat2), flash_attn(flash_attn2) {
        SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };

        // Create pipes for child process
        if (!CreatePipe(&hChildStdoutRead, &hChildStdoutWrite, &sa, 0) || !CreatePipe(&hChildStdinRead, &hChildStdinWrite, &sa, 0)) {
            std::cerr << "CreatePipe failed: " << GetLastError() << std::endl;
            throw std::runtime_error("Pipe creation failed");
        }

        // Set up process startup info (UNICODE version)
        STARTUPINFOW si = { sizeof(si) };
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdInput = hChildStdinRead;
        si.hStdOutput = hChildStdoutWrite;
        si.hStdError = hChildStdoutWrite;

        PROCESS_INFORMATION pi;
        fs::path llamaCLI = fs::current_path() / "llamacpp" / "llama-cli.exe";
        
        // Create command line with proper escaping
        std::wstring cmdLine = L"\"" + llamaCLI.wstring() + L"\" -m \"" + std::wstring(modelName.begin(), modelName.end()) + L"\"";
        
        // Convert to mutable buffer for CreateProcessW
        std::vector<wchar_t> cmdLineBuffer(cmdLine.begin(), cmdLine.end());
        cmdLineBuffer.push_back(L'\0');

        // Create child process
        if (!CreateProcessW(NULL, cmdLineBuffer.data(), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
            DWORD err = GetLastError();
            CloseHandle(hChildStdinRead);
            CloseHandle(hChildStdinWrite);
            CloseHandle(hChildStdoutRead);
            CloseHandle(hChildStdoutWrite);
            std::cerr << "CreateProcess failed: " << err << std::endl;
            throw std::runtime_error("Process creation failed");
        }

        hChildProcess = pi.hProcess;
        CloseHandle(pi.hThread);
        CloseHandle(hChildStdinRead);
        CloseHandle(hChildStdoutWrite);
    }

    ~Model () {
        if (hChildStdinWrite) {
            CloseHandle(hChildStdinWrite);
        }
        if (hChildStdoutRead) {
            CloseHandle(hChildStdoutRead);
        }
        if (hChildProcess) {
            TerminateProcess(hChildProcess, 0);
            CloseHandle(hChildProcess);
        }
    }

    std::string prompt (const std::string& question, bool streamOutput) {
        // Send question to child process
        std::string formatted = question + "\n";
        DWORD bytesWritten;
        WriteFile(hChildStdinWrite, formatted.c_str(), formatted.size(), &bytesWritten, NULL);
        
        // Read response
        std::string response;
        char buffer[4096];
        DWORD bytesRead;
        
        while (true) {
            if (!ReadFile(hChildStdoutRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) || bytesRead == 0) {
                DWORD err = GetLastError();
                if (err == ERROR_BROKEN_PIPE) {
                    break;  // Child process closed pipe
                }
                continue;
            }
            
            // Null-terminate the buffer
            buffer[bytesRead] = '\0';
            std::string chunk(buffer);
            response += chunk;
            
            // Stream to console if requested
            if (streamOutput) {
                std::cout << chunk;
                std::cout.flush();
            }
            
            // Check for termination condition
            if (chunk.find("\n>") != std::string::npos || chunk.find("end of text") != std::string::npos) {
                break;
            }
        }
        
        return response;
    }

private:
    const std::string name;
    HANDLE hChildStdinRead = NULL;
    HANDLE hChildStdinWrite = NULL;
    HANDLE hChildStdoutRead = NULL;
    HANDLE hChildStdoutWrite = NULL;
    HANDLE hChildProcess = NULL;
};

#endif