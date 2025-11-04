#ifndef ROWITERATORS_PROJECTITERATOR_HPP
#define ROWITERATORS_PROJECTITERATOR_HPP

#include "RowIterator.hpp"

#include <memory>

namespace RowIterators {
    class ProjectIterator : public RowIterator {
    public:
        struct FieldDefinition {
            unsigned int index;
            std::string name;
        };

        ProjectIterator(std::unique_ptr<RowIterator> inputIterator, std::vector<FieldDefinition> fields);

        Record::Schema &schema() override;

        void start() override;
        bool valid() override;
        void next() override;
        bool remove() override;
        bool modify(std::vector<ModifyEntry> entries) override;

        Value getField(unsigned int index) override;

    private:
        std::unique_ptr<RowIterator> mInputIterator;
        std::vector<FieldDefinition> mFields;
        Record::Schema mSchema;
    };
}
#endif