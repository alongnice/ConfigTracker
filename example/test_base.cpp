// test/test_configtracker.cpp
#include "configtracker/config_tracker.h"
#include <cassert>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <thread>

using namespace configtracker;

void test_basic_functionality() {
    // 创建测试配置
    TrackConfig config;
    config.repoRoot = ".configtracker_test";
    config.watchPaths = {"./test_config"};
    config.enableAutoCommit = true;
    config.retentionDays = 7;
    
    // 确保测试目录存在
    std::filesystem::create_directories("./test_config");
    
    // 初始化 ConfigTracker
    ConfigTracker tracker(config);
    tracker.start();
    
    // 创建测试文件
    std::ofstream testFile("./test_config/test.txt");
    testFile << "Initial content" << std::endl;
    testFile.close();
    
    // 等待文件变更被检测到
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 修改文件触发自动提交
    testFile.open("./test_config/test.txt", std::ios::app);
    testFile << "Updated content" << std::endl;
    testFile.close();
    
    // 等待变更被检测
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 停止监控
    tracker.stop();
    
    // 清理测试文件和目录
    std::filesystem::remove_all("./test_config");
    std::filesystem::remove_all(".configtracker_test");
    
    std::cout << "Basic functionality test completed." << std::endl;
}

int main() {
    test_basic_functionality();
    return 0;
}
