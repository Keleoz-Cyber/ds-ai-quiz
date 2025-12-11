#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include "Question.h"

// 做题记录结构
struct Record {
    int questionId;      // 题号
    bool correct;        // 是否答对
    int usedSeconds;     // 作答用时（秒）
    long long timestamp; // 时间戳（秒）
};

// 全局做题记录
extern std::vector<Record> g_records;                                  // 所有做题记录
extern std::unordered_map<int, std::vector<Record>> g_recordsByQuestion;    // 按题目分组
extern std::unordered_set<int> g_wrongQuestions;                       // 当前仍为错题的题号集合

// 从文件加载历史做题记录
bool loadRecordsFromFile(const std::string& filename);

// 追加记录到文件
void appendRecordToFile(const Record& r, const std::string& filename = "data/records.csv");

// 完成一道题的答题过程
void doQuestion(const Question& q);
