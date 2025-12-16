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

// 启动自检：检查必要资源文件是否存在
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

int main() {
    #ifdef _WIN32
    SetConsoleOutputCP(65001);
    #endif

    // 启动自检
    if (!performStartupCheck()) {
        std::cerr << "程序因缺少必要文件而无法启动。\n";
        std::cerr << "按回车键退出...\n";
        std::cin.get();
        return 1;
    }

    // 从文件加载题库
    std::filesystem::path questionsPath = getDataDir() / "questions.csv";
    if (!loadQuestionsFromFile(questionsPath.string())) {
        return 1;
    }

    // 用户登录：输入学号或用户名
    std::cout << "=============================\n";
    std::cout << " 数据结构智能刷题系统\n";
    std::cout << "=============================\n";
    std::cout << "请输入学号或用户名：";

    std::string userId;
    while (true) {
        std::getline(std::cin, userId);
        // 去除前后空格
        userId.erase(0, userId.find_first_not_of(" \t\n\r"));
        userId.erase(userId.find_last_not_of(" \t\n\r") + 1);

        if (!userId.empty()) {
            break;
        }
        std::cout << "用户标识不能为空，请重新输入：";
    }

    loginUser(userId);
    std::cout << "\n";

    // 从文件加载当前用户的历史做题记录
    loadRecordsFromFile(getRecordFilePath());

    // 加载知识点依赖图
    std::filesystem::path knowledgeGraphPath = getDataDir() / "knowledge_graph.txt";
    loadKnowledgeGraphFromFile(knowledgeGraphPath.string());

    // 运行主菜单循环
    runMenuLoop();

    return 0;
}
