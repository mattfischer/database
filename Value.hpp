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
    Value(int value);
    Value(float value);
    Value(const std::string &value);

    Type type();

    void setValue(int value);
    void setValue(float value);
    void setValue(const std::string &value);

    int intValue();
    float floatValue();
    const std::string &stringValue();

    void print();

    bool operator<(Value &other);
    bool operator<=(Value &other);
    bool operator==(Value &other);
    bool operator!=(Value &other);
    bool operator>=(Value &other);
    bool operator>(Value &other);

private:
    Type mType;
    std::variant<int, float, std::string> mValue;
};
#endif