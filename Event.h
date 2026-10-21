#ifndef SIMWORLD_EVENT_H
#define SIMWORLD_EVENT_H

#include <string>

enum class EventKind { RepairRequest = 1, MedicalRequest, SupplyDelivery };

// Queue values reference stable IDs, never pointers. A missing target is handled
// at processing time; deleting a source does not cancel an accepted request.
struct Event {
    EventKind kind = EventKind::RepairRequest;
    int sourceId = 0; // Zero denotes an external, user-entered request.
    int targetId = 0;
    int amount = 0;
    std::string description;
};

const char* eventName(EventKind kind);

#endif
