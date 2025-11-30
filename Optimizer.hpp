#ifndef OPTIMIZER_HPP
#define OPTIMIZER_HPP

#include "Database.hpp"

class Optimizer {
public:
    Optimizer(Database &database);

    void optimize(Database::Query &query);

private:
    Database &mDatabase;
};

#endif