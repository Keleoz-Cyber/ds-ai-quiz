#include "Question.h"
#include <iostream>
#include <fstream>
#include <sstream>

// 全局题库定义
std::vector<Question> g_questions;
std::unordered_map<int, const Question*> g_questionById;

bool loadQuestionsFromFile(const std::string& filename) {
    std::ifstream fin(filename);
    if (!fin.is_open()) {
        std::cout << "无法打开题库文件: " << filename << std::endl;
        return false;
    }

    std::string line;
    int lineNum = 0;
    while (std::getline(fin, line)) {
        ++lineNum;
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string field;
        std::vector<std::string> fields;

        // 用逗号 , 分隔
        while (std::getline(ss, field, ',')) {
            fields.push_back(field);
        }

        if (fields.size() < 9) {
            std::cout << "第 " << lineNum << " 行字段数量不足，已跳过。\n";
            continue;
        }

        Question q;
        try {
            q.id = std::stoi(fields[0]);
            q.text = fields[1];
            q.options = {fields[2], fields[3], fields[4], fields[5]};
            q.answer = std::stoi(fields[6]);      // 0~3
            q.knowledge = fields[7];
            q.difficulty = std::stoi(fields[8]);  // 1~5
        } catch (...) {
            std::cout << "第 " << lineNum << " 行解析出错，已跳过。\n";
            continue;
        }

        g_questions.push_back(q);
    }

    // 建立题号 -> Question* 的索引
    for (const auto& q : g_questions) {
        g_questionById[q.id] = &q;
    }

    std::cout << "题库加载完成，共读取到 " << g_questions.size() << " 道题。\n";
    return !g_questions.empty();
}
