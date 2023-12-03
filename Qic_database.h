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
        Result open(); // TODO: this function should open the database, scan it and separate each table in different files (qic extension) it should also put the path of those file in a vector to make the merge easier

        Result close(); //TODO: this function should merge all the changes into the database file and delete all the temporary files whit qic extension

        explicit DataBase(std::string databaseName) : databasePosition(std::move(databaseName)) {}

        //TODO convert all of the functions to work whit the new system
/*
        Result createTable(std::string tableName, std::vector<dataType> data); //not converted

        Result addValueToTable(std::string tableName, std::vector<Value> Values);//not converted

        Result dropTable(std::string table);//not converted

        Result eraseValuesFromTable(std::string table, std::vector<Value> value);//not converted

        Result eraseValuesFromTableWithLimit(std::string table, std::vector<Value> value, int limit);//not converted

        Result flush();//not converted

        std::vector<std::string> getAllTables();//not converted

        std::vector<std::vector<Value>> getAllValuesFromTable(std::string table);//not converted
*/

    private:
        bool opened = false;
        std::fstream file;
        std::string databasePosition;
        std::vector<std::string> file_position;
    };
}
#endif //HRODEBERT_DB_QIC_DATABASE_H

