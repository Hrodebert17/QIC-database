#include "qic.h"
#include <iostream>
int main() {
  data_base db;
  db.open_database("test.txt");
  auto operation =
      db.add_table("test", std::unordered_map<std::string, data_type>{
                               {"test", STRING}, {"test2", INT}});
  if (operation.stat == FAILED) {
    std::cout << operation.error << std::endl;
  } else {
    std::cout << "No error!" << std::endl;
  }
  db.close();
  return 0;
}
