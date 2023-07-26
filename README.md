# File-Based-KV-Database
一个基于Append-Only File的KV数据库
## stage1
支持GET查询、SET存储、DEL删除和PURGE整理操作以及异常处理
## stage2
支持内存索引，支持超时类接口
## stage3
支持LRU Cache
## how to use
指令大小写均可：set Set SET……均可识别
set key value
del key
get key
purge
expire key time
