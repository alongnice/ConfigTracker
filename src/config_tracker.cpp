#include "configtracker/config_tracker.h"
#include "configtracker/git_repo_manager.h"  
#include "configtracker/file_watcher.h"      

#include <iostream>


using namespace configtracker;

ConfigTracker::ConfigTracker(const TrackConfig& config)
    : config_(config), running_(false) {
    std::cout << "[Init] ConfigTracker initialized.\n";
}

// src/config_tracker.cpp
void ConfigTracker::start() {
    std::cout << "[Start] Monitoring started.\n";
    running_ = true;
    // 初始化Git仓库管理器
    git_ = std::make_unique<GitRepoManager>(config_.repoRoot);
    git_->init();
    
    // 初始化文件监控器
    watcher_ = std::make_unique<FileWatcher>();
    // 添加所有监控路径
    for (const auto& path : config_.watchPaths) {
        watcher_->addWatch(path);
    }
    
    // 启动监控并设置回调函数
    watcher_->startWatching([this](std::string changedPath) {
        if (config_.enableAutoCommit) {
            git_->addFile(changedPath);
            git_->commit("Auto commit: " + changedPath + " changed");
        }
    });
    
    // 延迟清理旧提交，仅当已有提交时执行
    if (config_.retentionDays > 0) {
        // 检查是否有任何提交
        auto commits = git_->listCommits();
        if (!commits.empty()) {
            cleanOld();
        } else {
            std::cout << "[Clean] Skipping cleanup as repository has no commits yet.\n";
        }
    }
}



// 此处删除重复的 stop 方法定义，保留后面完整的版本
// src/config_tracker.cpp
void ConfigTracker::cleanOld() {
    std::cout << "[Clean] Old commits cleanup triggered.\n";
    if (git_) {
        git_->squashCommitsOlderThan(config_.retentionDays);
    }
}

void ConfigTracker::manualCommit() {
    std::cout << "[Manual] Commit triggered.\n";
    if (git_) {
        git_->commit("Manual commit");
    }
}

void ConfigTracker::stop() {
    std::cout << "[Stop] Monitoring stopped.\n";
    running_ = false;
    if (watcher_) {
        watcher_->stop();
    }
}

void ConfigTracker::restoreTo(const std::string& hash) {
    std::cout << "[Restore] Checkout to commit: " << hash << "\n";
    if (git_) {
        git_->checkoutCommit(hash);
    }
}
