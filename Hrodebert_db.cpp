#include "Hrodebert_db.h"
#include <string>
#include <iostream>

const int version[3] = {0,6,0};

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
                    std::cout << Values.at(i).get_int_value() << std::endl;
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

Hrodebert_db_result DataBase::eraseValuesFromTable(std::string table, std::vector<ValueKey> value) {
    // we close the file if its open
    if (file.is_open()) {
        file.close();
    }
    // we open the file
    file.open(databasePosition, std::ios::in);
    // we cheek if the file is open
    if (file.is_open()) {
        std::string line;
        std::string file_text;
        std::string string_value = "";
        std::string value_start = "from-table:" + table;

        int i;
        int f = 0;

        int current_line = 0;

        bool ignore_next_line = false;
        bool deleting = false;

        // while the file has lines
        while (std::getline(file,line)) {
            if (line != value_start && !deleting) {
                file_text += line;
                file_text += "\n";
                current_line ++;
                continue;
            }
            if (line == value_start) {
                if (!deleting ) {
                    if (!ignore_next_line) {
                        deleting = true;
                        string_value = "";
                        i = 0;
                        f = 1;
                    } else {
                        file_text += line;
                        file_text += "\n";
                        ignore_next_line = false;
                        current_line ++;
                    }
                }
            }

            if (deleting) {
                if (line == "{" || line == "}") {
                    f++;
                    continue;
                }
                if (line == "}⨂") {
                    f++;
                    deleting = false;
                    current_line += f;
                    continue;
                }
                if (line.starts_with("boolean:")) {
                    if (value.at(i).getType() == Boolean) {
                        int result = (value.at(i).get_bool_value()) ? 1 : 0;
                        if ( result + '0'  == (line.at(8))) {
                            i++;
                            f++;
                            continue;
                        }
                        std::cout << (int )(line.at(8)) <<std::endl;
                        deleting = false;
                    }
                }
                if (line.starts_with("String-:")) {
                    if (value.at(i).getType() == String) {
                        std::string str;
                        std::string last_string;
                        f++;
                        while (line != "}¦" && std::getline(file,line)) {
                            if (line != "}¦") {
                                str += line;
                                f++;
                                last_string = str;
                                str += "\n";
                            } else {
                                f++;
                                str = last_string;
                            }
                        }
                        if (str == value.at(i).get_string_value()) {
                            i++;
                            continue;
                        }
                        deleting = false;
                    }
                }
                if (line.starts_with("integer:")) {
                    if (value.at(i).getType() == Integer) {
                        int integer_val;
                        std::string str;
                        str = line;
                        str.erase(0,8);
                        integer_val = std::stoi(str);
                        if (integer_val == value.at(i).get_int_value()) {
                            i++;
                            f++;
                            continue;
                        }
                        deleting = false;
                    }
                }
                if (line.starts_with("Double-:")) {
                    if (value.at(i).getType() == Double) {
                        double dobule_val;
                        std::string str;
                        str = line;
                        str.erase(0,8);
                        dobule_val = std::stod(str);
                        std::cout << dobule_val << std::endl;
                        if (dobule_val == value.at(i).get_double_value()) {
                            i++;
                            f++;
                            continue;
                        }
                        deleting = false;
                    }
                }
                if (line != value_start) {
                    i++;
                }
                if (!deleting) {
                    ignore_next_line = true;
                    file.close();
                    file.open(databasePosition, std::ios::in);
                    for (int q = 0; q < current_line; q++) {
                        std::getline(file,line);
                    }
                    continue;
                }


            }
        }
        file.clear();
        file.close();
        file.open(databasePosition,  std::ios::trunc);
        file.close();
        file.open(databasePosition, std::ios::out);
        if (file.is_open()) {
            file << file_text;
            file.close();
            return HB_SUCCESS;
        }
        return HB_FAILED;
    } else {
        return HB_FAILED;
    }
    return HB_FAILED;
}