#ifndef HRODEBERT_DB_QIC_DATABASE_H
#define HRODEBERT_DB_QIC_DATABASE_H
#include <string>
#include <iostream>
#include <fstream>
#include <utility>
#include <vector>

namespace qic {

    enum Result {
        QIC_FAILED, QIC_SUCCESS
    };

    enum dataType {
        Boolean, String, Integer, Double
    };

/*returns the version of the library*/
    std::string Version();

    class Value {
    public:
        Value(dataType ptype) : type(ptype) {};


        Value(dataType ptype, int value) : type(ptype) { this->set_int_value(value); };

        Value(dataType ptype, double value) : type(ptype) { this->set_double_value(value); };

        Value(dataType ptype, bool value) : type(ptype) { this->set_bool_value(value); };

        Value(dataType ptype, std::string value) : type(ptype) { this->set_string_value(value); };


        dataType getType() { return type; }

        void set_int_value(int value) { int_value = value; }

        void set_double_value(double value) { double_Value = value; }

        void set_bool_value(bool value) { boolean_val = value; }

        void set_string_value(std::string value) { string_value = std::move(value); }


        int get_int_value() { return int_value; }

        bool get_bool_value() { return boolean_val; }

        std::string get_string_value() { return string_value; }

        double get_double_value() { return double_Value; }

    private:
        dataType type;
        bool boolean_val;
        std::string string_value;
        int int_value;
        double double_Value;
    };

    class DataBase {
    public:
        Result open();

        Result close();

        explicit DataBase(std::string databaseName) : databasePosition(std::move(databaseName)) {}

        std::vector<std::string> getAllTables();

        Result createTable(std::string tableName, std::vector<dataType> data);

        Result dropTable(std::string table);

        Result flush();

        Result addValueToTable(std::string tableName, std::vector<Value> Values);

        Result eraseValuesFromTable(std::string table, std::vector<Value> value);

        Result eraseValuesFromTableWithLimit(std::string table, std::vector<Value> value, int limit);

        std::vector<std::vector<Value>> getAllValuesFromTable(std::string table);



    private:
        bool opened = false;
        std::fstream file;
        std::string databasePosition;
        std::vector<std::string> file_position;
    };
}
#endif //HRODEBERT_DB_QIC_DATABASE_H

