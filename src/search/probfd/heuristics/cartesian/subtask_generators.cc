#include "probfd/heuristics/cartesian/subtask_generators.h"

#include "heuristics/additive_heuristic.h"

#include "probfd/task_utils/task_properties.h"

#include "probfd/tasks/all_outcomes_determinization.h"
#include "probfd/tasks/domain_abstracted_task_factory.h"
#include "probfd/tasks/modified_goals_task.h"

#include "probfd/heuristics/cartesian/utils.h"

#include "cegar/utils_landmarks.h"

#include "utils/logging.h"
#include "utils/rng.h"
#include "utils/rng_options.h"

#include "option_parser.h"
#include "plugin.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

using namespace std;

namespace probfd {
namespace heuristics {
namespace cartesian {

class SortFactsByIncreasingHaddValues {
    // Can't store as unique_ptr since the class needs copy-constructor.
    shared_ptr<::additive_heuristic::AdditiveHeuristic> hadd;

    int get_cost(const FactPair& fact)
    {
        return hadd->get_cost_for_cegar(fact.var, fact.value);
    }

public:
    explicit SortFactsByIncreasingHaddValues(
        const shared_ptr<ProbabilisticTask>& task)
        : hadd(create_additive_heuristic(task))
    {
        TaskBaseProxy task_proxy(*task);
        hadd->compute_heuristic_for_cegar(task_proxy.get_initial_state());
    }

