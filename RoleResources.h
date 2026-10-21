#ifndef SIMWORLD_ROLE_RESOURCES_H
#define SIMWORLD_ROLE_RESOURCES_H

#include "EntitySettings.h"
#include <algorithm>
#include <stdexcept>

// Value member reused by all professions: centralizes skill, inventory bounds,
// and resource consumption without forcing unrelated professions into one base.
class RoleResources {
    int skill_;
    int supplies_;
public:
    explicit RoleResources(const EntitySettings& settings)
        : skill_(settings.skill), supplies_(settings.supplies) { settings.validate(); }
    int skill() const { return skill_; }
    int supplies() const { return supplies_; }
    int strength() const { return 8 + 2 * skill_; }
    bool consume(int amount) {
        if (amount < 0) throw std::invalid_argument("Negative resource cost.");
        if (supplies_ < amount) return false;
        supplies_ -= amount;
        return true;
    }
    int add(int amount) {
        if (amount < 0) throw std::invalid_argument("Negative supply amount.");
        const int actual = std::min(amount, 10000 - supplies_);
        supplies_ += actual;
        return actual;
    }
    void configure(const EntitySettings& settings) {
        settings.validate();
        skill_ = settings.skill;
        supplies_ = settings.supplies;
    }
    void fillSettings(EntitySettings& settings) const {
        settings.skill = skill_;
        settings.supplies = supplies_;
    }
};

#endif
