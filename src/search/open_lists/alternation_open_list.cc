#include "alternation_open_list.h"
#include "open_list_factory.h"
#include "../option_parser.h"
#include "../plugin.h"

#include <cassert>
#include <cstdlib>
using namespace std;

template<class Entry>
AlternationOpenList<Entry>::AlternationOpenList(const options::Options &opts)
    : open_lists(opts.get_list<std::shared_ptr<OpenList<Entry> > >("sublists")),
      priorities(open_lists.size(), 0), size(0),
      boosting(opts.get<int>("boost")) {
}

template<class Entry>
AlternationOpenList<Entry>::AlternationOpenList(const vector<std::shared_ptr<OpenList<Entry> > > &sublists,
                                                int boost_influence)
    : open_lists(sublists), priorities(sublists.size(), 0), size(0),
      boosting(boost_influence) {
}

template<class Entry>
AlternationOpenList<Entry>::~AlternationOpenList() {
}

template<class Entry>
int AlternationOpenList<Entry>::insert(const Entry &entry) {
    int new_entries = 0;
    for (size_t i = 0; i < open_lists.size(); ++i)
        if (!open_lists[i]->is_dead_end())
            new_entries += open_lists[i]->insert(entry);
    size += new_entries;
    return new_entries;
}

template<class Entry>
Entry AlternationOpenList<Entry>::remove_min(vector<int> *key) {
    assert(size > 0);
    if (key) {
        cerr << "not implemented -- see msg639 in the tracker" << endl;
        utils::exit_with(utils::ExitCode::SEARCH_UNSUPPORTED);
    }
    int best = -1;
    for (size_t i = 0; i < open_lists.size(); ++i) {
        if (!open_lists[i]->empty() &&
            (best == -1 || priorities[i] < priorities[best])) {
            best = i;
        }
    }
    last_used_list = best;
    std::shared_ptr<OpenList<Entry> > best_list = open_lists[best];
    assert(!best_list->empty());
    --size;
    ++priorities[best];
    return best_list->remove_min(0);
}

template<class Entry>
bool AlternationOpenList<Entry>::empty() const {
    return size == 0;
}

template<class Entry>
void AlternationOpenList<Entry>::clear() {
    size = 0;
    for (size_t i = 0; i < open_lists.size(); ++i)
        open_lists[i]->clear();
}

template<class Entry>
void AlternationOpenList<Entry>::evaluate(int g, bool preferred) {
    /*
      Treat as a dead end if
      1. at least one heuristic reliably recognizes it as a dead end, or
      2. all heuristics unreliably recognize it as a dead end
      In case 1., the dead end is reliable; in case 2. it is not.
     */

    dead_end = true;
    dead_end_reliable = false;
    for (size_t i = 0; i < open_lists.size(); ++i) {
        open_lists[i]->evaluate(g, preferred);
        if (open_lists[i]->is_dead_end()) {
            if (open_lists[i]->dead_end_is_reliable()) {
                dead_end = true; // Might have been set to false.
                dead_end_reliable = true;
                break;
            }
        } else {
            dead_end = false;
        }
    }
}

template<class Entry>
bool AlternationOpenList<Entry>::is_dead_end() const {
    return dead_end;
}

template<class Entry>
bool AlternationOpenList<Entry>::dead_end_is_reliable() const {
    return dead_end_reliable;
}

template<class Entry>
void AlternationOpenList<Entry>::get_involved_heuristics(std::set<Heuristic *> &hset) {
    for (size_t i = 0; i < open_lists.size(); ++i)
        open_lists[i]->get_involved_heuristics(hset);
}

template<class Entry>
int AlternationOpenList<Entry>::boost_preferred() {
    int total_boost = 0;
    for (size_t i = 0; i < open_lists.size(); ++i) {
        // if the open list is not an alternation open list
        // (these have always only_preferred==false) and
        // it takes only preferred states, we boost it
        if (open_lists[i]->only_preferred_states()) {
            priorities[i] -= boosting;
            total_boost += boosting;
        }
        // otherwise, we tell it to boost its lists (which
        // has no effect on non-alterntion lists)
        else {
            int boosted = open_lists[i]->boost_preferred();
            // now we have to boost this alternation open list
            // as well to give its boosting some effect
            priorities[i] -= boosted;
            total_boost += boosted;
        }
    }
    return total_boost; // can be used by "parent" alternation list
}

template<class Entry>
void AlternationOpenList<Entry>::boost_last_used_list() {
    priorities[last_used_list] -= boosting;

    // for the case that the last used list is an alternation
    // list
    open_lists[last_used_list]->boost_last_used_list();
}

