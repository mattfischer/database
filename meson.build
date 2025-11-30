project('database', 'cpp')

sources = [
    'BTree.cpp',
    'BTreePage.cpp',
    'Database.cpp',
    'Expression.cpp',
    'Index.cpp',
    'Main.cpp',
    'Page.cpp',
    'PageSet.cpp',
    'Parser.cpp',
    'Record.cpp',
    'RowIterator.cpp',
    'RowIterators/AggregateIterator.cpp',
    'RowIterators/ForeignKeyJoinIterator.cpp',
    'RowIterators/IndexIterator.cpp',
    'RowIterators/ProjectIterator.cpp',
    'RowIterators/SelectIterator.cpp',
    'RowIterators/SortIterator.cpp',
    'RowIterators/TableIterator.cpp',
    'Table.cpp',
    'Value.cpp'
]

executable('database', sources, cpp_args: ['/std:c++20'])