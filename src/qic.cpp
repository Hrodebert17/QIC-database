#include "qic.h"
#include "libs/HBZPack/hbz.h"
#include <filesystem>
void data_base::openDatabase(std::filesystem::path path) {
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
    return;
  }
}
