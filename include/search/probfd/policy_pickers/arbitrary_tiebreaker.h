#ifndef PROBFD_POLICY_PICKER_ARBITRARY_TIEBREAKER_H
#define PROBFD_POLICY_PICKER_ARBITRARY_TIEBREAKER_H

#include "probfd/policy_pickers/stable_policy_picker.h"

namespace plugins {
class Options;
}

namespace probfd {
namespace policy_pickers {

template <typename State, typename Action>
class ArbitraryTiebreaker
    : public StablePolicyPicker<
          State,
          Action,
          ArbitraryTiebreaker<State, Action>> {
public:
    explicit ArbitraryTiebreaker(const plugins::Options& opts);
    explicit ArbitraryTiebreaker(bool stable_policy);

    int pick_index(
        MDP<State, Action>&,
        StateID,
        std::optional<Action>,
        const std::vector<Transition<Action>>&,
        algorithms::StateProperties&) override;
};

} // namespace policy_pickers
} // namespace probfd

#include "probfd/policy_pickers/arbitrary_tiebreaker_impl.h"

#endif // __ARBITRARY_TIEBREAKER_H__