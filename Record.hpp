#ifndef RECORD_HPP
#define RECORD_HPP

#include <string>
#include <vector>

#include "Value.hpp"

struct RecordSchema {
    struct Field {
        Value::Type type;
        std::string name;
    };

    std::vector<Field> fields;
};

class RecordWriter {
public:
    RecordWriter(const RecordSchema &schema);

    void setField(unsigned int index, const Value &value);
    Value &field(unsigned int index);

    unsigned int dataSize();
    void write(void *data);

private:
    const RecordSchema &mSchema;
    std::vector<Value> mValues;
    uint16_t mExtraOffset;
};

class RecordReader {
public:
    RecordReader(const RecordSchema &schema, const void *data);

    Value readField(unsigned int index);

    void print();

private:
    const RecordSchema &mSchema;
    const uint8_t *mData;
};

#endif