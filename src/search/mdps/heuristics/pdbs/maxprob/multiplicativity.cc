#include "multiplicativity.h"

namespace probabilistic {
namespace pdbs{
namespace multiplicativity {

using namespace syntactic_projection;

using VariableOrthogonality = std::vector<std::vector<bool>>;

namespace {
template <typename T>
bool are_disjoint(const std::set<T>& A, const std::set<T>& B) {
    std::vector<T> intersection;

    std::set_intersection(
        A.cbegin(), A.cend(), B.cbegin(), B.cend(),
        std::back_inserter(intersection));

    return intersection.empty();
}

using probabilistic::pdbs::syntactic_projection::SyntacticProjection;

bool is_pseudo_deterministic(SyntacticProjection op) {
    if (op.size() == 2) {
        auto it = op.begin();

        if (it->first.empty() || (++it)->first.empty()) {
            return true;
        }
    }

    return false;
}

}

std::vector<std::vector<int>> build_compatibility_graph_weak_orthogonality(
    const PatternCollection& patterns)
{
    using namespace syntactic_projection;
    using OperatorID = std::size_t;

    std::vector<std::vector<int>> cgraph;
    cgraph.resize(patterns.size());

    std::vector<std::set<OperatorID>> pattern2proboperators;
    pattern2proboperators.resize(patterns.size());

    // Compute operators that are probabilistic when projected for each pattern
    for (std::size_t i = 0; i != g_operators.size(); ++i) {
        const ProbabilisticOperator& op = g_operators[i];

        // A lot of actions are already deterministic in the first place, so
        // we can save some work
        if (!op.is_stochastic()) {
            continue;
        }

        for (std::size_t j = 0; j != patterns.size(); ++j) {
            const Pattern& pattern = patterns[j];

            // Get the syntactically projected operator
            SyntacticProjection syntactic_proj_op =
                build_syntactic_projection(pattern, op);

            // If the operator is "truly stochastic" add it to the set
            if (is_stochastic(syntactic_proj_op) &&
                !is_pseudo_deterministic(syntactic_proj_op))
            {
                pattern2proboperators[j].insert(i);
            }
        }
    }

    // There is an edge from pattern i to j if they don't have common operators
    // that are probabilistic when projected
    for (std::size_t i = 0; i != patterns.size(); ++i) {
        const auto& prob_operators_i = pattern2proboperators[i];

        for (std::size_t j = i + 1; j != patterns.size(); ++j) {
            const auto& prob_operators_j = pattern2proboperators[j];

            if (are_disjoint(prob_operators_i, prob_operators_j)) {
                cgraph[i].push_back(j);
                cgraph[j].push_back(i);
            }
        }
    }

    return cgraph;
}

}
}
}
