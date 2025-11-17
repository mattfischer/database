#include "QueryParser.hpp"

#include <sstream>

struct ParseError {
    std::string message;
    unsigned int pos;
};

QueryParser::QueryParser(const std::string &queryString)
: mQueryString(queryString)
{
}

std::unique_ptr<Query> QueryParser::parse()
{
    mPos = 0;
    skipWhitespace();

    try {
        return parseQuery();
    } catch (ParseError err) {
        std::stringstream ss;

        ss << "Error, character " << err.pos << ": " << err.message;
        mErrorMessage = ss.str();
        return nullptr;
    }
}

const std::string &QueryParser::errorMessage()
{
    return mErrorMessage;
}

bool QueryParser::isEnd()
{
    return mPos >= mQueryString.size();
}

void QueryParser::throwExpected(const std::string &expected)
{
    std::stringstream ss;
    ss << "Expected: " << expected;
    throw ParseError { ss.str(), mPos };
}

void QueryParser::skipWhitespace()
{
    while(!isEnd()) {
        if(mQueryString[mPos] == ' ' || mQueryString[mPos] == '\t') {
            mPos++;
            continue;
        } else {
            break;
        }
    }
}

bool QueryParser::matchLiteral(const std::string &literal)
{
    if(mPos + literal.size() > mQueryString.size()) {
        return false;
    }

    if(std::strncmp(&mQueryString[mPos], &literal[0], literal.size()) == 0) {
        mPos += literal.size();
        skipWhitespace();
        return true;
    }

    return false;
}

void QueryParser::expectLiteral(const std::string &literal)
{
    if(!matchLiteral(literal)) {
        throwExpected(literal);
    }
}

std::optional<std::string> QueryParser::matchIdentifier()
{
    int pos = mPos;
    while(pos < mQueryString.size()) {
        if(std::isalnum(mQueryString[pos])) {
            pos++;
            continue;
        } else {
            break;
        }
    }

    std::string result = mQueryString.substr(mPos, pos - mPos);
    if(result.size() == 0) {
        return std::nullopt;
    }

    mPos = pos;
    skipWhitespace();

    return result;
}

std::string QueryParser::expectIdentifier()
{
    auto id = matchIdentifier();
    if(id) {
        return *id;
    } else {
        throwExpected("<identifier>");
    }
}

Value::Type QueryParser::expectType()
{
    Value::Type result;
    if(matchLiteral("INTEGER") || matchLiteral("INT")) {
        result = Value::Type::Int;
    } else if(matchLiteral("BOOLEAN") || matchLiteral("BOOL")) {
        result = Value::Type::Boolean;
    } else if(matchLiteral("FLOAT")) {
        result = Value::Type::Float;
    } else if(matchLiteral("VARCHAR") || matchLiteral("STRING")) {
        result = Value::Type::String;
    } else {
        throwExpected("<type>");
    }

    skipWhitespace();
    return result;
}

std::optional<Value> QueryParser::matchValue()
{
    if(mQueryString[mPos] == '\"') {
        int pos = mPos + 1;
        while(pos < mQueryString.size()) {
            if(mQueryString[pos] == '\"') {
                break;
            }
            pos++;
        }

        if(pos == mQueryString.size()) {
            throw ParseError{"Unterminated string constant"};
        }
        std::string substr = mQueryString.substr(mPos + 1, pos - mPos - 1);
        mPos = pos + 1;
        skipWhitespace();
        return Value(substr);
    } else if(std::isdigit(mQueryString[mPos]) || mQueryString[mPos] == '.') {
        bool isFloat = false;
        int pos = mPos;
        while(pos < mQueryString.size()) {
            if(mQueryString[pos] == '.') {
                isFloat = true;
            } else if(!std::isdigit(mQueryString[pos])) {
                break;
            }
            pos++;
        }

        std::string substr = mQueryString.substr(mPos, pos - mPos);
        mPos = pos;
        skipWhitespace();
        if(isFloat) {
            return Value((float)std::atof(substr.c_str()));
        } else {
            return Value(std::atoi(substr.c_str()));
        }
    } else if(matchLiteral("true")) {
        return Value(true);
    } else if(matchLiteral("false")) {
        return Value(false);
    } else {
        return std::nullopt;
    }
}

