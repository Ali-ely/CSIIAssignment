#include "Colonist.h"
#include <algorithm>
#include <ostream>
#include <stdexcept>

Colonist::Colonist() : Colonist(EntitySettings{}) {}
Colonist::Colonist(const EntitySettings& settings)
    : Entity(settings), stamina_(settings.energy), maxStamina_(settings.maxEnergy) {}

// Entity owns/copies the history; stamina is copied by value at this level.
Colonist::Colonist(const Colonist& other) = default;
Colonist& Colonist::operator=(const Colonist& other) = default;
Colonist::~Colonist() = default;

bool Colonist::spendStamina(int amount) {
    if (amount < 0) throw std::invalid_argument("Stamina cost cannot be negative.");
    if (stamina_ < amount) return false;
    stamina_ -= amount;
    return true;
}

void Colonist::rest(std::ostream& out) {
    const int gain = std::min(25, maxStamina_ - stamina_);
    stamina_ += gain;
    addLog("Rested; recovered " + std::to_string(gain) + " stamina.");
    out << "  #" << id() << ' ' << name() << " rests (+" << gain << " stamina).\n";
}

void Colonist::applyColonistSettings(const EntitySettings& settings) {
    applyCommonSettings(settings);
    stamina_ = settings.energy;
    maxStamina_ = settings.maxEnergy;
}

EntitySettings Colonist::colonistSettings() const {
    auto result = commonSettings();
    result.energy = stamina_;
    result.maxEnergy = maxStamina_;
    return result;
}

void Colonist::printColonist(std::ostream& out) const {
    printCommon(out);
    out << "  Stamina: " << stamina_ << '/' << maxStamina_ << '\n';
}
