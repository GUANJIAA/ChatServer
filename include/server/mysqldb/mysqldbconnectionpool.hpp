#ifndef MYSQLDBCONNECTIONPOOL_HPP
#define MYSQLDBCONNECTIONPOOL_HPP

#include <stdio.h>
#include <list>
#include <mysql/mysql.h>
#include <error.h>
#include <string.h>
#include <string>
#include <muduo/base/Logging.h>
#include <semaphore.h>

#include "mysqldb.hpp"

using namespace std;

class connection_pool
{
public:
    // 获取数据库连接
    MYSQL *GetConnection();
    // 释放当前使用的连接
    bool ReleaseConnection(MYSQL *conn);
    // 销毁所有连接
    void DestroyPool();

    // 饿汉单例模式
    static connection_pool *GetInstance()
    {
        static connection_pool connPool;
        return &connPool;
    }

    // 构造初始化
    void init(string url, string User, string PassWord,
              string DataBaseName, int Port, int MaxConn);

public:
    string m_Url;          // 主机地址
    string m_Port;         // 数据库端口号
    string m_User;         // 登录数据库用户名
    string m_PassWord;     // 登录数据库密码
    string m_DataBaseName; // 使用数据库名

private:
    connection_pool()
    {
        m_CurConn = 0;
        m_FreeConn = 0;
    }
    ~connection_pool()
    {
        DestroyPool();
    }

    int m_MaxConn;          // 最大连接数
    int m_CurConn;          // 当前已使用的连接数
    int m_FreeConn;         // 当前空闲的连接数
    list<MYSQL *> connList; // 连接池
    sem_t m_sem;
    pthread_mutex_t m_mutex;
};

#endif