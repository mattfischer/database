#include "Value.hpp"

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

