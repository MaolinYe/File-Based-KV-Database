//
// Created by YEZI on 2024/5/20.
//

#ifndef APPLICATION_H
#define APPLICATION_H
#include<iostream>
#include"KVDBHandler.h"

class Application {
    std::string &Standardize(std::string &todo) {
        for (auto &it: todo)
            if (std::isupper(it))
                it = it + 'a' - 'A';
        return todo;
    }

public:
    void Run() {
        std::string todo, key, value;
        KVDBHandler kvdb_handler("KVDB.txt");
        while (std::cin >> todo) {
            todo = Standardize(todo);
            if (todo == "set") {
                std::cin >> key >> value;
                if (kvdb_handler.Set(key, value) == KVDB_OK)
                    std::cout << "Key: " << key << '\t' << "value: " << value << std::endl;
            } else if (todo == "get") {
                std::cin >> key;
                if (kvdb_handler.Get(key, value) == KVDB_OK)
                    std::cout << "The value of " << key << " is " << value << std::endl;
            } else if (todo == "del") {
                std::cin >> key;
                if (kvdb_handler.Del(key) == KVDB_OK)
                    std::cout << key << " is gone successfully." << std::endl;
            } else if (todo == "purge") {
                if (kvdb_handler.Purge() == KVDB_OK)
                    std::cout << "Purge is successful." << std::endl;
            } else if (todo == "clear") {
                std::cout << "DO you want to clear your database? yes/NO?" << std::endl;
                std::cin >> todo;
                if (todo == "yes" && kvdb_handler.Clear() == KVDB_OK)
                    std::cout << "Clear is successful." << std::endl;
            } else if (todo == "expire") {
                int time;
                std::cin >> key >> time;
                if (kvdb_handler.Expire(key, time) == KVDB_OK)
                    std::cout << "The value of " << key << " will die after " << time << " seconds." << std::endl;
            } else if (todo == "show") {
                kvdb_handler.show();
            } else if (todo == "exit")
                break;
            else std::cout << "Sorry, please input the right instruction." << std::endl;
        }
    }
};
#endif //APPLICATION_H
