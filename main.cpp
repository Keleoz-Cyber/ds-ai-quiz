/**
 * @file main.cpp
 * @brief 数据结构智能刷题系统 - 程序入口
 *
 * 【模块职责】
 * main 函数作为唯一入口，保持"入口极简"理念，仅负责：
 * 1. Windows 平台 UTF-8 编码设置（保证控制台中文显示）
 * 2. 启动自检（检查 questions.csv、knowledge_graph.txt 等必要文件）
 * 3. 资源加载（题库、用户登录、做题记录、知识图）
 * 4. 进入主菜单循环（由 App.cpp 的 runMenuLoop 接管）
 *
 * 【设计原则】
 * - 不含业务逻辑，所有功能由各模块（Question/Record/Stats/KnowledgeGraph/App）实现
 * - 失败时明确返回错误，不做过多容错
 *
 * 【依赖模块】
 * - Question.h/cpp：题库管理与加载
 * - Record.h/cpp：用户登录、做题记录管理
 * - KnowledgeGraph.h/cpp：知识点依赖图加载
 * - App.h/cpp：主菜单与各功能模式（刷题/推荐/统计等）
 * - Utils.h/cpp：路径工具（getDataDir）、清屏、暂停
 */

#include "Question.h"
#include "Record.h"
#include "Stats.h"
#include "KnowledgeGraph.h"
#include "App.h"
#include "Utils.h"
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#include <iostream>
#endif

/**
 * @brief 启动自检：检查必要资源文件是否存在
 *
 * 【功能】
 * 在主程序运行前验证必要的数据文件是否存在：
 * - data/questions.csv（必需）：缺失时无法运行
 * - data/knowledge_graph.txt（可选）：缺失时仅影响知识点路径推荐功能
 *
 * 【实现要点】
 * - 使用 std::filesystem::exists 检查文件
 * - 对于缺失文件输出详细的错误提示，包括：
 *   - 当前查找目录
 *   - 建议的目录结构
 *   - 如何从其他路径运行的说明
 *
 * @return true  所有必需文件存在
 * @return false 缺少必需文件（questions.csv）
 *
 * @note 知识图谱文件缺失不返回 false，仅输出警告
 * @complexity O(1) - 仅文件系统检查
 */
bool performStartupCheck() {
    std::filesystem::path dataDir = getDataDir();
    std::filesystem::path questionsFile = dataDir / "questions.csv";
    std::filesystem::path knowledgeGraphFile = dataDir / "knowledge_graph.txt";

    bool allOk = true;

    // 检查题库文件
    if (!std::filesystem::exists(questionsFile)) {
        std::cerr << "\n========================================\n";
        std::cerr << "错误：找不到题库文件！\n";
        std::cerr << "========================================\n";
        std::cerr << "程序需要以下文件才能正常运行：\n";
        std::cerr << "  - data/questions.csv（题库文件）\n\n";
        std::cerr << "当前程序正在以下目录中查找：\n";
        std::cerr << "  " << dataDir.string() << "\n\n";
        std::cerr << "建议解决方案：\n";
        std::cerr << "1. 确保您使用的是完整的发行版压缩包\n";
        std::cerr << "2. 确保 DS_AI_Quiz.exe 与 data/ 文件夹在同一目录\n";
        std::cerr << "3. 目录结构应该是：\n";
        std::cerr << "   DS_AI_Quiz/\n";
        std::cerr << "   ├── DS_AI_Quiz.exe\n";
        std::cerr << "   └── data/\n";
        std::cerr << "       ├── questions.csv\n";
        std::cerr << "       └── knowledge_graph.txt\n\n";
        std::cerr << "4. 如果从其他路径运行，请先 cd 到程序所在目录：\n";
        std::cerr << "   cd <程序所在目录>\n";
        std::cerr << "   然后再运行：.\\DS_AI_Quiz.exe\n";
        std::cerr << "========================================\n\n";
        allOk = false;
    }

    // 检查知识图谱文件
    if (!std::filesystem::exists(knowledgeGraphFile)) {
        std::cerr << "\n========================================\n";
        std::cerr << "警告：找不到知识图谱文件！\n";
        std::cerr << "========================================\n";
        std::cerr << "缺少文件：data/knowledge_graph.txt\n\n";
        std::cerr << "当前程序正在以下目录中查找：\n";
        std::cerr << "  " << dataDir.string() << "\n\n";
        std::cerr << "影响：知识点复习路径推荐功能将不可用。\n";
        std::cerr << "其他功能（刷题、错题本、统计等）不受影响。\n";
        std::cerr << "========================================\n\n";
        // 知识图谱文件缺失不是致命错误，允许继续运行
    }

    return allOk;
}

