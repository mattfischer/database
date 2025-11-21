#include "RowIterator.hpp"

class IteratorEvaluateContext : public Expression::EvaluateContext {
public:
    IteratorEvaluateContext(RowIterator &iterator) : mIterator(iterator) {}

    Value fieldValue(unsigned int field) {
        return mIterator.getField(field);
    }

private:
    RowIterator &mIterator;
};

Value RowIterator::evaluateExpression(Expression &expression, RowIterator &iterator)
{
    IteratorEvaluateContext context(iterator);

    return expression.evaluate(context);
}