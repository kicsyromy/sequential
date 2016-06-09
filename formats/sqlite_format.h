#ifndef SQLITE_FORMAT_H
#define SQLITE_FORMAT_H

#include <utility>
#include <string>
#include <list>
#include <map>
#include <sqlite3.h>

#include <fmt/format.h>

class SQLiteFormat
{
public:
    SQLiteFormat(const std::string &path, std::string &&table) :
        dbHandle_(nullptr),
        table_(table),
        columns_(),
        query_(),
        error_(nullptr)
    {
        sqlite3_open(path.c_str(), &dbHandle_);
    }

    ~SQLiteFormat()
    {
        sqlite3_close(dbHandle_);
    }

public:
    void write(const std::pair<const char *, int> &attribute)
    {
        if (!columns_.empty())
            columns_.append(", ");
        columns_.append(fmt::format("{} INTEGER", attribute.first));

        if (!query_.empty())
            query_.append(", ");
        query_.append(fmt::format("{}", attribute.second));
    }

    void write(const std::pair<const char *, double> &attribute)
    {
        if (!columns_.empty())
            columns_.append(", ");
        columns_.append(fmt::format("{} REAL", attribute.first));

        if (!query_.empty())
            query_.append(", ");
        query_.append(fmt::format("{}", attribute.second));
    }

    void write(const std::pair<const char *, std::string> &attribute)
    {
        if (!columns_.empty())
            columns_.append(", ");
        columns_.append(fmt::format("{} TEXT", attribute.first));

        if (!query_.empty())
            query_.append(", ");
        query_.append(fmt::format("'{}'", attribute.second));
    }

    void write(const std::pair<const char *, const char *> &attribute)
    {
        if (!columns_.empty())
            columns_.append(", ");
        columns_.append(fmt::format("{} GLOB", attribute.first));

        if (!query_.empty())
            query_.append(", ");
        query_.append(fmt::format("'{}'", attribute.second));
    }

    void write(const std::pair<const char *, bool> &attribute)
    {
        if (!columns_.empty())
            columns_.append(", ");
        columns_.append(fmt::format("{} INTEGER", attribute.first));

        if (!query_.empty())
            query_.append(", ");
        query_.append(fmt::format("{}", attribute.second ? 1 : 0));
    }

    template<typename ValueType>
    const ValueType get(const char *key, const ValueType * = nullptr) const
    {
        ValueType value;

        if (!tableData_.empty())
        {
            auto column = *(tableData_.begin());
            value = magic_cast(column.at(key), static_cast<ValueType *>(nullptr));
        }

        return value;
    }

    void populate(std::string *error = nullptr)
    {
        if (sqlite3_exec(dbHandle_,
                         fmt::format("SELECT * FROM {};", table_).c_str(),
                         &selectCallback, this, &error_) != SQLITE_OK)
        {
            if (error)
                *error = error_;
            sqlite3_free(error_);
            return;
        }
    }

    void flush(std::string *error = nullptr)
    {
        if (error_ != nullptr)
            sqlite3_free(error_);

        if (sqlite3_exec(dbHandle_,
                         fmt::format("CREATE TABLE IF NOT EXISTS {} ({});", table_, columns_).c_str(),
                         nullptr, nullptr, &error_) != SQLITE_OK)
        {
            if (error)
                *error = error_;
            sqlite3_free(error_);
            return;
        }

        if (sqlite3_exec(dbHandle_,
                         fmt::format("INSERT INTO {} VALUES({});", table_, query_).c_str(),
                         nullptr, nullptr, &error_) != SQLITE_OK)
        {
            if (error)
                *error = error_;
            sqlite3_free(error_);
        }
    }

    void removeFormatedRow()
    {
        tableData_.erase(tableData_.begin());
    }

private:
    static int selectCallback(void *sqliteFormat, int columnCount, char **value, char **columnName)
    {
        SQLiteFormat *self = static_cast<SQLiteFormat *>(sqliteFormat);
        std::map<std::string, std::string> column;

        for (int it = 0; it < columnCount; ++it)
        {
            column[columnName[it]] = value[it];
        }

        self->tableData_.push_back(std::move(column));

        return 0;
    }

    int magic_cast(const std::string &value, int * = nullptr) const
    {
        return std::stoi(value);
    }

    double magic_cast(const std::string &value, double * = nullptr) const
    {
        return std::stold(value);
    }

    std::string magic_cast(const std::string &value, std::string * = nullptr) const
    {
        return value;
    }

    const char *magic_cast(const std::string &value, const char * = nullptr) const
    {
        /* I have no idea how do deserialize GLOB, */
        /* just make it faulty and stupid          */
        return value.c_str();
    }

    bool magic_cast(const std::string &value, bool * = nullptr) const
    {
        return std::stoi(value);
    }

private:
    sqlite3 *dbHandle_;
    const std::string table_;
    std::string columns_;
    std::string query_;
    char *error_;
    std::list<std::map<std::string, std::string>> tableData_;
};

#endif // SQLITE_FORMAT_H
