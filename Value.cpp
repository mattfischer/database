#include "Value.hpp"

#include <iostream>

Value::Value(int value)
{
    mType = Type::Int;
    mValue = value;
}

Value::Value(float value)
{
    mType = Type::Float;
    mValue = value;
}

Value::Value(const std::string &value)
{
    mType = Type::String;
    mValue = value;
}

Value::Type Value::type()
{
    return mType;
}

void Value::setValue(int value)
{
    mType = Type::Int;
    mValue = value;
}

void Value::setValue(float value)
{
    mType = Type::Float;
    mValue = value;
}

void Value::setValue(const std::string &value)
{
    mType = Type::String;
    mValue = value;
}

int Value::intValue()
{
    return std::get<int>(mValue);
}

float Value::floatValue()
{
    return std::get<float>(mValue);
}

const std::string &Value::stringValue()
{
    return std::get<std::string>(mValue);
}

void Value::print()
{
    switch(type()) {
        case Value::Type::Int:
            std::cout << intValue();
            break;

        case Value::Type::Float:
            std::cout << floatValue();
            break;

        case Value::Type::String:
            std::cout << "\"" << stringValue() << "\"";  
            break;
    }
}