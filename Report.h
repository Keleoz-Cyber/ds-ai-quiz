/**
 * @file Report.h
 * @brief 学习报告导出模块头文件
 * @details 提供学习报告生成和导出功能，支持 Markdown 格式输出
 */

#pragma once

#include <string>

/**
 * @brief 导出当前用户的学习报告（Markdown 格式）
 * @details 生成包含以下内容的完整学习报告：
 *          1. 报告标题与用户信息（用户ID、生成时间、数据来源）
 *          2. 总体概览（总作答题数、答对/答错题数、总体正确率、当前错题数）
 *          3. 按知识点统计（各知识点的作答次数、答对次数、正确率）
 *          4. 错题分布（按知识点和难度分类统计错题数量）
 *          5. 复习建议（薄弱知识点提示、错题本练习建议、智能推荐功能介绍）
 *
 * @note 报告文件存储在 reports/ 目录下
 * @note 文件命名规则：report_{用户ID}_{时间戳}.md
 * @note 时间戳格式：YYYYMMDD_HHMM（例如：20231225_1430）
 *
 * @complexity 时间复杂度 O(M)，其中 M 为做题记录数量
 *             - 遍历所有做题记录一次进行统计
 *             - 按知识点和难度分类统计复用同一次遍历的数据
 * @complexity 空间复杂度 O(K + D)，其中 K 为知识点数量，D 为难度等级数量
 *             - 需要维护知识点统计表和错题分布表
 *
 * @see getTimeStringForFilename() 获取文件名用时间戳
 * @see getTimeStringForDisplay() 获取显示用时间字符串
 * @see g_records 全局做题记录容器
 * @see g_wrongQuestions 全局错题集合
 * @see g_questionById 全局题目索引
 */
void exportLearningReport();
