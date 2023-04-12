#include "mysqldbconnectionpool.hpp"

void connection_pool::init(string Url, string User, string PassWord, string DBName, int Port, int MaxConn)
{
    m_Url = Url;
    m_Port = Port;
    m_User = User;
    m_PassWord = PassWord;
    m_DataBaseName = DBName;

    for (int i = 0; i < MaxConn; i++)
    {
        MYSQL *con = NULL;

        con = mysql_init(con);
        if (con == NULL)
        {
            LOG_INFO << "MYSQL Error";
            exit(1);
        }

        con = mysql_real_connect(con, Url.c_str(), User.c_str(), PassWord.c_str(), DBName.c_str(), Port, NULL, 0);
        if (con != nullptr)
        {
            // C和C++代码默认的编码字符时ASCII,如果不设置，从MySQL上拉下来的中文显示是？
            mysql_query(con, "set names gbk");
            LOG_INFO << "connect mysql success!";
        }
        else
        {
            LOG_INFO << "connect mysql fail!";
        }
        connList.push_back(con);
        ++m_FreeConn;
    }
    sem_init(&m_sem, 0, m_FreeConn);
    pthread_mutex_init(&m_mutex, NULL);
    m_MaxConn = m_FreeConn;
}

// 当有请求时，从数据库连接池中返回一个可用连接，更新使用和空闲连接数
MYSQL *connection_pool::GetConnection()
{
    MYSQL *con = NULL;
    if (connList.size() == 0)
    {
        return NULL;
    }
    sem_wait(&m_sem);

    pthread_mutex_lock(&m_mutex);
    con = connList.front();
    connList.pop_front();
    --m_FreeConn;
    ++m_CurConn;
    pthread_mutex_unlock(&m_mutex);

    return con;
}

bool connection_pool::ReleaseConnection(MYSQL *con)
{
    if (con == NULL)
    {
        return false;
    }

    pthread_mutex_lock(&m_mutex);
    connList.push_back(con);
    ++m_FreeConn;
    --m_CurConn;
    pthread_mutex_unlock(&m_mutex);

    sem_post(&m_sem);
    return true;
}

void connection_pool::DestroyPool()
{
    pthread_mutex_lock(&m_mutex);
    if (connList.size() > 0)
    {
        list<MYSQL *>::iterator it;
        for (it = connList.begin(); it != connList.end(); it++)
        {
            MYSQL *con = *it;
            mysql_close(con);
        }
        m_CurConn = 0;
        m_FreeConn = 0;
        connList.clear();
    }
    pthread_mutex_unlock(&m_mutex);
}