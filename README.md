# ConfigTracker - 配置文件版本跟踪工具

ConfigTracker 是一个轻量级的配置文件版本跟踪工具，通过 Git 版本控制系统自动管理配置文件的更改历史。它能监控指定目录中的文件变化，自动提交更改，并提供便捷的历史版本访问功能。

## 主要功能

- **实时文件监控**：自动监测指定目录中的文件变更
- **自动版本控制**：基于 Git 自动创建提交记录
- **版本历史管理**：支持查看和恢复历史版本
- **手动提交控制**：除自动提交外，也支持手动触发提交
- **可定制保留策略**：支持设置版本历史保留天数

## 安装要求

- C++17 或更高版本
- CMake 3.15 或更高版本
- libgit2 库

## 快速开始

### 编译安装

```bash
mkdir build && cd build
cmake ..
make
```

### 基本使用示例

```cpp
#include "configtracker/config_tracker.h"

int main() {
    // 配置跟踪器
    configtracker::TrackConfig config;
    config.repoRoot = ".configtracker";  // Git 仓库位置
    config.watchPaths = {"./config"};    // 要监控的目录
    config.enableAutoCommit = true;      // 启用自动提交
    config.retentionDays = 7;            // 保留7天历史记录
    
    // 创建并启动跟踪器
    configtracker::ConfigTracker tracker(config);
    tracker.start();  // 开始监控
    
    // ... 应用程序逻辑 ...
    
    // 手动触发提交
    tracker.manualCommit();
    
    // 停止监控
    tracker.stop();
    
    return 0;
}
```

## 核心组件

- **ConfigTracker**：主要接口类，提供配置跟踪服务
- **FileWatcher**：负责监控文件系统变更
- **GitRepoManager**：封装 Git 操作，管理版本历史

## API 参考

### ConfigTracker

- `ConfigTracker(const TrackConfig& config)`：构造函数，初始化配置跟踪器
- `start()`：启动文件监控和版本跟踪
- `stop()`：停止文件监控
- `manualCommit()`：手动触发提交

### TrackConfig 结构体

- `repoRoot`：Git 仓库根目录路径
- `watchPaths`：需要监控的目录路径列表
- `enableAutoCommit`：是否启用自动提交
- `retentionDays`：历史版本保留天数


