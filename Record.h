/**
 * @file Record.h
 * @brief 做题记录管理模块 - 数据结构与接口
 *
 * 【模块职责】
 * 1. 定义做题记录结构 Record
 * 2. 维护全局做题记录容器：g_records、g_recordsByQuestion、g_wrongQuestions
 * 3. 多用户隔离：每个用户的记录存储于独立的 CSV 文件
 * 4. 提供做题流程：展示题目、计时、判分、记录、持久化
 *
 * 【关键数据结构】
 * - Record 结构体：单次作答记录（题号、正误、用时、时间戳）
 * - std::vector<Record> g_records：所有做题记录的时间序列
 * - std::unordered_map<int, std::vector<Record>> g_recordsByQuestion：按题号分组的记录
 * - std::unordered_set<int> g_wrongQuestions：当前错题集合（最后一次答错的题号）
 *
 * 【输入/输出文件格式】
 * - 文件名：data/records_<userId>.csv（多用户隔离）
 * - 格式：CSV（逗号分隔），UTF-8 编码
 * - 列定义（4 列）：
 *   [0] questionId   题号
 *   [1] correct      是否答对（1/0）
 *   [2] usedSeconds  作答用时（秒）
 *   [3] timestamp    时间戳（秒，Unix epoch）
 * - 写入策略：追加模式（std::ios::app）
 *
 * 【与其他模块依赖】
 * - Question.cpp：通过 g_questionById 查询题目详情
 * - Stats.cpp：从 g_records、g_recordsByQuestion 构建统计信息
 * - Recommender.cpp：根据 g_questionStats 计算推荐分数
 * - App.cpp：各功能模式（随机刷题、错题本、考试）调用 doQuestion()
 */

#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include "Question.h"

/**
 * @brief 做题记录数据结构
 *
 * 存储单次作答的完整信息，用于统计分析、错题管理、智能推荐等。
 */
struct Record {
    int questionId;      ///< 题号（对应 Question.id）
    bool correct;        ///< 是否答对（true/false）
    int usedSeconds;     ///< 作答用时（秒，通过 chrono 计时）
    long long timestamp; ///< 时间戳（秒，Unix epoch，用于计算时间间隔）
};

/**
 * @brief 全局做题记录容器（时间序列）
 *
 * 按时间顺序存储所有做题记录，适用场景：
 * - 总体统计（总题数、正确率）
 * - 考试模式统计（统计本次考试的记录）
 * - 学习报告导出
 */
extern std::vector<Record> g_records;

/**
 * @brief 按题号分组的做题记录
 *
 * 题号 -> 该题的所有做题记录（按时间顺序）
 * 适用场景：
 * - 题目维度统计（某题的作答次数、正确率、平均用时）
 * - 判断最后一次作答结果（构建错题集）
 */
extern std::unordered_map<int, std::vector<Record>> g_recordsByQuestion;

/**
 * @brief 当前错题集合（最后一次答错的题号）
 *
 * 维护策略：
 * - 最后一次答错：加入错题集
 * - 最后一次答对：从错题集移除
 * 适用场景：
 * - 错题本练习（从错题集随机抽题）
 * - 错题数统计
 */
extern std::unordered_set<int> g_wrongQuestions;

/**
 * @brief 当前用户 ID（学号或用户名）
 *
 * 用于多用户隔离，不同用户的记录存储于不同文件。
 */
extern std::string g_currentUserId;

/**
 * @brief 获取当前用户的记录文件路径
 *
 * @return std::string 记录文件路径（如 "data/records_202001.csv"）
 * @note 若 g_currentUserId 为空，返回默认路径 "data/records.csv"
 */
std::string getRecordFilePath();

/**
 * @brief 用户登录：设置当前用户 ID
 *
 * @param userId 用户标识（学号或用户名）
 * @note 调用后会在控制台输出"当前用户：<userId>"
 */
void loginUser(const std::string& userId);

/**
 * @brief 清空当前用户的所有记录数据（用于切换用户）
 *
 * 清空：g_records、g_recordsByQuestion、g_wrongQuestions
 * @note 调用时机：切换用户、重新加载记录前
 */
void clearUserRecords();

/**
 * @brief 从文件加载历史做题记录
 *
 * 【功能】
 * - 读取 records_<userId>.csv 文件（UTF-8 编码）
 * - 逐行解析，每行一条记录（逗号分隔）
 * - 重建索引：g_records、g_recordsByQuestion、g_wrongQuestions
 * - 调用 buildQuestionStats() 构建统计信息
 *
 * 【解析策略】
 * - 空行：跳过
 * - 列数不足（< 4 列）：跳过
 * - 字段解析失败：跳过
 * - 文件不存在：不报错（首次使用），输出提示信息
 *
 * 【错题集构建规则】
 * - 对每个题号，取最后一次作答记录
 * - 若最后一次答错，加入 g_wrongQuestions
 *
 * @param filename 记录文件路径
 * @return true  加载成功（包括文件不存在的情况）
 * @return false 保留，当前实现总是返回 true
 *
 * @note 输出信息：控制台显示"历史做题记录加载完成，共 X 条，当前错题数 Y 道"
 * @complexity O(M)，M 为记录数量（逐行解析 + 索引构建）
 */
bool loadRecordsFromFile(const std::string& filename);

/**
 * @brief 追加记录到文件
 *
 * 【功能】
 * - 以追加模式（std::ios::app）打开文件
 * - 写入一行：questionId,correct,usedSeconds,timestamp
 * - 立即落盘（fout 析构时自动 flush）
 *
 * @param r 做题记录
 * @param filename 记录文件路径
 * @note 若文件打开失败，输出警告信息，不影响内存中的记录
 * @complexity O(1)
 */
void appendRecordToFile(const Record& r, const std::string& filename);

/**
 * @brief 完成一道题的答题过程
 *
 * 【流程】
 * 1. 展示题目：题号、知识点、难度、题目文本、选项
 * 2. 等待用户输入答案（整数，对应选项下标）
 * 3. 计时：使用 std::chrono::steady_clock 记录开始/结束时间
 * 4. 判分：比较用户答案与正确答案（q.answer）
 * 5. 输出结果：正确或错误（显示正确答案）
 * 6. 记录：构造 Record 对象
 * 7. 更新内存结构：
 *    - g_records.push_back(r)
 *    - g_recordsByQuestion[qid].push_back(r)
 *    - 更新 g_wrongQuestions（答对移除，答错加入）
 * 8. 持久化：调用 appendRecordToFile() 追加到 CSV
 *
 * 【边界条件】
 * - 用时为 0：设为 1 秒（避免统计异常）
 * - 非法输入（非整数）：std::cin 读取失败，视为错误（当前实现未做额外处理）
 *
 * @param q 题目对象（来自 g_questionById）
 * @note 该函数会阻塞等待用户输入
 * @note 计时精度：秒级（duration_cast<seconds>）
 * @complexity O(1) - 单次记录写入
 */
void doQuestion(const Question& q);
