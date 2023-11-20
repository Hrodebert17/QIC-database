#include "Hrodebert_db.h"
#include <string>
#include <iostream>

const int version[3] = {0,5,3};

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

Hrodebert_db_result DataBase::addValueToTable(std::string tableName, std::vector<ValueKey> Values) {
    file.close();
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
                dataType type = Values.at(i).getType();
                if (type == Boolean && type == keys[i].getType()) {
                    file << "boolean:"<< Values.at(i).get_bool_value() << std::endl;
                    continue;
                }
                if (type == Integer && type == keys[i].getType()) {
                    file << "integer:"<< Values.at(i).get_int_value() << std::endl;
                    continue;
                }
                if (type == Double && type == keys[i].getType()) {

                    file << "Double-:"<< Values.at(i).get_double_value() << std::endl;
                    continue;
                }
                if (type == String && type == keys[i].getType()) {
                    file << "String-:{\n";
                    file << Values.at(i).get_string_value() << "\n}¦"<<std::endl;
                    continue;
                }
            }
            file << "}⨂" << std::endl;
            file.close();
            return HB_SUCCESS;
        }
    }

    return HB_FAILED;
}

Hrodebert_db_result DataBase::dropTable(std::string table) {
    // we open the file for input
    file.close();
    file.open(databasePosition,std::ios::in);
    std::string fileLine;
    std::string file_text;
    bool deleting_smt = false;
    while (std::getline(file,fileLine)) {
        if (fileLine == table + ":") {
            deleting_smt = true;
        }
        if (fileLine.ends_with(":" + table)) {
            deleting_smt = true;
        }
        if (fileLine == "}" || fileLine == "}⨂") {
            if (deleting_smt) {
                deleting_smt = false;
                continue;
            }
        }
        if (!deleting_smt) {
            file_text += fileLine;
            file_text += "\n";
        }

    }
    file.close();
    file.open(databasePosition,std::ios::trunc);
    file.close();
    file.open(databasePosition,std::ios::out);
    file << file_text;
    file.close();
    return HB_SUCCESS;
}


std::vector<std::vector<ValueKey>> DataBase::getAllValuesFromTable(std::string table) {
    file.close();
    file.open(databasePosition, std::ios::in);
    std::string line;
    std::vector<std::vector<ValueKey>> values;
    bool saving_value = false;
    while (std::getline(file,line)) {
        if (line.ends_with(":" + table)) {
            if (!saving_value) {
                saving_value = true;
                continue;
            }
        }
        if (saving_value) {
            bool savingAString = false;
            std::vector<ValueKey> vector;
            while (saving_value) {
                if (savingAString) {
                    std::string string_value;
                    while (savingAString) {
                        if (line != "}¦") {
                            string_value +=line;
                            std::cout << string_value << std::endl;
                            std::getline(file,line);
                        } else {
                            ValueKey valueKey(String,static_cast<std::string>(string_value));
                            vector.push_back(valueKey);
                            savingAString = false;

                        }
                    }
                }
                if (line == "{") {
                    std::getline(file,line);
                    continue;
                }
                if (line.starts_with("boolean:")) {
                    bool value = false;
                    if (line[8] == '1') {
                        value = true;
                    }
                    ValueKey valueKey(Boolean,value);
                    vector.push_back(valueKey);

                    std::getline(file,line);

                }
                if (line.starts_with("integer:")) {
                    int value;
                    std::string new_line;
                    new_line = line;
                    new_line.erase(0,8);
                    value = std::stoi(new_line);
                    ValueKey valueKey(Integer,value);
                    vector.push_back(valueKey);

                    std::getline(file,line);

                }
                if (line.starts_with("Double-:")) {
                    double value;
                    std::string new_line;
                    new_line = line;
                    new_line.erase(0,8);
                    value = std::stod(new_line);
                    ValueKey valueKey(Double,(double)value);
                    vector.push_back(valueKey);

                    std::getline(file,line);
                }
                if (line.starts_with("String-:{")) {
                    savingAString = true;


                }
                if (line == "}⨂") {
                    saving_value = false;
                    values.push_back(vector);
                    continue;
                }
                if (!std::getline(file,line)) {
                    break;
                }

            }
        }
    }


    file.close();
    return values;
}


