#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <sstream>
#include <algorithm>

using namespace std;

struct Column {
    string name;
    string type;
    int size;
    string defaultValue;
};

class Table {
public:
    Table(const string& tableName) : name(tableName) {}

    string getName() const { return name; }

    void addColumn(const string& columnName, const string& type, int size, const string& defaultValue) {
        columns.push_back({columnName, type, size, defaultValue});
    }

    const vector<Column>& getColumns() const { return columns; }

    void display() const {
        cout << "Table: " << name << endl;
        cout << "Columns: " << columns.size() << endl;
        for (size_t i = 0; i < columns.size(); ++i) {
            cout << "Column " << i + 1 << ":\n";
            cout << "  Name: " << columns[i].name << endl;
            cout << "  Type: " << columns[i].type << endl;
            cout << "  Dimension: " << columns[i].size << endl;
            cout << "  Default: " << columns[i].defaultValue << endl;
        }
    }

    void createFile(const string& tableName) {
        ofstream table(tableName + ".txt");
        table.close();
    }

private:
    string name;
    vector<Column> columns;
};

class Processor {
public:
    void processCommand(const string& command) {
        if (command.find("CREATE TABLE") == 0) {
            CreateTable(command);
        }
        else if (command.find("DROP TABLE") == 0) {
            DropTable(command);
        }
        else if (command.find("INSERT INTO") == 0) {
            InsertIntoTable(command);
        }
        else if (command.find("SELECT") == 0) {
            SelectFromTable(command);
        }
        else {
            cout << "Error: Unknown command type.\n";
        }
    }

private:
    vector<Table> tables;

    void CreateTable(const string& command) {
        string tableName;
        vector<Column> columns;

        try {
            regex createTableRegex(R"(CREATE TABLE (\w+)\s*\((.*)\))", regex::icase);
            smatch matches;
            if (regex_search(command, matches, createTableRegex)) {
                tableName = matches[1];
                string columnDefinitions = matches[2];

                regex columnRegex(R"((\w+)\s+(\w+)\s+(\d+)\s*('?.*'?))");
                sregex_iterator begin(columnDefinitions.begin(), columnDefinitions.end(), columnRegex);
                sregex_iterator end;

                for (sregex_iterator it = begin; it != end; ++it) {
                    smatch match = *it;
                    string name = match[1];
                    string type = match[2];
                    int size = stoi(match[3]);
                    string defaultValue = match[4];

                    columns.push_back({name, type, size, defaultValue});
                }

                if (columns.empty()) {
                    throw "No valid columns found.";
                }

                for (const auto& table : tables) {
                    if (table.getName() == tableName) {
                        cout << "Error: Table '" << tableName << "' already exists.\n";
                        return;
                    }
                }

                Table newTable(tableName);
                newTable.createFile(tableName);
                for (const Column& col : columns) {
                    newTable.addColumn(col.name, col.type, col.size, col.defaultValue);
                }

                tables.push_back(newTable);
                newTable.display();
            } else {
                throw "Failed to parse command.";
            }
        } catch (const exception& e) {
            cout << "Error: " << e.what() << endl;
        } catch (const char* msg) {
            cout << "Error: " << msg << endl;
        }
    }

    void DropTable(const string& command) {
        string tableName;

        try {
            regex dropTableRegex(R"(DROP TABLE (\w+))", regex::icase);
            smatch matches;
            if (regex_search(command, matches, dropTableRegex)) {
                tableName = matches[1];

                auto it = find_if(tables.begin(), tables.end(), [&](const Table& table) {
                    return table.getName() == tableName;
                });

                if (it != tables.end()) {
                    tables.erase(it);
                    cout << "Table '" << tableName << "' has been dropped.\n";
                } else {
                    cout << "Error: Table '" << tableName << "' does not exist.\n";
                }
            } else {
                throw "Failed to parse DROP TABLE command.";
            }
        } catch (const exception& e) {
            cout << "Error: " << e.what() << endl;
        } catch (const char* msg) {
            cout << "Error: " << msg << endl;
        }
    }

    void InsertIntoTable(const string& command) {
        // Implementation for inserting data into the table
    }

    void SelectFromTable(const string& command) {
        try {
            regex selectRegex(R"(SELECT\s*(ALL|\(.*\))\s*FROM\s*(\w+)(\s*WHERE\s*(\w+)\s*=\s*'([^']+)')?)", regex::icase);
            smatch matches;

            if (regex_search(command, matches, selectRegex)) {
                string columnsPart = matches[1];  // Columns or ALL
                string tableName = matches[2];    // Table name
                string whereClause = matches[4];  // WHERE column
                string whereValue = matches[5];   // Value in WHERE

                auto it = find_if(tables.begin(), tables.end(), [&](const Table& table) {
                    return table.getName() == tableName;
                });

                if (it == tables.end()) {
                    cout << "Error: Table '" << tableName << "' does not exist.\n";
                    return;
                }

                Table& table = *it;

                if (columnsPart == "ALL") {
                    cout << "Table: " << tableName << endl;
                    cout << "Columns: All\n";
                    for (const auto& col : table.getColumns()) {
                        cout << "Column: " << col.name << endl;
                    }
                } else {
                    stringstream ss(columnsPart.substr(1, columnsPart.size() - 2));  // remove parentheses
                    string column;
                    vector<string> columns;
                    while (getline(ss, column, ',')) {
                        columns.push_back(column);
                    }

                    cout << "Table: " << tableName << endl;
                    cout << "Columns: " << columns.size() << endl;
                    for (const string& col : columns) {
                        cout << "Column: " << col << endl;
                    }
                }

                if (!whereClause.empty()) {
                    cout << "Filter: yes\n";
                    cout << "Filter column: " << whereClause << " with value " << whereValue << endl;
                } else {
                    cout << "Filter: no\n";
                }
            } else {
                throw "Failed to parse SELECT command.";
            }
        } catch (const exception& e) {
            cout << "Error: " << e.what() << endl;
        } catch (const char* msg) {
            cout << "Error: " << msg << endl;
        }
    }
};

int main() {
    Processor processor;

    string command;
    cout << "Enter command: ";
    getline(cin, command);

    processor.processCommand(command);

    return 0;
}
