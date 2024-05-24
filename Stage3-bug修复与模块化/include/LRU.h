//
// Created by YEZI on 2024/5/20.
//

#ifndef LRU_H
#define LRU_H
#include<iostream>
#include<unordered_map>

struct LRU_Node {
    std::string key;
    std::string value;
    LRU_Node *prior = nullptr;
    LRU_Node *next = nullptr;
};

class LRU {
    std::unordered_map<std::string, LRU_Node *> cache;
    LRU_Node *head;
    LRU_Node *tail;
    const int volume;
    int size = 0;

    void moveToHead(LRU_Node *node) {
        if (node == nullptr)
            return;
        node->prior = head;
        node->next = head->next;
        head->next->prior = node;
        head->next = node;
    }

public:
    LRU(int volume = 10) : head(new LRU_Node()), tail(new LRU_Node()), volume(volume) {
        head->next = tail;
        tail->prior = head;
    }

    void Set(const std::string &key, const std::string &value) {
        LRU_Node *node = nullptr;
        if (cache.find(key) == cache.end()) {
            if (++size > volume) {
                size = volume;
                node = tail->prior;
                cache.erase(node->key);
                node->key = key;
                node->value = value;
            } else {
                node = new LRU_Node(key, value, nullptr, nullptr);
            }
            cache.emplace(key, node);
        } else {
            node = cache[key];
            node->value = value;
        }
        moveToHead(node);
    }

    bool Get(const std::string &key, std::string &value) {
        if (cache.find(key) == cache.end())
            return false;
        LRU_Node *node = cache[key];
        value = node->value;
        moveToHead(node);
        return true;
    }

    bool Del(const std::string &key) {
        if (cache.find(key) == cache.end())
            return false;
        LRU_Node *node = cache[key];
        cache.erase(key);
        node->prior->next = node->next;
        node->next->prior = node->prior;
        delete node;
        --size;
        return true;
    }
};
#endif //LRU_H
