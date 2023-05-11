#ifndef PROBFD_TASK_UTILS_TASK_PROPERTIES_H
#define PROBFD_TASK_UTILS_TASK_PROPERTIES_H

#include "probfd/task_proxy.h"

#include "task_utils/task_properties.h"

#include "operator_cost.h"

namespace probfd {
namespace task_properties {

bool is_unit_cost(const ProbabilisticTaskProxy& task);

value_t get_adjusted_action_cost(
    const ProbabilisticOperatorProxy& op,
    OperatorCost cost_type,
    bool is_unit_cost);

inline bool
is_applicable(const ProbabilisticOperatorProxy& op, const State& state)
{
    for (FactProxy precondition : op.get_preconditions()) {
        if (state[precondition.get_variable()] != precondition) return false;
    }
    return true;
}

template <typename OutputIterator>
void get_affected_vars(
    const ProbabilisticOutcomeProxy& outcome,
    OutputIterator it)
{
    for (const auto& effect : outcome.get_effects()) {
        *it = effect.get_fact().get_variable().get_id();
    }
}

template <typename OutputIterator>
void get_affected_vars(const ProbabilisticOperatorProxy& op, OutputIterator it)
{
    for (const ProbabilisticOutcomeProxy& outcome : op.get_outcomes()) {
        get_affected_vars(outcome, it);
    }
}

// Runtime: O(n), where n is the number of effects.
extern bool has_conditional_effects(const ProbabilisticTaskProxy& task);

/*
  Report an error and exit with ExitCode::UNSUPPORTED if the task has
  conditional effects.
  Runtime: O(n), where n is the number of operators.
*/
extern void verify_no_conditional_effects(const ProbabilisticTaskProxy& task);

/*
  Return the total number of effects of the task, including the
  effects of axioms.
  Runtime: O(m + n), where m is the number of axioms and n is the total number
  of operator effects.
*/
extern int get_num_total_effects(const ProbabilisticTaskProxy& task_proxy);

extern void dump_probabilistic_task(const ProbabilisticTaskProxy& task_proxy);

/*
   Create or retrieve a determinization from cache. If determinizations are
   created with this function, we build at most one determinization per
   ProbabilisticTask.
*/
extern const AbstractTask& get_determinization(const ProbabilisticTask* task);

} // namespace task_properties
} // namespace probfd

#endif
