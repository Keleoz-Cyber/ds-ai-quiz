#pragma once

#include <unordered_map>
#include <string>

// 题目统计信息
struct QuestionStat {
    int totalAttempts = 0;      // 总作答次数
    int correctAttempts = 0;    // 答对次数
    int totalTime = 0;          // 累计作答时间
    long long lastTimestamp = 0;// 最近一次作答时间
};

// 知识点统计信息
struct KnowledgeStat {
    int total = 0;
    int correct = 0;
    double accuracy = 0.0;
};

// 题号 -> 统计信息
extern std::unordered_map<int, QuestionStat> g_questionStats;

// 从记录构建题目统计信息
void buildQuestionStats();

// 构建知识点统计信息
std::unordered_map<std::string, KnowledgeStat> buildKnowledgeStats();

// 显示统计信息
void showStatistics();
