#ifndef EXPRESSION_HPP
#define EXPRESSION_HPP

#include "Value.hpp"

#include <memory>
#include <string>

class Expression {
public:
    virtual ~Expression() = default;

    class EvaluateContext {
    public:
        virtual ~EvaluateContext() = default;
        virtual Value fieldValue(unsigned int field) = 0;
    };
    virtual Value evaluate(EvaluateContext &context) = 0;

    class BindContext {
    public:
        virtual ~BindContext() = default;
        virtual std::tuple<int, Value::Type> field(const std::string &name) = 0;
    };

    struct BindError {
        std::string name;
    };

    virtual void bind(BindContext &context) = 0;
    virtual Value::Type type() = 0;
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
    Value evaluate(EvaluateContext &context) override;
    void bind(BindContext &context) override;
    Value::Type type() override;

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
    Value evaluate(EvaluateContext &context) override;
    void bind(BindContext &context) override;
    Value::Type type() override;

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
        Divide,
        Negate
    };

    ArithmeticExpression(ArithmeticType arithmeticType, std::unique_ptr<Expression> leftOperand, std::unique_ptr<Expression> rightOperand);
    Value evaluate(EvaluateContext &context) override;
    void bind(BindContext &context) override;
    Value::Type type() override;

private:
    ArithmeticType mArithmeticType;
    std::unique_ptr<Expression> mLeftOperand;
    std::unique_ptr<Expression> mRightOperand;
};

class ConstantExpression : public Expression {
public:
    ConstantExpression(Value value);
    Value evaluate(EvaluateContext &context) override;
    void bind(BindContext &context) override;
    Value::Type type() override;

private:
    Value mValue;
};

class FieldExpression : public Expression {
public:
    FieldExpression(int field);
    FieldExpression(const std::string &name);

    const std::string &name();

    Value evaluate(EvaluateContext &context) override;
    void bind(BindContext &context) override;
    Value::Type type() override;

private:
    int mField;
    std::string mName;
    Value::Type mType;
};

#endif