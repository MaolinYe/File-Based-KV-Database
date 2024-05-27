//
// Created by YEZI on 2024/5/20.
//

#ifndef APPLICATION_H
#define APPLICATION_H
#include<iostream>
#include"KVDBHandler.h"
#include<sstream>

class Application {
    KVDBHandler *kvdb_handler = nullptr;
    std::string todo, key, value;

public:
    Application() {
        kvdb_handler = new KVDBHandler("KVDB.txt");
    }

    static std::string &Standardize(std::string &todo) {
        for (auto &it: todo)
            if (std::isupper(it))
                it = it + 'a' - 'A';
        return todo;
    }

    // 从控制台读取输入
    void ConsoleRun() {
        // 提示信息
        KVDBHandler::Help();
        while (std::cin >> todo) {
            todo = Standardize(todo);
            if (todo == "set") {
                std::cin >> key >> value;
                if (kvdb_handler->Set(key, value) == KVDB_OK)
                    std::cout << "Key: " << key << '\t' << "value: " << value << std::endl;
            } else if (todo == "get") {
                std::cin >> key;
                if (kvdb_handler->Get(key, value) == KVDB_OK)
                    std::cout << "The value of " << key << " is " << value << std::endl;
            } else if (todo == "del") {
                std::cin >> key;
                if (kvdb_handler->Del(key) == KVDB_OK)
                    std::cout << key << " is gone successfully." << std::endl;
            } else if (todo == "purge") {
                if (kvdb_handler->Purge() == KVDB_OK)
                    std::cout << "Purge is successful." << std::endl;
            } else if (todo == "clear") {
                std::cout << "DO you want to clear your database? yes/NO?" << std::endl;
                std::cin >> todo;
                if (todo == "yes" && kvdb_handler->Clear() == KVDB_OK)
                    std::cout << "Clear is successful." << std::endl;
            } else if (todo == "expire") {
                int time;
                std::cin >> key >> time;
                if (kvdb_handler->Expire(key, time) == KVDB_OK)
                    std::cout << "The value of " << key << " will die after " << time << " seconds." << std::endl;
            } else if (todo == "show") {
                kvdb_handler->show();
            } else if (todo == "exit")
                break;
            else std::cout << "Sorry, please input the right instruction." << std::endl;
        }
    }

    // 从字节流中解析指令读取输入
    std::string ParseInstruction(const char *buffer) {
        std::istringstream iss(buffer);
        std::ostringstream oss;
        while (iss >> todo) {
            todo = Standardize(todo);
            if (todo == "set") {
                iss >> key >> value;
                if (kvdb_handler->Set(key, value) == KVDB_OK)
                    oss << "Key: " << key << '\t' << "value: " << value;
            } else if (todo == "get") {
                iss >> key;
                if (kvdb_handler->Get(key, value) == KVDB_OK)
                    oss << "The value of " << key << " is " << value;
            } else if (todo == "del") {
                iss >> key;
                if (kvdb_handler->Del(key) == KVDB_OK)
                    oss << key << " is gone successfully.";
            } else if (todo == "purge") {
                if (kvdb_handler->Purge() == KVDB_OK)
                    oss << "Purge is successful.";
            } else if (todo == "clear") {
                if (kvdb_handler->Clear() == KVDB_OK)
                    oss << "Clear is successful.";
            } else if (todo == "expire") {
                int time;
                iss >> key >> time;
                if (kvdb_handler->Expire(key, time) == KVDB_OK)
                    oss << "The value of " << key << " will die after " << time << " seconds.";
            } else if (todo == "show") {
                kvdb_handler->show();
            } else if (todo == "exit")
                break;
            else oss << "Sorry, please input the right instruction.";
        }
        oss << std::endl;
        return oss.str();
    }
};
#endif //APPLICATION_H
