//
// Created by YEZI on 2024/5/20.
//

#ifndef KVDBHANDLER_H
#define KVDBHANDLER_H
#include"LRU.h"
#include<iostream>
#include <utility>
#include<vector>
#include<unordered_map>
#include<algorithm>
#include<unistd.h>
#include<sys/stat.h>
#include "fcntl.h"
#include<ctime>
const int KVDB_ERROR = -1;
const int KVDB_OK = 0;
const int KVDB_INVALID_AOF_PATH = 1;
const int KVDB_INVALID_KEY = 2;
const int KVDB_INVALID_VALUE = 3;
const int KVDB_NO_SPACE_LEFT_ON_DEVICES = 4;
const int KVDB_NO_FILE = 5;
const int KVDB_FILE_WRITE_ERROR = 6;
const int KVDB_IO_ERROR = 7;
const int INDEX_OK = 9;
const int INDEX_ERROR = 10;

struct Survival {
    long long time = 0;
    std::string key;

    Survival() = default;

    Survival(long long time, std::string key): time(time), key(std::move(key)) {
    }

    bool operator<(const Survival &a) const {
        return time > a.time;
    }
};


class KVDBHandler {
private:
    int database_address = -1;
    std::string database_path;
    std::unordered_map<std::string, size_t> index;
    std::vector<Survival> life;
    LRU LRUCache;

public:
    static void Help() {
        std::cout << std::endl << "----HELP----" << std::endl <<
                "set key value" << std::endl <<
                "get key" << std::endl <<
                "del key" << std::endl <<
                "expire key time" << std::endl <<
                "show" << std::endl <<
                "purge" << std::endl <<
                "clear" << std::endl <<
                "exit" << std::endl <<
                "------------" << std::endl;
    }

    explicit KVDBHandler(std::string database_path) : database_path(std::move(database_path)) {
        OpenKVDB();
        LoadIndex();
    }

    ~KVDBHandler() {
        CloseKVDB();
    }

    int LoadIndex() {
        index.clear();
        const off_t end = lseek(database_address, 0, SEEK_END);
        off_t cur = lseek(database_address, 0, SEEK_SET);
        size_t key_length = 0;
        int value_length = 0;
        std::string key;
        while (end != cur) {
            read(database_address, &key_length, sizeof(key_length));
            key.resize(key_length);
            read(database_address, (char *) key.data(), key_length);
            index[key] = lseek(database_address, 0, SEEK_CUR);
            read(database_address, &value_length, sizeof(value_length));
            if (value_length == -1)
                value_length = 0;
            cur = lseek(database_address, value_length, SEEK_CUR);
        }
        return INDEX_OK;
    }

    int OpenKVDB() {
        this->database_address = open(this->database_path.c_str(), O_CREAT | O_RDWR, S_IRWXU);
        if (0 > this->database_address) {
            printf("Failed to open DB file. [file='%s' res=%d]", this->database_path.c_str(), this->database_address);
            return KVDB_ERROR;
        }
        return KVDB_OK;
    }

    int CloseKVDB() {
        if (0 <= database_address) {
            close(database_address);
            return KVDB_OK;
        }
        return KVDB_ERROR;
    }

    int Set(const std::string &key, const std::string &value) {
        for (int i = 0; i < life.size(); i++) {
            // refresh the life of the key
            if (life[i].key == key)
                life.erase(life.begin() + i);
        }
        LRUCache.Set(key, value); // load LRUCache
        size_t key_length = key.length(); // write in the file
        int value_length = value.length();
        lseek(database_address, 0,SEEK_END); // 移到文件末尾写入
        write(database_address, &key_length, sizeof(key_length));
        write(database_address, (char *) key.c_str(), key.length());
        index[key] = lseek(database_address, 0, SEEK_CUR);
        write(database_address, &value_length, sizeof(value_length));
        write(database_address, (char *) value.c_str(), value.length());
        return KVDB_OK;
    }

    int Del(const std::string &key) {
        if (index.find(key) == index.end()) {
            // check index
            printf("Sorry, No Found!\n");
            return KVDB_IO_ERROR;
        }
        index.erase(key); // refresh index
        for (int i = 0; i < life.size(); i++) {
            //refresh the key_life
            if (life[i].key == key) {
                life.erase(life.begin() + i);
            }
        }
        LRUCache.Del(key); // refresh LRUCache
        size_t key_length = 0; // write in the file
        int value_length = 0;
        key_length = key.length();
        value_length = -1;
        lseek(database_address, 0,SEEK_END); // 移到文件末尾写入
        write(database_address, &key_length, sizeof(key_length));
        write(database_address, (char *) key.c_str(), key.length());
        write(database_address, &value_length, sizeof(value_length));
        return KVDB_OK;
    }

    int Get(const std::string &key, std::string &value) {
        if (index.find(key) == index.end()) {
            // check index
            printf("Sorry, No Found!\n");
            return KVDB_IO_ERROR;
        }
        time_t current_time;
        time(&current_time);
        while (!life.empty() && life[0].time < current_time) {
            // check key_life
            Del(life[0].key);
        }
        if (index.find(key) == index.end()) {
            // check index after clear the dying
            printf("Sorry, what you want to find has been deleted!\n");
            return KVDB_IO_ERROR;
        }
        if (LRUCache.Get(key, value)) // search in the LRUCache
            return KVDB_OK;
        lseek(database_address, index[key], SEEK_SET);
        int value_length = 0;
        read(database_address, &value_length, sizeof(value_length));
        value.resize(value_length);
        read(database_address, (char *) value.data(), value_length);
        return KVDB_OK;
    }

    int Clear() {
        CloseKVDB();
        if (unlink(database_path.c_str()) < 0) {
            perror(database_path.c_str());
            return KVDB_ERROR;
        }
        return KVDB_OK;
    }

    int Purge();

    int Expire(const std::string &key, const int &n) {
        if (index.find(key) == index.end()) {
            printf("Sorry, No Found!\n");
            return KVDB_IO_ERROR;
        }
        time_t seconds;
        time(&seconds);
        seconds += n;
        Survival key_life = {seconds, key};
        for (int i = 0; i < life.size(); i++) {
            if (life[i].key == key) {
                life[i] = key_life;
                std::sort(life.begin(), life.end());
                return KVDB_OK;
            }
        }
        life.push_back(key_life);
        std::sort(life.begin(), life.end());
        return KVDB_OK;
    }

    void show() {
        for (auto &[key, offset]: index) {
            std::string value;
            Get(key, value);
            std::cout << key << '\t' << value << std::endl;
        }
    }
};

int KVDBHandler::Purge() {
    std::string kvdb_file_temp = "KVDB_TEMP.txt";
    KVDBHandler kvdb_handler(kvdb_file_temp.c_str());
    for (auto &[key, offset]: index) {
        std::string value;
        Get(key, value);
        kvdb_handler.Set(key, value);
    }
    Clear();
    kvdb_handler.CloseKVDB();
    rename(kvdb_file_temp.c_str(), database_path.c_str());
    CloseKVDB();
    return KVDB_OK;
}
#endif //KVDBHANDLER_H
