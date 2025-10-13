#include "Row.hpp"

#include <iostream>

RowWriter::RowWriter(const RowSchema &schema)
: mSchema(schema)
{
    mValues.resize(mSchema.fields.size());
}

void RowWriter::setField(unsigned int index, const Field &value)
{
    mValues[index] = value;
}

unsigned int RowWriter::dataSize()
{
    unsigned int size = mSchema.fields.size() * sizeof(uint16_t);
    for(unsigned int i=0; i<mSchema.fields.size(); i++) {
        const RowSchema::Field &field = mSchema.fields[i];
        switch(field.type) {
            case RowSchema::Field::Int:
                size += sizeof(int);
                break;
            case RowSchema::Field::Float:
                size += sizeof(float);
                break;
            case RowSchema::Field::String:
                size += mValues[i].stringValue.size() + 1;
                break;
        }
    }

    return size;
}

void RowWriter::write(void *data)
{
    uint16_t dataOffset = mSchema.fields.size() * sizeof(uint16_t);

    uint8_t *current = reinterpret_cast<uint8_t*>(data);
    for(unsigned int i=0; i<mSchema.fields.size(); i++) {
        *reinterpret_cast<uint16_t*>(current) = dataOffset;
        current += sizeof(uint16_t);

        const RowSchema::Field &field = mSchema.fields[i];
        switch(field.type) {
            case RowSchema::Field::Int:
                dataOffset += sizeof(int);
                break;
            case RowSchema::Field::Float:
                dataOffset += sizeof(float);
                break;
            case RowSchema::Field::String:
                dataOffset += mValues[i].stringValue.size() + 1;
                break;
        }
    }

    for(unsigned int i=0; i<mSchema.fields.size(); i++) {
        const RowSchema::Field &field = mSchema.fields[i];
        switch(field.type) {
            case RowSchema::Field::Int:
                *reinterpret_cast<int*>(current) = mValues[i].intValue;
                current += sizeof(int);
                break;
            case RowSchema::Field::Float:
                *reinterpret_cast<float*>(current) = mValues[i].floatValue;
                current += sizeof(float);
                break;
            case RowSchema::Field::String:
                strcpy((char*)current, mValues[i].stringValue.data());
                current += mValues[i].stringValue.size() + 1;
                break;
        }
    }
}

RowReader::RowReader(const RowSchema &schema, const void *data)
: mSchema(schema)
{
    mData = reinterpret_cast<const uint8_t*>(data);
}

int RowReader::readInt(unsigned int index)
{
    const uint16_t *offsets = reinterpret_cast<const uint16_t*>(mData);
    return *reinterpret_cast<const int*>(mData + offsets[index]);
}

float RowReader::readFloat(unsigned int index)
{
    const uint16_t *offsets = reinterpret_cast<const uint16_t*>(mData);
    return *reinterpret_cast<const float*>(mData + offsets[index]);
}

std::string RowReader::readString(unsigned int index)
{
    const uint16_t *offsets = reinterpret_cast<const uint16_t*>(mData);
    return std::string(reinterpret_cast<const char*>(mData + offsets[index]));
}

void RowReader::print()
{
    for(unsigned int i=0; i<mSchema.fields.size(); i++) {
        switch(mSchema.fields[i].type) {
            case RowSchema::Field::Int:
                std::cout << readInt(i) << " ";
                break;
            case RowSchema::Field::Float:
                std::cout << readFloat(i) << " ";
                break;
            case RowSchema::Field::String:
                std::cout << "\"" << readString(i) << "\" ";
                break;
        }
    }
}