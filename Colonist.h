#ifndef SIMWORLD_COLONIST_H
#define SIMWORLD_COLONIST_H

#include "Entity.h"

// The biological middle level owns stamina and shared rest behavior. Concrete
// professions choose work, but do not duplicate energy validation/recovery.
class Colonist : public Entity {
    int stamina_;
    int maxStamina_;

protected:
    Colonist();
    explicit Colonist(const EntitySettings& settings);
    bool spendStamina(int amount);
    void rest(std::ostream& out);
    void applyColonistSettings(const EntitySettings& settings);
    EntitySettings colonistSettings() const;
    void printColonist(std::ostream& out) const;

public:
    bool isBiological() const final { return true; }
    int stamina() const { return stamina_; }
};

#endif
