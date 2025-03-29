#include "qic.h"
#include "libs/HBZPack/hbz.h"
#include <filesystem>
#include <fstream>

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

    // we scan for tables
    for (auto content : std::filesystem::recursive_directory_iterator(temp)) {
      if (!content.is_directory()) {
        // we save each element so we can compress it later
        this->content.push_back(content.path());
        if (content.path().extension() == ".table") {
          // if the element has the .table extension which is for
          // the qic table extension we will add it to a map
          this->tables[content.path().stem()] = content.path();
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
    std::filesystem::current_path(this->path);
    std::vector<std::string> filePaths;
    for (auto path : this->content) {
      filePaths.push_back(path.string());
    }
    compressFiles(filePaths, this->db.filename().string());
    std::filesystem::current_path(current);

    if (std::filesystem::exists(this->db)) {
      std::filesystem::remove(this->db);
    }
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
        // we call the function to generate a header (important info in the head
        // of the file) for the table and then save it into the file
        std::string fileContent = this->build_table_header_from_map(content);
        offFile.write(fileContent.c_str(), fileContent.size());
        offFile.close();
        // after saving the file we also have to tell the database about this
        // table firstly it needs to know about the file being part of the
        // database then we can tell it that the file we just created is also a
        // table
        this->content.push_back(this->path / (name + ".table"));
        this->tables[name] = this->path / (name + ".table");
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
