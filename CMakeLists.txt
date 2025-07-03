cmake_minimum_required(VERSION 3.15)
project(ConfigTracker)

set(CMAKE_CXX_STANDARD 17)

# 设置头文件路径
include_directories(include)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBGIT2 REQUIRED libgit2)

# 传递 include 路径和链接库
include_directories(${LIBGIT2_INCLUDE_DIRS})
link_directories(${LIBGIT2_LIBRARY_DIRS})


# 添加源码文件
add_library(configtracker
    src/config_tracker.cpp
    src/git_repo_manager.cpp
    src/file_watcher.cpp
)

# 链接 libgit2
target_link_libraries(configtracker PRIVATE ${LIBGIT2_LIBRARIES})
# target_link_libraries(configtracker PRIVATE git2)

# 示例程序
add_executable(example example/main.cpp)
target_link_libraries(example PRIVATE configtracker)

# 设置 include 路径
target_include_directories(configtracker PUBLIC include)
target_include_directories(example PRIVATE include)