Value QueryParser::expectValue()
{
    auto val = matchValue();
    if(val) {
        return *val;
    } else {
        throwExpected("<value>");
    }
}

std::unique_ptr<Query> QueryParser::parseQuery()
{
    if(matchLiteral("CREATE")) {
        if(matchLiteral("TABLE")) {
            return parseCreateTable();
        } else if(matchLiteral("INDEX")) {
            return parseCreateIndex();
        }

        throwExpected("TABLE | INDEX");
    } else if(matchLiteral("INSERT")) {
        expectLiteral("INTO");
        return parseInsertInto();
    } else if(matchLiteral("SELECT")) {
        return parseSelect();
    }

    throwExpected("<query>");
}

std::unique_ptr<Query> QueryParser::parseCreateTable()
{
    Query::CreateTable createTable;

    createTable.tableName = expectIdentifier();
    expectLiteral("(");
    while(!matchLiteral(")")) {
        Value::Type type = expectType();
        std::string name = expectIdentifier();
        createTable.schema.fields.push_back({type, name});

        if(matchLiteral(",")) {
            continue;
        }
    }

    auto query = std::make_unique<Query>();
    query->type = Query::Type::CreateTable;
    query->query = std::move(createTable);

    return query;    
}

std::unique_ptr<Query> QueryParser::parseCreateIndex()
{
    Query::CreateIndex createIndex;

    createIndex.indexName = expectIdentifier();
    expectLiteral("ON");
    createIndex.tableName = expectIdentifier();

    expectLiteral("(");
    while(!matchLiteral(")")) {
        std::string name = expectIdentifier();
        createIndex.columns.push_back(name);

        if(matchLiteral(",")) {
            continue;
        }
    }

    auto query = std::make_unique<Query>();
    query->type = Query::Type::CreateIndex;
    query->query = std::move(createIndex);

    return query;    
}

std::unique_ptr<Query> QueryParser::parseInsertInto()
{
    Query::InsertInto insertInto;

    insertInto.tableName = expectIdentifier();
    expectLiteral("VALUES");

    expectLiteral("(");
    while(!matchLiteral(")")) {
        Value value = expectValue();
        insertInto.values.push_back(value);

        if(matchLiteral(",")) {
            continue;
        }
    }

    auto query = std::make_unique<Query>();
    query->type = Query::Type::InsertInto;
    query->query = std::move(insertInto);

    return query;
}

std::unique_ptr<Query> QueryParser::parseSelect()
{
    Query::Select select;

    expectLiteral("*");
    while(true) {
        if(matchLiteral("FROM")) {
            select.tableName = expectIdentifier();
        } else if(matchLiteral("WHERE")) {
            select.predicate = expectExpression();
        } else {
            break;
        }
    }

    auto query = std::make_unique<Query>();
    query->type = Query::Type::Select;
    query->query = std::move(select);

    return query;
}

std::unique_ptr<Expression> QueryParser::expectExpression()
{
    return parseAndExpression();
}

std::unique_ptr<Expression> QueryParser::parseOrExpression()
{
    std::unique_ptr<Expression> result = parseAndExpression();
    while(matchLiteral("||")) {
        auto rhs = parseAndExpression();
        result = std::make_unique<LogicalExpression>(LogicalExpression::Or, std::move(result), std::move(rhs));
    }

    return result;
}

