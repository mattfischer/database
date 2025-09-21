#ifndef ROW_HPP
#define ROW_HPP

#include <string>
#include <vector>

struct RowSchema {
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

class RowWriter {
public:
    RowWriter(const RowSchema &schema);

    struct Field {
        int intValue;
        float floatValue;
        std::string stringValue;
    };

    void setField(unsigned int index, const Field &value);

    unsigned int dataSize();
    void write(void *data);

private:
    const RowSchema &mSchema;
    std::vector<Field> mValues;
    uint16_t mExtraOffset;
};

class RowReader {
public:
    RowReader(const RowSchema &schema, const void *data);

    int readInt(unsigned int index);
    float readFloat(unsigned int index);
    std::string readString(unsigned int index);

private:
    const RowSchema &mSchema;
    const uint8_t *mData;
};

#endif