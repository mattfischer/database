#include "Expression.hpp"

CompareExpression::CompareExpression(CompareType compareType, std::unique_ptr<Expression> leftOperand, std::unique_ptr<Expression> rightOperand)
: mCompareType(compareType)
, mLeftOperand(std::move(leftOperand))
, mRightOperand(std::move(rightOperand))
{
}

Value CompareExpression::evaluate(EvaluateContext &context)
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

void CompareExpression::bind(BindContext &context)
{
    mLeftOperand->bind(context);
    mRightOperand->bind(context);
}

LogicalExpression::LogicalExpression(LogicalType logicalType, std::unique_ptr<Expression> leftOperand, std::unique_ptr<Expression> rightOperand)
: mLogicalType(logicalType)
, mLeftOperand(std::move(leftOperand))
, mRightOperand(std::move(rightOperand))
{
}

Value LogicalExpression::evaluate(EvaluateContext &context)
{
    Value leftValue = mLeftOperand->evaluate(context);
    Value rightValue = (mLogicalType != Not) ? mRightOperand->evaluate(context) : Value();

    switch(mLogicalType) {
        case LogicalType::And:
            return Value(leftValue.booleanValue() && rightValue.booleanValue());
        case LogicalType::Or:
            return Value(leftValue.booleanValue() || rightValue.booleanValue());
        case LogicalType::Not:
            return Value(!leftValue.booleanValue());
    }
}

void LogicalExpression::bind(BindContext &context)
{
    mLeftOperand->bind(context);
    if(mRightOperand) mRightOperand->bind(context);
}

ArithmeticExpression::ArithmeticExpression(ArithmeticType arithmeticType, std::unique_ptr<Expression> leftOperand, std::unique_ptr<Expression> rightOperand)
: mArithmeticType(arithmeticType)
, mLeftOperand(std::move(leftOperand))
, mRightOperand(std::move(rightOperand))
{
}

Value ArithmeticExpression::evaluate(EvaluateContext &context)
{
    Value leftValue = mLeftOperand->evaluate(context);
    Value rightValue = (mArithmeticType != Negate) ? mRightOperand->evaluate(context) : Value();

    switch(mArithmeticType) {
        case ArithmeticType::Add:
            return leftValue + rightValue;
        case ArithmeticType::Subtract:
            return leftValue - rightValue;
        case ArithmeticType::Multiply:
            return leftValue * rightValue;
        case ArithmeticType::Divide:
            return leftValue / rightValue;
        case ArithmeticType::Negate:
            return -leftValue;
    }
}

void ArithmeticExpression::bind(BindContext &context)
{
    mLeftOperand->bind(context);
    if(mRightOperand) mRightOperand->bind(context);
}

ConstantExpression::ConstantExpression(Value value)
: mValue(value)
{
}

Value ConstantExpression::evaluate(EvaluateContext &)
{
    return mValue;
}

void ConstantExpression::bind(BindContext &context)
{
}

FieldExpression::FieldExpression(int field)
: mField(field)
{
}

FieldExpression::FieldExpression(const std::string &name)
: mName(name)
{
    mField = -1;
}

Value FieldExpression::evaluate(EvaluateContext &context)
{
    return context.fieldValue(mField);
}

void FieldExpression::bind(BindContext &context)
{
    mField = context.field(mName);
    if(mField == -1) {
        throw BindError {mName};
    }
}
