#include "Qic_database.h"
#include <iostream>
#include <fstream>
#include <filesystem>

const int version[3] = {0,10,2};

std::string qic::Version() {
    std::string version_str;
    version_str += std::to_string(version[0]);
    version_str += ".";
    version_str += std::to_string(version[1]);
    version_str += ".";
    version_str += std::to_string(version[2]);
    return version_str;
}

qic::Result qic::DataBase::open() {
    file.open(databasePosition, std::ios::in);
    if (file.is_open()) {
        std::string line;
        bool readingTable = false;
        bool scanningTable = false;
        std::string table_name;
        std::ofstream tableFile;
        bool scanningValue = false;
        bool readingValue = false;
        while (std::getline(file,line)) {
            if (line.starts_with("Tables {")) {
                readingTable = true;
            }
            if (line.starts_with("Values {")) {
                readingValue = true;
                continue;
            }
            if (readingTable) {
                if (line.starts_with("table-") && line.ends_with("{") && !scanningTable) {
                    scanningTable = true;
                    table_name = line;
                    table_name.erase(0, 6);
                    table_name.erase(table_name.size() - 2, 2);
                    tableFile.open(table_name + ".qic");
                    file_position.push_back(table_name + ".qic");
                }
                if (scanningTable) {
                    tableFile.write(line.c_str(), line.size());
                    tableFile.write("\n", std::string ("\n").size());
                    if (line.ends_with("};")) {
                        scanningTable = false;
                        tableFile.close();
                    }
                }
                if (line.ends_with("}") ){
                    readingTable = false;
                }
            }
            if (readingValue) {
                if (line.starts_with("from-")) {
                    tableFile.close();
                    std::string currentScanningTableValue;
                    currentScanningTableValue = line;
                    currentScanningTableValue.erase(0,5);
                    currentScanningTableValue.erase(currentScanningTableValue.size() - 2,2);
                    tableFile.close();
                    tableFile.open(currentScanningTableValue + ".qic", std::ios_base::app);
                    tableFile.write(line.c_str(),line.size());
                    tableFile.write(("\n"),std::string("\n").size());
                    continue;
                }
                if (line.starts_with("}-end_Values")) {
                    tableFile.close();
                    continue;
                }
                tableFile.write(line.c_str(),line.size());
                tableFile.write(("\n"),std::string("\n").size());

            }
        }
        this->opened = true;
        file.close();
        return qic::Result::QIC_SUCCESS;
    } else {
        std::ofstream newDbFile(databasePosition);
        if (newDbFile.is_open()) {
            newDbFile.write("Tables {\n}\n", std::string("Tables {\n}\n").size());
            newDbFile.write("Values {\n}", std::string("Values {\n}").size());
            newDbFile.close();
            this->opened = true;
            return qic::Result::QIC_SUCCESS;
        }
    }
    return qic::Result::QIC_FAILED;
}

qic::Result qic::DataBase::close() {
    file.open(databasePosition,std::ios::trunc | std::ios::out);
    if (file.is_open()) {
        std::string tables = "Tables {\n";
        std::string values = "Values {\n";
        std::ifstream inputFile;
        for (int i = 0; i < file_position.size(); i++) {
            inputFile.open(file_position.at(i));
            if (inputFile.is_open(),std::ios::in) {
                std::string line;
                bool scanningTable = false;
                bool scanningValues = false;
                while (std::getline(inputFile,line)) {
                    if (line.starts_with("table-") && line.ends_with("{") && !scanningTable) {
                        scanningTable = true;
                        tables += line;
                        tables += "\n";
                        continue;
                    }
                    if (scanningTable) {
                        tables += line;
                        tables += "\n";
                        if (line.ends_with("};")) {
                            scanningTable = false;
                        }
                    }
                    if (line.starts_with("from")) {
                        scanningValues = true;
                        values += line;
                        values += "\n";
                        continue;
                    }
                    if (scanningValues) {
                        values += line;
                        values += "\n";
                        if (line.starts_with("}-end_Values")) {
                            scanningValues = false;
                        }
                    }
                }
            }
            inputFile.close();
            std::filesystem::remove(file_position.at(i));
        }
        tables +="}\n";
        values += "}-end_Values";
        file.write(tables.c_str(),tables.size());
        file.write(values.c_str(),values.size());
        file.close();
        this->opened = false;
        return qic::Result::QIC_SUCCESS;
    }
    return qic::Result::QIC_FAILED;
}

