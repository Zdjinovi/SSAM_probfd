#include "probfd/heuristics/dead_end_pruning.h"

#include "probfd/cost_function.h"

#include "downward/utils/system.h"

#include "downward/evaluation_context.h"
#include "downward/evaluation_result.h"
#include "downward/evaluator.h"

#include "downward/plugins/plugin.h"

namespace probfd {
namespace heuristics {

DeadEndPruningHeuristic::DeadEndPruningHeuristic(
    std::shared_ptr<::Evaluator> pruning_function,
    value_t dead_end_value)
    : pruning_function_(std::move(pruning_function))
    , dead_end_value_(dead_end_value)
{
    if (!pruning_function_->dead_ends_are_reliable()) {
        utils::g_log << "Dead end pruning heuristic was constructed with an "
                        "evaluator that has unreliable dead ends!"
                     << std::endl;
        utils::exit_with(utils::ExitCode::SEARCH_INPUT_ERROR);
    }
}

EvaluationResult DeadEndPruningHeuristic::evaluate(const State& state) const
{
    EvaluationContext context(state);
    ::EvaluationResult result = pruning_function_->compute_result(context);

    if (result.is_infinite()) {
        return EvaluationResult(true, dead_end_value_);
    }
    return EvaluationResult(false, 0_vt);
}

void DeadEndPruningHeuristic::print_statistics() const
{
    // pruning_function_->print_statistics();
}

namespace {
class DeadEndPruningHeuristicFactory : public TaskEvaluatorFactory {
    const std::shared_ptr<::Evaluator> evaluator_;

public:
    /**
     * @brief Construct from options.
     *
     * @param opts - Only one option is available:
     * + heuristic - Specifies the underlying classical heuristic.
     */
    explicit DeadEndPruningHeuristicFactory(const plugins::Options& opts);

    std::unique_ptr<TaskEvaluator> create_evaluator(
        std::shared_ptr<ProbabilisticTask> task,
        std::shared_ptr<TaskCostFunction> task_cost_function) override;
};

DeadEndPruningHeuristicFactory::DeadEndPruningHeuristicFactory(
    const plugins::Options& opts)
    : evaluator_(opts.get<std::shared_ptr<::Evaluator>>("evaluator"))
{
}

std::unique_ptr<TaskEvaluator> DeadEndPruningHeuristicFactory::create_evaluator(
    std::shared_ptr<ProbabilisticTask>,
    std::shared_ptr<TaskCostFunction> task_cost_function)
{
    return std::make_unique<DeadEndPruningHeuristic>(
        evaluator_,
        task_cost_function->get_non_goal_termination_cost());
}

class DeadEndPruningHeuristicFactoryFeature
    : public plugins::
          TypedFeature<TaskEvaluatorFactory, DeadEndPruningHeuristicFactory> {
public:
    DeadEndPruningHeuristicFactoryFeature()
        : TypedFeature("prune_dead_ends")
    {
        add_option<std::shared_ptr<::Evaluator>>("evaluator");
    }
};

static plugins::FeaturePlugin<DeadEndPruningHeuristicFactoryFeature> _plugin;
} // namespace

} // namespace heuristics
} // namespace probfd
