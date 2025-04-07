#include "qic.h"
#include "libs/HBZPack/hbz.h"
#include <any>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

std::mutex content_mutex;
void handleThread(size_t start, size_t end, std::string &content,
                  std::vector<std::unordered_map<std::string, std::any>> *table,
                  std::unordered_map<std::string, data_type> *type) {
  ;
  for (size_t i = start; i < end; i++) {
    std::stringstream local;
    local << "\n[";

    auto table_values = table->at(i);
    for (auto &column : table_values) {
      local << "\n" << column.first << " : ";
      data_type t = (*type)[column.first];

      if (t == INT)
        local << std::to_string(std::any_cast<int>(column.second));
      else if (t == DOUBLE)
        local << std::to_string(std::any_cast<double>(column.second));
      else if (t == FLOAT)
        local << std::to_string(std::any_cast<float>(column.second));
      else if (t == STRING)
        local << compress(std::any_cast<std::string>(column.second));
      else if (t == BOOL)
        local << std::to_string(std::any_cast<bool>(column.second));
    }

    local << "\n]";

    // SAFELY write to shared content
    std::lock_guard<std::mutex> lock(content_mutex);
    content += local.str();
  }
}
void data_base::open_database(std::filesystem::path path) {
  if (std::filesystem::exists(path)) {
    // creates the temp folder and copy the database inside of it
    std::filesystem::path temp(std::filesystem::current_path() /
                               (".temp_db_" + path.string()));
    std::filesystem::path current(std::filesystem::current_path());
    std::filesystem::create_directory(temp);
    std::filesystem::copy(path, temp);
    // we then extract the database
    std::filesystem::current_path(temp);
    decompressFiles(temp.string() / path);
    std::filesystem::current_path(current);

    // we then remove the temp file we created before
    std::filesystem::remove(temp.string() / path);
    this->tables_ptr = std::make_shared<table_container>();
    for (auto content : std::filesystem::recursive_directory_iterator(temp)) {
      if (!content.is_directory()) {
        // we save each element so we can compress it later
        this->content.push_back(content.path());
        if (content.path().extension() == ".table") {
          // if the element has the .table extension which is for
          // the qic table extension we will add it to a map
          this->tables_ptr->operator[](content.path().stem()) = std::make_pair(
              content.path(),
              std::vector<std::unordered_map<std::string, std::any>>());
        }
      }
    }

    // we save the temp path for the database
    this->path = temp;
    this->db = path;
    this->open = true;
    return;
  }
}

void data_base::close() {
  if (std::filesystem::exists(this->path)) {
    std::filesystem::path current(std::filesystem::current_path());
    if (std::filesystem::exists(this->db.filename().string())) {
      std::filesystem::remove(this->db.filename().string());
    }
    std::filesystem::current_path(this->path);
    std::vector<std::string> filePaths;
    for (auto path : this->content) {
      filePaths.push_back(path.string());
    }

    compressFiles(filePaths, this->db.filename().string());
    std::filesystem::current_path(current);
    std::filesystem::copy(this->path / (this->db.filename().string()), current);
    std::filesystem::remove_all(this->path);
    this->open = false;
    return;
  }
}

std::string data_base::build_table_header_from_map(
    std::unordered_map<std::string, data_type> content) {
  std::string header;
  header += "{\n";
  for (auto type : content) {
    std::string data_type_string;
    switch (type.second) {
    case BOOL:
      data_type_string = "bool";
      break;
    case INT:
      data_type_string = "int";
      break;
    case DOUBLE:
      data_type_string = "double";
      break;
    case FLOAT:
      data_type_string = "float";
      break;
    case STRING:
      data_type_string = "string";
      break;
    }
    header += type.first + " : " + data_type_string + "\n";
  }
  header += "}";
  return header;
}

