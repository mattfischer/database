#include "RowPredicates/LogicalPredicate.hpp"

namespace RowPredicates {
    LogicalPredicate::LogicalPredicate(std::unique_ptr<RowPredicate> leftPredicate, Operation operation, std::unique_ptr<RowPredicate> rightPredicate)
    : mLeftPredicate(std::move(leftPredicate))
    , mOperation(operation)
    , mRightPredicate(std::move(rightPredicate))
    {
    }

    bool LogicalPredicate::evaluate(RowIterator &iterator)
    {
        switch(mOperation) {
            case Operation::And:
                return mLeftPredicate->evaluate(iterator) && mRightPredicate->evaluate(iterator);
            case Operation::Or:
                return mLeftPredicate->evaluate(iterator) || mRightPredicate->evaluate(iterator);
            case Operation::Not:
                return !mLeftPredicate->evaluate(iterator);
        }
    }
}