std::unique_ptr<Expression> QueryParser::parseAndExpression()
{
    std::unique_ptr<Expression> result = parseCompareExpression();
    while(matchLiteral("&&")) {
        auto rhs = parseCompareExpression();
        result = std::make_unique<LogicalExpression>(LogicalExpression::And, std::move(result), std::move(rhs));
    }

    return result;
}

std::unique_ptr<Expression> QueryParser::parseCompareExpression()
{
    std::unique_ptr<Expression> result = parseAddSubExpression();
    if(matchLiteral("<=")) {
        auto rhs = parseAddSubExpression();
        result = std::make_unique<CompareExpression>(CompareExpression::LessThanEqual, std::move(result), std::move(rhs));
    } else if(matchLiteral("<")) {
        auto rhs = parseAddSubExpression();
        result = std::make_unique<CompareExpression>(CompareExpression::LessThan, std::move(result), std::move(rhs));
    } else if(matchLiteral("==")) {
        auto rhs = parseAddSubExpression();
        result = std::make_unique<CompareExpression>(CompareExpression::Equal, std::move(result), std::move(rhs));
    } else if(matchLiteral("!=")) {
        auto rhs = parseAddSubExpression();
        result = std::make_unique<CompareExpression>(CompareExpression::NotEqual, std::move(result), std::move(rhs));
    } else if(matchLiteral(">=")) {
        auto rhs = parseAddSubExpression();
        result = std::make_unique<CompareExpression>(CompareExpression::GreaterThanEqual, std::move(result), std::move(rhs));
    } else if(matchLiteral(">")) {
        auto rhs = parseAddSubExpression();
        result = std::make_unique<CompareExpression>(CompareExpression::GreaterThan, std::move(result), std::move(rhs));
    }

    return result;
}

std::unique_ptr<Expression> QueryParser::parseAddSubExpression()
{
    std::unique_ptr<Expression> result = parseMulDivExpression();
    while(true) {
        if(matchLiteral("+")) {
            auto rhs = parseMulDivExpression();
            result = std::make_unique<ArithmeticExpression>(ArithmeticExpression::Add, std::move(result), std::move(rhs));
        } else if(matchLiteral("-")) {
            auto rhs = parseMulDivExpression();
            result = std::make_unique<ArithmeticExpression>(ArithmeticExpression::Subtract, std::move(result), std::move(rhs));
        } else {
            break;
        }
    }

    return result;
}

std::unique_ptr<Expression> QueryParser::parseMulDivExpression()
{
    std::unique_ptr<Expression> result = parseUnaryExpression();
    while(true) {
        if(matchLiteral("*")) {
            auto rhs = parseUnaryExpression();
            result = std::make_unique<ArithmeticExpression>(ArithmeticExpression::Multiply, std::move(result), std::move(rhs));
        } else if(matchLiteral("/")) {
            auto rhs = parseUnaryExpression();
            result = std::make_unique<ArithmeticExpression>(ArithmeticExpression::Divide, std::move(result), std::move(rhs));
        } else {
            break;
        }
    }

    return result;
}

std::unique_ptr<Expression> QueryParser::parseUnaryExpression()
{
    if(matchLiteral("!")) {
        auto arg = parseUnaryExpression();
        return std::make_unique<LogicalExpression>(LogicalExpression::Not, std::move(arg), nullptr);
    } else if(matchLiteral("-")) {
        auto arg = parseUnaryExpression();
        return std::make_unique<ArithmeticExpression>(ArithmeticExpression::Negate, std::move(arg), nullptr);
    } else {
        return parseBaseExpression();
    }
}

std::unique_ptr<Expression> QueryParser::parseBaseExpression()
{
    if(matchLiteral("(")) {
        auto exp = expectExpression();
        expectLiteral(")");
        return exp;
    } else if(auto value = matchValue()) {
        return std::make_unique<ConstantExpression>(*value);
    } else if(auto id = matchIdentifier()) {
        return std::make_unique<FieldExpression>(*id);
    } else {
        throwExpected("<expression>");
    }
}