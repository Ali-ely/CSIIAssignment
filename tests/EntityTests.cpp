// Focused hierarchy milestone test. Build separately from the application's
// main and the broader simulation tests; no external test library is needed.
#include "Engineer.h"
#include "Medic.h"
#include "MaintenanceBot.h"
#include "MedicalDrone.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeinfo>

namespace {
void check(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

bool sameSettings(const EntitySettings& left, const EntitySettings& right) {
    return left.name == right.name && left.health == right.health &&
           left.maxHealth == right.maxHealth && left.active == right.active &&
           left.energy == right.energy && left.maxEnergy == right.maxEnergy &&
           left.skill == right.skill && left.supplies == right.supplies;
}

template <typename T>
void testEntity(const char* defaultName, bool biological) {
    static_assert(std::has_virtual_destructor<T>::value, "Base-pointer deletion must be safe");
    T defaultEntity;
    EntitySettings expectedDefaults;
    expectedDefaults.name = defaultName;
    check(sameSettings(defaultEntity.settings(), expectedDefaults), "sensible default attributes");
    check(defaultEntity.id() == 0 && defaultEntity.logSize() == 0, "default identity and empty history");
    check(defaultEntity.isBiological() == biological, "correct middle-level base");

    EntitySettings custom{"Vega", 67, 150, false, 42, 80, 7, 23};
    T original(custom);
    check(sameSettings(original.settings(), custom), "parameterized constructor preserves all attributes");
    for (int i = 0; i < 12; ++i) original.addLog("Entry " + std::to_string(i));
    check(defaultEntity.logSize() == 0 && original.logSize() == 12, "per-entity variable-sized history");

    T copied(original);
    check(sameSettings(copied.settings(), custom), "copy constructor preserves inherited and role state");
    check(&copied.logEntry(0) != &original.logEntry(0), "copy constructor allocates separate storage");
    for (std::size_t i = 0; i < original.logSize(); ++i)
        check(copied.logEntry(i) == original.logEntry(i), "copy constructor preserves history contents");
    auto changed = custom;
    changed.name = "Independent copy";
    changed.health = 20;
    changed.energy = 5;
    changed.supplies = 1;
    copied.configure(changed);
    copied.addLog("Copy only");
    check(sameSettings(original.settings(), custom) && original.logSize() == 12,
          "editing a copy leaves original attributes and dynamic data unchanged");

    T assigned;
    for (int i = 0; i < 30; ++i) assigned.addLog("Old destination entry");
    assigned = original;
    assigned = assigned;
    check(sameSettings(assigned.settings(), custom) && assigned.logSize() == 12,
          "assignment replaces old state and supports self-assignment");
    check(&assigned.logEntry(0) != &original.logEntry(0), "assignment allocates separate storage");
    original.addLog("Original only");
    check(assigned.logSize() == 12, "assigned history is independent");

    std::unique_ptr<Entity> owner = std::make_unique<T>(custom);
    owner->addLog("Survives deletion");
    auto clone = owner->clone();
    check(typeid(*clone) == typeid(T) && sameSettings(clone->settings(), custom),
          "polymorphic clone preserves concrete type and all attributes");
    check(&clone->logEntry(0) != &owner->logEntry(0), "polymorphic clone has independent storage");
    owner.reset(); // The virtual destructor must release the original's array.
    check(clone->logEntry(0) == "Survives deletion", "clone survives deletion through base pointer");
    clone->addLog("Clone only");

    const auto reject = [&](const EntitySettings& invalid) {
        bool constructorRejected = false;
        try { T invalidEntity(invalid); }
        catch (const std::invalid_argument&) { constructorRejected = true; }
        check(constructorRejected, "invalid constructor attributes must be rejected");
        bool editRejected = false;
        try { assigned.configure(invalid); }
        catch (const std::invalid_argument&) { editRejected = true; }
        check(editRejected && sameSettings(assigned.settings(), custom) && assigned.logSize() == 12,
              "invalid edits leave an existing entity unchanged");
    };
    const auto rejectNumber = [&](int EntitySettings::* field, int value) {
        auto invalid = custom;
        invalid.*field = value;
        reject(invalid);
    };
    for (const std::string& name : {std::string(), std::string("   "), std::string(61, 'x'),
                                   std::string("Bad\tName"), std::string("Bad\nName"),
                                   std::string("Bad\0Name", 8), std::string(1, '\v')}) {
        auto invalid = custom;
        invalid.name = name;
        reject(invalid);
    }
    rejectNumber(&EntitySettings::health, -1);
    rejectNumber(&EntitySettings::health, 151);
    rejectNumber(&EntitySettings::maxHealth, 0);
    rejectNumber(&EntitySettings::maxHealth, 1001);
    rejectNumber(&EntitySettings::energy, -1);
    rejectNumber(&EntitySettings::energy, 81);
    rejectNumber(&EntitySettings::maxEnergy, 0);
    rejectNumber(&EntitySettings::maxEnergy, 1001);
    rejectNumber(&EntitySettings::skill, 0);
    rejectNumber(&EntitySettings::skill, 11);
    rejectNumber(&EntitySettings::supplies, -1);
    rejectNumber(&EntitySettings::supplies, 10001);
    auto invalid = custom;
    invalid.health = 0;
    invalid.active = true;
    reject(invalid);

    const EntitySettings low{"Minimum", 0, 1, false, 0, 1, 1, 0};
    const EntitySettings high{"Maximum", 1000, 1000, true, 1000, 1000, 10, 10000};
    T minimum(low), maximum(high);
    check(sameSettings(minimum.settings(), low) && sameSettings(maximum.settings(), high),
          "valid attribute boundaries must be accepted");
    std::cout << "PASS " << defaultEntity.kindName()
              << " constructors, validation, independent copies and cloning\n";
}
}

int main() {
    static_assert(std::is_abstract<Entity>::value && std::is_abstract<Colonist>::value &&
                  std::is_abstract<Robot>::value, "General hierarchy levels must remain abstract");
    static_assert(std::has_virtual_destructor<Entity>::value, "Entity needs virtual destruction");
    try {
        testEntity<Engineer>("Engineer", true);
        testEntity<Medic>("Medic", true);
        testEntity<MaintenanceBot>("Maintenance Bot", false);
        testEntity<MedicalDrone>("Medical Drone", false);
        std::cout << "All entity milestone tests passed.\n";
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
