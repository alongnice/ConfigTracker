#pragma once

#include <string>
#include <vector>
#include <git2.h>
#include <filesystem>



namespace configtracker {

class GitRepoManager {
public:
    GitRepoManager(const std::string& repoPath);
    ~GitRepoManager();
    
    void init();
    void addFile(const std::string& path);
    void commit(const std::string& message);
    std::vector<std::string> listCommits();
    std::string getLatestCommit();
    bool checkoutCommit(const std::string& hash);
    void squashCommitsOlderThan(int days);
    
private:
    std::string repoPath_;
    git_repository* repo_;
};

}
