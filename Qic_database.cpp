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
                        if (line.ends_with("}")) {
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
        }
        tables +="}\n";
        values += "}-end_Values";
        file.write(tables.c_str(),tables.size());
        file.write(values.c_str(),values.size());
        file.close();
    }
    return qic::Result::QIC_FAILED;
}


