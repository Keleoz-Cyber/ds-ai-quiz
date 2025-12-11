#pragma once

#include "Question.h"
#include "Stats.h"

// 推荐项结构
struct RecommendItem {
    int questionId;
    double score;

    // priority_queue 默认是大顶堆，这里按 score 从大到小排列
    bool operator<(const RecommendItem& other) const {
        return score < other.score;
    }
};

// 计算推荐分数
double computeRecommendScore(const Question& q, const QuestionStat& st, long long now);

// AI 智能推荐模式
void aiRecommendMode();
