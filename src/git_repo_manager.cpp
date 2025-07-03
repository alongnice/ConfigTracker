#include "configtracker/git_repo_manager.h"
#include <iostream>

using namespace configtracker;

GitRepoManager::GitRepoManager(const std::string& repoPath) : repoPath_(repoPath), repo_(nullptr) {
    std::cout << "[GitRepoManager] Created for path: " << repoPath << "\n";
}

GitRepoManager::~GitRepoManager() {
    if (repo_) {
        git_repository_free(repo_);
        repo_ = nullptr;
    }
    git_libgit2_shutdown();
}

// src/git_repo_manager.cpp
void GitRepoManager::init() {
    std::cout << "[Git] Initialized repository.\n";
    
    // 检查仓库是否已存在
    if (!std::filesystem::exists(repoPath_)) {
        std::filesystem::create_directories(repoPath_);
    }
    
    // 初始化Git仓库
    git_libgit2_init();
    
    git_repository* repo = nullptr;
    int error = 0;
    
    if (!std::filesystem::exists(repoPath_ + "/.git")) {
        error = git_repository_init(&repo, repoPath_.c_str(), 0);
        if (error < 0) {
            const git_error* e = git_error_last();
            std::cerr << "Error initializing repository: " << e->message << std::endl;
            return;
        }
    } else {
        error = git_repository_open(&repo, repoPath_.c_str());
        if (error < 0) {
            const git_error* e = git_error_last();
            std::cerr << "Error opening repository: " << e->message << std::endl;
            return;
        }
    }
    
    repo_ = repo;
}

