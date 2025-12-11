#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <queue>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <random>


#ifdef _WIN32
#include <windows.h>
#endif


using namespace std;

// 一道题目的结构
struct Question {
    int id;
    string text;
    vector<string> options;
    int answer;          // 正确选项下标，从 0 开始
    string knowledge;    // 知识点
    int difficulty;      // 难度 1~5
};

struct Record {
    int questionId;      // 题号
    bool correct;        // 是否答对
    int usedSeconds;     // 作答用时（秒）
    long long timestamp; // 时间戳（秒）
};

struct QuestionStat {
    int totalAttempts = 0;      // 总作答次数
    int correctAttempts = 0;    // 答对次数
    int totalTime = 0;          // 累计作答时间
    long long lastTimestamp = 0;// 最近一次作答时间
};

struct RecommendItem {
    int questionId;
    double score;

    // priority_queue 默认是大顶堆，这里按 score 从大到小排列
    bool operator<(const RecommendItem& other) const {
        return score < other.score;
    }
};


unordered_map<int, QuestionStat> g_questionStats; // 题号 -> 统计信息

vector<Record> g_records;                                  // 所有做题记录
unordered_map<int, vector<Record>> g_recordsByQuestion;    // 按题目分组
unordered_set<int> g_wrongQuestions;                       // 当前仍为错题的题号集合
unordered_map<int, const Question*> g_questionById;        // 题号 -> Question*

// 知识点依赖图相关数据结构
unordered_map<string, vector<string>> g_knowledgePrereq;   // 知识点 -> 前置知识点列表
unordered_set<string> g_allKnowledgeNodes;                 // 所有知识点节点

// 全局题库（后面会改成从文件读取）
vector<Question> g_questions;

void buildQuestionStats() {
    g_questionStats.clear();
    for (const auto& p : g_recordsByQuestion) {
        int qid = p.first;
        const auto& vec = p.second;
        QuestionStat st;
        for (const auto& r : vec) {
            st.totalAttempts++;
            if (r.correct) st.correctAttempts++;
            st.totalTime += r.usedSeconds;
            if (r.timestamp > st.lastTimestamp) {
                st.lastTimestamp = r.timestamp;
            }
        }
        g_questionStats[qid] = st;
    }
}

// 初始化几个示例题目（后面会换成读 data/questions.csv）
bool loadQuestionsFromFile(const string& filename) {
    ifstream fin(filename);
    if (!fin.is_open()) {
        cout << "无法打开题库文件: " << filename << endl;
        return false;
    }

    string line;
    int lineNum = 0;
    while (getline(fin, line)) {
        ++lineNum;
        if (line.empty()) continue;

        stringstream ss(line);
        string field;
        vector<string> fields;

        // 用逗号 , 分隔
        while (getline(ss, field, ',')) {
            fields.push_back(field);
        }

        if (fields.size() < 9) {
            cout << "第 " << lineNum << " 行字段数量不足，已跳过。\n";
            continue;
        }

        Question q;
        try {
            q.id = stoi(fields[0]);
            q.text = fields[1];
            q.options = {fields[2], fields[3], fields[4], fields[5]};
            q.answer = stoi(fields[6]);      // 0~3
            q.knowledge = fields[7];
            q.difficulty = stoi(fields[8]);  // 1~5
        } catch (...) {
            cout << "第 " << lineNum << " 行解析出错，已跳过。\n";
            continue;
        }

        g_questions.push_back(q);
    }

    // 建立题号 -> Question* 的索引
    for (const auto& q : g_questions) {
        g_questionById[q.id] = &q;
    }

    cout << "题库加载完成，共读取到 " << g_questions.size() << " 道题。\n";
    return !g_questions.empty();
}

