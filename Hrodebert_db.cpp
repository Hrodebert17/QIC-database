#include "Hrodebert_db.h"
#include <string>
#include <iostream>

const int version[3] = {0,3,0};

std::string hrodebert_db_version() {
    std::string version_str;
    version_str += std::to_string(version[0]);
    version_str += ".";
    version_str += std::to_string(version[1]);
    version_str += ".";
    version_str += std::to_string(version[2]);
    return version_str;
}

DataBase::DataBase(std::string databaseName) {
    databasePosition = databaseName;
}

Hrodebert_db_result DataBase::open() {
    file.open(databasePosition);
    if (file.is_open()) {
        return HB_SUCCESS;
    }
    return HB_FAILED;
}

Hrodebert_db_result DataBase::close() {
    if (file.is_open()) {
        file.close();
        return HB_SUCCESS;
    }
    return HB_FAILED;
}

Hrodebert_db_result DataBase::createTable(std::string tableName, std::vector<dataType> data) {
    file.open(databasePosition,std::ios::in);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if (line == tableName + ":") {
                return HB_FAILED;
            }
        }
        file.close();
        file.open(databasePosition,std::ios::app);
        file << "" << std::endl;
        file << tableName << ":" << std::endl;
        file << "{" << std::endl;
        for (int i = 0; i < data.size(); i++) {
            std::string value;
            if (data.at(i) == Boolean) {
                value = "bool";
            } else if (data.at(i) == String) {
                value = "string";
            } else if (data.at(i) == Integer) {
                value = "integer";
            } else if (data.at(i) == Double) {
                value = "double";
            }
            file << value << std::endl;
        }
        file << "}" << std::endl;
        file.close();
        return HB_SUCCESS;
    }
    return HB_FAILED;
}

Hrodebert_db_result DataBase::addValueToTable(std::string tableName, std::vector<ValueKey *> Values) {
    file.open(databasePosition,std::ios::in);
    std::string line;
    while (std::getline(file, line)) {
        if (line == tableName + ":") {
            std::string NewLine;
            std::vector<ValueKey> keys;
            while (std::getline(file, NewLine)) {
                if (NewLine[0] != '}') {
                    if (NewLine[0] == '{') {
                        continue;
                    }
                    if (NewLine == "bool") {
                        keys.push_back(ValueKey(Boolean));
                    } else if (NewLine == "string") {
                        keys.push_back(ValueKey(String));
                    } else if (NewLine == "integer") {
                        keys.push_back(ValueKey(Integer));
                    } else if (NewLine == "double") {
                        keys.push_back(ValueKey(Double));
                    }

                } else {
                    break;
                }
            }
            file.close();
            file.open(databasePosition, std::ios::app | std::ios::ate);
            file << "from-table:" << tableName << std::endl;
            file << "{" <<std::endl;
            for (int i = 0; i < Values.size(); i++) {
                dataType type = Values.at(i)->getType();
                if (type == Boolean && type == keys[i].getType()) {
                    BoolValueKey* key = dynamic_cast<BoolValueKey*>(Values.at(i));
                    if (key != nullptr) {
                        file << "boolean:"<< key->value << std::endl;
                        continue;
                    }
                }
                if (type == Integer && type == keys[i].getType()) {
                    IntValueKey* key= dynamic_cast<IntValueKey*>(Values.at(i));
                    if (key != nullptr) {
                        file << "integer:"<< key->value << std::endl;
                        continue;
                    }
                }
                if (type == Double && type == keys[i].getType()) {
                    DoubleValueKey* key= dynamic_cast<DoubleValueKey*>(Values.at(i));
                    if (key != nullptr) {
                        file << "Double-:"<< key->value << std::endl;
                        continue;
                    }
                }
                if (type == String && type == keys[i].getType()) {
                    StringValueKey* key= dynamic_cast<StringValueKey*>(Values.at(i));
                    if (key != nullptr) {
                        file << "String-:"<< key->value << std::endl;
                        continue;
                    }
                }
            }
            file << "}⨂" << std::endl;
            file.close();
        }
    }

    return HB_FAILED;
}



ValueKey::ValueKey(dataType ptype) {
    type = ptype;
}

BoolValueKey::BoolValueKey(dataType ptype, bool avalue) : ValueKey(ptype) {
    value = avalue;
}

StringValueKey::StringValueKey(dataType ptype1, std::string avalue) : ValueKey(ptype1) {
    value = avalue;
}

IntValueKey::IntValueKey(dataType ptype, int avalue) : ValueKey(ptype) {
    value = avalue;
}

DoubleValueKey::DoubleValueKey(dataType ptype, double avalue) : ValueKey(ptype) {
    value = avalue;
}
