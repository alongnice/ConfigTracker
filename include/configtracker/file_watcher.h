#pragma once

#include <string>
#include <vector>
#include <functional>
#include <thread>
#include <atomic>
#include <map>
#include <filesystem>

namespace configtracker {

class FileWatcher {
public:
    FileWatcher() : running_(false) {}
    ~FileWatcher() { stop(); }
    
    void addWatch(const std::string& path);
    void startWatching(std::function<void(std::string)> onChange);
    void stop();
    
private:
    std::vector<std::string> watchPaths_;
    std::atomic<bool> running_;
    std::thread watchThread_;
    std::map<std::string, std::filesystem::file_time_type> fileStates_;
    
    void checkForChanges(const std::string& path, const std::function<void(std::string)>& onChange);
};

}
