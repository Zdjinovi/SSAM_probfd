#ifndef MDPS_HEURISTICS_PDBS_MAXPROB_PROJECTION_H
#define MDPS_HEURISTICS_PDBS_MAXPROB_PROJECTION_H

#include "../../globals.h"
#include "../../value_utils.h"

#include "../constant_evaluator.h"

#include "abstract_policy.h"
#include "probabilistic_projection.h"

namespace pdbs {
class PatternDatabase;
}

namespace probabilistic {
namespace pdbs {

class MaxProbProjection : public pdbs::ProbabilisticProjection {
    std::vector<value_utils::IntervalValue> value_table;

public:
    MaxProbProjection(
        const Pattern& pattern,
        const std::vector<int>& domains = ::g_variable_domain,
        const AbstractStateEvaluator& heuristic =
            ConstantEvaluator<AbstractState>(value_type::one));

    MaxProbProjection(
        AbstractStateMapper* mapper,
        const AbstractStateEvaluator& heuristic =
            ConstantEvaluator<AbstractState>(value_type::one));

    MaxProbProjection(const ::pdbs::PatternDatabase& pdb);

    MaxProbProjection(const MaxProbProjection& pdb, int add_var);

    [[nodiscard]] value_type::value_t lookup(const GlobalState& s) const;
    [[nodiscard]] value_type::value_t lookup(const AbstractState& s) const;

    [[nodiscard]] EvaluationResult evaluate(const GlobalState& s) const;
    [[nodiscard]] EvaluationResult evaluate(const AbstractState& s) const;

    AbstractPolicy get_optimal_abstract_policy() const;

    void dump_graphviz(
        const std::string& path,
        bool transition_labels = true,
        bool values = true);

private:
    void compute_value_table(const AbstractStateEvaluator& heuristic);

#ifndef NDEBUG
    void verify(const StateIDMap<AbstractState>& state_id_map);
#endif
};

} // namespace pdbs
} // namespace probabilistic

#endif // __MAXPROB_PROJECTION_H__