#pragma once

#include <mysql/mysql.h>

extern MYSQL *db;

extern void mysql_connect();

class hStmt
{
public:
    hStmt(const std::string &query);
   ~hStmt();

    hStmt           (const hStmt &) = delete;
    hStmt &operator=(const hStmt &) = delete;

    void bind_and_execute(MYSQL_BIND *param);

    MYSQL_STMT *pStmt;
};

struct blob
{
    /* @note template vibes */
    void f32(float val) {
        int size = mData.size();
        mData.resize(size + sizeof(float));
        memcpy(mData.data() + size, &val, sizeof(float));
    }
    void i32(int val)   { 
        int size = mData.size();
        mData.resize(size + sizeof(int));
        memcpy(mData.data() + size, &val, sizeof(int));
    }
    void u32(u_int val)   { 
        int size = mData.size();
        mData.resize(size + sizeof(u_int));
        memcpy(mData.data() + size, &val, sizeof(u_int));
    }
    void i16(short val) {
        int size = mData.size();
        mData.resize(size + sizeof(short));
        memcpy(mData.data() + size, &val, sizeof(short));
    }
    void u8(u_char val) {
        mData.push_back(val);
    }
    void push_back(const blob &blob) 
    { 
        const auto &data = blob.data();
        
        mData.insert(mData.end(), data.cbegin(), data.cend());
    }
    const std::vector<u_char> &data() const noexcept { 
        return mData; 
    }
    size_t size() const noexcept { 
        return mData.size(); 
    }

private:
    std::vector<u_char> mData;
};

extern MYSQL_BIND make_bind_in(const signed &buffer);
extern MYSQL_BIND make_bind_in(const unsigned &buffer);
extern MYSQL_BIND make_bind_in(const long &buffer);
extern MYSQL_BIND make_bind_in(const long long &buffer);
extern MYSQL_BIND make_bind_in(const float &buffer);
extern MYSQL_BIND make_bind_in(const std::string &buffer);
extern MYSQL_BIND make_bind_in(const std::vector<u_char> &buffer);

extern MYSQL_BIND make_bind_out(signed &buffer);
extern MYSQL_BIND make_bind_out(unsigned &buffer);
extern MYSQL_BIND make_bind_out(long &buffer);
extern MYSQL_BIND make_bind_out(long long &buffer);
extern MYSQL_BIND make_bind_out(float &buffer);
extern MYSQL_BIND make_bind_out(std::string &buffer);
extern MYSQL_BIND make_bind_out(std::vector<u_char> &buffer);
