#include "configtracker/file_watcher.h"
#include <iostream>
#include <algorithm>  // 为std::find添加头文件

using namespace configtracker;

void FileWatcher::addWatch(const std::string& path) {
    std::cout << "[Watcher] Watching path: " << path << "\n";
    auto it = std::find(watchPaths_.begin(), watchPaths_.end(), path);
    
    // 如果路径不在监视列表中，添加它
    if (it == watchPaths_.end()) {
        watchPaths_.push_back(path);
    }
}


// src/file_watcher.cpp
void FileWatcher::startWatching(std::function<void(std::string)> onChange) {
    std::cout << "[Watcher] Start watching...\n";
    running_ = true;
    
    // 创建监控线程
    watchThread_ = std::thread([this, onChange]() {
        while (running_) {
            for (const auto& path : watchPaths_) {
                checkForChanges(path, onChange);
            }
            std::this_thread::sleep_for(std::chrono::seconds(2)); // 每2秒检查一次
        }
    });
}

void FileWatcher::checkForChanges(const std::string& path, const std::function<void(std::string)>& onChange) {
    // 实现文件检查逻辑，例如比较文件修改时间或哈希值
    // 这里简化实现，实际应当保存文件的上一个状态进行比较
    
    std::filesystem::path watchPath(path);
    
    if (std::filesystem::exists(watchPath)) {
        if (std::filesystem::is_directory(watchPath)) {
            for (const auto& entry : std::filesystem::directory_iterator(watchPath)) {
                std::string filePath = entry.path().string();
                auto now = std::filesystem::last_write_time(entry.path());
                
                auto it = fileStates_.find(filePath);
                if (it == fileStates_.end()) {
                    // 新文件
                    fileStates_[filePath] = now;
                    onChange(filePath);
                } else if (it->second != now) {
                    // 文件已修改
                    it->second = now;
                    onChange(filePath);
                }
            }
        } else {
            // 单个文件的情况
            auto now = std::filesystem::last_write_time(watchPath);
            auto it = fileStates_.find(path);
            if (it == fileStates_.end()) {
                fileStates_[path] = now;
                onChange(path);
            } else if (it->second != now) {
                it->second = now;
                onChange(path);
            }
        }
    }
}


void FileWatcher::stop() {
    std::cout << "[Watcher] Stop watching.\n";
    running_ = false;
    
    // 等待监视线程结束
    if (watchThread_.joinable()) {
        watchThread_.join();
    }
}
