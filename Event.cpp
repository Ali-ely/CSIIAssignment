#include "Event.h"

const char* eventName(EventKind kind) {
    switch (kind) {
    case EventKind::RepairRequest: return "Repair request";
    case EventKind::MedicalRequest: return "Medical request";
    case EventKind::SupplyDelivery: return "Supply delivery";
    }
    return "Unknown event";
}
