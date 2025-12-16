#pragma once

#include <string>
#include <filesystem>

// 清屏函数
void clearScreen();

// 暂停等待用户按回车键继续
void pauseForUser(const std::string& message = "按回车键继续...");

// 路径工具函数
// 获取可执行文件所在目录
std::filesystem::path getExeDir();

// 获取数据目录（exeDir/data）
std::filesystem::path getDataDir();

// 获取报告目录（exeDir/reports）
std::filesystem::path getReportsDir();
