#ifndef PROBFD_SUCCESSOR_SAMPLERS_UNIFORM_SUCCESSOR_SAMPLER_H
#define PROBFD_SUCCESSOR_SAMPLERS_UNIFORM_SUCCESSOR_SAMPLER_H

#include "probfd/algorithms/successor_sampler.h"

#include <memory>

namespace plugins {
class Options;
}

namespace utils {
class RandomNumberGenerator;
}

namespace probfd {
namespace successor_samplers {

template <typename Action>
class UniformSuccessorSampler : public algorithms::SuccessorSampler<Action> {
    std::shared_ptr<utils::RandomNumberGenerator> rng_;

public:
    explicit UniformSuccessorSampler(const plugins::Options& opts);

    explicit UniformSuccessorSampler(
        std::shared_ptr<utils::RandomNumberGenerator> rng);

protected:
    StateID sample(
        StateID state,
        Action action,
        const Distribution<StateID>& successors,
        algorithms::StateProperties& properties) override final;
};

} // namespace successor_samplers
} // namespace probfd

#include "probfd/successor_samplers/uniform_successor_sampler_impl.h"

#endif // __UNIFORM_SUCCESSOR_SAMPLER_H__