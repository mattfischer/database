#include <iostream>

#include "Database.hpp"
#include "PageSet.hpp"
#include "Record.hpp"
#include "Table.hpp"
#include "Index.hpp"

#include "RowIterators/AggregateIterator.hpp"
#include "RowIterators/IndexIterator.hpp"
#include "RowIterators/ProjectIterator.hpp"
#include "RowIterators/SelectIterator.hpp"
#include "RowIterators/SortIterator.hpp"
#include "RowIterators/TableIterator.hpp"

#include <iostream>
#include <sstream>
#include <algorithm>

void printIterator(RowIterator &iterator)
{
    std::vector<int> widths;

    std::cout << "+";
    for(int i = 0; i < iterator.schema().fields.size() + 1; i++) {
        int width = (i == 0) ? 4 : std::max((int)iterator.schema().fields[i - 1].name.size(), Value::minPrintWidth(iterator.schema().fields[i - 1].type)); 
        widths.push_back(width);
        for(int j = 0; j < width + 2; j++) std::cout << "-";
        std::cout << "+";
    }
    std::cout << std::endl;

    std::cout << "|";
    for(int i = 0; i < iterator.schema().fields.size() + 1; i++) {
        std::cout << " ";
        std::cout.width(widths[i]);
        std::cout << ((i == 0) ? "#" : iterator.schema().fields[i - 1].name);
        std::cout << " |";
    }
    std::cout << std::endl;

    std::cout << "+";
    for(int i = 0; i < iterator.schema().fields.size() + 1; i++) {
        for(int j = 0; j < widths[i] + 2; j++) std::cout << "-";
        std::cout << "+";
    }
    std::cout << std::endl;

    for(int i = 0; iterator.valid(); i++) {
        std::cout << "|";
        for(int j = 0; j < iterator.schema().fields.size() + 1; j++) {
            std::cout << " ";
            if(j == 0) {
                std::cout.width(widths[j]);
                std::cout << i;
            } else {
                Value value = iterator.getField(j - 1);
                value.print(widths[j]);
            }
            std::cout << " |";
        }
        std::cout << std::endl;

        iterator.next();
    }

    std::cout << "+";
    for(int i = 0; i < iterator.schema().fields.size() + 1; i++) {
        for(int j = 0; j < widths[i] + 2; j++) std::cout << "-";
        std::cout << "+";
    }
    std::cout << std::endl;
}

int main(int argc, char *argv[])
{
    Database database;

    auto query = [&](const std::string &queryString) {
        Database::QueryResult result = database.executeQuery(queryString);
        if(!result.message.empty()) {
            std::cout << result.message << std::endl;
        }
        if(result.iterator) {
            result.iterator->start();
            printIterator(*result.iterator);
            std::cout << std::endl;
        }
    };

    query("CREATE TABLE Table (STRING name, INTEGER value, INTEGER value2)");
    query("CREATE INDEX Table1 ON Table (value, value2)");

    srand(12345);
    for(int i=0; i<64; i++) {
        int key = rand() % 5;
        int key2 = rand() % 1000;
        std::stringstream ss;
        ss << "INSERT INTO Table VALUES (\"Row " << i << "\", " << key << ", " << key2 << ")";
        query(ss.str());
    }

    query("SELECT AVG(value2) FROM Table GROUP BY value");

    return 0;
}