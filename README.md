# File-Based-KV-Database
一个基于Append-Only File的高并发集群KV数据库，支持GET查询、SET存储、DEL删除和PURGE整理操作以及异常处理，支持内存索引，支持超时类接口，支持LRU Cache
## stage1 基本功能实现
支持GET查询、SET存储、DEL删除和PURGE整理操作以及异常处理，使用C++调用POSIX接口实现基于文件存储的KV数据库的查询、存储，删除和整理功能，并对异常输入、重复删除、覆盖写、删除重写等功能进行了单元测试和bug处理
## stage2 内存索引与过期删除
支持内存索引，支持超时类接口，使用HashMap存储有效key和对于文件位置偏移量实现内存索引，重写基本操作的实现，使用Min-Heap记录key的过期时间，增加expire操作，设置key的生存周期，实现过期自动删除功能
## stage3 LUR缓存
支持LRU Cache，使用HashMap和双向链表实现LRU Cache
## stage3 bug修复与模块化
修复了已发现的bug，为了后期实现更多功能需要将现有代码框架重构划分模块
## stage4 高并发
原有版本是单机控制台读取输入版，同一时刻只能处理来着一个客户端的请求，现改用服务器-客户端模式，使用socket编程，用基于epoll实现的IO多路复用可以同时处理大量客户端的请求，只能在Linux系统上运行，因为Windows系统木有提供IO多路复用方案  
### 关于高并发读写数据不一致的问题
因为采用单进程IO多路复用方案，无需加锁，所有请求均单进程串行处理，不会出现数据错误的情况
### 关于客户端程序
目前没有单独实现客户端程序，在Linux系统上可以直接使用一下命令连接服务器并使用本KV数据库服务
```shell
telnet localhost 8888
```
## stage5 数据分片集群
使用数据分片集群解决高并发、海量数据存储问题，客户端请求访问集群任意节点，节点利用哈希一致性选出处理key的节点并正确转发
# how to use
指令大小写均可：set Set SET……均可识别
## 存储
set key value
## 删除
del key
## 查询
get key
## 整理
purge
## 设计生存时间
expire key time