/**
 * @brief 主函数：程序唯一入口
 *
 * 【执行流程】
 * 1. Windows 平台设置控制台代码页为 UTF-8 (CP 65001)
 *    - 作用：保证中文字符在 Windows 控制台正确显示
 *    - 原因：Windows 默认使用 GBK/本地代码页，不设置会出现乱码
 * 2. 执行启动自检 performStartupCheck()
 *    - 检查 data/questions.csv（必需）
 *    - 检查 data/knowledge_graph.txt（可选）
 *    - 若必需文件缺失，输出错误信息并退出
 * 3. 加载题库：loadQuestionsFromFile("data/questions.csv")
 *    - 解析 CSV 并填充全局容器 g_questions、g_questionById
 * 4. 用户登录：输入学号/用户名
 *    - 调用 loginUser(userId) 设置 g_currentUserId
 *    - 多用户隔离：不同用户的做题记录存于不同文件
 * 5. 加载用户做题记录：loadRecordsFromFile("data/records_<userId>.csv")
 *    - 重建 g_records、g_recordsByQuestion、g_wrongQuestions
 * 6. 加载知识点依赖图：loadKnowledgeGraphFromFile("data/knowledge_graph.txt")
 *    - 若文件不存在，仅输出警告，不影响其他功能
 * 7. 进入主菜单循环：runMenuLoop()
 *    - 由 App.cpp 接管用户交互
 *
 * @return 0  正常退出
 * @return 1  启动自检失败或题库加载失败
 */
int main() {
    // ========== 1. Windows UTF-8 设置 ==========
    // Windows 平台：设置控制台输出代码页为 65001 (UTF-8)
    // 原因：Windows 默认使用本地代码页（如 GBK），不设置会导致中文乱码
    // 影响范围：std::cout、std::cerr 输出的所有中文字符
    #ifdef _WIN32
    SetConsoleOutputCP(65001);
    #endif

    // ========== 2. 启动自检 ==========
    // 检查 data/questions.csv 和 data/knowledge_graph.txt
    // 若必需文件不存在，输出详细错误信息并退出
    if (!performStartupCheck()) {
        std::cerr << "程序因缺少必要文件而无法启动。\n";
        std::cerr << "按回车键退出...\n";
        std::cin.get();
        return 1;
    }

    // ========== 3. 加载题库 ==========
    // 从 data/questions.csv 加载所有题目
    // 填充全局容器：g_questions（vector）、g_questionById（unordered_map）
    std::filesystem::path questionsPath = getDataDir() / "questions.csv";
    if (!loadQuestionsFromFile(questionsPath.string())) {
        return 1;
    }

    // ========== 4. 用户登录 ==========
    // 输入学号/用户名，设置 g_currentUserId
    // 多用户隔离：不同用户的做题记录存于 data/records_<userId>.csv
    std::cout << "=============================\n";
    std::cout << " 数据结构智能刷题系统\n";
    std::cout << "=============================\n";
    std::cout << "请输入学号或用户名：";

    std::string userId;
    while (true) {
        std::getline(std::cin, userId);
        // 去除前后空格（包括制表符、换行符）
        userId.erase(0, userId.find_first_not_of(" \t\n\r"));
        userId.erase(userId.find_last_not_of(" \t\n\r") + 1);

        if (!userId.empty()) {
            break;
        }
        std::cout << "用户标识不能为空，请重新输入：";
    }

    loginUser(userId);
    std::cout << "\n";

    // ========== 5. 加载用户做题记录 ==========
    // 从 data/records_<userId>.csv 加载历史做题记录
    // 重建索引：g_records、g_recordsByQuestion、g_wrongQuestions
    // 若文件不存在（首次使用），不报错，从空记录开始
    loadRecordsFromFile(getRecordFilePath());

    // ========== 6. 加载知识点依赖图 ==========
    // 从 data/knowledge_graph.txt 加载知识点依赖关系
    // 若文件不存在，仅输出警告，不影响其他功能（刷题/统计/推荐等仍可用）
    std::filesystem::path knowledgeGraphPath = getDataDir() / "knowledge_graph.txt";
    loadKnowledgeGraphFromFile(knowledgeGraphPath.string());

    // ========== 7. 进入主菜单循环 ==========
    // 交由 App.cpp 的 runMenuLoop() 接管用户交互
    // 菜单包括：随机刷题、错题本、AI 推荐、统计、考试、知识点路径、导出报告、切换用户
    runMenuLoop();

    return 0;
}
