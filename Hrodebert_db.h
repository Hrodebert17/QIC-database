#ifndef HRODEBERT_DB_HRODEBERT_DB_H
#define HRODEBERT_DB_HRODEBERT_DB_H
#include <string>
#include <iostream>
#include <fstream>
#include <utility>
#include <vector>

enum Hrodebert_db_result {
    HB_FAILED, HB_SUCCESS
};

enum dataType {
    Boolean, String, Integer, Double
};

/*returns the version of the library*/
std::string hrodebert_db_version();

class ValueKey {
public:
    ValueKey(dataType ptype): type(ptype) {};


    ValueKey(dataType ptype, int value) : type(ptype) { this->set_int_value(value);};
    ValueKey(dataType ptype, double value) : type(ptype) { this->set_double_value(value);};
    ValueKey(dataType ptype, bool value) : type(ptype) { this->set_bool_value(value);};
    ValueKey(dataType ptype, std::string value) : type(ptype) {this->set_string_value(value);};


    dataType getType() {return type;}
    void set_int_value(int value) {int_value = value;}
    void set_double_value(double value) {double_Value = value;}
    void set_bool_value(bool value) {boolean_val = value;}
    void set_string_value(std::string value) {string_value = std::move(value);}


    int get_int_value() {return int_value;}
    bool get_bool_value() {return boolean_val;}
    std::string get_string_value() {return string_value;}
    double get_double_value() {return double_Value;}

private:
    dataType type;
    bool boolean_val;
    std::string string_value;
    int int_value;
    double double_Value;
};

class DataBase {
public:
    DataBase(std::string databaseName);

    Hrodebert_db_result createTable(std::string tableName, std::vector<dataType> data );

    Hrodebert_db_result addValueToTable(std::string tableName, std::vector<ValueKey> Values );

    Hrodebert_db_result eraseValuesFromTable(std::string table, std::vector<ValueKey> value);

    Hrodebert_db_result eraseValuesFromTableWithLimit(std::string table, std::vector<ValueKey> value, int limit);

    Hrodebert_db_result flush();

    Hrodebert_db_result dropTable(std::string table);

    std::vector<std::vector<ValueKey>> getAllValuesFromTable(std::string table);

private:
    Hrodebert_db_result open();
    Hrodebert_db_result close();
    std::fstream file;
    std::string databasePosition;
};

#endif //HRODEBERT_DB_HRODEBERT_DB_H

