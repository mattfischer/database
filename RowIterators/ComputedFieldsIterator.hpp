#ifndef ROWITERATORS_COMPUTEDFIELDSITERATOR_HPP
#define ROWITERATORS_COMPUTEDFIELDSITERATOR_HPP

#include "RowIterator.hpp"
#include "Expression.hpp"

namespace RowIterators {
    class ComputedFieldsIterator : public RowIterator {
    public:
        struct ComputedField {
            std::string name;
            Value::Type type;
            std::unique_ptr<Expression> expression;
        };

        ComputedFieldsIterator(RowIterator &inputIterator, std::vector<ComputedField> computedFields);

        RecordSchema &schema() override;

        void start() override;
        bool valid() override;
        void next() override;

        Value getField(unsigned int index) override;

    private:
        void updateComputedFields();

        RecordSchema mSchema;
        RowIterator &mInputIterator;
        std::vector<ComputedField> mComputedFields;
        std::vector<Value> mComputedValues;
    };
}

#endif