#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

// 知识点依赖图相关数据结构
extern std::unordered_map<std::string, std::vector<std::string>> g_knowledgePrereq;   // 知识点 -> 前置知识点列表
extern std::unordered_set<std::string> g_allKnowledgeNodes;                 // 所有知识点节点

// 从文件加载知识点依赖图
bool loadKnowledgeGraphFromFile(const std::string& filename);

// DFS生成复习路径（拓扑顺序）
void dfsReviewPath(const std::string& node, std::unordered_set<std::string>& visited, std::vector<std::string>& path);

// 知识点复习路径推荐
void recommendReviewPath();
