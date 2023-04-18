#include "probfd/heuristics/pdbs/pattern_collection_generator_multiple.h"

#include "probfd/heuristics/pdbs/probabilistic_pattern_database.h"

#include "probfd/cost_model.h"

#include "task_utils/task_properties.h"

#include "utils/countdown_timer.h"
#include "utils/logging.h"
#include "utils/markup.h"
#include "utils/rng.h"
#include "utils/rng_options.h"

#include "option_parser.h"
#include "plugin.h"

#include <vector>

using namespace std;

namespace probfd {
namespace heuristics {
namespace pdbs {

namespace {

vector<FactPair> get_goals_in_random_order(
    const ProbabilisticTaskProxy& task_proxy,
    utils::RandomNumberGenerator& rng)
{
    vector<FactPair> goals =
        ::task_properties::get_fact_pairs(task_proxy.get_goals());
    rng.shuffle(goals);
    return goals;
}

vector<int> get_non_goal_variables(const ProbabilisticTaskProxy& task_proxy)
{
    size_t num_vars = task_proxy.get_variables().size();
    GoalsProxy goals = task_proxy.get_goals();
    vector<bool> is_goal(num_vars, false);
    for (FactProxy goal : goals) {
        is_goal[goal.get_variable().get_id()] = true;
    }

    vector<int> non_goal_variables;
    non_goal_variables.reserve(num_vars - goals.size());
    for (int var_id = 0; var_id < static_cast<int>(num_vars); ++var_id) {
        if (!is_goal[var_id]) {
            non_goal_variables.push_back(var_id);
        }
    }
    return non_goal_variables;
}

} // namespace

PatternCollectionGeneratorMultiple::PatternCollectionGeneratorMultiple(
    options::Options& opts,
    std::string implementation_name)
    : PatternCollectionGenerator(opts)
    , implementation_name(std::move(implementation_name))
    , max_pdb_size(opts.get<int>("max_pdb_size"))
    , max_collection_size(opts.get<int>("max_collection_size"))
    , pattern_generation_max_time(
          opts.get<double>("pattern_generation_max_time"))
    , total_max_time(opts.get<double>("total_max_time"))
    , stagnation_limit(opts.get<double>("stagnation_limit"))
    , blacklisting_start_time(
          total_max_time * opts.get<double>("blacklist_trigger_percentage"))
    , enable_blacklist_on_stagnation(
          opts.get<bool>("enable_blacklist_on_stagnation"))
    , rng(utils::parse_rng_from_options(opts))
    , random_seed(opts.get<int>("random_seed"))
{
}

bool PatternCollectionGeneratorMultiple::collection_size_limit_reached(
    int remaining_collection_size) const
{
    if (remaining_collection_size <= 0) {
        /*
          This value can become negative if the given size limits for
          pdb or collection size are so low that compute_pattern already
          violates the limit, possibly even with only using a single goal
          variable.
        */
        if (log.is_at_least_normal()) {
            log << "collection size limit reached" << endl;
        }
        return true;
    }
    return false;
}

bool PatternCollectionGeneratorMultiple::time_limit_reached(
    const utils::CountdownTimer& timer) const
{
    if (timer.is_expired()) {
        if (log.is_at_least_normal()) {
            log << "time limit reached" << endl;
        }
        return true;
    }
    return false;
}

PatternCollectionInformation PatternCollectionGeneratorMultiple::generate(
    const shared_ptr<ProbabilisticTask>& task)
{
    if (log.is_at_least_normal()) {
        log << "max pdb size: " << max_pdb_size << endl;
        log << "max collection size: " << max_collection_size << endl;
        log << "max time: " << total_max_time << endl;
        log << "stagnation time limit: " << stagnation_limit << endl;
        log << "timer after which blacklisting is enabled: "
            << blacklisting_start_time << endl;
        log << "enable blacklisting after stagnation: "
            << enable_blacklist_on_stagnation << endl;
    }

    ProbabilisticTaskProxy task_proxy(*task);
    TaskCostFunction* task_cost_function = g_cost_model->get_cost_function();

    utils::CountdownTimer timer(total_max_time);

    // Store the set of goals in random order.
    vector<FactPair> goals = get_goals_in_random_order(task_proxy, *rng);

    // Store the non-goal variables for potential blacklisting.
    vector<int> non_goal_variables = get_non_goal_variables(task_proxy);

    if (log.is_at_least_debug()) {
        log << "goal variables: ";
        for (FactPair goal : goals) {
            log << goal.var << ", ";
        }
        log << endl;
        log << "non-goal variables: " << non_goal_variables << endl;
    }

    // Collect all unique patterns and their PDBs.
    set<Pattern> generated_patterns;
    shared_ptr<PPDBCollection> generated_pdbs = make_shared<PPDBCollection>();

    shared_ptr<utils::RandomNumberGenerator> pattern_computation_rng =
        make_shared<utils::RandomNumberGenerator>(random_seed);

    int num_iterations = 1;
    int goal_index = 0;
    bool blacklisting = false;
    double time_point_of_last_new_pattern = 0.0;
    int remaining_collection_size = max_collection_size;

    while (true) {
        // Check if blacklisting should be started.
        if (!blacklisting &&
            timer.get_elapsed_time() > blacklisting_start_time) {
            blacklisting = true;
            /*
              Also treat this time point as having seen a new pattern to avoid
              stopping due to stagnation right after enabling blacklisting.
            */
            time_point_of_last_new_pattern = timer.get_elapsed_time();
            if (log.is_at_least_normal()) {
                log << "given percentage of total time limit "
                    << "exhausted; enabling blacklisting." << endl;
            }
        }

        // Get blacklisted variables
        unordered_set<int> blacklisted_variables;
        if (blacklisting && !non_goal_variables.empty()) {
            /*
              Randomize the number of non-goal variables for blacklisting.
              We want to choose at least 1 non-goal variable, so we pick a
              random value in the range [1, |non-goal variables|].
            */
            int blacklist_size = rng->random(non_goal_variables.size());
            ++blacklist_size;
            rng->shuffle(non_goal_variables);
            blacklisted_variables.insert(
                non_goal_variables.begin(),
                non_goal_variables.begin() + blacklist_size);
            if (log.is_at_least_debug()) {
                log << "blacklisting " << blacklist_size << " out of "
                    << non_goal_variables.size() << " non-goal variables: ";
                for (int var : blacklisted_variables) {
                    log << var << ", ";
                }
                log << endl;
            }
        }

        int remaining_pdb_size = min(remaining_collection_size, max_pdb_size);
        double remaining_time =
            min(static_cast<double>(timer.get_remaining_time()),
                pattern_generation_max_time);

        PatternInformation pattern_info = compute_pattern(
            remaining_pdb_size,
            remaining_time,
            pattern_computation_rng,
            task_proxy,
            *task_cost_function,
            goals[goal_index],
            std::move(blacklisted_variables));

        const Pattern& pattern = pattern_info.get_pattern();
        if (log.is_at_least_debug()) {
            log << "generated pattern " << pattern << endl;
        }

        if (generated_patterns.insert(std::move(pattern)).second) {
            /*
              compute_pattern generated a new pattern. Create/retrieve
              corresponding PDB, update collection size and reset
              time_point_of_last_new_pattern.
            */
            time_point_of_last_new_pattern = timer.get_elapsed_time();
            shared_ptr pdb = pattern_info.get_pdb();
            remaining_collection_size -= pdb->num_states();
            generated_pdbs->push_back(std::move(pdb));
        }

        if (collection_size_limit_reached(remaining_collection_size) ||
            time_limit_reached(timer)) {
            break;
        }

        // Test if no new pattern was generated for longer than
        // stagnation_limit.
        if (timer.get_elapsed_time() - time_point_of_last_new_pattern >
            stagnation_limit) {
            if (enable_blacklist_on_stagnation) {
                if (blacklisting) {
                    if (log.is_at_least_normal()) {
                        log << "stagnation limit reached "
                            << "despite blacklisting, terminating" << endl;
                    }
                    break;
                } else {
                    if (log.is_at_least_normal()) {
                        log << "stagnation limit reached, "
                            << "enabling blacklisting" << endl;
                    }
                    blacklisting = true;
                    time_point_of_last_new_pattern = timer.get_elapsed_time();
                }
            } else {
                if (log.is_at_least_normal()) {
                    log << "stagnation limit reached, terminating" << endl;
                }
                break;
            }
        }

        ++num_iterations;
        ++goal_index;
        goal_index = goal_index % goals.size();
        assert(utils::in_bounds(goal_index, goals));
    }

    shared_ptr<PatternCollection> patterns = make_shared<PatternCollection>();
    patterns->reserve(generated_pdbs->size());
    for (const auto pdb : *generated_pdbs) {
        patterns->push_back(pdb->get_pattern());
    }

    PatternCollectionInformation result(
        task_proxy,
        task_cost_function,
        patterns);
    result.set_pdbs(generated_pdbs);

    if (log.is_at_least_normal()) {
        log << implementation_name
            << " number of iterations: " << num_iterations << endl;
        log << implementation_name << " average time per generator: "
            << timer.get_elapsed_time() / num_iterations << endl;
    }

    return result;
}

void add_multiple_algorithm_implementation_notes_to_parser(
    options::OptionParser& parser)
{
    parser.document_note(
        "Short description of the 'multiple algorithm framework'",
        "This algorithm is a general framework for computing a pattern "
        "collection "
        "for a given planning task. It requires as input a method for "
        "computing a "
        "single pattern for the given task and a single goal of the task. The "
        "algorithm works as follows. It first stores the goals of the task in "
        "random order. Then, it repeatedly iterates over all goals and for "
        "each "
        "goal, it uses the given method for computing a single pattern. If the "
        "pattern is new (duplicate detection), it is kept for the final "
        "collection.\n"
        "The algorithm runs until reaching a given time limit. Another "
        "parameter allows "
        "exiting early if no new patterns are found for a certain time "
        "('stagnation'). "
        "Further parameters allow enabling blacklisting for the given pattern "
        "computation "
        "method after a certain time to force some diversification or to "
        "enable said "
        "blacklisting when stagnating.",
        true);
    parser.document_note(
        "Implementation note about the 'multiple algorithm framework'",
        "A difference compared to the original implementation used in the "
        "paper is that the original implementation of stagnation in "
        "the multiple CEGAR/RCG algorithms started counting the time towards "
        "stagnation only after having generated a duplicate pattern. Now, "
        "time towards stagnation starts counting from the start and is reset "
        "to the current time only when having found a new pattern or when "
        "enabling blacklisting.",
        true);
}

void add_multiple_options_to_parser(options::OptionParser& parser)
{
    parser.add_option<int>(
        "max_pdb_size",
        "maximum number of states for each pattern database, computed "
        "by compute_pattern (possibly ignored by singleton patterns consisting "
        "of a goal variable)",
        "1M",
        Bounds("1", "infinity"));
    parser.add_option<int>(
        "max_collection_size",
        "maximum number of states in all pattern databases of the "
        "collection (possibly ignored, see max_pdb_size)",
        "10M",
        Bounds("1", "infinity"));
    parser.add_option<double>(
        "pattern_generation_max_time",
        "maximum time in seconds for each call to the algorithm for "
        "computing a single pattern",
        "infinity",
        Bounds("0.0", "infinity"));
    parser.add_option<double>(
        "total_max_time",
        "maximum time in seconds for this pattern collection generator. "
        "It will always execute at least one iteration, i.e., call the "
        "algorithm for computing a single pattern at least once.",
        "100.0",
        Bounds("0.0", "infinity"));
    parser.add_option<double>(
        "stagnation_limit",
        "maximum time in seconds this pattern generator is allowed to run "
        "without generating a new pattern. It terminates prematurely if this "
        "limit is hit unless enable_blacklist_on_stagnation is enabled.",
        "20.0",
        Bounds("1.0", "infinity"));
    parser.add_option<double>(
        "blacklist_trigger_percentage",
        "percentage of total_max_time after which blacklisting is enabled",
        "0.75",
        Bounds("0.0", "1.0"));
    parser.add_option<bool>(
        "enable_blacklist_on_stagnation",
        "if true, blacklisting is enabled when stagnation_limit is hit "
        "for the first time (unless it was already enabled due to "
        "blacklist_trigger_percentage) and pattern generation is terminated "
        "when stagnation_limit is hit for the second time. If false, pattern "
        "generation is terminated already the first time stagnation_limit is "
        "hit.",
        "true");
    add_generator_options_to_parser(parser);
    utils::add_rng_options(parser);
}

} // namespace pdbs
} // namespace heuristics
} // namespace probfd
