#include "probfd/transition_sampler/uniform_successor_sampler_factory.h"
#include "probfd/transition_sampler/uniform_successor_sampler.h"

#include "utils/rng_options.h"

namespace probfd {
namespace transition_sampler {

UniformSuccessorSamplerFactory::UniformSuccessorSamplerFactory(
    const options::Options& opts)
    : rng(utils::parse_rng_from_options(opts))
{
}

void UniformSuccessorSamplerFactory::add_options_to_parser(
    options::OptionParser& parser)
{
    utils::add_rng_options(parser);
}

std::shared_ptr<engine_interfaces::TransitionSampler<OperatorID>>
UniformSuccessorSamplerFactory::create_sampler(
    engine_interfaces::HeuristicSearchConnector*,
    engine_interfaces::StateIDMap<State>*,
    engine_interfaces::ActionIDMap<OperatorID>*)
{
    return std::make_shared<UniformSuccessorSampler>(rng);
}

} // namespace transition_sampler
} // namespace probfd
