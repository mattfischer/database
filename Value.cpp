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

Value::Value(bool value)
{
    mType = Type::Boolean;
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

void Value::setValue(bool value)
{
    mType = Type::Boolean;
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

bool Value::booleanValue()
{
    return std::get<bool>(mValue);
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

        case Value::Type::Boolean:
            std::cout << (booleanValue() ? "true" : "false");
            break;
    }
}

bool Value::operator<(Value &other)
{
    switch(type()) {
        case Type::Int:
            return intValue() < other.intValue();
        case Type::Float:
            return floatValue() < other.floatValue();
        case Type::String:
            return stringValue() < other.stringValue();
        default:
            return false;
    }
}

bool Value::operator<=(Value &other)
{
    return (*this < other) || (*this == other);
}

bool Value::operator==(Value &other)
{
    switch(type()) {
        case Type::Int:
            return intValue() == other.intValue();
        case Type::Float:
            return floatValue() == other.floatValue();
        case Type::String:
            return stringValue() == other.stringValue();
        case Type::Boolean:
            return booleanValue() == other.booleanValue();
    }
}

bool Value::operator!=(Value &other)
{
    return !(*this == other);
}

bool Value::operator>=(Value &other)
{
    return (*this > other) || (*this == other);
}

bool Value::operator>(Value &other)
{
    switch(type()) {
        case Type::Int:
            return intValue() > other.intValue();
        case Type::Float:
            return floatValue() > other.floatValue();
        case Type::String:
            return stringValue() > other.stringValue();
        default:
            return false;
    }
}

Value Value::operator+(Value &other)
{
    switch(type()) {
        case Type::Int:
            return Value(intValue() + other.intValue());
        case Type::Float:
            return Value(floatValue() + other.floatValue());
    }
    return Value();
}

Value Value::operator-(Value &other)
{
    switch(type()) {
        case Type::Int:
            return Value(intValue() - other.intValue());
        case Type::Float:
            return Value(floatValue() - other.floatValue());
    }
    return Value();
}

Value Value::operator*(Value &other)
{
    switch(type()) {
        case Type::Int:
            return Value(intValue() * other.intValue());
        case Type::Float:
            return Value(floatValue() * other.floatValue());
    }
    return Value();
}

Value Value::operator/(Value &other)
{
    switch(type()) {
        case Type::Int:
            return Value(intValue() / other.intValue());
        case Type::Float:
            return Value(floatValue() / other.floatValue());
    }
    return Value();
}
