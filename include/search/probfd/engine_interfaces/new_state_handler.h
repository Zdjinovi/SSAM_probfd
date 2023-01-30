#ifndef MDPS_ENGINE_INTERFACES_NEW_STATE_HANDLER_H
#define MDPS_ENGINE_INTERFACES_NEW_STATE_HANDLER_H

#include <memory>
#include <vector>

namespace probfd {
namespace engine_interfaces {

/**
 * @brief Function object used as a callback when a new state is encountered
 * during search.
 *
 * @tparam State - The state type of the underlying MDP model.
 */
template <typename State>
struct NewStateHandler {
    virtual ~NewStateHandler() = default;

    /**
     * Called when a new state is encountered.
     */
    virtual void touch(const State&) {}

    /**
     * Called when a new goal state is encountered.
     */
    virtual void touch_goal(const State&) {}

    /**
     * Called when a new dead end state is encountered.
     */
    virtual void touch_dead_end(const State&) {}
};

template <typename State>
class NewStateHandlerList : public NewStateHandler<State> {
public:
    explicit NewStateHandlerList(
        std::vector<std::shared_ptr<NewStateHandler<State>>> handlers)
        : handlers_(std::move(handlers))
    {
    }

    virtual ~NewStateHandlerList() = default;

    virtual void touch(const State& s) override
    {
        for (auto& handler : handlers_) {
            handler->touch(s);
        }
    }

    virtual void touch_dead_end(const State& s) override
    {
        for (auto& handler : handlers_) {
            handler->touch_dead_end(s);
        }
    }

    virtual void touch_goal(const State& s) override
    {
        for (auto& handler : handlers_) {
            handler->touch_goal(s);
        }
    }

private:
    std::vector<std::shared_ptr<NewStateHandler<State>>> handlers_;
};

} // namespace engine_interfaces
} // namespace probfd

// Convenience type aliases

class State;

namespace probfd {
using TaskNewStateHandler = engine_interfaces::NewStateHandler<State>;
using TaskNewStateHandlerList = engine_interfaces::NewStateHandlerList<State>;
} // namespace probfd

#endif // __NEW_STATE_HANDLER_H__