#ifndef ROWITERATORS_PROJECTITERATOR_HPP
#define ROWITERATORS_PROJECTITERATOR_HPP

#include "RowIterator.hpp"
#include "Expression.hpp"

namespace RowIterators {
    class ProjectIterator : public RowIterator {
    public:
        struct FieldDefinition {
            std::string name;
            std::unique_ptr<Expression> expression;
        };

        ProjectIterator(std::unique_ptr<RowIterator> inputIterator, std::vector<FieldDefinition> fieldDefinitions);

        Record::Schema &schema() override;

        void start() override;
        bool valid() override;
        void next() override;
        bool remove() override;
        bool modify(const std::vector<ModifyEntry> &entries) override;

        Value getField(unsigned int index) override;

    private:
        void updateValues();

        Record::Schema mSchema;
        std::unique_ptr<RowIterator> mInputIterator;
        std::vector<FieldDefinition> mFields;
        std::vector<Value> mValues;
    };
}

#endif