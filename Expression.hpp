#ifndef EXPRESSION_HPP
#define EXPRESSION_HPP

#include "Value.hpp"

#include <memory>

class Expression {
public:
    virtual ~Expression() = default;

    class Context {
    public:
        virtual ~Context() = default;
        virtual Value fieldValue(unsigned int field) = 0;
    };
    virtual Value evaluate(Context &context) = 0;
};

class CompareExpression : public Expression {
public:
    enum CompareType {
        LessThan,
        LessThanEqual,
        Equal,
        NotEqual,
        GreaterThan,
        GreaterThanEqual,
    };

    CompareExpression(CompareType compareType, std::unique_ptr<Expression> leftOperand, std::unique_ptr<Expression> rightOperand);
    Value evaluate(Context &context) override;

private:
    CompareType mCompareType;
    std::unique_ptr<Expression> mLeftOperand;
    std::unique_ptr<Expression> mRightOperand;
};

class LogicalExpression : public Expression {
public:
    enum LogicalType {
        And,
        Or,
        Not
    };

    LogicalExpression(LogicalType logicalType, std::unique_ptr<Expression> leftOperand, std::unique_ptr<Expression> rightOperand);
    Value evaluate(Context &context) override;

private:
    LogicalType mLogicalType;
    std::unique_ptr<Expression> mLeftOperand;
    std::unique_ptr<Expression> mRightOperand;
};

class ArithmeticExpression : public Expression {
public:
    enum ArithmeticType {
        Add,
        Subtract,
        Multiply,
        Divide
    };

    ArithmeticExpression(ArithmeticType arithmeticType, std::unique_ptr<Expression> leftOperand, std::unique_ptr<Expression> rightOperand);
    Value evaluate(Context &context) override;

private:
    ArithmeticType mArithmeticType;
    std::unique_ptr<Expression> mLeftOperand;
    std::unique_ptr<Expression> mRightOperand;
};

class ConstantExpression : public Expression {
public:
    ConstantExpression(Value value);
    Value evaluate(Context &context) override;

private:
    Value mValue;
};

class FieldExpression : public Expression {
public:
    FieldExpression(unsigned int field);
    Value evaluate(Context &context) override;

private:
    unsigned int mField;
};

#endif