AlternationOpenListFactory::AlternationOpenListFactory(const options::Options& opts)
    : opts(opts)
{
}

AlternationOpenListFactory::AlternationOpenListFactory(
        const std::vector<std::shared_ptr<OpenListFactory > > &sublists,
        int boost_influence)
{
    opts.set<std::vector<std::shared_ptr<OpenListFactory > >>("sublists", sublists);
    opts.set<int>("boost", boost_influence);
}

std::shared_ptr<OpenList<StateID> >
AlternationOpenListFactory::create_state_open_list()
{
    options::Options list_opts;
    list_opts.set<int>("boost", opts.get<int>("boost"));
    std::vector<std::shared_ptr<OpenList<StateID> > > sublists;
    for (std::shared_ptr<OpenListFactory> f : opts.get_list<std::shared_ptr<OpenListFactory> >("sublists")) {
        sublists.push_back(f->create_state_open_list());
    }
    list_opts.set<std::vector<std::shared_ptr<OpenList<StateID> > > >("sublists", sublists);
    return std::make_shared<AlternationOpenList<StateID> >(list_opts);
}

std::shared_ptr<OpenList<std::pair<StateID, std::pair<int, const GlobalOperator*> > > >
AlternationOpenListFactory::create_ehc_open_list()
{
    options::Options list_opts;
    list_opts.set<int>("boost", opts.get<int>("boost"));
    std::vector<std::shared_ptr<OpenList<std::pair<StateID, std::pair<int, const GlobalOperator*> >> > > sublists;
    for (std::shared_ptr<OpenListFactory> f : opts.get_list<std::shared_ptr<OpenListFactory> >("sublists")) {
        sublists.push_back(f->create_ehc_open_list());
    }
    list_opts.set<std::vector<std::shared_ptr<OpenList<std::pair<StateID, std::pair<int, const GlobalOperator*> >> > > >("sublists", sublists);
    return std::make_shared<AlternationOpenList<std::pair<StateID, std::pair<int, const GlobalOperator*> >> >(list_opts);
}

std::shared_ptr<OpenList<std::pair<StateID, const GlobalOperator*> > >
AlternationOpenListFactory::create_lazy_open_list()
{
    options::Options list_opts;
    list_opts.set<int>("boost", opts.get<int>("boost"));
    std::vector<std::shared_ptr<OpenList<std::pair<StateID, const GlobalOperator*>> > > sublists;
    for (std::shared_ptr<OpenListFactory> f : opts.get_list<std::shared_ptr<OpenListFactory> >("sublists")) {
        sublists.push_back(f->create_lazy_open_list());
    }
    list_opts.set<std::vector<std::shared_ptr<OpenList<std::pair<StateID, const GlobalOperator*>> > > >("sublists", sublists);
    return std::make_shared<AlternationOpenList<std::pair<StateID, const GlobalOperator*>> >(list_opts);
}

INSTANTIATE_OPEN_LIST(AlternationOpenList)

static std::shared_ptr<OpenListFactory> _parse(options::OptionParser &parser) {
    parser.document_synopsis("Alternation open list",
                             "alternates between several open lists.");
    parser.document_note(
        "Preferred operators",
        "Preferred operators are only taken from sub-open-lists "
        "that do not consider the evaluated state a dead end.");
    parser.document_note(
        "Dead ends",
        "A state is considered a dead end if "
        "either all alternated open lists agree that it is a dead end "
        "or at least one reliable open list considers it a dead end. "
        "A state is never inserted into a sub-open-list "
        "that considers it a dead end.");
    parser.document_note(
        "Note",
        "The treatment of dead ends is different from "
        "the one described in the "
        "[technical report http://tr.informatik.uni-freiburg.de/reports/report258/report00258.ps.gz] "
        "\"The More, the Merrier: Combining Heuristic Estimators "
        "for Satisficing Planning (Extended Version)\" "
        "(Department of Computer Science at Freiburg University, "
        "No. 258, 2010)");
    parser.add_list_option<std::shared_ptr<OpenListFactory> >("sublists", "sub open lists");
    parser.add_option<int>("boost",
                           "boost value for sub-open-lists "
                           "that are restricted to preferred operator nodes",
                           "0");

    options::Options opts = parser.parse();
    if (parser.help_mode())
        return 0;

    if (opts.get_list<std::shared_ptr<OpenListFactory> >("sublists").empty())
        parser.error("need at least one internal open list");
    if (parser.dry_run())
        return 0;
    else
        return std::make_shared<AlternationOpenListFactory>(opts);
}

static Plugin<OpenListFactory> _plugin("alt", _parse);

