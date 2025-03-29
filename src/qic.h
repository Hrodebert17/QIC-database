#ifndef QIC_H
#define QIC_H
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>
class data_base {
public:
  void openDatabase(std::filesystem::path dbPath);
  void close();

private:
  std::filesystem::path path;
  std::filesystem::path db;
  std::vector<std::filesystem::path> content;
  std::unordered_map<std::string, std::filesystem::path> tables;
};
#endif
