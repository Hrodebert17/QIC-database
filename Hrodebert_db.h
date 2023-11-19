#ifndef HRODEBERT_DB_HRODEBERT_DB_H
#define HRODEBERT_DB_HRODEBERT_DB_H
#include <string>
#include <iostream>
#include <fstream>
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
    ValueKey(dataType ptype);
    virtual dataType getType() {return type;}
protected:
    dataType type;
};

class BoolValueKey : public ValueKey{
public:
    BoolValueKey(dataType ptype, bool avalue);
    bool value;
};

class StringValueKey : public ValueKey{
public:
    StringValueKey(dataType ptype1, std::string avalue);
    std::string value;
};

class IntValueKey : public ValueKey{
public:
    IntValueKey(dataType ptype, int avalue);
    int value;
};

class DoubleValueKey : public ValueKey{
public:
    DoubleValueKey(dataType ptype, double avalue);
    double value;
};

class DataBase {
public:
    DataBase(std::string databaseName);

    Hrodebert_db_result createTable(std::string tableName, std::vector<dataType> data );

    Hrodebert_db_result addValueToTable(std::string tableName, std::vector<ValueKey*> Values );

    Hrodebert_db_result dropTable(std::string table);

private:
    Hrodebert_db_result open();
    Hrodebert_db_result close();
    std::fstream file;
    std::string databasePosition;
};

#endif //HRODEBERT_DB_HRODEBERT_DB_H
