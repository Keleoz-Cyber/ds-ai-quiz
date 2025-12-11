#pragma once

#include <string>
#include <vector>
#include <unordered_map>

// 一道题目的结构
struct Question {
    int id;
    std::string text;
    std::vector<std::string> options;
    int answer;          // 正确选项下标，从 0 开始
    std::string knowledge;    // 知识点
    int difficulty;      // 难度 1~5
};

// 全局题库
extern std::vector<Question> g_questions;

// 题号 -> Question* 的映射
extern std::unordered_map<int, const Question*> g_questionById;

// 从文件加载题库
bool loadQuestionsFromFile(const std::string& filename);