void GitRepoManager::addFile(const std::string& path) {
    std::cout << "[Git] Add file: " << path << "\n";
    
    if (!repo_) {
        std::cerr << "Error: Repository not initialized" << std::endl;
        return;
    }
    
    git_index* index = nullptr;
    int error = git_repository_index(&index, repo_);
    if (error < 0) {
        const git_error* e = git_error_last();
        std::cerr << "Error getting index: " << e->message << std::endl;
        return;
    }
    
    std::filesystem::path filePath(path);
    // 计算相对于仓库的路径
    std::string relativePath;
    
    try {
        // 计算文件相对于仓库的路径
        std::filesystem::path repoAbsPath = std::filesystem::absolute(repoPath_);
        std::filesystem::path fileAbsPath = std::filesystem::absolute(filePath);
        
        // 检查文件是否存在
        if (!std::filesystem::exists(fileAbsPath)) {
            std::cerr << "Error: File does not exist: " << fileAbsPath << std::endl;
            git_index_free(index);
            return;
        }
        
        // 如果文件在仓库目录外，复制到仓库内的相应位置
        if (!fileAbsPath.string().find(repoAbsPath.string()) == 0) {
            // 创建仓库内的目标路径
            std::filesystem::path targetPath = repoAbsPath / filePath.filename();
            std::filesystem::copy_file(fileAbsPath, targetPath, 
                                      std::filesystem::copy_options::overwrite_existing);
            relativePath = filePath.filename().string();
        } else {
            // 计算相对路径
            relativePath = std::filesystem::relative(fileAbsPath, repoAbsPath).string();
        }
        
        // 添加到索引
        error = git_index_add_bypath(index, relativePath.c_str());
        if (error < 0) {
            const git_error* e = git_error_last();
            std::cerr << "Error adding file to index: " << e->message << std::endl;
        } else {
            // 写入索引
            error = git_index_write(index);
            if (error < 0) {
                const git_error* e = git_error_last();
                std::cerr << "Error writing index: " << e->message << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception while adding file: " << e.what() << std::endl;
    }
    
    git_index_free(index);
}

// src/git_repo_manager.cpp
void GitRepoManager::commit(const std::string& message) {
    std::cout << "[Git] Commit with message: " << message << "\n";
    
    // 检查仓库是否初始化
    if (!repo_) {
        std::cerr << "Error: Repository not initialized" << std::endl;
        return;
    }
    
    git_index* index = nullptr;
    git_oid commit_id, tree_id;
    git_tree* tree = nullptr;
    git_signature* signature = nullptr;
    git_commit* parent = nullptr;
    
    // 获取索引
    int error = git_repository_index(&index, repo_);
    if (error < 0) goto cleanup;
    
    // 写入索引到树
    error = git_index_write_tree(&tree_id, index);
    if (error < 0) goto cleanup;
    
    // 创建树对象
    error = git_tree_lookup(&tree, repo_, &tree_id);
    if (error < 0) goto cleanup;
    
    // 创建签名
    error = git_signature_default(&signature, repo_);
    if (error < 0) {
        // 如果没有默认签名，创建一个
        error = git_signature_now(&signature, "ConfigTracker", "configtracker@example.com");
        if (error < 0) goto cleanup;
    }
    
    // 创建提交
    git_oid parent_id;
    
    if (git_reference_name_to_id(&parent_id, repo_, "HEAD") == 0) {
        // 有上一个提交
        git_commit_lookup(&parent, repo_, &parent_id);
        git_commit* parents[] = {parent};
        error = git_commit_create(&commit_id, repo_, "HEAD", signature, signature, 
                               "UTF-8", message.c_str(), tree, 1, (const git_commit**)parents);
    } else {
        // 首次提交
        error = git_commit_create(&commit_id, repo_, "HEAD", signature, signature, 
                               "UTF-8", message.c_str(), tree, 0, nullptr);
    }
    
cleanup:
    if (parent) git_commit_free(parent);
    git_index_free(index);
    git_tree_free(tree);
    git_signature_free(signature);
    
    if (error < 0) {
        const git_error* e = git_error_last();
        std::cerr << "Error creating commit: " << e->message << std::endl;
    } else if (error == 0) {
        std::cout << "[Git] Commit successful" << std::endl;
    }
}


std::vector<std::string> GitRepoManager::listCommits() {
    std::vector<std::string> result;
    
    if (!repo_) {
        std::cerr << "Error: Repository not initialized" << std::endl;
        return result;
    }
    
    git_revwalk* walker = nullptr;
    int error = git_revwalk_new(&walker, repo_);
    if (error < 0) {
        const git_error* e = git_error_last();
        std::cerr << "Error creating revision walker: " << e->message << std::endl;
        return result;
    }
    
    // 设置遍历方向为时间顺序
    git_revwalk_sorting(walker, GIT_SORT_TIME);
    
    // 从HEAD开始遍历
    error = git_revwalk_push_head(walker);
    if (error < 0) {
        const git_error* e = git_error_last();
        std::cerr << "Error pushing HEAD to revision walker: " << e->message << std::endl;
        git_revwalk_free(walker);
        return result;
    }
    
    // 遍历所有提交
    git_oid oid;
    while (git_revwalk_next(&oid, walker) == 0) {
        git_commit* commit = nullptr;
        error = git_commit_lookup(&commit, repo_, &oid);
        
        if (error < 0) {
            const git_error* e = git_error_last();
            std::cerr << "Error looking up commit: " << e->message << std::endl;
            continue;
        }
        
        // 获取提交的哈希值
        char hash[GIT_OID_HEXSZ + 1] = {0};
        git_oid_fmt(hash, &oid);
        result.push_back(std::string(hash));
        
        // 释放提交对象
        git_commit_free(commit);
    }
    
    git_revwalk_free(walker);
    return result;
}

void GitRepoManager::squashCommitsOlderThan(int days) {
    std::cout << "[Git] Squash commits older than " << days << " days\n";
    if (!repo_) {
        std::cerr << "Error: Repository not initialized" << std::endl;
        return;
    }
    
    // 获取当前时间
    auto now = std::chrono::system_clock::now();
    auto cutoff_time = now - std::chrono::hours(24 * days);
    time_t cutoff_time_t = std::chrono::system_clock::to_time_t(cutoff_time);
    std::cout << "Squashing commits older than: " << std::ctime(&cutoff_time_t);
    
    // 声明变量，将其移到goto之前
    git_revwalk* walker = nullptr;
    std::vector<git_oid> old_commits;
    bool has_old_commits = false;
    git_oid oid;
    int error = 0;
    
    // 获取所有提交
    error = git_revwalk_new(&walker, repo_);
    if (error < 0) goto cleanup;
    git_revwalk_sorting(walker, GIT_SORT_TIME);
    error = git_revwalk_push_head(walker);
    if (error < 0) goto cleanup;
    
    // 收集旧提交
    while (git_revwalk_next(&oid, walker) == 0) {
        git_commit* commit = nullptr;
        int commit_error = git_commit_lookup(&commit, repo_, &oid);
        
        if (commit_error == 0) {
            git_time_t commit_time = git_commit_time(commit);
            time_t time_t_commit = static_cast<time_t>(commit_time);
            
            if (time_t_commit < cutoff_time_t) {
                // 保存旧提交
                old_commits.push_back(oid);
                has_old_commits = true;
            }
            
            git_commit_free(commit);
        }
    }
    
    // 如果有旧提交，创建一个合并提交
    if (has_old_commits) {
        // 实际项目中这里可以实现真正的压缩合并逻辑
        std::cout << "Found " << old_commits.size() << " old commits that could be squashed.\n";
        std::cout << "In a full implementation, these would be combined into a single historical commit.\n";
    } else {
        std::cout << "No commits found older than specified days.\n";
    }
    
cleanup:
    if (walker) git_revwalk_free(walker);
    if (error < 0) {
        const git_error* e = git_error_last();
        std::cerr << "Error during squash operation: " << e->message << std::endl;
    }
}

bool GitRepoManager::checkoutCommit(const std::string& hash) {
    std::cout << "[Git] Checkout commit: " << hash << "\n";
    
    if (!repo_) {
        std::cerr << "Error: Repository not initialized" << std::endl;
        return false;
    }
    
    // 将哈希字符串转换为OID
    git_oid oid;
    int error = git_oid_fromstr(&oid, hash.c_str());
    if (error < 0) {
        const git_error* e = git_error_last();
        std::cerr << "Error parsing commit hash: " << e->message << std::endl;
        return false;
    }
    
    // 查找提交对象
    git_commit* commit = nullptr;
    error = git_commit_lookup(&commit, repo_, &oid);
    if (error < 0) {
        const git_error* e = git_error_last();
        std::cerr << "Error looking up commit: " << e->message << std::endl;
        return false;
    }
    
    // 获取提交的树
    git_tree* tree = nullptr;
    error = git_commit_tree(&tree, commit);
    if (error < 0) {
        const git_error* e = git_error_last();
        std::cerr << "Error getting commit tree: " << e->message << std::endl;
        git_commit_free(commit);
        return false;
    }
    
    // 创建checkout选项
    git_checkout_options opts = GIT_CHECKOUT_OPTIONS_INIT;
    opts.checkout_strategy = GIT_CHECKOUT_SAFE | GIT_CHECKOUT_RECREATE_MISSING;
    
    // 执行checkout
    error = git_checkout_tree(repo_, (git_object*)tree, &opts);
    if (error < 0) {
        const git_error* e = git_error_last();
        std::cerr << "Error checking out tree: " << e->message << std::endl;
        git_tree_free(tree);
        git_commit_free(commit);
        return false;
    }
    
    // 更新HEAD引用为当前提交
    git_reference* head_ref = nullptr;
    error = git_repository_head(&head_ref, repo_);
    if (error < 0) {
        std::cerr << "Warning: Could not update HEAD reference" << std::endl;
    } else {
        const char* head_name = git_reference_name(head_ref);
        error = git_reference_set_target(NULL, head_ref, &oid, "checkout: moving to commit");
        if (error < 0) {
            const git_error* e = git_error_last();
            std::cerr << "Error updating HEAD reference: " << e->message << std::endl;
        }
        git_reference_free(head_ref);
    }
    
    git_tree_free(tree);
    git_commit_free(commit);
    
    return (error == 0);
}

std::string GitRepoManager::getLatestCommit() {
    if (!repo_) {
        std::cerr << "Error: Repository not initialized" << std::endl;
        return "";
    }
    
    git_oid oid;
    int error = git_reference_name_to_id(&oid, repo_, "HEAD");
    if (error < 0) {
        const git_error* e = git_error_last();
        std::cerr << "Error getting HEAD reference: " << e->message << std::endl;
        return "";
    }
    
    // 转换OID为字符串
    char hash[GIT_OID_HEXSZ + 1] = {0};
    git_oid_fmt(hash, &oid);
    
    return std::string(hash);
}