operation
data_base::add_table(std::string name,
                     std::unordered_map<std::string, data_type> content) {
  operation returnValue;
  returnValue.stat = FAILED;
  // In order to create a table we need the db to be open
  if (this->open) {
    // we then move inside the database folder and check if the table exists
    std::filesystem::path current(std::filesystem::current_path());
    std::filesystem::current_path(this->path);
    if (!std::filesystem::exists(this->path / (name + ".table"))) {
      // as the table does not exist we can go on and create the table file
      std::ofstream offFile(this->path / (name + ".table"));
      if (offFile.is_open()) {
        // we call the function to generate a header (important info in the
        // head of the file) for the table and then save it into the file
        std::string fileContent = this->build_table_header_from_map(content);
        offFile.write(fileContent.c_str(), fileContent.size());
        offFile.close();
        // after saving the file we also have to tell the database about this
        // table firstly it needs to know about the file being part of the
        // database then we can tell it that the file we just created is also
        // a table
        this->content.push_back(this->path / (name + ".table"));
        this->tables_ptr->operator[](name) = std::make_pair(name, table_vec());
        // after all of it we can return a success operation
        returnValue.stat = SUCCESS;
      } else {
        returnValue.error = "Unable to create file.";
      }
    } else {
      returnValue.error = "Table exists";
    }
    std::filesystem::current_path(current);
  } else {
    returnValue.error = "Database not open";
  }
  return returnValue;
}

operation data_base::remove_table(std::string name) {
  operation returnValue;
  returnValue.stat = FAILED;
  // In order to create a table we need the db to be open
  if (this->open) {
    // we then move inside the database folder and check if the table exists
    std::filesystem::path current(std::filesystem::current_path());
    std::filesystem::current_path(this->path);
    if (std::filesystem::exists(this->path / (name + ".table"))) {
      std::filesystem::remove(this->path / (name + ".table"));
      this->tables_ptr->erase(name);
      returnValue.stat = SUCCESS;
    } else {
      returnValue.error = "Table does not exists";
    }
    std::filesystem::current_path(current);
  } else {
    returnValue.error = "Database not open";
  }
  return returnValue;
}

operation
data_base::add_value(std::string table,
                     std::unordered_map<std::string, std::any> values) {
  operation returnValue;
  returnValue.stat = FAILED;

  // In order to create a table we need the db to be open
  if (this->open) {
    // we then move inside the database folder and check if the table exists
    std::filesystem::path current(std::filesystem::current_path());
    std::filesystem::current_path(this->path);
    if (std::filesystem::exists(this->path / (table + ".table"))) {
      this->tables_ptr->operator[](table).second.push_back(values);
      returnValue.stat = SUCCESS;
      returnValue.error = "New elements added to the table";
    } else {
      returnValue.error = "Table does not exists";
    }
    std::filesystem::current_path(current);
  } else {
    returnValue.error = "Database not open";
  }
  return returnValue;
}

table_vec *data_base::get_values_from_table(std::string table) {
  // In order to create a table we need the db to be open
  return &(this->tables_ptr->operator[](table).second);
}

void data_base::save() {
  std::filesystem::path current(std::filesystem::current_path());
  std::filesystem::current_path(this->path);
  for (auto table : *this->tables_ptr) {
    // In order to create a table we need the db to be open
    auto type = this->get_table_content_header(table.first);
    if (this->open) {
      // we then move inside the database folder and check if the table exists

      if (std::filesystem::exists(this->path / (table.first + ".table"))) {
        std::ifstream ifFile(this->path / (table.first + ".table"));
        if (ifFile.is_open()) {
          std::string line;
          std::string content;
          while (std::getline(ifFile, line)) {
            bool is_first = false;
            if (content.empty()) {
              is_first = true;
            }
            if (!is_first) {
              content += "\n";
            }
            content += line;
            if (line == "}") {
              break;
            }
          }
          auto tables = table.second.second;
          auto size = tables.size();
          if (size == 0) {
            continue;
          }
          int thread_to_use = this->compiling_threads;
          while (thread_to_use > size) {
            thread_to_use -= 1;
          }
          while (size % thread_to_use != 0) {
            thread_to_use -= 1;
          }
          size_t chunkSize = size / thread_to_use;
          std::vector<std::thread> threads;
          for (int i = 0; i < thread_to_use; i++) {
            size_t start = i * chunkSize;
            size_t end = (i + 1) * chunkSize;
            threads.emplace_back([start, end, &content, &tables, &type]() {
              handleThread(start, end, content, &tables, &type);
            });
          }
          for (int i = 0; i < threads.size(); i++) {
            threads.at(i).join();
          }
          ifFile.close();
          std::ofstream offFile(this->path / (table.first + ".table"));
          if (offFile.is_open()) {
            offFile.write(content.c_str(), content.size());
            offFile.close();
          }
        }
      }
    }
  }
  std::filesystem::current_path(current);
}