bool loadRecordsFromFile(const string& filename) {
    ifstream fin(filename);
    if (!fin.is_open()) {
        // 没有记录文件不算错误，可能是第一次使用
        cout << "未找到做题记录文件，将从空记录开始。\n";
        return true;
    }

    string line;
    while (getline(fin, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string field;
        vector<string> fields;
        while (getline(ss, field, ',')) {
            fields.push_back(field);
        }
        if (fields.size() < 4) continue;

        Record r;
        try {
            r.questionId = stoi(fields[0]);
            r.correct = (fields[1] == "1");
            r.usedSeconds = stoi(fields[2]);
            r.timestamp = stoll(fields[3]);
        } catch (...) {
            continue;
        }

        g_records.push_back(r);
        g_recordsByQuestion[r.questionId].push_back(r);
    }

    // 根据“最后一次作答结果”构建当前错题集合
    for (auto& p : g_recordsByQuestion) {
        int qid = p.first;
        auto& vec = p.second;
        if (!vec.empty()) {
            const Record& last = vec.back();
            if (!last.correct) {
                g_wrongQuestions.insert(qid);
            }
        }
    }

    cout << "历史做题记录加载完成，共 " << g_records.size() << " 条，当前错题数 "
     << g_wrongQuestions.size() << " 道。\n";

    // 加这一句：
    buildQuestionStats();

    return true;
}

bool loadKnowledgeGraphFromFile(const string& filename) {
    ifstream fin(filename);
    if (!fin.is_open()) {
        cout << "警告：未找到知识点依赖图文件 " << filename << "，复习路径推荐功能将不可用。\n";
        return false;
    }

    g_knowledgePrereq.clear();
    g_allKnowledgeNodes.clear();

    string line;
    int lineNum = 0;
    while (getline(fin, line)) {
        ++lineNum;
        if (line.empty()) continue;

        // 按 '|' 分隔
        size_t pos = line.find('|');
        if (pos == string::npos) {
            cout << "知识图第 " << lineNum << " 行格式错误，已跳过。\n";
            continue;
        }

        string currentKnowledge = line.substr(0, pos);
        string prereqStr = line.substr(pos + 1);

        // 去除 currentKnowledge 前后空格
        size_t start = currentKnowledge.find_first_not_of(" \t\r\n");
        size_t end = currentKnowledge.find_last_not_of(" \t\r\n");
        if (start != string::npos && end != string::npos) {
            currentKnowledge = currentKnowledge.substr(start, end - start + 1);
        }

        if (currentKnowledge.empty()) continue;

        // 将当前知识点加入节点集合
        g_allKnowledgeNodes.insert(currentKnowledge);

        // 解析前置知识点列表（逗号分隔）
        vector<string> prereqs;
        stringstream ss(prereqStr);
        string prereq;
        while (getline(ss, prereq, ',')) {
            // 去除空格
            start = prereq.find_first_not_of(" \t\r\n");
            end = prereq.find_last_not_of(" \t\r\n");
            if (start != string::npos && end != string::npos) {
                prereq = prereq.substr(start, end - start + 1);
            }
            if (!prereq.empty()) {
                prereqs.push_back(prereq);
                g_allKnowledgeNodes.insert(prereq);
            }
        }

        g_knowledgePrereq[currentKnowledge] = prereqs;
    }

    cout << "知识点依赖图加载完成，共 " << g_allKnowledgeNodes.size() << " 个知识点。\n";
    return true;
}

double computeRecommendScore(const Question& q, const QuestionStat& st, long long now) {
    // 1. 错误率部分
    double errorRate = 1.0; // 没做过就当成 1（需要了解）
    if (st.totalAttempts > 0) {
        errorRate = (st.totalAttempts - st.correctAttempts) * 1.0 / st.totalAttempts;
    }

    // 2. 时间间隔部分：最近一次做题距现在多少天（越久没做权重越高）
    double timeGapDays = 1.0; // 默认 1 天
    if (st.lastTimestamp > 0) {
        double seconds = difftime(now, (time_t)st.lastTimestamp);
        if (seconds < 0) seconds = 0;
        timeGapDays = seconds / 86400.0;
    } else {
        // 从未做过，给它一个中等偏上的时间权重
        timeGapDays = 7.0;
    }
    // 把时间间隔映射到 0~1，超过 7 天按 1 算
    double timeScore = timeGapDays / 7.0;
    if (timeScore > 1.0) timeScore = 1.0;

    // 3. 难度权重，难度 1~5 映射到 0.2~1.0
    double diffScore = 0.2 + (q.difficulty - 1) * 0.2;
    if (diffScore < 0.2) diffScore = 0.2;
    if (diffScore > 1.0) diffScore = 1.0;

    // 4. 没做过的题稍微加一点 bonus
    double unseenBonus = (st.totalAttempts == 0 ? 0.2 : 0.0);

    // 综合评分：可以给出一套权重，你可以在报告里写
    double score = 0.6 * errorRate + 0.3 * timeScore + 0.1 * diffScore + unseenBonus;

    // 限制到 [0, 2] 之间（理论上不会超）
    if (score < 0.0) score = 0.0;
    if (score > 2.0) score = 2.0;

    return score;
}


void appendRecordToFile(const Record& r, const string& filename = "data/records.csv") {
    ofstream fout(filename, ios::app);
    if (!fout.is_open()) {
        cout << "警告：无法写入做题记录文件。\n";
        return;
    }
    fout << r.questionId << ','
         << (r.correct ? 1 : 0) << ','
         << r.usedSeconds << ','
         << r.timestamp << '\n';
}

void doQuestion(const Question& q) {
    using namespace std::chrono;

    cout << "题号: " << q.id << endl;
    cout << "[知识点] " << q.knowledge << "    [难度] " << q.difficulty << endl;
    cout << q.text << endl;
    for (size_t i = 0; i < q.options.size(); ++i) {
        cout << i << ". " << q.options[i] << endl;
    }

    cout << "请输入你的答案编号：";

    auto start = steady_clock::now();

    int userAns;
    cin >> userAns;

    auto end = steady_clock::now();
    int usedSeconds = (int)duration_cast<seconds>(end - start).count();
    if (usedSeconds <= 0) usedSeconds = 1;

    bool correct = (userAns == q.answer);
    if (correct) {
        cout << "回答正确！\n";
    } else {
        cout << "回答错误，正确答案是：" << q.answer
             << "，" << q.options[q.answer] << endl;
    }

    // 构建记录
    Record r;
    r.questionId = q.id;
    r.correct = correct;
    r.usedSeconds = usedSeconds;
    r.timestamp = (long long)time(nullptr);

    // 更新内存结构
    g_records.push_back(r);
    g_recordsByQuestion[q.id].push_back(r);

    // 更新错题集合：最后一次答对就从错题集中移除，答错就加入
    if (correct) {
        g_wrongQuestions.erase(q.id);
    } else {
        g_wrongQuestions.insert(q.id);
    }

    // 追加写入文件
    appendRecordToFile(r);
}


// 简单的随机刷题模式（后面会接入错题本、推荐系统）
void randomPractice() {
    if (g_questions.empty()) {
        cout << "题库为空，请先导入题库。\n";
        return;
    }

    srand((unsigned)time(nullptr));
    int idx = rand() % g_questions.size();
    const Question& q = g_questions[idx];

    doQuestion(q);
}

// 这里以后会加：错题本模式、AI 推荐模式
void wrongBookMode() {
    if (g_wrongQuestions.empty()) {
        cout << "当前没有错题，先去刷题或者做几道题再来吧。\n";
        return;
    }

    // 把错题号拷贝到 vector 里便于随机
    vector<int> ids;
    ids.reserve(g_wrongQuestions.size());
    for (int id : g_wrongQuestions) {
        ids.push_back(id);
    }

    srand((unsigned)time(nullptr));
    int idx = rand() % ids.size();
    int qid = ids[idx];

    auto it = g_questionById.find(qid);
    if (it == g_questionById.end()) {
        cout << "错误：找不到错题对应的题目内容。\n";
        return;
    }

    cout << "【错题本练习】\n";
    doQuestion(*it->second);
}


void aiRecommendMode() {
    if (g_questions.empty()) {
        cout << "题库为空，无法推荐。\n";
        return;
    }

    // 每次进入推荐模式时，确保统计是最新的（因为刚刚可能做了新题）
    buildQuestionStats();

    long long now = (long long)time(nullptr);

    priority_queue<RecommendItem> pq;

    // 对每道题计算推荐分数，丢进堆里
    for (const auto& q : g_questions) {
        QuestionStat st;
        auto it = g_questionStats.find(q.id);
        if (it != g_questionStats.end()) {
            st = it->second;
        }
        double s = computeRecommendScore(q, st, now);
        pq.push({q.id, s});
    }

    int K = 5; // 一次推荐 5 道，你可以改
    if ((int)g_questions.size() < K) {
        K = (int)g_questions.size();
    }

    cout << "【AI 智能推荐模式】本次为你推荐 " << K << " 道题。\n";
    cout << "根据你的历史做题记录，优先推荐错误率高、长期未练习或难度较高的题目。\n\n";

    vector<int> selected;
    selected.reserve(K);
    while (!pq.empty() && (int)selected.size() < K) {
        selected.push_back(pq.top().questionId);
        pq.pop();
    }

    // 逐题练习
    for (size_t i = 0; i < selected.size(); ++i) {
        int qid = selected[i];
        auto itQ = g_questionById.find(qid);
        if (itQ == g_questionById.end()) continue;

        cout << "-----------------------------\n";
        cout << "第 " << (i + 1) << " 道推荐题：\n";
        doQuestion(*itQ->second);
        cout << "\n";
    }

    cout << "本轮 AI 推荐练习结束。\n";
}

void showStatistics() {
    if (g_records.empty()) {
        cout << "当前还没有任何做题记录。\n";
        return;
    }

    // 总体统计
    int total = (int)g_records.size();
    int correct = 0;
    for (const auto& r : g_records) {
        if (r.correct) correct++;
    }
    double acc = correct * 1.0 / total * 100.0;

    cout << "===== 总体统计 =====\n";
    cout << "总作答题数： " << total << "\n";
    cout << "答对题数：   " << correct << "\n";
    cout << "总体正确率： " << acc << "%\n\n";

    // 知识点统计
    cout << "===== 按知识点统计 =====\n";

    struct KnowledgeStat {
        int total = 0;
        int correct = 0;
    };
    unordered_map<string, KnowledgeStat> ks;

    // 根据每条记录找到题目对应的知识点
    for (const auto& r : g_records) {
        auto itQ = g_questionById.find(r.questionId);
        if (itQ == g_questionById.end()) continue;
        const string& kd = itQ->second->knowledge;
        ks[kd].total++;
        if (r.correct) ks[kd].correct++;
    }

    for (const auto& p : ks) {
        const string& kd = p.first;
        const KnowledgeStat& s = p.second;
        double kacc = s.correct * 1.0 / s.total * 100.0;
        cout << "[知识点] " << kd
             << "  总题数: " << s.total
             << "  正确数: " << s.correct
             << "  正确率: " << kacc << "%\n";
    }

    cout << "\n当前错题数： " << g_wrongQuestions.size() << " 道。\n";
}

void examMode() {
    if (g_questions.empty()) {
        cout << "题库为空，无法进行考试。\n";
        return;
    }

    // 步骤 1：让用户输入考试题目数量
    cout << "========== 模拟考试模式 ==========\n";
    cout << "题库共有 " << g_questions.size() << " 道题。\n";
    cout << "请输入本次考试的题目数量：";

    int N;
    cin >> N;

    // 限制范围
    if (N < 1) N = 1;
    if (N > (int)g_questions.size()) N = (int)g_questions.size();

    cout << "本次考试共 " << N << " 道题。\n";

    // 步骤 2（可选）：简单提示时间限制
    cout << "提示：本次考试不设置强制时间限制，请按自己的节奏完成。\n";
    cout << "（系统会记录每道题的作答时间）\n\n";

    // 步骤 3：随机抽取 N 道互不重复的题目
    vector<int> indices;
    indices.reserve(g_questions.size());
    for (size_t i = 0; i < g_questions.size(); ++i) {
        indices.push_back(i);
    }

    // 使用 C++11 的 random_device 和 mt19937 进行洗牌
    random_device rd;
    mt19937 rng(rd());
    shuffle(indices.begin(), indices.end(), rng);

    // 取前 N 个
    vector<int> selectedIndices(indices.begin(), indices.begin() + N);

    // 步骤 4：记录考试开始前的记录数
    size_t prevSize = g_records.size();

    cout << "考试开始！\n";
    cout << "====================================\n\n";

    // 顺序出题
    for (int i = 0; i < N; ++i) {
        cout << "【第 " << (i + 1) << "/" << N << " 题】\n";
        const Question& q = g_questions[selectedIndices[i]];
        doQuestion(q);
        cout << "\n";
    }

    cout << "====================================\n";
    cout << "考试结束！正在统计成绩...\n\n";

    // 步骤 5：统计本次考试结果
    int totalExam = (int)(g_records.size() - prevSize);
    int correctExam = 0;

    for (size_t i = prevSize; i < g_records.size(); ++i) {
        if (g_records[i].correct) {
            correctExam++;
        }
    }

    double examAcc = 0.0;
    if (totalExam > 0) {
        examAcc = correctExam * 100.0 / totalExam;
    }

    // 步骤 6：按知识点统计本次考试表现
    struct KnowledgeStat {
        int total = 0;
        int correct = 0;
    };
    unordered_map<string, KnowledgeStat> examKnowledge;

    for (size_t i = prevSize; i < g_records.size(); ++i) {
        const Record& r = g_records[i];
        auto itQ = g_questionById.find(r.questionId);
        if (itQ == g_questionById.end()) continue;

        const string& kd = itQ->second->knowledge;
        examKnowledge[kd].total++;
        if (r.correct) {
            examKnowledge[kd].correct++;
        }
    }

    // 步骤 7：打印考试报告
    cout << "========== 考试成绩报告 ==========\n";
    cout << "本次考试题目数： " << totalExam << "\n";
    cout << "答对题数：       " << correctExam << "\n";
    cout << "答错题数：       " << (totalExam - correctExam) << "\n";
    cout << "本次正确率：     " << examAcc << "%\n\n";

    cout << "===== 按知识点统计 =====\n";
    for (const auto& p : examKnowledge) {
        const string& kd = p.first;
        const KnowledgeStat& s = p.second;
        double kacc = 0.0;
        if (s.total > 0) {
            kacc = s.correct * 100.0 / s.total;
        }
        cout << "[知识点] " << kd
             << "  题数: " << s.total
             << "  正确: " << s.correct
             << "  正确率: " << kacc << "%\n";
    }

    cout << "====================================\n";
    cout << "考试记录已保存，可在\"做题统计查看\"中查看历史数据。\n";
}

// 知识点统计辅助函数
struct KnowledgeStat {
    int total = 0;
    int correct = 0;
    double accuracy = 0.0;
};

unordered_map<string, KnowledgeStat> buildKnowledgeStats() {
    unordered_map<string, KnowledgeStat> ks;

    // 根据每条记录找到题目对应的知识点
    for (const auto& r : g_records) {
        auto itQ = g_questionById.find(r.questionId);
        if (itQ == g_questionById.end()) continue;
        const string& kd = itQ->second->knowledge;
        ks[kd].total++;
        if (r.correct) ks[kd].correct++;
    }

    // 计算正确率
    for (auto& p : ks) {
        if (p.second.total > 0) {
            p.second.accuracy = p.second.correct * 100.0 / p.second.total;
        }
    }

    return ks;
}

// DFS生成复习路径（拓扑顺序）
void dfsReviewPath(const string& node, unordered_set<string>& visited, vector<string>& path) {
    if (visited.count(node)) return;
    visited.insert(node);

    // 先访问所有前置知识点
    auto it = g_knowledgePrereq.find(node);
    if (it != g_knowledgePrereq.end()) {
        for (const string& prereq : it->second) {
            dfsReviewPath(prereq, visited, path);
        }
    }

    // 再将当前节点加入路径
    path.push_back(node);
}

void recommendReviewPath() {
    if (g_allKnowledgeNodes.empty()) {
        cout << "知识点依赖图未加载，无法提供复习路径推荐。\n";
        cout << "请确保 data/knowledge_graph.txt 文件存在。\n";
        return;
    }

    // 步骤 1：统计知识点掌握情况
    auto knowledgeStats = buildKnowledgeStats();

    cout << "========== 知识点复习路径推荐 ==========\n\n";
    cout << "===== 当前知识点掌握情况 =====\n";

    // 用于排序的临时结构
    struct KnowledgeItem {
        string name;
        int total;
        int correct;
        double accuracy;
        bool operator<(const KnowledgeItem& other) const {
            // 按正确率从低到高排序（掌握最弱的在前）
            return accuracy < other.accuracy;
        }
    };

    vector<KnowledgeItem> items;
    for (const string& kd : g_allKnowledgeNodes) {
        KnowledgeItem item;
        item.name = kd;
        if (knowledgeStats.count(kd)) {
            item.total = knowledgeStats[kd].total;
            item.correct = knowledgeStats[kd].correct;
            item.accuracy = knowledgeStats[kd].accuracy;
        } else {
            item.total = 0;
            item.correct = 0;
            item.accuracy = 0.0;
        }
        items.push_back(item);
    }

    sort(items.begin(), items.end());

    // 显示掌握情况
    for (size_t i = 0; i < items.size(); ++i) {
        cout << (i + 1) << ". [" << items[i].name << "]  "
             << "题数: " << items[i].total
             << "  正确: " << items[i].correct
             << "  正确率: " << items[i].accuracy << "%";
        if (items[i].total == 0) {
            cout << " (未练习)";
        }
        cout << "\n";
    }

    // 步骤 2：让用户选择目标知识点
    cout << "\n请输入要复习的知识点编号（1-" << items.size() << "）：";
    int choice;
    cin >> choice;

    if (choice < 1 || choice > (int)items.size()) {
        cout << "无效的选择。\n";
        return;
    }

    string targetKnowledge = items[choice - 1].name;

    // 步骤 3：生成复习路径
    unordered_set<string> visited;
    vector<string> path;
    dfsReviewPath(targetKnowledge, visited, path);

    // 步骤 4：打印推荐复习路径
    cout << "\n========== 推荐复习路径 ==========\n";
    cout << "目标知识点：" << targetKnowledge << "\n\n";

    if (path.empty()) {
        cout << "未找到复习路径。\n";
        return;
    }

    cout << "建议按以下顺序复习：\n\n";
    for (size_t i = 0; i < path.size(); ++i) {
        const string& kd = path[i];
        cout << (i + 1) << ". " << kd;

        // 显示该知识点的掌握情况
        if (knowledgeStats.count(kd)) {
            cout << "  [题数: " << knowledgeStats[kd].total
                 << ", 正确率: " << knowledgeStats[kd].accuracy << "%]";
            if (knowledgeStats[kd].accuracy < 60.0 && knowledgeStats[kd].total > 0) {
                cout << " ⚠ 薄弱环节";
            }
        } else {
            cout << "  [未练习] ⚠ 需要学习";
        }

        // 箭头指向下一个
        if (i < path.size() - 1) {
            cout << " →";
        }
        cout << "\n";
    }

    cout << "\n建议：先巩固前置知识点，再学习后续内容，效果更佳！\n";
    cout << "====================================\n";
}


void showMenu() {
    cout << "=============================\n";
    cout << " 数据结构智能刷题与错题推荐系统\n";
    cout << "=============================\n";
    cout << "1. 随机刷题\n";
    cout << "2. 错题本练习\n";
    cout << "3. AI 智能推荐练习\n";
    cout << "4. 做题统计查看\n";
    cout << "5. 模拟考试模式\n";
    cout << "6. 知识点复习路径推荐\n";
    cout << "0. 退出\n";
    cout << "请选择：";
}

int main() {
    #ifdef _WIN32
    SetConsoleOutputCP(65001);
    #endif

    // 从文件加载题库
    if (!loadQuestionsFromFile("data/questions.csv")) {
        cout << "题库加载失败，程序结束。\n";
        return 0;
    }

    // 从文件加载历史做题记录（如果没有文件会从空开始）
    loadRecordsFromFile("data/records.csv");

    // 加载知识点依赖图
    loadKnowledgeGraphFromFile("data/knowledge_graph.txt");

    while (true) {
        showMenu();
        int choice;
        if (!(cin >> choice)) {
            break;
        }
        if (choice == 0) {
            cout << "再见！\n";
            break;
        } else if (choice == 1) {
            randomPractice();
        } else if (choice == 2) {
            wrongBookMode();
        } else if (choice == 3) {
            aiRecommendMode();
        } else if (choice == 4) {
            showStatistics();
        } else if (choice == 5) {
            examMode();
        } else if (choice == 6) {
            recommendReviewPath();
        } else {
            cout << "无效选项，请重新输入。\n";
        }

    }

    return 0;
}

