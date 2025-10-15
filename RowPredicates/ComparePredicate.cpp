#include "RowPredicates/ComparePredicate.hpp"

namespace RowPredicates {
    ComparePredicate::ComparePredicate(unsigned int leftField, Comparison comparison, unsigned int rightField)
    {
        mFields[0] = leftField;
        mFields[1] = rightField;
        mComparison = comparison;
        mPredicateType = PredicateType::FieldField;
    }

    ComparePredicate::ComparePredicate(unsigned int field, Comparison comparison, Value value)
    {
        mFields[0] = field;
        mValue = std::move(value);
        mComparison = comparison;
        mPredicateType = PredicateType::FieldValue;
    }

    bool ComparePredicate::evaluate(RowIterator &iterator)
    {
        Value leftValue = iterator.getField(mFields[0]);
        Value rightValue;
        switch(mPredicateType) {
            case PredicateType::FieldField:
                rightValue = iterator.getField(mFields[1]);
                break;
            
            case PredicateType::FieldValue:
                rightValue = mValue;
                break;
        }
        
        switch(mComparison) {
            case Comparison::LessThan:
                return leftValue < rightValue;
            case Comparison::LessThanEqual:
                return leftValue <= rightValue;
            case Comparison::Equal:
                return leftValue == rightValue;
            case Comparison::NotEqual:
                return leftValue != rightValue;
            case Comparison::GreaterThanEqual:
                return leftValue >= rightValue;
            case Comparison::GreaterThan:
                return leftValue > rightValue;
        }
    }
}