//
// Created by YEZI on 2024/5/25.
//

#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

// 定义日志级别
enum class LogLevel { DEBUG, INFO, WARN, ERROR, FATAL };

class Logger {
private:
    int fd_ = -1;
    // 获取当前时间戳
    static std::string getCurrentTime() {
        // 获取当前时间点
        auto now = std::chrono::system_clock::now();
        // 将时间点转换为time_t
        auto now_time_t = std::chrono::system_clock::to_time_t(now);
        // 获取tm结构体
        std::tm time_info = *std::localtime(&now_time_t);
        // 构造时间字符串
        std::ostringstream oss;
        oss << std::put_time(&time_info, "%Y-%m-%d %H:%M:%S"); // 格式化时间
        // 获取微秒部分
        auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(
                                now.time_since_epoch()) % 1000000;
        oss << "." << std::setfill('0') << std::setw(6) << microseconds.count(); // 微秒部分
        return oss.str();
    }

    // 私有化构造函数
    explicit Logger(const char *filename) {
        // 打开日志文件
        fd_ = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0666);
        if (fd_ == -1) {
            std::cerr << "Failed to open log file\n";
        }
    }

public:
    ~Logger() {
        // 关闭日志文件
        if (fd_ != -1) {
            close(fd_);
        }
    }

    //初始化日志文件
    static void InitLogger(const char *filename) {
        getInstance(filename);
    }

    // 禁用拷贝构造函数和赋值运算符
    Logger(const Logger &) = delete;

    Logger &operator=(const Logger &) = delete;

    // 获取 Logger 实例的静态方法
    static Logger &getInstance(const char *filename = nullptr) {
        // 静态变量只会初始化一次
        static Logger instance(filename);
        return instance;
    }

    // 写日志函数
    void log(LogLevel level, const char *message, const char *file, int line, const char *function) const {
        std::string logLevelStr;
        switch (level) {
            case LogLevel::DEBUG:
                logLevelStr = "DEBUG";
                break;
            case LogLevel::INFO:
                logLevelStr = "INFO";
                break;
            case LogLevel::WARN:
                logLevelStr = "WARN";
                break;
            case LogLevel::ERROR:
                logLevelStr = "ERROR";
                break;
            case LogLevel::FATAL:
                logLevelStr = "FATAL";
                break;
        }

        // 构建日志消息
        std::string logMessage = getCurrentTime() + " [" + logLevelStr + "] " + message +
                                 " (File=" + file + " Function=" + function + " Line=" + std::to_string(line) + ")\n";

        // 写入日志到文件
        write(fd_, logMessage.c_str(), logMessage.size());
    }
};

// 宏定义简化日志调用
#define LOG(level, message) Logger::getInstance().log(level, message, __FILE__, __LINE__, __FUNCTION__)

#endif //LOGGER_H
