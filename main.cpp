#include "Question.h"
#include "Record.h"
#include "Stats.h"
#include "KnowledgeGraph.h"
#include "App.h"

#ifdef _WIN32
#include <windows.h>
#endif

int main() {
    #ifdef _WIN32
    SetConsoleOutputCP(65001);
    #endif

    // 从文件加载题库
    if (!loadQuestionsFromFile("data/questions.csv")) {
        return 0;
    }

    // 从文件加载历史做题记录
    loadRecordsFromFile("data/records.csv");

    // 加载知识点依赖图
    loadKnowledgeGraphFromFile("data/knowledge_graph.txt");

    // 运行主菜单循环
    runMenuLoop();

    return 0;
}
