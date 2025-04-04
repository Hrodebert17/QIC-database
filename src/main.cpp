#include "qic.h"
#include <any>
#include <iostream>
#include <unordered_map>
#include <utility>

int main() {
  data_base db;
  db.open_database("test.txt");
  auto operation = db.add_table(
      "test", std::unordered_map<std::string, data_type>{{"test", STRING}});

  if (operation.stat == FAILED) {
    std::cout << operation.error << std::endl;
  } else {
    std::cout << "No error!" << std::endl;
  }
  std::cout << "1";
  std::unordered_map<std::string, std::any> values;
  auto a = std::any(std::string("hello"));
  values.insert(std::make_pair(std::string("test"), a));
  auto s = db.add_value("test", values);
  if (s.stat == FAILED) {
    std::cout << s.error << std::endl;
  }
  std::string str;
  db.save();
  std::cin >> str;
  db.close();
  return 0;
}
