/**
 * @file Recommender.h
 * @brief AI 智能推荐系统头文件
 *
 * 实现基于多维度评分的题目推荐算法，综合考虑用户的历史做题记录、
 * 错误率、时间间隔、题目难度等因素，为用户推荐最适合练习的题目。
 */

#pragma once

#include "Question.h"
#include "Stats.h"

/**
 * @struct RecommendItem
 * @brief 推荐项结构体
 *
 * 用于存储题目推荐信息，包含题目 ID 和对应的推荐评分。
 * 重载了 operator< 以支持优先队列的大顶堆排序。
 */
struct RecommendItem {
    int questionId;    ///< 题目 ID
    double score;      ///< 推荐评分（越高越值得推荐）

    /**
     * @brief 优先队列比较运算符重载
     *
     * priority_queue 默认是大顶堆，通过定义 operator< 实现按 score 从大到小排列。
     * 这样可以高效地取出 Top-K 个最值得推荐的题目。
     *
     * @param other 另一个推荐项
     * @return true 如果当前项的分数小于 other，用于构建大顶堆
     *
     * @note 使用大顶堆的原因：
     *       1. 自动维护分数最高的 K 个题目
     *       2. 插入操作 O(log N)，取堆顶 O(1)
     *       3. 整体时间复杂度 O(N log N)，空间复杂度 O(N)
     */
    bool operator<(const RecommendItem& other) const {
        return score < other.score;
    }
};

/**
 * @brief 计算题目的推荐评分
 *
 * 基于多维度加权计算推荐分数，综合考虑以下因素：
 *
 * 1. **错误率维度** (权重 0.6)
 *    - 公式：errorRate = (总尝试次数 - 正确次数) / 总尝试次数
 *    - 未做过的题目错误率视为 1.0（最高优先级）
 *    - 错误率越高说明该题是薄弱点，越需要练习
 *
 * 2. **时间间隔维度** (权重 0.3)
 *    - 公式：timeScore = min(距上次做题天数 / 7.0, 1.0)
 *    - 未做过的题目给予 7 天的默认间隔
 *    - 符合遗忘曲线理论，间隔越久越需要复习
 *
 * 3. **难度维度** (权重 0.1)
 *    - 公式：diffScore = 0.2 + (难度 - 1) × 0.2，映射 [1,5] → [0.2,1.0]
 *    - 适当倾向推荐难度较高的题目，促进能力提升
 *
 * 4. **未做奖励** (固定加分 +0.2)
 *    - 对从未尝试过的题目给予额外奖励
 *    - 鼓励用户探索新题目，避免只刷熟悉的题
 *
 * **综合评分公式：**
 * @code
 * score = 0.6 × errorRate + 0.3 × timeScore + 0.1 × diffScore + unseenBonus
 * @endcode
 *
 * @param q 题目对象，包含难度等静态信息
 * @param st 题目统计信息，包含做题记录、正确率、时间戳等
 * @param now 当前时间戳（秒）
 *
 * @return 推荐评分，范围 [0.0, 2.0]，分数越高表示越值得推荐
 *
 * @note 权重可调性：
 *       - 当前权重配置：错误率 0.6 > 时间间隔 0.3 > 难度 0.1
 *       - 可根据实际需求调整权重，例如：
 *         * 强化复习：提高时间间隔权重
 *         * 攻克难题：提高难度权重
 *         * 补弱项：提高错误率权重
 *
 * @note 时间复杂度：O(1)，每道题的评分计算都是常数时间操作
 */
double computeRecommendScore(const Question& q, const QuestionStat& st, long long now);

/**
 * @brief AI 智能推荐模式主函数
 *
 * 基于用户的历史做题记录，运用推荐算法为用户推荐最适合练习的 K 道题目。
 *
 * **算法流程：**
 * 1. 检查题库是否为空
 * 2. 更新所有题目的统计信息（buildQuestionStats）
 * 3. 获取当前时间戳
 * 4. 遍历所有题目，计算每道题的推荐评分
 * 5. 使用优先队列（大顶堆）维护 Top-K 推荐题目
 * 6. 从堆中取出分数最高的 K 道题
 * 7. 按推荐顺序逐题展示并让用户作答
 * 8. 记录作答结果并更新统计信息
 *
 * **时间复杂度分析：**
 * - 遍历所有题目：O(N)
 * - 每道题计算评分：O(1)
 * - 插入优先队列：O(log N)
 * - 总体复杂度：O(N log N)
 * - 取出 Top-K：O(K log N)，通常 K << N，因此接近 O(N log N)
 *
 * **空间复杂度：**
 * - 优先队列存储所有题目：O(N)
 * - 推荐列表：O(K)
 * - 总体：O(N)
 *
 * @note 推荐数量：
 *       - 默认推荐 K=5 道题
 *       - 若题库总数少于 K，则推荐全部题目
 *       - K 值可根据用户偏好或题库规模调整
 *
 * @note 推荐特点：
 *       - 自适应：根据用户历史表现动态调整推荐
 *       - 平衡性：兼顾错误率、遗忘曲线、难度梯度
 *       - 探索性：对未做题目给予适当倾斜
 *
 * @see computeRecommendScore() 评分算法实现
 * @see RecommendItem 推荐项数据结构
 */
void aiRecommendMode();