qic::Result qic::DataBase::createTable(std::string tableName, std::vector<dataType> data) {
    if (!this->opened) {
        return qic::QIC_FAILED;
    }
    for (int i = 0; i < this->file_position.size(); i++) {
        if (file_position.at(i) == tableName + ".qic") {
            return qic::QIC_FAILED;
        }
    }
    if (!tableName.contains("}") && !tableName.contains("{") && !tableName.contains(" ")) {
        std::string table;
        table += "table-";
        table += tableName;
        table += " {";
        for (int i = 0; i < data.size(); i++) {
            table += "\n";
            if (data.at(i) == qic::dataType::String) {table += "String";}
            if (data.at(i) == qic::dataType::Integer) {table += "int";}
            if (data.at(i) == qic::dataType::Double) {table += "double";}
            if (data.at(i) == qic::dataType::Boolean) {table += "Bool";}
        }
        table += "\n";
        table += "};";
        std::ofstream newTableFile(tableName + ".qic");
        newTableFile.write(table.c_str(),table.size());
        newTableFile.close();
        this->file_position.push_back(tableName + ".qic");
        return qic::QIC_SUCCESS;
    } else {
        return qic::QIC_FAILED;
    }

}

qic::Result qic::DataBase::dropTable(std::string table) {
    if (!this->opened) {
        return qic::QIC_FAILED;
    }
    for (int i = 0; i < this->file_position.size(); i++) {
        if (this->file_position.at(i) == table + ".qic") {
            if (std::filesystem::remove(this->file_position.at(i))) {
                this->file_position.erase(this->file_position.begin() + i);
                return qic::QIC_SUCCESS;
            }
        }
    }
    return qic::QIC_FAILED;
}

qic::Result qic::DataBase::flush() {
    std::string blankDatabase;
    blankDatabase += "Tables {\n}\nValues {\n}-end_Values";
    file.open(this->databasePosition,std::ios::trunc | std::ios::out);
    if (file.is_open()) {
        file.write(blankDatabase.c_str(), blankDatabase.size());
        file.close();
        for (int i = 0; i < this->file_position.size(); i++) {
            std::filesystem::remove(this->file_position.at(i));
        }
        file_position.clear();
        return qic::QIC_SUCCESS;
    }
    return qic::QIC_FAILED;
}

std::vector<std::string> qic::DataBase::getAllTables() {
    std::vector<std::string> tables;
    std::string table;
    for (int i = 0; i < this->file_position.size(); i++) {
        table = file_position.at(i);
        table.erase(table.size() - 4,4);
        tables.push_back(table);
    }
    return tables;
}

