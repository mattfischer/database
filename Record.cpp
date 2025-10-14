#include "Record.hpp"

#include <iostream>

RecordWriter::RecordWriter(const RecordSchema &schema)
: mSchema(schema)
{
    mValues.resize(mSchema.fields.size());
}

void RecordWriter::setField(unsigned int index, const Field &value)
{
    mValues[index] = value;
}

unsigned int RecordWriter::dataSize()
{
    unsigned int size = mSchema.fields.size() * sizeof(uint16_t);
    for(unsigned int i=0; i<mSchema.fields.size(); i++) {
        const RecordSchema::Field &field = mSchema.fields[i];
        switch(field.type) {
            case RecordSchema::Field::Int:
                size += sizeof(int);
                break;
            case RecordSchema::Field::Float:
                size += sizeof(float);
                break;
            case RecordSchema::Field::String:
                size += mValues[i].stringValue.size() + 1;
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
            case RecordSchema::Field::Int:
                dataOffset += sizeof(int);
                break;
            case RecordSchema::Field::Float:
                dataOffset += sizeof(float);
                break;
            case RecordSchema::Field::String:
                dataOffset += mValues[i].stringValue.size() + 1;
                break;
        }
    }

    for(unsigned int i=0; i<mSchema.fields.size(); i++) {
        const RecordSchema::Field &field = mSchema.fields[i];
        switch(field.type) {
            case RecordSchema::Field::Int:
                *reinterpret_cast<int*>(current) = mValues[i].intValue;
                current += sizeof(int);
                break;
            case RecordSchema::Field::Float:
                *reinterpret_cast<float*>(current) = mValues[i].floatValue;
                current += sizeof(float);
                break;
            case RecordSchema::Field::String:
                strcpy((char*)current, mValues[i].stringValue.data());
                current += mValues[i].stringValue.size() + 1;
                break;
        }
    }
}

RecordReader::RecordReader(const RecordSchema &schema, const void *data)
: mSchema(schema)
{
    mData = reinterpret_cast<const uint8_t*>(data);
}

int RecordReader::readInt(unsigned int index)
{
    const uint16_t *offsets = reinterpret_cast<const uint16_t*>(mData);
    return *reinterpret_cast<const int*>(mData + offsets[index]);
}

float RecordReader::readFloat(unsigned int index)
{
    const uint16_t *offsets = reinterpret_cast<const uint16_t*>(mData);
    return *reinterpret_cast<const float*>(mData + offsets[index]);
}

std::string RecordReader::readString(unsigned int index)
{
    const uint16_t *offsets = reinterpret_cast<const uint16_t*>(mData);
    return std::string(reinterpret_cast<const char*>(mData + offsets[index]));
}

void RecordReader::print()
{
    for(unsigned int i=0; i<mSchema.fields.size(); i++) {
        switch(mSchema.fields[i].type) {
            case RecordSchema::Field::Int:
                std::cout << readInt(i) << " ";
                break;
            case RecordSchema::Field::Float:
                std::cout << readFloat(i) << " ";
                break;
            case RecordSchema::Field::String:
                std::cout << "\"" << readString(i) << "\" ";
                break;
        }
    }
}