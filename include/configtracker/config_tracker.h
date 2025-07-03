#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <memory>
#include <atomic>

#include "git_repo_manager.h"  
#include "file_watcher.h"      


namespace configtracker {

struct TrackConfig {
    std::vector<std::string> watchPaths;
    int retentionDays = 7;
    bool enableAutoCommit = true;
    std::string repoRoot = ".configtracker";
};

class GitRepoManager;
class FileWatcher;

class ConfigTracker {
public:
    ConfigTracker(const TrackConfig& config);
    void start();
    void stop();
    void manualCommit();
    void cleanOld();
    void restoreTo(const std::string& commitHash);

private:
    TrackConfig config_;
    std::unique_ptr<GitRepoManager> git_;
    std::unique_ptr<FileWatcher> watcher_;
    std::atomic<bool> running_;
};

}
