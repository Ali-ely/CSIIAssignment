#include "EntityFactory.h"
#include "Engineer.h"
#include "Medic.h"
#include "MaintenanceBot.h"
#include "MedicalDrone.h"
#include <stdexcept>

std::unique_ptr<Entity> EntityFactory::createDefault(EntityKind kind) {
    switch (kind) {
    case EntityKind::Engineer: return std::make_unique<Engineer>();
    case EntityKind::Medic: return std::make_unique<Medic>();
    case EntityKind::MaintenanceBot: return std::make_unique<MaintenanceBot>();
    case EntityKind::MedicalDrone: return std::make_unique<MedicalDrone>();
    }
    throw std::invalid_argument("Unknown entity kind.");
}

std::unique_ptr<Entity> EntityFactory::create(EntityKind kind, const EntitySettings& settings) {
    settings.validate();
    switch (kind) {
    case EntityKind::Engineer: return std::make_unique<Engineer>(settings);
    case EntityKind::Medic: return std::make_unique<Medic>(settings);
    case EntityKind::MaintenanceBot: return std::make_unique<MaintenanceBot>(settings);
    case EntityKind::MedicalDrone: return std::make_unique<MedicalDrone>(settings);
    }
    throw std::invalid_argument("Unknown entity kind.");
}
