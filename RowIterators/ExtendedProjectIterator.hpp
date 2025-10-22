#ifndef ROWITERATORS_EXTENDEDPROJECTITERATOR_HPP
#define ROWITERATORS_EXTENDEDPROJECTITERATOR_HPP

#include "RowIterator.hpp"
#include "Expression.hpp"

namespace RowIterators {
    class ExtendedProjectIterator : public RowIterator {
    public:
        struct FieldDefinition {
            std::string name;
            Value::Type type;
            std::unique_ptr<Expression> expression;
        };

        ExtendedProjectIterator(std::unique_ptr<RowIterator> inputIterator, std::vector<FieldDefinition> fieldDefinitions);

        RecordSchema &schema() override;

        void start() override;
        bool valid() override;
        void next() override;

        Value getField(unsigned int index) override;

    private:
        void updateValues();

        RecordSchema mSchema;
        std::unique_ptr<RowIterator> mInputIterator;
        std::vector<FieldDefinition> mFields;
        std::vector<Value> mValues;
    };
}

#endif