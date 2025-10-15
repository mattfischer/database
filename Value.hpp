#ifndef VALUE_HPP
#define VALUE_HPP

#include <string>
#include <variant>

class Value {
public:
    enum Type {
        Int,
        Float,
        String
    };

    Value() = default;
    Value(Type type);

    Type type();

    void setIntValue(int value);
    void setFloatValue(float value);
    void setStringValue(const std::string &value);

    int intValue();
    float floatValue();
    const std::string &stringValue();

private:
    Type mType;
    std::variant<int, float, std::string> mValue;
};
#endif