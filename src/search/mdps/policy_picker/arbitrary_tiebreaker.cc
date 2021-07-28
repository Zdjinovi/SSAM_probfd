#include "arbitrary_tiebreaker.h"

#include "../../option_parser.h"
#include "../../plugin.h"

namespace probabilistic {
namespace policy_tiebreaking {

ArbitraryTiebreaker::ArbitraryTiebreaker()
{
}

ArbitraryTiebreaker::ArbitraryTiebreaker(const options::Options&)
{
}

void ArbitraryTiebreaker::add_options_to_parser(options::OptionParser&)
{
}

int ArbitraryTiebreaker::pick(
    const StateID&,
    const ActionID&,
    const std::vector<const ProbabilisticOperator*>&,
    const std::vector<Distribution<StateID>>&)
{
    return 0;
}

static Plugin<ProbabilisticOperatorPolicyPicker> _plugin(
    "arbitrary_policy_tiebreaker",
    options::parse<ProbabilisticOperatorPolicyPicker, ArbitraryTiebreaker>);

} // namespace policy_tiebreaking
} // namespace probabilistic
