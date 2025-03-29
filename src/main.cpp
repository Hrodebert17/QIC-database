#include "qic.h"
#include <iostream>
int main() {
  data_base db;
  db.openDatabase("test.txt");
  int s;
  std::cin >> s;
  db.close();
  return 0;
}
