#include <iostream>

#include "Database.hpp"
#include "PageSet.hpp"
#include "Record.hpp"
#include "Table.hpp"
#include "Index.hpp"

#include "RowIterators/AggregateIterator.hpp"
#include "RowIterators/ExtendedProjectIterator.hpp"
#include "RowIterators/IndexIterator.hpp"
#include "RowIterators/ProjectIterator.hpp"
#include "RowIterators/SelectIterator.hpp"
#include "RowIterators/SortIterator.hpp"
#include "RowIterators/TableIterator.hpp"

#include <iostream>
#include <sstream>

void printIterator(RowIterator &iterator)
{
    for(int i = 0; iterator.valid(); i++) {
        std::cout << i << ": ";
        for(int j=0; j<iterator.schema().fields.size(); j++) {
            Value value = iterator.getField(j);
            value.print();
            std::cout << " ";
        }
        std::cout << std::endl;

        iterator.next();
    }
}

void modifyIterator(RowIterator &iterator)
{
    while(iterator.valid()) {
        std::vector<RowIterator::ModifyEntry> entries;
        entries.push_back({2, Value(1000)});
        iterator.modify(std::move(entries));

        iterator.next();
    }
}

int main(int argc, char *argv[])
{
    Database database;

    auto query = [&](const std::string &queryString) {
        Database::QueryResult result = database.executeQuery(queryString);
        std::cout << result.message << std::endl;
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

    database.table("Table").print();

    RowIterators::TableIterator tableIterator(database.table("Table"));
    tableIterator.start();
    printIterator(tableIterator);
    std::cout << std::endl;
    std::cout << "-----------" << std::endl;
    std::cout << std::endl;

    Index &index = database.index("Table1");
    Record::Writer startWriter(index.keySchema());
    startWriter.setField(0, Value(3));
    RowIterators::IndexIterator::Limit startLimit = { BTree::SearchComparison::Equal, BTree::SearchPosition::First, startWriter, 1 };
    
    Record::Writer endWriter(index.keySchema());
    endWriter.setField(0, Value(4));
    RowIterators::IndexIterator::Limit endLimit = { BTree::SearchComparison::Equal, BTree::SearchPosition::Last, endWriter, 1 };
    
    RowIterators::IndexIterator indexIterator(index, startLimit, endLimit);
    indexIterator.start();
    printIterator(indexIterator);

    return 0;
}