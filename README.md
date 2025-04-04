# QIC Database Library Documentation

The QIC Database Library is a lightweight, file-based database solution designed to manage simple databases through direct file manipulation, compression, and custom table formats. It uses the C++ filesystem library for file and directory operations and relies on custom compression (via HBZPack) to store and retrieve data.

## Overview

The library facilitates:
- **Opening and closing databases:** It creates a temporary working directory to avoid in-place modifications.
- **Table management:** Creating, removing, and scanning table files with the custom `.table` extension.
- **Record operations:** Adding and retrieving records (values) to/from tables.
- **Persistence:** Constructing file headers based on table schemas and saving records with type-safe conversions.
- **Compression support:** Working with compressed files for efficient storage.

## Requirements

- **Compiler:** C++17 or later (for features like `std::filesystem` and `std::any`).
- **Libraries:**  
  - **HBZPack:** For file compression and decompression.  
  - Standard C++ libraries: `<any>`, `<cstddef>`, `<filesystem>`, `<fstream>`, `<memory>`, `<string>`, `<unordered_map>`, `<utility>`, `<vector>`.

## Setup and Installation

1. **Include the Header:**  
   Ensure you include `"qic.h"` in your project.

2. **Link Dependencies:**  
   Link the HBZPack library and make sure your build environment supports C++17.

3. **Directory Permissions:**  
   The library creates temporary directories and works with file copy and removal operations; make sure the application has the necessary permissions.

## Key Concepts

### Temporary Directory Model

When a database file is opened:
- A temporary folder is created (named `.temp_db_<database_path>`).
- The database is copied into this folder and decompressed.
- All table files (files with the `.table` extension) are loaded and registered.
- Operations are performed on the content in the temporary folder.
- When closing, the database is recompressed, the temporary folder is cleaned up, and the final file is copied back.

### Data Types

The library supports a limited set of basic data types:
- **BOOL:** Boolean values.
- **INT:** Integer values.
- **DOUBLE:** Double-precision floating-point values.
- **FLOAT:** Single-precision floating-point values.
- **STRING:** Text values (stored compressed when written to disk).

### Operation Structure

Many functions return an `operation` object. This structure includes:
- **stat:** A status indicator (`SUCCESS` or `FAILED`).
- **error:** A descriptive error message when an operation fails.

## API Reference

### Class: `data_base`

#### 1. `void open_database(std::filesystem::path path)`
Opens an existing database file, creates a temporary directory, decompresses the database, and scans for table files (`.table` extension).

#### 2. `void close()`
Closes the database, compresses all files back into the final database file, and cleans up temporary resources.

#### 3. `std::string build_table_header_from_map(std::unordered_map<std::string, data_type> content)`
Generates a textual table header based on the schema.

#### 4. `operation add_table(std::string name, std::unordered_map<std::string, data_type> content)`
Creates a new table with a given name and schema, registers it in the database.

#### 5. `operation remove_table(std::string name)`
Deletes an existing table and unregisters it from the database.

#### 6. `operation add_value(std::string table, std::unordered_map<std::string, std::any> values)`
Adds a new record to a specified table.

#### 7. `std::vector<std::unordered_map<std::string, std::any>>* get_values_from_table(std::string table)`
Retrieves all records from a given table.

#### 8. `void save()`
Persists all in-memory changes to disk.

#### 9. `std::unordered_map<std::string, data_type> get_table_content_header(std::string name)`
Retrieves the schema of a specified table.

#### 10. `operation load_table(std::string table)`
Loads records from a specified table file into memory.

## Usage Example

```cpp
#include "qic.h"
#include <filesystem>
#include <iostream>
#include <unordered_map>

int main() {
    data_base db;
    db.open_database("mydatabase.db");

    std::unordered_map<std::string, data_type> tableStructure = {
        {"id", INT},
        {"name", STRING},
        {"active", BOOL},
        {"balance", DOUBLE}
    };

    operation op = db.add_table("users", tableStructure);
    if (op.stat == SUCCESS) {
        std::cout << "Table 'users' successfully created!\n";
    }

    std::unordered_map<std::string, std::any> newRecord = {
        {"id", std::any(1)},
        {"name", std::any(std::string("Alice"))},
        {"active", std::any(true)},
        {"balance", std::any(2500.35)}
    };

    op = db.add_value("users", newRecord);
    if (op.stat == SUCCESS) {
        std::cout << "Record added to table 'users'.\n";
    }

    db.save();
    db.close();

    return 0;
}
