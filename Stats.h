/**
 * @file Stats.h
 * @brief 统计模块头文件 - 提供题目和知识点的统计功能
 *
 * 本模块负责：
 * 1. 聚合用户做题记录，计算每道题目的统计数据（正确率、用时等）
 * 2. 按知识点维度统计正确率，帮助用户了解薄弱环节
 * 3. 提供统计信息的展示功能
 */

#pragma once

#include <unordered_map>
#include <string>

/**
 * @struct QuestionStat
 * @brief 单个题目的统计信息
 *
 * 用于记录某道题目的历史作答情况，包括作答次数、正确次数、
 * 累计用时和最近作答时间等信息。这些数据可用于智能推荐
 * （如优先推荐错误率高或长时间未作答的题目）。
 */
struct QuestionStat {
    int totalAttempts = 0;       ///< 总作答次数：该题被作答的总次数
    int correctAttempts = 0;     ///< 答对次数：该题被答对的次数
    int totalTime = 0;           ///< 累计作答时间：所有作答的总用时（秒）
    long long lastTimestamp = 0; ///< 最近一次作答时间：Unix 时间戳，用于判断题目是否长时间未复习
};

/**
 * @struct KnowledgeStat
 * @brief 知识点的统计信息
 *
 * 用于记录某个知识点下所有题目的整体表现，包括总题数、
 * 答对题数和正确率。可用于分析用户在不同知识点上的掌握情况。
 */
struct KnowledgeStat {
    int total = 0;      ///< 该知识点的总作答题数
    int correct = 0;    ///< 该知识点的答对题数
    double accuracy = 0.0; ///< 该知识点的正确率（百分比形式，范围 0.0-100.0）
};

/**
 * @brief 全局题目统计数据表
 *
 * 键为题目 ID，值为该题目的统计信息。
 * 该数据由 buildQuestionStats() 构建，用于快速查询单个题目的统计。
 */
extern std::unordered_map<int, QuestionStat> g_questionStats;

/**
 * @brief 从做题记录构建题目统计信息
 *
 * 遍历全局记录数据 g_recordsByQuestion，对每道题目的所有作答记录进行聚合，
 * 计算总作答次数、答对次数、累计用时和最近作答时间，并更新到 g_questionStats 中。
 *
 * @note 每次调用会清空并重建 g_questionStats
 * @complexity 时间复杂度 O(M)，其中 M 为总记录数
 * @see g_recordsByQuestion (Record.h)
 * @see g_questionStats
 */
void buildQuestionStats();

/**
 * @brief 构建知识点统计信息
 *
 * 遍历全局记录数据 g_records，根据每条记录对应的题目查找其所属知识点，
 * 累加各知识点的总题数和答对题数，最后计算每个知识点的正确率。
 *
 * @return 知识点统计映射表，键为知识点名称，值为该知识点的统计信息
 * @complexity 时间复杂度 O(M)，其中 M 为总记录数
 * @note 如果题目 ID 在题库中不存在，则跳过该记录
 * @see g_records (Record.h)
 * @see g_questionById (Question.h)
 */
std::unordered_map<std::string, KnowledgeStat> buildKnowledgeStats();

/**
 * @brief 显示统计信息（交互式展示）
 *
 * 在控制台展示以下统计内容：
 * 1. 总体统计：总作答题数、答对题数、总体正确率
 * 2. 按知识点统计：每个知识点的题数、正确数、正确率
 * 3. 当前错题数量
 *
 * @complexity 时间复杂度 O(M)，其中 M 为总记录数
 * @note 如果没有做题记录，会提示用户
 * @note 显示完成后会调用 pauseForUser() 等待用户确认
 */
void showStatistics();
