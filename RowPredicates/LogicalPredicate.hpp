#ifndef ROWPREDICATES_LOGICALPREDICATE_HPP
#define ROWPREDICATES_LOGICALPREDICATE_HPP

#include "RowPredicate.hpp"

#include <memory>

namespace RowPredicates {
    class LogicalPredicate : public RowPredicate {
    public:
        enum Operation {
            And,
            Or,
            Not
        };

        LogicalPredicate(std::unique_ptr<RowPredicate> leftPredicate, Operation operation, std::unique_ptr<RowPredicate> rightPredicate = nullptr);

        bool evaluate(RowIterator &iterator) override;

    private:
        std::unique_ptr<RowPredicate> mLeftPredicate;
        Operation mOperation;
        std::unique_ptr<RowPredicate> mRightPredicate;
    };
}
#endif