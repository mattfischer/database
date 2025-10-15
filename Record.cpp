#include "Record.hpp"

#include <iostream>

RecordWriter::RecordWriter(const RecordSchema &schema)
: mSchema(schema)
{
    mValues.resize(mSchema.fields.size());
}

void RecordWriter::setField(unsigned int index, const Value &value)
{
    mValues[index] = value;
}

unsigned int RecordWriter::dataSize()
{
    unsigned int size = mSchema.fields.size() * sizeof(uint16_t);
    for(unsigned int i=0; i<mSchema.fields.size(); i++) {
        const RecordSchema::Field &field = mSchema.fields[i];
        switch(field.type) {
            case Value::Type::Int:
                size += sizeof(int);
                break;
            case Value::Type::Float:
                size += sizeof(float);
                break;
            case Value::Type::String:
                size += mValues[i].stringValue().size() + 1;
                break;
        }
    }

    return size;
}

void RecordWriter::write(void *data)
{
    uint16_t dataOffset = mSchema.fields.size() * sizeof(uint16_t);

    uint8_t *current = reinterpret_cast<uint8_t*>(data);
    for(unsigned int i=0; i<mSchema.fields.size(); i++) {
        *reinterpret_cast<uint16_t*>(current) = dataOffset;
        current += sizeof(uint16_t);

        const RecordSchema::Field &field = mSchema.fields[i];
        switch(field.type) {
            case Value::Type::Int:
                dataOffset += sizeof(int);
                break;
            case Value::Type::Float:
                dataOffset += sizeof(float);
                break;
            case Value::Type::String:
                dataOffset += mValues[i].stringValue().size() + 1;
                break;
        }
    }

    for(unsigned int i=0; i<mSchema.fields.size(); i++) {
        const RecordSchema::Field &field = mSchema.fields[i];
        switch(field.type) {
            case Value::Type::Int:
                *reinterpret_cast<int*>(current) = mValues[i].intValue();
                current += sizeof(int);
                break;
            case Value::Type::Float:
                *reinterpret_cast<float*>(current) = mValues[i].floatValue();
                current += sizeof(float);
                break;
            case Value::Type::String:
                strcpy((char*)current, mValues[i].stringValue().data());
                current += mValues[i].stringValue().size() + 1;
                break;
        }
    }
}

RecordReader::RecordReader(const RecordSchema &schema, const void *data)
: mSchema(schema)
{
    mData = reinterpret_cast<const uint8_t*>(data);
}

Value RecordReader::readField(unsigned int index)
{
    Value value(mSchema.fields[index].type);

    const uint16_t *offsets = reinterpret_cast<const uint16_t*>(mData);
    switch(value.type()) {
        case Value::Type::Int:
            value.setIntValue(*reinterpret_cast<const int*>(mData + offsets[index]));
            break;

        case Value::Type::Float:
            value.setFloatValue(*reinterpret_cast<const float*>(mData + offsets[index]));
            break;

        case Value::Type::String:
            value.setStringValue(std::string(reinterpret_cast<const char*>(mData + offsets[index])));
            break;
    }

    return value;
}

void RecordReader::print()
{
    for(unsigned int i=0; i<mSchema.fields.size(); i++) {
        Value value = readField(i);
        value.print();
        std::cout << " ";
    }
}