/**
 * @file App.h
 * @brief 应用程序主界面与交互控制模块
 *
 * 本模块负责主菜单显示、用户交互与各功能模块的调度。
 *
 * @section design_clearscreen 清屏设计说明
 * 【重要】清屏统一放在菜单循环开头（runMenuLoop 中），而不是在各功能函数结束时清屏。
 * 这样设计的优点：
 * - 用户可以在功能执行完后看到完整输出结果
 * - 按回车返回菜单时才清屏，交互更自然
 * - 避免功能函数内部分散管理清屏逻辑
 *
 * @author Keleoz
 * @date 2025
 */

#pragma once

/**
 * @brief 显示主菜单
 *
 * 输出系统标题和所有可用功能选项：
 * - 1. 随机刷题：调用 randomPractice()
 * - 2. 错题本练习：调用 wrongBookMode()
 * - 3. AI 智能推荐练习：调用 aiRecommendMode()（Recommender 模块）
 * - 4. 做题统计查看：调用 showStatistics()（Stats 模块）
 * - 5. 模拟考试模式：调用 examMode()
 * - 6. 知识点复习路径推荐：调用 recommendReviewPath()（KnowledgeGraph 模块）
 * - 7. 导出学习报告：调用 exportLearningReport()（Report 模块）
 * - 8. 切换用户：调用 switchUser()
 * - 0. 退出程序
 *
 * @note 本函数只负责显示，不处理用户输入（输入处理在 runMenuLoop 中）
 */
void showMenu();

/**
 * @brief 随机刷题模式
 *
 * 流程：
 * 1. 检查题库是否为空（g_questions）
 * 2. 使用 rand() 随机选择一道题目
 * 3. 调用 doQuestion() 进行答题（Question 模块）
 * 4. 答题结束后调用 pauseForUser() 等待用户确认
 *
 * 交互特点：
 * - 每次只出一道题
 * - 答题后立即反馈正误
 * - 错题自动记入错题本（由 doQuestion 内部处理）
 *
 * @see doQuestion() 答题核心逻辑
 * @see pauseForUser() 暂停等待用户
 */
void randomPractice();

/**
 * @brief 错题本练习模式
 *
 * 流程：
 * 1. 检查错题集是否为空（g_wrongQuestions）
 * 2. 将错题 ID 从 unordered_set 复制到 vector 便于随机访问
 * 3. 使用 rand() 随机选择一道错题
 * 4. 通过 g_questionById 映射查找题目内容
 * 5. 调用 doQuestion() 进行答题
 * 6. 答题结束后调用 pauseForUser() 等待用户确认
 *
 * 交互特点：
 * - 只从历史错题中抽取
 * - 答对后会从错题本移除（由 doQuestion 内部处理）
 * - 答错则保持在错题本中
 *
 * @note 错题集使用 unordered_set 存储，需要转换为 vector 才能随机访问
 * @see doQuestion() 答题核心逻辑
 */
void wrongBookMode();

/**
 * @brief 模拟考试模式
 *
 * 流程：
 * 1. 提示题库总数，让用户输入考试题目数量 N
 * 2. 范围限制：N ∈ [1, 题库总数]
 * 3. 使用 shuffle + 截取策略抽取 N 道互不重复的题目：
 *    - 将所有题目索引放入 vector
 *    - 使用 std::shuffle 和 mt19937 随机打乱
 *    - 取前 N 个索引作为考试题目
 * 4. 记录考试开始前的做题记录数（prevSize）
 * 5. 顺序出题，每道题调用 doQuestion()
 * 6. 考试结束后统计本次成绩：
 *    - 总题数、答对数、答错数、正确率
 *    - 按知识点分组统计（使用 unordered_map）
 * 7. 打印详细考试报告
 * 8. 调用 pauseForUser() 等待用户确认
 *
 * 抽题去重策略分析：
 * - 方法：Fisher-Yates shuffle + 截取前 N 个
 * - 时间复杂度：O(M)，M 为题库总数
 * - 空间复杂度：O(M)，需要存储全部索引
 * - 优点：实现简单，保证完全随机且无重复
 * - 适用场景：题库规模不超过万级别时性能良好
 *
 * 交互特点：
 * - 用户可自定义考试题目数量
 * - 不设置强制时间限制（可扩展）
 * - 系统记录每道题作答时间（由 doQuestion 内部处理）
 * - 考试结束后提供详细统计报告
 *
 * @note 本模式使用 std::random_device 和 std::mt19937 生成高质量随机数
 * @see doQuestion() 答题核心逻辑
 * @see g_records 全局做题记录
 */
void examMode();

/**
 * @brief 切换用户
 *
 * 流程：
 * 1. 提示用户输入新的学号或用户名
 * 2. 使用 std::getline 读取输入（支持空格）
 * 3. 去除前后空格，验证非空
 * 4. 调用 loginUser() 切换当前用户（Record 模块）
 * 5. 重新加载新用户的做题记录（loadRecordsFromFile）
 * 6. 输出切换成功提示
 * 7. 调用 pauseForUser() 等待用户确认
 *
 * 交互特点：
 * - 支持多用户独立数据管理
 * - 切换后自动加载新用户的历史记录
 * - 输入验证：不允许空用户标识
 *
 * @note 需要先调用 cin.ignore() 清除输入缓冲区残留的换行符
 * @see loginUser() 用户登录逻辑
 * @see loadRecordsFromFile() 加载做题记录
 * @see g_currentUserId 全局当前用户 ID
 */
void switchUser();

/**
 * @brief 主菜单循环
 *
 * 核心流程：
 * 1. 【清屏设计】循环开始时调用 clearScreen()，确保菜单显示清爽
 * 2. 调用 showMenu() 显示菜单选项
 * 3. 读取用户输入的选项
 * 4. 根据选项调度对应功能模块：
 *    - choice 0：退出程序
 *    - choice 1：randomPractice()（随机刷题）
 *    - choice 2：wrongBookMode()（错题本）
 *    - choice 3：aiRecommendMode()（AI 推荐，Recommender 模块）
 *    - choice 4：showStatistics()（统计查看，Stats 模块）
 *    - choice 5：examMode()（模拟考试）
 *    - choice 6：recommendReviewPath()（知识图谱，KnowledgeGraph 模块）
 *    - choice 7：exportLearningReport()（导出报告，Report 模块）
 *    - choice 8：switchUser()（切换用户）
 *    - 其他：提示无效选项，调用 pauseForUser() 等待用户确认
 * 5. 功能执行完毕后，用户按回车键返回，进入下一次循环（此时清屏）
 *
 * clearScreen 位置设计原则：
 * - ✅ 放在循环开头：用户看完上一功能的输出后，按回车才清屏显示菜单
 * - ❌ 放在循环结尾：会导致功能输出立即被清除，用户无法查看结果
 * - ❌ 放在各功能函数内：分散管理，逻辑不清晰，且用户体验差
 *
 * 异常处理：
 * - 如果 std::cin 读取失败（如输入非数字），跳出循环退出程序
 *
 * @note 本函数是整个应用的主控制器，负责协调所有模块
 * @see clearScreen() 清屏函数（Utils 模块）
 * @see showMenu() 显示菜单
 * @see pauseForUser() 暂停等待用户
 */
void runMenuLoop();
