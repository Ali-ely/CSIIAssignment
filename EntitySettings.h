#ifndef SIMWORLD_ENTITY_SETTINGS_H
#define SIMWORLD_ENTITY_SETTINGS_H

#include <string>

// A value object for validated creation/editing. Energy means stamina for a
// colonist and battery for a robot; skill and supplies have role-specific names.
struct EntitySettings {
    std::string name = "Colony resident";
    int health = 100;
    int maxHealth = 100;
    bool active = true;
    int energy = 100;
    int maxEnergy = 100;
    int skill = 5;
    int supplies = 10;

    void validate() const;
};

enum class EntityKind { Engineer = 1, Medic, MaintenanceBot, MedicalDrone };

#endif