qic::Result qic::DataBase::addValueToTable(std::string tableName, std::vector<Value> Values) {
    if (this->opened) {
        for (int i = 0; i < this->file_position.size(); i++) {
            if (file_position.at(i) == tableName + ".qic") {
                std::string insertion;
                insertion += "\nfrom-" + tableName + " {";
                std::string line;
                std::fstream inOutFile(file_position.at(i),std::ios::in);
                bool scanningTableAcceptedValues = false;
                std::vector<qic::dataType> acceptedData;
                while (std::getline(inOutFile,line)) {
                    if (line == "table-" + tableName) {
                        scanningTableAcceptedValues = true;
                    }
                    if (scanningTableAcceptedValues) {
                        if (line.starts_with("Bool")) {
                            acceptedData.push_back(qic::Boolean);
                            continue;
                        }
                        if (line.starts_with("double")) {
                            acceptedData.push_back(qic::Double);
                            continue;
                        }
                        if (line.starts_with("int")) {
                            acceptedData.push_back(qic::Integer);
                            continue;
                        }
                        if (line.starts_with("String")) {
                            acceptedData.push_back(qic::String);
                            continue;
                        }
                    }
                    if (scanningTableAcceptedValues && line.starts_with("};") ) {
                        inOutFile.close();
                        break;
                    }
                }
                if (acceptedData.size() == Values.size()) {
                    for (int j = 0; j < acceptedData.size(); j++ ) {
                        if (acceptedData.at(j) != Values.at(j).getType()) {
                            return qic::QIC_FAILED;
                        }
                    }
                }
                for (int j = 0; j <Values.size(); j++) {
                    insertion += "\n";
                    if (Values.at(j).getType() == String) {
                        if (Values.at(j).get_string_value().contains("}.")) {
                            return qic::QIC_FAILED;
                        }
                        insertion += "s:";
                        insertion += "\n";
                        insertion += "-" + Values.at(j).get_string_value();
                        insertion += "\n";
                        insertion += "}.";
                        continue;
                    }
                    if (Values.at(j).getType() == Boolean) {
                        insertion += "b:";
                        insertion += std::to_string(Values.at(j).get_bool_value());
                    }
                    if (Values.at(j).getType() == Double) {
                        insertion += "d:";
                        insertion += std::to_string(Values.at(j).get_double_value());
                    }
                    if (Values.at(j).getType() == Integer) {
                        insertion += "i:";
                        insertion += std::to_string(Values.at(j).get_int_value());
                    }
                }
                insertion += "\n";
                insertion += "}";
                inOutFile.close();
                inOutFile.open(this->file_position.at(i),std::ios::out | std::ios::app);
                if (inOutFile.is_open()) {
                    inOutFile.write(insertion.c_str(), insertion.size());
                }
                inOutFile.close();
                return qic::QIC_SUCCESS;
            }
        }
    }
    return qic::QIC_FAILED;
}

qic::Result qic::DataBase::eraseValuesFromTable(std::string table, std::vector<Value> value) {
    std::string insertion;
    insertion += "\nfrom-" + table + " {";
    for (int j = 0; j <value.size(); j++) {
        insertion += "\n";
        if (value.at(j).getType() == String) {
            if (value.at(j).get_string_value().contains("}.")) {
                return qic::QIC_FAILED;
            }
            insertion += "s:";
            insertion += "\n";
            insertion += "-" + value.at(j).get_string_value();
            insertion += "\n";
            insertion += "}.";
            continue;
        }
        if (value.at(j).getType() == Boolean) {
            insertion += "b:";
            insertion += std::to_string(value.at(j).get_bool_value());
        }
        if (value.at(j).getType() == Double) {
            insertion += "d:";
            insertion += std::to_string(value.at(j).get_double_value());
        }
        if (value.at(j).getType() == Integer) {
            insertion += "i:";
            insertion += std::to_string(value.at(j).get_int_value());
        }
    }
    insertion += "\n";
    insertion += "}";
    std::vector<std::string> tables = this->getAllTables();
    for (int i = 0; i < tables.size(); i++) {
        if (tables.at(i) == table) {
            std::string line;
            std::ifstream inFile{table + ".qic"};
            std::string newFile,compareLine;
            std::string lastLine;
            while (std::getline(inFile,line)) {
                if (line == "from-" + table + " {") {
                    std::string tmpValue;
                    tmpValue += "from-" + table + " {";
                    std::istringstream compare(insertion);
                    bool matches = true;
                    std::getline(compare, compareLine);
                    std::getline(compare, compareLine);
                    for (int j = 2; std::getline(inFile, line) && std::getline(compare, compareLine); j++) {
                        if (line != compareLine) {
                            matches = false;
                        }
                        tmpValue += "\n";
                        tmpValue += line;
                        line = "";
                    }
                    if (!matches) {
                        newFile += "\n";
                        newFile += tmpValue;
                        continue;
                    }

                }
                if (lastLine != "") {
                    newFile += "\n";
                }
                newFile += line;
                lastLine = line;
            }
            inFile.close();
            std::ofstream outFIle{table + ".qic",std::ios::trunc};
            outFIle.write(newFile.c_str(),newFile.size());
            outFIle.close();
            return qic::QIC_SUCCESS;
        }
    }
    return qic::QIC_FAILED;
}

