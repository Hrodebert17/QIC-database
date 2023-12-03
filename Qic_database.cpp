#include "Qic_database.h"
#include <iostream>
#include <fstream>
#include <filesystem>

const int version[3] = {0,10,0};

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