    bool operator()(const FactPair& a, const FactPair& b)
    {
        return get_cost(a) < get_cost(b);
    }
};

static void
remove_initial_state_facts(const TaskBaseProxy& task_proxy, Facts& facts)
{
    State initial_state = task_proxy.get_initial_state();
    std::erase_if(facts, [&](FactPair fact) {
        return initial_state[fact.var].get_value() == fact.value;
    });
}

static void order_facts(
    const shared_ptr<ProbabilisticTask>& task,
    FactOrder fact_order,
    vector<FactPair>& facts,
    utils::RandomNumberGenerator& rng,
    utils::LogProxy& log)
{
    if (log.is_at_least_verbose()) {
        log << "Sort " << facts.size() << " facts" << endl;
    }
    switch (fact_order) {
    case FactOrder::ORIGINAL:
        // Nothing to do.
        break;
    case FactOrder::RANDOM: rng.shuffle(facts); break;
    case FactOrder::HADD_UP:
    case FactOrder::HADD_DOWN:
        sort(facts.begin(), facts.end(), SortFactsByIncreasingHaddValues(task));
        if (fact_order == FactOrder::HADD_DOWN)
            reverse(facts.begin(), facts.end());
        break;
    default:
        cerr << "Invalid task order: " << static_cast<int>(fact_order) << endl;
        utils::exit_with(utils::ExitCode::SEARCH_INPUT_ERROR);
    }
}

static Facts filter_and_order_facts(
    const shared_ptr<ProbabilisticTask>& task,
    FactOrder fact_order,
    Facts& facts,
    utils::RandomNumberGenerator& rng,
    utils::LogProxy& log)
{
    ProbabilisticTaskProxy task_proxy(*task);
    remove_initial_state_facts(task_proxy, facts);
    order_facts(task, fact_order, facts, rng, log);
    return facts;
}

TaskDuplicator::TaskDuplicator(const Options& opts)
    : num_copies(opts.get<int>("copies"))
{
}

SharedTasks TaskDuplicator::get_subtasks(
    const shared_ptr<ProbabilisticTask>& task,
    utils::LogProxy&) const
{
    SharedTasks subtasks;
    subtasks.reserve(num_copies);
    for (int i = 0; i < num_copies; ++i) {
        subtasks.push_back(task);
    }
    return subtasks;
}

GoalDecomposition::GoalDecomposition(const Options& opts)
    : fact_order(opts.get<FactOrder>("order"))
    , rng(utils::parse_rng_from_options(opts))
{
}

SharedTasks GoalDecomposition::get_subtasks(
    const shared_ptr<ProbabilisticTask>& task,
    utils::LogProxy& log) const
{
    SharedTasks subtasks;
    ProbabilisticTaskProxy task_proxy(*task);
    Facts goal_facts =
        ::task_properties::get_fact_pairs(task_proxy.get_goals());
    filter_and_order_facts(task, fact_order, goal_facts, *rng, log);
    for (const FactPair& goal : goal_facts) {
        shared_ptr<ProbabilisticTask> subtask =
            make_shared<extra_tasks::ModifiedGoalsTask>(task, Facts{goal});
        subtasks.push_back(subtask);
    }
    return subtasks;
}

LandmarkDecomposition::LandmarkDecomposition(const Options& opts)
    : fact_order(opts.get<FactOrder>("order"))
    , combine_facts(opts.get<bool>("combine_facts"))
    , rng(utils::parse_rng_from_options(opts))
{
}

shared_ptr<ProbabilisticTask>
LandmarkDecomposition::build_domain_abstracted_task(
    const shared_ptr<ProbabilisticTask>& parent,
    const landmarks::LandmarkGraph& landmark_graph,
    const FactPair& fact) const
{
    assert(combine_facts);
    extra_tasks::VarToGroups value_groups;
    for (auto& pair : cegar::get_prev_landmarks(landmark_graph, fact)) {
        int var = pair.first;
        vector<int>& group = pair.second;
        if (group.size() >= 2) value_groups[var].push_back(group);
    }
    return extra_tasks::build_domain_abstracted_task(parent, value_groups);
}

SharedTasks LandmarkDecomposition::get_subtasks(
    const shared_ptr<ProbabilisticTask>& task,
    utils::LogProxy& log) const
{
    auto determinzation_task =
        std::make_shared<tasks::AODDeterminizationTask>(task.get());
    SharedTasks subtasks;
    shared_ptr<landmarks::LandmarkGraph> landmark_graph =
        cegar::get_landmark_graph(determinzation_task);
    Facts landmark_facts = cegar::get_fact_landmarks(*landmark_graph);
    filter_and_order_facts(task, fact_order, landmark_facts, *rng, log);
    for (const FactPair& landmark : landmark_facts) {
        shared_ptr<ProbabilisticTask> subtask =
            make_shared<extra_tasks::ModifiedGoalsTask>(task, Facts{landmark});
        if (combine_facts) {
            subtask = build_domain_abstracted_task(
                subtask,
                *landmark_graph,
                landmark);
        }
        subtasks.push_back(subtask);
    }
    return subtasks;
}

static shared_ptr<SubtaskGenerator> _parse_original(OptionParser& parser)
{
    parser.add_option<int>(
        "copies",
        "number of task copies",
        "1",
        Bounds("1", "infinity"));
    Options opts = parser.parse();
    if (parser.dry_run())
        return nullptr;
    else
        return make_shared<TaskDuplicator>(opts);
}

static void add_fact_order_option(OptionParser& parser)
{
    vector<string> fact_orders;
    fact_orders.push_back("ORIGINAL");
    fact_orders.push_back("RANDOM");
    fact_orders.push_back("HADD_UP");
    fact_orders.push_back("HADD_DOWN");
    parser.add_enum_option<FactOrder>(
        "order",
        fact_orders,
        "ordering of goal facts",
        "HADD_DOWN");
    utils::add_rng_options(parser);
}

static shared_ptr<SubtaskGenerator> _parse_goals(OptionParser& parser)
{
    add_fact_order_option(parser);
    Options opts = parser.parse();
    if (parser.dry_run())
        return nullptr;
    else
        return make_shared<GoalDecomposition>(opts);
}

static shared_ptr<SubtaskGenerator> _parse_landmarks(OptionParser& parser)
{
    add_fact_order_option(parser);
    parser.add_option<bool>(
        "combine_facts",
        "combine landmark facts with domain abstraction",
        "true");
    Options opts = parser.parse();
    if (parser.dry_run())
        return nullptr;
    else
        return make_shared<LandmarkDecomposition>(opts);
}

static Plugin<SubtaskGenerator>
    _plugin_original("pcegar_original", _parse_original);
static Plugin<SubtaskGenerator> _plugin_goals("pcegar_goals", _parse_goals);
static Plugin<SubtaskGenerator>
    _plugin_landmarks("pcegar_landmarks", _parse_landmarks);

static PluginTypePlugin<SubtaskGenerator> _type_plugin(
    "PSubtaskGenerator",
    "Subtask generator (used by the CEGAR heuristic).");

} // namespace cartesian
} // namespace heuristics
} // namespace probfd