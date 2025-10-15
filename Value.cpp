#include "Value.hpp"

#include <iostream>

Value::Value(Type type)
: mType(type)
{
}

Value::Type Value::type()
{
    return mType;
}

void Value::setIntValue(int value)
{
    mValue = value;
}

void Value::setFloatValue(float value)
{
    mValue = value;
}

void Value::setStringValue(const std::string &value)
{
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
            std::cout << "\"" << stringValue() << "\" ";  
            break;
    }
}