#include "probfd/transition_sampler/arbitrary_selector_factory.h"
#include "probfd/transition_sampler/most_likely_selector_factory.h"
#include "probfd/transition_sampler/random_successor_sampler_factory.h"
#include "probfd/transition_sampler/uniform_successor_sampler_factory.h"
#include "probfd/transition_sampler/vbiased_successor_sampler_factory.h"
#include "probfd/transition_sampler/vdiff_successor_sampler_factory.h"

#include "option_parser.h"
#include "plugin.h"

namespace probfd {
namespace transition_sampler {

static PluginTypePlugin<TaskTransitionSamplerFactory>
    _type_plugin("TaskTransitionSamplerFactory", "");

static Plugin<TaskTransitionSamplerFactory> _plugin_arbitary(
    "arbitrary_successor_selector_factory",
    options::parse_without_options<
        TaskTransitionSamplerFactory,
        ArbitrarySuccessorSelectorFactory>);

static Plugin<TaskTransitionSamplerFactory> _plugin_most_likely(
    "most_likely_successor_selector_factory",
    options::parse_without_options<
        TaskTransitionSamplerFactory,
        MostLikelySuccessorSelectorFactory>);

static Plugin<TaskTransitionSamplerFactory> _plugin_random(
    "random_successor_sampler_factory",
    options::
        parse<TaskTransitionSamplerFactory, RandomSuccessorSamplerFactory>);

static Plugin<TaskTransitionSamplerFactory> _plugin_uniform(
    "uniform_random_successor_sampler_factory",
    options::
        parse<TaskTransitionSamplerFactory, UniformSuccessorSamplerFactory>);

static Plugin<TaskTransitionSamplerFactory> _plugin_vbiased(
    "vbiased_successor_sampler_factory",
    options::
        parse<TaskTransitionSamplerFactory, VBiasedSuccessorSamplerFactory>);

static Plugin<TaskTransitionSamplerFactory> _plugin_value_gap(
    "value_gap_successor_sampler_factory",
    options::parse<TaskTransitionSamplerFactory, VDiffSuccessorSamplerFactory>);

} // namespace transition_sampler
} // namespace probfd