#ifndef PROBFD_QUOTIENT_SYSTEM_HEURISTIC_SEARCH_INTERFACE_H
#define PROBFD_QUOTIENT_SYSTEM_HEURISTIC_SEARCH_INTERFACE_H

#include "probfd/engine_interfaces/open_list.h"
#include "probfd/engine_interfaces/policy_picker.h"
#include "probfd/engine_interfaces/successor_sampler.h"

#include "probfd/quotient_system.h"

namespace probfd {

namespace quotients {

template <typename State, typename Action = OperatorID>
class RepresentativePolicyPicker
    : public engine_interfaces::
          PolicyPicker<State, quotients::QuotientAction<Action>> {
    using QuotientSystem = quotients::QuotientSystem<State, Action>;
    using QuotientAction = quotients::QuotientAction<Action>;

    std::vector<Action> choices_;
    std::shared_ptr<engine_interfaces::PolicyPicker<State, Action>> original_;

public:
    RepresentativePolicyPicker(
        std::shared_ptr<engine_interfaces::PolicyPicker<State, Action>>
            original)
        : original_(original)
    {
    }

    int pick_index(
        MDP<State, QuotientAction>& mdp,
        StateID state,
        std::optional<QuotientAction> prev_policy,
        const std::vector<QuotientAction>& action_choices,
        const std::vector<Distribution<StateID>>& successors,
        engine_interfaces::StateProperties& properties) override
    {
        assert(dynamic_cast<QuotientSystem*>(&mdp));
        QuotientSystem& quotient = static_cast<QuotientSystem&>(mdp);

        std::optional<Action> oprev = std::nullopt;

        if (prev_policy) oprev = prev_policy->action;

        choices_.clear();
        choices_.reserve(action_choices.size());
        for (unsigned i = 0; i < action_choices.size(); ++i) {
            choices_.push_back(action_choices[i].action);
        }

        return original_->pick_index(
            *quotient.get_parent_mdp(),
            state,
            oprev,
            choices_,
            successors,
            properties);
    }

    void print_statistics(std::ostream& out) override
    {
        original_->print_statistics(out);
    }
};

template <typename State>
class RepresentativeSuccessorSampler
    : public engine_interfaces::SuccessorSampler<
          quotients::QuotientAction<OperatorID>> {
    using QuotientAction = quotients::QuotientAction<OperatorID>;

    std::shared_ptr<TaskSuccessorSampler> original_;

public:
    RepresentativeSuccessorSampler(
        std::shared_ptr<TaskSuccessorSampler> original)
        : original_(original)
    {
    }

    StateID sample(
        StateID state,
        QuotientAction qaction,
        const Distribution<StateID>& transition,
        engine_interfaces::StateProperties& properties) override
    {
        return original_->sample(state, qaction.action, transition, properties);
    }

    void print_statistics(std::ostream& out) const override
    {
        original_->print_statistics(out);
    }
};

template <typename State>
class RepresentativeOpenList
    : public engine_interfaces::OpenList<
          quotients::QuotientAction<OperatorID>> {
    using QuotientAction = quotients::QuotientAction<OperatorID>;

    std::shared_ptr<TaskOpenList> original_;

public:
    RepresentativeOpenList(std::shared_ptr<TaskOpenList> original)
        : original_(original)
    {
    }

    StateID pop() override { return original_->pop(); }

    void push(StateID state_id) override { original_->push(state_id); }

    /*void
    /push(StateID parent, QuotientAction qaction, value_t prob, StateID
    state_id) override
    {
        original_->push(parent, qaction.action, prob, state_id);
    }*/

    unsigned size() const override { return original_->size(); }

    bool empty() const override { return original_->empty(); }

    void clear() override { original_->clear(); }
};

} // namespace quotients
} // namespace probfd

#endif // __state_properties.h_H__