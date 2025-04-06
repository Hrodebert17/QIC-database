#include "libs/HBZPack/hbz.h"
#include "qic.h"
#include <iostream>
#include <utility>

table_container::table_container(table_container &copy) {
  for (auto s : copy) {
    (*this)[(s.first)] = s.second;
  }
  this->type_for_tables = copy.type_for_tables;
  this->selector = copy.selector;
  this->type = copy.type;
}

table_container *table_container::select(std::string number) {
  if (number == "*") {
    this->selector = 0;
  } else {
    this->selector = std::stoi(number);
  }
  return this;
}

table_container *table_container::remove(std::string number) {
  if (number == "*") {
    this->selector = 0;
  } else {
    this->selector = std::stoi(number);
  }
  this->type = DELETE;
  return this;
}

table_container *table_container::from(std::string table_to_query) {
  this->table = &(*this)[table_to_query].second;
  this->table_name = table_to_query;
  this->returnValue.clear();
  for (int i = 0; i < this->table->size(); i++) {
    this->returnValue.push_back(std::make_pair(i, this->table->at(i)));
  }

  return this;
}

table_container *table_container::where(std::string value, std::any is,
                                        bool is_not) {
  std::vector<int> toErase;
  for (int i = 0; i < this->returnValue.size(); i++) {
    auto row = this->returnValue.at(i).second;
    if (returnValue.size() == selector && selector != 0) {
      break;
    }
    auto it = row.find(value);

    if (it != row.end()) {
      bool match = false;
      int t = this->type_for_tables[this->table_name][value];
      try {
        switch (t) {
        case INT:
          match = std::any_cast<int>(is) == std::any_cast<int>(it->second);

          break;
        case DOUBLE:
          match =
              std::any_cast<double>(is) == std::any_cast<double>(it->second);
          break;
        case FLOAT:
          match = std::any_cast<float>(is) == std::any_cast<float>(it->second);
          break;
        case STRING:
          match = std::any_cast<std::string>(is) ==
                  decompressString(std::any_cast<std::string>(it->second));
          break;
        case BOOL:
          match = std::any_cast<bool>(is) == std::any_cast<bool>(it->second);
          break;
        }
      } catch (const std::bad_any_cast &e) {
        std::cerr << "invalid cast: " << it->first << std::endl;
        continue;
      }

      if (match == is_not) {
        toErase.push_back(i);
      }
    }
  }
  for (int i = 0; i < 0; i++) {
    this->returnValue.erase(this->returnValue.begin() + toErase.at(i) - i);
  }
  return this;
}

table_vec table_container::get_output() {
  if (this->type == DELETE) {
    for (auto value : this->returnValue) {
      auto v = this->table;
      v->erase(v->begin() + value.first);
    }
  }
  table_vec vec{};
  for (auto v : this->returnValue) {
    vec.push_back(v.second);
  }
  return vec;
}