std::unordered_map<std::string, data_type>
data_base::get_table_content_header(std::string name) {
  std::unordered_map<std::string, data_type> returnValue;
  if (this->open) {
    // we then move inside the database folder and check if the table exists
    std::filesystem::path current(std::filesystem::current_path());
    std::filesystem::current_path(this->path);
    if (std::filesystem::exists(this->path / (name + ".table"))) {
      // as the table does not exist we can go on and create the table file
      std::ifstream ifFile(this->path / (name + ".table"));
      std::string line;
      while (std::getline(ifFile, line)) {
        if (line[0] == '}') {
          break;
        } else if (line[0] == '{') {
          continue;
        }
        std::string name;
        std::string type;
        bool split = false;
        for (auto c : line) {
          if (c == ':') {
            split = true;
            continue;
          }
          if (c == ' ') {
            continue;
          }
          if (split) {
            type += c;
          } else {
            name += c;
          }
        }
        data_type dt;
        if (type == "bool") {
          dt = BOOL;
        } else if (type == "int") {
          dt = INT;
        } else if (type == "double") {
          dt = DOUBLE;
        } else if (type == "string") {
          dt = STRING;
        }
        returnValue[name] = dt;
      }
    } else {
    }
    std::filesystem::current_path(current);
  } else {
  }
  return returnValue;
}

operation data_base::load_table(std::string table) {
  operation returnValue;
  returnValue.stat = FAILED;

  // In order to create a table we need the db to be open
  if (this->open) {
    // we then move inside the database folder and check if the table exists
    std::filesystem::path current(std::filesystem::current_path());
    std::filesystem::current_path(this->path);
    if (std::filesystem::exists(this->path / (table + ".table"))) {
      auto types = this->get_table_content_header(table);
      std::ifstream ifFile(this->path / (table + ".table"));
      if (ifFile.is_open()) {
        std::string line;
        bool scanning = false;
        while (std::getline(ifFile, line)) {
          if (scanning) {
            if (line == "]") {
              scanning = false;
              continue;
            }
            bool split = false;
            std::string name;
            std::string val;
            for (auto c : line) {
              if (c == ':') {
                split = true;
                continue;
              }
              if (c == ' ') {
                continue;
              }
              if (!split) {
                name += c;
              } else {
                val += c;
              }
            }
            // Access and modify the element in tables_ptr
            auto &current_table = this->tables_ptr->operator[](table);

            if (types[name] == STRING) {
              current_table.second[current_table.second.size() - 1][name] =
                  std::any(decompressString(val));
            } else if (types[name] == INT) {
              current_table.second[current_table.second.size() - 1][name] =
                  std::any(std::stoi(val));
            } else if (types[name] == DOUBLE) {
              current_table.second[current_table.second.size() - 1][name] =
                  std::any(std::stod(val));
            } else if (types[name] == FLOAT) {
              current_table.second[current_table.second.size() - 1][name] =
                  std::any(std::stof(val));
            } else if (types[name] == BOOL) {
              bool finalVal = (val == "1");
              current_table.second[current_table.second.size() - 1][name] =
                  std::any(finalVal);
            }
          } else {
            if (line == "[") {
              this->tables_ptr->operator[](table).second.push_back(
                  std::unordered_map<std::string, std::any>());
              scanning = true;
            }
          }
        }
      }
    } else {
      returnValue.error = "Table does not exists";
    }
    std::filesystem::current_path(current);
  } else {
    returnValue.error = "Database not open";
  }
  return returnValue;
}
