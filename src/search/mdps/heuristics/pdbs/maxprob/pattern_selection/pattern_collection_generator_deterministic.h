#ifndef PATTERN_COLLECTION_GENERATOR_DETERMINISTIC_H_
#define PATTERN_COLLECTION_GENERATOR_DETERMINISTIC_H_

#include "../../../../../pdbs/pattern_generator.h"
#include "pattern_generator.h"

#include "../../../../../options/options.h"

#include <memory>

namespace probabilistic {
namespace pdbs {
namespace maxprob {
namespace pattern_selection {

class PatternCollectionGeneratorDeterministic
    : public PatternCollectionGenerator {
    std::shared_ptr<::pdbs::PatternCollectionGenerator> gen;
    std::shared_ptr<multiplicativity::MultiplicativityStrategy>
        multiplicativity;

    PatternCollectionGeneratorDeterministic(
        std::shared_ptr<::pdbs::PatternCollectionGenerator> gen,
        std::shared_ptr<multiplicativity::MultiplicativityStrategy>
            multiplicativity);

public:
    PatternCollectionGeneratorDeterministic(options::Options& opts);

    virtual ~PatternCollectionGeneratorDeterministic() override = default;

    virtual PatternCollectionInformation
    generate(OperatorCost cost_type) override;

    std::shared_ptr<utils::Printable> get_report() const override;
};

} // namespace pattern_selection
} // namespace maxprob
} // namespace pdbs
} // namespace probabilistic

#endif // PATTERN_COLLECTION_GENERATOR_DETERMINISTIC_H_