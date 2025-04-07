#ifndef QIC_H
#define QIC_H
#include <any>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
enum status { FAILED, SUCCESS };

enum data_type { BOOL, INT, DOUBLE, FLOAT, STRING };

enum query_type { DELETE, SELECT };

typedef std::unordered_map<std::string, std::any> table_value;
typedef std::vector<std::unordered_map<std::string, std::any>> table_vec;
class data_base;

class table_container
    : public std::unordered_map<std::string,
                                std::pair<std::filesystem::path, table_vec>> {
public:
  table_container() {}
  table_container(table_container &copy);
  int selector;
  table_vec *table;
  std::string table_name;
  query_type type;
  std::unordered_map<std::string, std::unordered_map<std::string, data_type>>
      type_for_tables;

  table_container *select(std::string number);
  table_container *remove(std::string number);
  table_container *from(std::string table_to_query);
  table_container *where(std::string value, std::any is, bool is_not);

  table_vec get_output();

private:
  std::vector<std::pair<int, table_value>> returnValue;
};

table_vec parse_and_execute(data_base *db, std::string &input_string);
struct operation {
  status stat;
  std::string error;
};

class data_base {
public:
  int compiling_threads = 2;

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
  std::shared_ptr<table_container> container_execute() {
    this->tables_ptr->type_for_tables.clear();
    for (auto t : *this->tables_ptr) {
      this->tables_ptr->type_for_tables[t.first] =
          this->get_table_content_header(t.first);
    }
    return this->tables_ptr;
  }
  bool is_open() { return this->open; }
  std::vector<std::filesystem::path> get_all_tables() { return this->content; }

private:
  std::filesystem::path path;
  std::filesystem::path db;
  std::vector<std::filesystem::path> content;
  bool open = false;
  std::shared_ptr<table_container> tables_ptr;
  std::string build_table_header_from_map(
      std::unordered_map<std::string, data_type> content);
};
#endif
