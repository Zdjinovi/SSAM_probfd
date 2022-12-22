#ifndef PROBFD_PROBABILISTIC_TASK_H
#define PROBFD_PROBABILISTIC_TASK_H

#include "operator_id.h"

#include "algorithms/subscriber.h"
#include "utils/hash.h"

#include "abstract_task.h"

#include "probfd/value_type.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace options {
class Options;
}

namespace probfd {

class ProbabilisticTask
    : public subscriber::SubscriberService<ProbabilisticTask> {
public:
    ProbabilisticTask() = default;
    virtual ~ProbabilisticTask() override = default;

    virtual int get_num_variables() const = 0;
    virtual std::string get_variable_name(int var) const = 0;
    virtual int get_variable_domain_size(int var) const = 0;
    virtual int get_variable_axiom_layer(int var) const = 0;
    virtual int get_variable_default_axiom_value(int var) const = 0;

    virtual std::string get_fact_name(const FactPair& fact) const = 0;
    virtual bool
    are_facts_mutex(const FactPair& fact1, const FactPair& fact2) const = 0;

    virtual int get_operator_cost(int index, bool is_axiom) const = 0;
    virtual std::string get_operator_name(int index, bool is_axiom) const = 0;
    virtual int get_num_operators() const = 0;
    virtual int
    get_num_operator_preconditions(int index, bool is_axiom) const = 0;
    virtual FactPair
    get_operator_precondition(int op_index, int fact_index, bool is_axiom)
        const = 0;

    virtual int
    get_num_operator_outcomes(int op_index, bool is_axiom) const = 0;

    virtual value_type::value_t get_operator_outcome_probability(
        int op_index,
        int outcome_index,
        bool is_axiom) const = 0;

    virtual int get_num_operator_outcome_effects(
        int op_index,
        int outcome_index,
        bool is_axiom) const = 0;
    virtual FactPair get_operator_outcome_effect(
        int op_index,
        int outcome_index,
        int eff_index,
        bool is_axiom) const = 0;

    virtual int get_num_operator_outcome_effect_conditions(
        int op_index,
        int outcome_index,
        int eff_index,
        bool is_axiom) const = 0;
    virtual FactPair get_operator_outcome_effect_condition(
        int op_index,
        int outcome_index,
        int eff_index,
        int cond_index,
        bool is_axiom) const = 0;

    virtual int get_num_axioms() const = 0;

    virtual int get_num_goals() const = 0;
    virtual FactPair get_goal_fact(int index) const = 0;

    virtual std::vector<int> get_initial_state_values() const = 0;

    /*
     * Convert state values from an ancestor task A (ancestor) into
     * state values from this task, C (child). Task A has to be an
     * ancestor of C in the sense that C is the result of a sequence of
     * task transformations on A.
     * The values are converted in-place to avoid unnecessary copies. If a
     * subclass needs to create a new vector, e.g., because the size changes,
     * it should create the new vector in a local variable and then swap it with
     * the parameter.
     */
    virtual void convert_ancestor_state_values(
        std::vector<int>& values,
        const ProbabilisticTask* ancestor_task) const = 0;

    /*
     * Convert an operator index from this task, C (child), into an operator
     * index from an ancestor task A (ancestor). Task A has to be an ancestor of
     * C in the sense that C is the result of a sequence of task transformations
     * on A.
     */
    virtual int
    convert_operator_index(int index, const ProbabilisticTask* ancestor_task)
        const = 0;

    std::unique_ptr<AbstractTask> build_all_outcomes_determinization();
};

} // namespace probfd

#endif
