#include "Expression.hpp"

CompareExpression::CompareExpression(CompareType compareType, std::unique_ptr<Expression> leftOperand, std::unique_ptr<Expression> rightOperand)
: mCompareType(compareType)
, mLeftOperand(std::move(leftOperand))
, mRightOperand(std::move(rightOperand))
{
}

Value CompareExpression::evaluate(Context &context)
{
    Value leftValue = mLeftOperand->evaluate(context);
    Value rightValue = mRightOperand->evaluate(context);

    switch(mCompareType) {
        case CompareType::LessThan:
            return leftValue < rightValue;
        case CompareType::LessThanEqual:
            return leftValue <= rightValue;
        case CompareType::Equal:
            return leftValue == rightValue;
        case CompareType::NotEqual:
            return leftValue != rightValue;
        case CompareType::GreaterThanEqual:
            return leftValue >= rightValue;
        case CompareType::GreaterThan:
            return leftValue > rightValue;
    }
}

LogicalExpression::LogicalExpression(LogicalType logicalType, std::unique_ptr<Expression> leftOperand, std::unique_ptr<Expression> rightOperand)
: mLogicalType(logicalType)
, mLeftOperand(std::move(leftOperand))
, mRightOperand(std::move(rightOperand))
{
}

Value LogicalExpression::evaluate(Context &context)
{
    Value leftValue = mLeftOperand->evaluate(context);
    Value rightValue = mRightOperand->evaluate(context);

    switch(mLogicalType) {
        case LogicalType::And:
            return Value(leftValue.booleanValue() && rightValue.booleanValue());
        case LogicalType::Or:
            return Value(leftValue.booleanValue() || rightValue.booleanValue());
        case LogicalType::Not:
            return Value(!leftValue.booleanValue());
    }
}

ArithmeticExpression::ArithmeticExpression(ArithmeticType arithmeticType, std::unique_ptr<Expression> leftOperand, std::unique_ptr<Expression> rightOperand)
: mArithmeticType(arithmeticType)
, mLeftOperand(std::move(leftOperand))
, mRightOperand(std::move(rightOperand))
{
}

Value ArithmeticExpression::evaluate(Context &context)
{
    Value leftValue = mLeftOperand->evaluate(context);
    Value rightValue = mRightOperand->evaluate(context);

    switch(mArithmeticType) {
        case ArithmeticType::Add:
            return leftValue + rightValue;
        case ArithmeticType::Subtract:
            return leftValue - rightValue;
        case ArithmeticType::Multiply:
            return leftValue * rightValue;
        case ArithmeticType::Divide:
            return leftValue / rightValue;
    }
}

ConstantExpression::ConstantExpression(Value value)
: mValue(value)
{
}

Value ConstantExpression::evaluate(Context &)
{
    return mValue;
}

FieldExpression::FieldExpression(unsigned int field)
: mField(field)
{
}

Value FieldExpression::evaluate(Context &context)
{
    return context.fieldValue(mField);
}