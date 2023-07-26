# File-Based-KV-Database
一个基于Append-Only File的KV数据库，支持GET查询、SET存储、DEL删除和PURGE整理操作以及异常处理，支持内存索引，支持超时类接口，支持LRU Cache
## stage1
支持GET查询、SET存储、DEL删除和PURGE整理操作以及异常处理，使用C++调用POSIX接口实现基于文件存储的KV数据库的查询、存储，删除和整理功能，并对异常输入、重复删除、覆盖写、删除重写等功能进行了单元测试和bug处理
## stage2
支持内存索引，支持超时类接口，使用HashMap存储有效key和对于文件位置偏移量实现内存索引，重写基本操作的实现，使用Min-Heap记录key的过期时间，增加expire操作，设置key的生存周期，实现过期自动删除功能
## stage3
支持LRU Cache，使用HashMap和双向链表实现LRU Cache
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
