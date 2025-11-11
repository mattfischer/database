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

std::string QueryParser::expectIdentifier()
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
        throwExpected("<identifier>");
    }

    mPos = pos;
    skipWhitespace();

    return result;
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

std::unique_ptr<Query> QueryParser::parseQuery()
{
    if(matchLiteral("CREATE")) {
        if(matchLiteral("TABLE")) {
            return parseCreateTable();
        } else if(matchLiteral("INDEX")) {
            return parseCreateIndex();
        }

        throwExpected("TABLE | INDEX");
    }

    throwExpected("query");
}

std::unique_ptr<Query> QueryParser::parseCreateTable()
{
    Query::CreateTable createTable;

    createTable.name = expectIdentifier();
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