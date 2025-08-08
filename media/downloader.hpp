#include <iostream>
#include <string>
#include <filesystem>
#include <windows.h>

class downloader {
public:
    // Function to parse filename and extract metadata
    void extractMetadataFromFile(const std::string& filename, std::string& title, std::string& uploader, std::string& uploadDate) {
        // Expected filename format: "[title] [upload_date] [uploader].ext"
        size_t titleStart = filename.find('[') + 1;
        size_t titleEnd = filename.find(']');

        size_t dateStart = filename.find('[', titleEnd) + 1;
        size_t dateEnd = filename.find(']', dateStart);

        size_t uploaderStart = filename.find('[', dateEnd) + 1;
        size_t uploaderEnd = filename.find(']', uploaderStart);

        title = filename.substr(titleStart, titleEnd - titleStart);
        uploadDate = filename.substr(dateStart, dateEnd - dateStart);
        uploader = filename.substr(uploaderStart, uploaderEnd - uploaderStart);
    }

    // Function to convert upload date string (YYYYMMDD) to SYSTEMTIME
    bool convertDateStringToSystemTime(const std::string& dateStr, SYSTEMTIME& sysTime) {
        if (dateStr.length() != 8) {
            std::cerr << "Invalid date format. Expected YYYYMMDD." << std::endl;
            return false;
        }
        
        // Extract year, month, day from the date string
        sysTime.wYear = std::stoi(dateStr.substr(0, 4));
        sysTime.wMonth = std::stoi(dateStr.substr(4, 2));
        sysTime.wDay = std::stoi(dateStr.substr(6, 2));

        // Set default values for time (can be modified as needed)
        sysTime.wHour = 0;
        sysTime.wMinute = 0;
        sysTime.wSecond = 0;
        sysTime.wMilliseconds = 0;
        
        return true;
    }

    // Function to set file's last modified date to match upload date
    void setFileModificationDate(const std::filesystem::path& filePath, const SYSTEMTIME& uploadTime) {
        // Convert path to std::string
        std::string pathString = filePath.string();

        HANDLE hFile = CreateFileA(pathString.c_str(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) {
            std::cerr << "Failed to open file: " << pathString << std::endl;
            return;
        }

        // Convert SYSTEMTIME to FILETIME
        FILETIME fileTime;
        SystemTimeToFileTime(&uploadTime, &fileTime);

        // Set the file's last modified time
        if (!SetFileTime(hFile, NULL, NULL, &fileTime)) {
            std::cerr << "Failed to set file time." << std::endl;
        } else {
            std::cout << "File time updated successfully." << std::endl;
        }

        CloseHandle(hFile);
    }

    // Function to apply metadata and move from cache to music directory
    void makeMetadataAndMove(const std::string& filePath, const std::filesystem::path& musicDir) {
        // Extract metadata from the filename
        std::string filename = std::filesystem::path(filePath).filename().string();
        std::string title, uploader, uploadDate;
        extractMetadataFromFile(filename, title, uploader, uploadDate);

        // Convert uploadDate to SYSTEMTIME
        SYSTEMTIME uploadTime;
        if (!convertDateStringToSystemTime(uploadDate, uploadTime)) {
            return;
        }

        // Set the file's last modified date to the upload date
        setFileModificationDate(filePath, uploadTime);

        // Move the file from cache to the music directory
        std::filesystem::path newFilePath = musicDir / filename;
        std::filesystem::rename(filePath, newFilePath);

        // Output metadata for verification
        std::cout << "Moved file to: " << newFilePath << std::endl;
        std::cout << "Title: " << title << std::endl;
        std::cout << "Uploader: " << uploader << std::endl;
        std::cout << "Upload Date: " << uploadDate << std::endl;
    }

    void downloadMusic(const bool isMP3, const std::string& link) {
        std::string first_command = "";
        std::string final_command = "";

        // Get the cache and music directory paths
        std::filesystem::path cacheDir = std::filesystem::current_path() / "cache";
        std::filesystem::path musicDir = std::filesystem::current_path() / "music";
        
        // Ensure the cache and music directories exist
        std::filesystem::create_directory(cacheDir);
        std::filesystem::create_directory(musicDir);

        // Set the download path to the cache directory
        first_command = cacheDir.u8string();

        std::cout << "Downloading music from: " << link << (isMP3 ? " as MP3" : " as MP4") << std::endl;

        // Determine the yt-dlp command for downloading to the cache directory
        if (isMP3) { 
            final_command = "yt-dlp --retries 100 -f bestaudio -x --audio-format mp3 -o \"" + first_command + "/[%(title)s] [%(upload_date)s] [%(uploader)s].%(ext)s\" " + link;
        } else {
            final_command = "yt-dlp --retries 100 -f bestaudio+bestvideo --merge-output-format mp4 -o \"" + first_command + "/[%(title)s] [%(upload_date)s] [%(uploader)s].%(ext)s\" " + link;
        }

        // Run the yt-dlp command
        system(final_command.c_str());

        // Find the newly downloaded file in the cache directory
        std::filesystem::directory_iterator dirIter(cacheDir);
        for (const auto& file : dirIter) {
            if (file.is_regular_file()) {
                std::string fileName = file.path().filename().string();
                if (fileName.find(".mp3") != std::string::npos || fileName.find(".mp4") != std::string::npos) {
                    // Apply metadata and move the file to the music directory
                    makeMetadataAndMove(file.path().string(), musicDir);
                }
            }
        }
    }
};