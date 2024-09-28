#include "probfd/solvers/mdp_solver.h"

#include "probfd/tasks/root_task.h"

#include "probfd/task_utils/task_properties.h"

#include "probfd/caching_task_state_space.h"

#include "probfd/evaluator.h"
#include "probfd/interval.h"
#include "probfd/mdp_algorithm.h"
#include "probfd/task_cost_function_factory.h"
#include "probfd/task_evaluator_factory.h"

#include "downward/utils/timer.h"

#include "downward/utils/rng.h"
#include "downward/utils/rng_options.h"

#include "downward/plugins/options.h"
#include "downward/plugins/plugin.h"

#include <fstream>
#include <iostream>
#include <limits>
#include <optional>

class Evaluator;
class State;

namespace probfd {
class TaskCostFunctionFactory;
class TaskEvaluatorFactory;
} // namespace probfd

namespace probfd::solvers {

using namespace plugins;

MDPSolver::MDPSolver(const Options& opts)
    : task_(tasks::g_root_task)
    , task_cost_function_(
          opts.get<std::shared_ptr<TaskCostFunctionFactory>>("costs")
              ->create_cost_function(task_))
    , log_(utils::get_log_from_options(opts))
    , task_mdp_(
          opts.get<bool>("cache")
              ? new CachingTaskStateSpace(
                    task_,
                    log_,
                    task_cost_function_,
                    opts.get_list<std::shared_ptr<::Evaluator>>(
                        "path_dependent_evaluators"))
              : new TaskStateSpace(
                    task_,
                    log_,
                    task_cost_function_,
                    opts.get_list<std::shared_ptr<::Evaluator>>(
                        "path_dependent_evaluators")))
    , heuristic_(opts.get<std::shared_ptr<TaskEvaluatorFactory>>("eval")
                     ->create_evaluator(task_, task_cost_function_))
    , progress_(
          opts.contains("report_epsilon")
              ? std::optional<value_t>(opts.get<value_t>("report_epsilon"))
              : std::nullopt,
          std::cout,
          opts.get<bool>("report_enabled"))
    , max_time_(opts.get<double>("max_time"))
    , policy_filename(opts.get<std::string>("policy_file"))
    , print_fact_names(opts.get<bool>("print_fact_names"))
    , trajectories(opts.get<int>("trajectories"))
    , trajectory_length(opts.get<int>("trajectory_length"))
    , rng(utils::parse_rng_from_options(opts))
{
    progress_.register_print([&ss = *this->task_mdp_](std::ostream& out) {
        out << "registered=" << ss.get_num_registered_states();
    });
}

MDPSolver::~MDPSolver() = default;

void MDPSolver::solve()
{
    std::cout << "Running MDP algorithm " << get_algorithm_name();

    if (max_time_ != std::numeric_limits<double>::infinity()) {
        std::cout << " with a time limit of " << max_time_ << " seconds";
    }

    std::cout << "..." << std::endl;

    try {
        utils::Timer total_timer;
        std::unique_ptr<FDRMDPAlgorithm> algorithm = create_algorithm();

        const State& initial_state = task_mdp_->get_initial_state();

        std::unique_ptr<Policy<State, OperatorID>> policy =
            algorithm->compute_policy(
                *task_mdp_,
                *heuristic_,
                initial_state,
                progress_,
                max_time_);
        total_timer.stop();

        std::cout << "analysis done. [t=" << utils::g_timer << "]" << std::endl;

        std::cout << std::endl;

        if (policy) {
            using namespace std;

            print_analysis_result(
                policy->get_decision(initial_state)->q_value_interval);

            auto print_state = [this](const State& state, std::ostream& out) {
                if (print_fact_names) {
                    out << state[0].get_name();
                    for (const FactProxy& fact : state | views::drop(1)) {
                        out << ", " << fact.get_name();
                    }
                } else {
                    out << "{ " << state[0].get_variable().get_id() << " -> "
                        << state[0].get_value();

                    for (const FactProxy& fact : state | views::drop(1)) {
                        const auto [var, val] = fact.get_pair();
                        out << ", " << var << " -> " << val;
                    }
                    out << " }";
                }
            };

            auto print_action =
                [this](const OperatorID& op_id, std::ostream& out) {
                    out << this->task_->get_operator_name(op_id.get_index());
                };

            {
                std::ofstream out(policy_filename);
                policy->print(out, print_state, print_action);
            }

            {
                ProbabilisticTaskProxy task_proxy(*task_);
                ProbabilisticOperatorsProxy operators =
                    task_proxy.get_operators();

                StateRegistry& state_registry = task_mdp_->get_state_registry();

                for (int i = 0; i < trajectories; ++i) {
                    std::ofstream out(
                        std::string("trajectory_") + std::to_string(i) +
                        ".plan");

                    State state = state_registry.get_initial_state();
                    value_t plan_cost = 0;
                    int step = 0;

                    while (auto decision = policy->get_decision(state)) {
                        auto op = operators[decision->action];

                        // Sample outcome
                        int outcome_index = 0;
                        value_t prob_sum = 0_vt;

                        // Ramdom number p in [0, 1)
                        double p = rng->random();
                        for (;;) {
                            prob_sum += op.get_outcomes()[outcome_index]
                                            .get_probability();
                            if (p <= prob_sum) break;
                            ++outcome_index;
                        }

                        auto outcome = op.get_outcomes()[outcome_index];

			print_state(state, out);
			out << "(" << op.get_name() << " [outcome "
                            << outcome_index << "])" << endl;

                        state = state_registry.get_successor_state(
                            state,
                            outcome.get_effects());
                        plan_cost += op.get_cost();

                        if (++step == trajectory_length) break;
                    }

                    print_state(state, out);
		    out << "; cost = " << plan_cost << " ("
                        << (task_properties::is_unit_cost(task_proxy)
                                ? "unit cost"
                                : "general cost")
                        << ")" << endl;
                }
            }
        }

        std::cout << std::endl;
        std::cout << "State space interface:" << std::endl;
        std::cout << "  Registered state(s): "
                  << task_mdp_->get_num_registered_states() << std::endl;
        task_mdp_->print_statistics();

        std::cout << std::endl;
        std::cout << "Algorithm " << get_algorithm_name()
                  << " statistics:" << std::endl;
        std::cout << "  Actual solver time: " << total_timer << std::endl;
        algorithm->print_statistics(std::cout);

        heuristic_->print_statistics();

        print_additional_statistics();
    } catch (utils::TimeoutException&) {
        std::cout << "Time limit reached. Analysis was aborted." << std::endl;
    }
}

void MDPSolver::add_options_to_feature(Feature& feature)
{
    feature.add_option<std::shared_ptr<TaskCostFunctionFactory>>(
        "costs",
        "",
        "ssp()");
    feature.add_option<std::shared_ptr<TaskEvaluatorFactory>>(
        "eval",
        "",
        "blind_eval()");
    feature.add_option<bool>("cache", "", "false");
    feature.add_list_option<std::shared_ptr<::Evaluator>>(
        "path_dependent_evaluators",
        "",
        "[]");
    feature.add_option<value_t>("report_epsilon", "", "1e-4");
    feature.add_option<bool>("report_enabled", "", "true");
    feature.add_option<double>("max_time", "", "infinity");
    feature.add_option<std::string>("policy_file", "", "\"my_policy.policy\"");
    feature.add_option<bool>("print_fact_names", "", "true");
    feature.add_option<int>("trajectories", "", "0");
    feature.add_option<int>("trajectory_length", "", "100");
    utils::add_rng_options(feature);

    utils::add_log_options_to_feature(feature);
}

} // namespace probfd::solvers
