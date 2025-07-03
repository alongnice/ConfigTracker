// example/main.cpp
#include "configtracker/config_tracker.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <chrono>
#include <thread>
#include <random>
#include <sstream>
#include <iomanip>

using namespace configtracker;

// 获取当前时间的格式化字符串
std::string get_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

// 生成随机配置值
std::string random_value(int length = 8) {
    static const char chars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_int_distribution<> dist(0, sizeof(chars) - 2);
    
    std::string result;
    result.reserve(length);
    for (int i = 0; i < length; ++i) {
        result += chars[dist(rng)];
    }
    return result;
}

void create_test_file(const std::string& path, const std::string& key) {
    std::ofstream file(path);
    std::string timestamp = get_timestamp();
    std::string value = random_value();
    file << "# Created at: " << timestamp << std::endl;
    file << key << "=" << value << std::endl;
    file.close();
    std::cout << "[" << timestamp << "] Created file: " << path << " with " << key << "=" << value << std::endl;
}

void modify_test_file(const std::string& path, const std::string& key) {
    std::ofstream file(path, std::ios::app);
    std::string timestamp = get_timestamp();
    std::string value = random_value();
    file << "# Modified at: " << timestamp << std::endl;
    file << key << "=" << value << std::endl;
    file.close();
    std::cout << "[" << timestamp << "] Modified file: " << path << " with " << key << "=" << value << std::endl;
}

int main() {
    // 准备测试环境
    std::filesystem::create_directories("./config");
    
    // 初始化配置
    TrackConfig config;
    config.repoRoot = ".configtracker";
    config.watchPaths = {"./config"};
    config.enableAutoCommit = true;
    config.retentionDays = 7;
    
    // 创建并启动配置跟踪器
    ConfigTracker tracker(config);
    tracker.start();
    
    // 生成几个随机文件名
    std::vector<std::string> config_files = {
        "./config/app.conf",
        "./config/database.conf",
        "./config/logging.conf"
    };
    
    // 随机选择文件进行创建
    std::mt19937 rng(std::random_device{}());
    std::shuffle(config_files.begin(), config_files.end(), rng);
    
    // 创建并修改文件
    for (const auto& file : config_files) {
        // 创建文件
        create_test_file(file, "setting_" + random_value(4));
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        // 随机决定是否修改文件
        if (std::uniform_int_distribution<>(0, 1)(rng)) {
            modify_test_file(file, "option_" + random_value(4));
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    }
    
    // 测试手动提交
    std::cout << "[" << get_timestamp() << "] Triggering manual commit..." << std::endl;
    tracker.manualCommit();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // 停止监控
    tracker.stop();
    
    std::cout << "[" << get_timestamp() << "] All tests completed. Check .configtracker directory for results.\n";
    return 0;
}