qic::Result qic::DataBase::eraseValuesFromTableWithLimit(std::string table, std::vector<Value> value, int limit) {
    std::string insertion;
    insertion += "\nfrom-" + table + " {";
    for (int j = 0; j <value.size(); j++) {
        insertion += "\n";
        if (value.at(j).getType() == String) {
            if (value.at(j).get_string_value().contains("}.")) {
                return qic::QIC_FAILED;
            }
            insertion += "s:";
            insertion += "\n";
            insertion += "-" + value.at(j).get_string_value();
            insertion += "\n";
            insertion += "}.";
            continue;
        }
        if (value.at(j).getType() == Boolean) {
            insertion += "b:";
            insertion += std::to_string(value.at(j).get_bool_value());
        }
        if (value.at(j).getType() == Double) {
            insertion += "d:";
            insertion += std::to_string(value.at(j).get_double_value());
        }
        if (value.at(j).getType() == Integer) {
            insertion += "i:";
            insertion += std::to_string(value.at(j).get_int_value());
        }
    }
    insertion += "\n";
    insertion += "}";
    std::vector<std::string> tables = this->getAllTables();
    for (int i = 0; i < tables.size(); i++) {
        if (tables.at(i) == table) {
            std::string line;
            std::ifstream inFile{table + ".qic"};
            std::string newFile,compareLine;
            std::string lastLine;
            while (std::getline(inFile,line)) {
                if (line == "from-" + table + " {") {
                    std::string tmpValue;
                    tmpValue += "from-" + table + " {";
                    std::istringstream compare(insertion);
                    bool matches = true;
                    std::getline(compare, compareLine);
                    std::getline(compare, compareLine);
                    for (int j = 2; std::getline(inFile, line) && std::getline(compare, compareLine); j++) {
                        if (line != compareLine) {
                            matches = false;
                            limit--;
                        }
                        tmpValue += "\n";
                        tmpValue += line;
                        line = "";
                    }
                    if (!matches || limit > 0) {
                        newFile += "\n";
                        newFile += tmpValue;
                        continue;
                    }

                }
                if (lastLine != "") {
                    newFile += "\n";
                }
                newFile += line;
                lastLine = line;
            }
            inFile.close();
            std::ofstream outFIle{table + ".qic",std::ios::trunc};
            outFIle.write(newFile.c_str(),newFile.size());
            outFIle.close();
            return qic::QIC_SUCCESS;
        }
    }
    return qic::QIC_FAILED;
}


std::vector<std::vector<qic::Value>> qic::DataBase::getAllValuesFromTable(std::string table) {
    std::vector<std::string> tablesList = this->getAllTables();
    std::vector<std::vector<qic::Value>> finalVec;
    for (int i = 0; i < tablesList.size(); i++ ) {
        if (tablesList.at(i) == table) {
            std::ifstream inFile{table + ".qic"};
            bool scanningTable = false;
            bool alredyScannedTable = false;
            bool scanningValues = false;
            std::string line;
            while (std::getline(inFile,line)) {
                if (line == "table-" + table  + " {") {
                    if (!alredyScannedTable) {
                        alredyScannedTable = true;
                        scanningTable = true;
                    }
                }
                if (scanningTable) {
                    if (line == "};") {
                        scanningTable = false;
                    }
                } else {
                    // get the values here
                    if (line == "from-" + table + " {") {
                        std::vector<qic::Value> values;
                        //we go to the next line and we start analyzing
                        while (std::getline(inFile,line)) {
                            if (line.starts_with("b:")) {
                                values.push_back(qic::Value(Boolean,(line.at(2) == '1')));
                                continue;
                            }
                            if (line.starts_with("i:")) {
                                line.erase(0,2);
                                values.push_back(qic::Value(Integer,std::stoi(line)));
                                continue;
                            }
                            if (line.starts_with("d:")) {
                                line.erase(0,2);
                                values.push_back(qic::Value(Double,std::stod(line)));
                                continue;
                            }
                            if (line.starts_with("s:")) {
                                //scans the string.
                                std::string storedString;
                                int index = 0;
                                while (std::getline(inFile,line)) {
                                    if (line.starts_with("}.")) {
                                        break;
                                    }
                                    line.erase(0,1);
                                    if (index != 0) {
                                        storedString += "\n";
                                    }
                                    storedString += line;
                                    index++;
                                }
                                values.push_back(qic::Value(String,storedString));
                                continue;
                            }
                            if (line.starts_with("}")) {
                                finalVec.push_back(values);
                                break;
                            }
                        }
                    }
                }
            }
            return finalVec;
        }
    }
}