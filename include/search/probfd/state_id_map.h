#ifndef MDPS_STATE_ID_MAP_H
#define MDPS_STATE_ID_MAP_H

#include "probfd/engine_interfaces/state_id_map.h"

#include "../task_proxy.h"

class StateRegistry;

namespace probfd {
namespace engine_interfaces {

template <>
class StateIDMap<State> {
public:
    explicit StateIDMap(StateRegistry* reg);
    StateID get_state_id(const State& state);
    State get_state(const StateID& state_id);

private:
    StateRegistry* reg_;
};

} // namespace engine_interfaces
} // namespace probfd

#endif // __STATE_ID_MAP_H__