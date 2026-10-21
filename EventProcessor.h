#ifndef SIMWORLD_EVENT_PROCESSOR_H
#define SIMWORLD_EVENT_PROCESSOR_H

#include "Event.h"
#include <iosfwd>
class World;

// Resolves queued requests once, at the end-of-turn boundary. This is event
// dispatch, not entity turn dispatch, so a switch on the event kind is suitable.
class EventProcessor {
public:
    static void apply(World& world, const Event& event, std::ostream& out);
};

#endif
