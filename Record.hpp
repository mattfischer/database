#ifndef RECORD_HPP
#define RECORD_HPP

#include <string>
#include <vector>
#include <ranges>

#include "Value.hpp"

namespace Record {
    struct Schema {
        struct Field {
            Value::Type type;
            std::string name;
        };

        std::vector<Field> fields;

        int fieldIndex(const std::string &name) {
            auto it = std::ranges::find_if(fields, [&](const auto &a) { return a.name == name; });
            return (it == fields.end()) ? -1 : (it - fields.begin());
        }
    };

    class Writer {
    public:
        Writer(const Schema &schema);

        void setField(unsigned int index, const Value &value);
        Value &field(unsigned int index);

        unsigned int dataSize();
        void write(void *data);

    private:
        const Schema &mSchema;
        std::vector<Value> mValues;
        uint16_t mExtraOffset;
    };

    class Reader {
    public:
        Reader(const Schema &schema, const void *data);

        Value readField(unsigned int index);

        void print();

    private:
        const Schema &mSchema;
        const uint8_t *mData;
    };
}

#endif