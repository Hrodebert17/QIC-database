#ifndef QIC_H
#define QIC_H
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

enum status { FAILED, SUCCESS };

enum data_type { BOOL, INT, DOUBLE, FLOAT, STRING };

struct operation {
  status stat;
  std::string error;
};

class data_base {
public:
  void open_database(std::filesystem::path dbPath);
  void close();
  operation add_table(std::string name,
                      std::unordered_map<std::string, data_type> content);

private:
  std::filesystem::path path;
  std::filesystem::path db;
  std::vector<std::filesystem::path> content;
  std::unordered_map<std::string, std::filesystem::path> tables;
  bool open = false;

  std::string build_table_header_from_map(
      std::unordered_map<std::string, data_type> content);
};
#endif
