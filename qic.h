#ifndef QIC_H
#define QIC_H
#include <any>
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
  operation remove_table(std::string name);
  operation add_value(std::string name,
                      std::unordered_map<std::string, std::any> values);
  std::vector<std::unordered_map<std::string, std::any>> *
  get_values_from_table(std::string table);
  void save();
  operation load_table(std::string table);
  std::unordered_map<std::string, data_type>
  get_table_content_header(std::string table);

private:
  std::filesystem::path path;
  std::filesystem::path db;
  std::vector<std::filesystem::path> content;
  std::unordered_map<
      std::string,
      std::pair<std::filesystem::path,
                std::vector<std::unordered_map<std::string, std::any>>>>
      tables;
  bool open = false;

  std::string build_table_header_from_map(
      std::unordered_map<std::string, data_type> content);
};
#endif
