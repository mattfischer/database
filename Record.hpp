#ifndef RECORD_HPP
#define RECORD_HPP

#include <string>
#include <vector>

struct RecordSchema {
    struct Field {
        enum Type {
            Int,
            Float,
            String
        };

        Type type;
        std::string name;
    };

    std::vector<Field> fields;
};

class RecordWriter {
public:
    RecordWriter(const RecordSchema &schema);

    struct Field {
        int intValue;
        float floatValue;
        std::string stringValue;
    };

    void setField(unsigned int index, const Field &value);

    unsigned int dataSize();
    void write(void *data);

private:
    const RecordSchema &mSchema;
    std::vector<Field> mValues;
    uint16_t mExtraOffset;
};

class RecordReader {
public:
    RecordReader(const RecordSchema &schema, const void *data);

    int readInt(unsigned int index);
    float readFloat(unsigned int index);
    std::string readString(unsigned int index);

    void print();

private:
    const RecordSchema &mSchema;
    const uint8_t *mData;
};

#endif