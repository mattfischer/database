#ifndef VALUE_HPP
#define VALUE_HPP

#include <string>
#include <variant>

class Value {
public:
    enum Type {
        Int,
        Float,
        String,
        Boolean
    };

    Value() = default;
    Value(int value);
    Value(float value);
    Value(const std::string &value);
    Value(bool value);

    Type type();

    void setValue(int value);
    void setValue(float value);
    void setValue(const std::string &value);
    void setValue(bool value);

    int intValue();
    float floatValue();
    const std::string &stringValue();
    bool booleanValue();

    void print();

    bool operator<(Value &other);
    bool operator<=(Value &other);
    bool operator==(Value &other);
    bool operator!=(Value &other);
    bool operator>=(Value &other);
    bool operator>(Value &other);

    Value operator+(Value &other);
    Value operator-(Value &other);
    Value operator-();
    Value operator*(Value &other);
    Value operator/(Value &other);

private:
    Type mType;
    std::variant<int, float, std::string, bool> mValue;
};
#endif