#ifndef ROWPREDICATES_COMPAREPREDICATE_HPP
#define ROWPREDICATES_COMPAREPREDICATE_HPP

#include "RowPredicate.hpp"

namespace RowPredicates {
    class ComparePredicate : public RowPredicate {
    public:
        enum Comparison {
            LessThan,
            LessThanEqual,
            Equal,
            NotEqual,
            GreaterThanEqual,
            GreaterThan
        };
        
        ComparePredicate(unsigned int leftField, Comparison comparison, unsigned int rightField);
        ComparePredicate(unsigned int field, Comparison comparison, Value value);

        bool evaluate(RowIterator &iterator) override;

    private:
        enum PredicateType {
            FieldField,
            FieldValue
        };

        PredicateType mPredicateType;
        unsigned int mFields[2];
        Comparison mComparison;
        Value mValue;
    };
}
#endif