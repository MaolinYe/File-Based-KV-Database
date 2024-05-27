//
// Created by YEZI on 2024/5/26.
//

#ifndef HASHRING_H
#define HASHRING_H
#include <algorithm>
#include<unordered_map>
#include<string>
#include<functional>
#include<vector>

// 数据分片集群所需的哈希节点环
class HashRing {
private:
    std::unordered_map<size_t, size_t> ring;
    // 哈希函数
    std::hash<std::string> hash;
    std::vector<size_t> ports;

    HashRing() {
        ports.emplace_back(6666);
        ports.emplace_back(7777);
        ports.emplace_back(8888);
        for (const auto &port: ports) {
            ring.insert({port, port});
        }
    }

public:
    // 禁用拷贝构造函数和赋值运算符
    HashRing(const HashRing &) = delete;

    HashRing &operator=(const HashRing &) = delete;

    static HashRing &getInstance() {
        static HashRing hash_ring;
        return hash_ring;
    }

    bool isPortValid(const size_t port) {
        const auto it = ring.find(port);
        if (it == ring.end())
            return false;
        return true;
    }

    std::vector<size_t> &getServerPorts() {
        return ports;
    }

    size_t getRightServer(const std::string &key) {
        // 使用哈希函数计算key的哈希值
        const size_t hashValue = hash(key);
        // 查找最接近哈希值的节点
        auto closeNode = ring.begin();
        for (auto &&it = ring.begin(); it != ring.end(); ++it) {
            if (std::abs(int(hashValue - it->first)) < std::abs(int(closeNode->first - hashValue)))
                closeNode = it;
        }
        return closeNode->second;
    }
};

#define GetHashRingServer(key) HashRing::getInstance().getRightServer(key)
#define IsPortValid(port) HashRing::getInstance().isPortValid(port)
#define ServerPorts HashRing::getInstance().getServerPorts()
#endif //HASHRING_H
