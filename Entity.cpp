#include "Entity.h"

#include <algorithm>
#include <limits>
#include <ostream>
#include <stdexcept>
#include <utility>

Entity::Entity() : Entity(EntitySettings{}) {}

Entity::Entity(const EntitySettings& settings)
    : name_(settings.name), health_(settings.health),
      maxHealth_(settings.maxHealth), active_(settings.active) {
    settings.validate();
    // Even an empty log has exclusive storage; it can grow as this entity acts.
    log_ = new std::string[4];
    logCapacity_ = 4;
}

Entity::Entity(const Entity& other)
    : id_(other.id_), name_(other.name_), health_(other.health_),
      maxHealth_(other.maxHealth_), active_(other.active_),
      logSize_(other.logSize_), logCapacity_(other.logCapacity_) {
    auto copy = std::make_unique<std::string[]>(logCapacity_);
    for (std::size_t i = 0; i < logSize_; ++i) copy[i] = other.log_[i];
    log_ = copy.release();
}

Entity& Entity::operator=(const Entity& other) {
    if (this == &other) return *this;
    // Allocate/copy before releasing anything: a failed copy leaves us intact.
    std::string newName(other.name_);
    auto copy = std::make_unique<std::string[]>(other.logCapacity_);
    for (std::size_t i = 0; i < other.logSize_; ++i) copy[i] = other.log_[i];
    delete[] log_;
    log_ = copy.release();
    name_.swap(newName);
    // Assignment replaces state, not the identity of an existing entity.
    // Copy construction still preserves IDs for exact world snapshots.
    // World::add assigns a fresh ID when a clone becomes a user duplicate.
    health_ = other.health_;
    maxHealth_ = other.maxHealth_;
    active_ = other.active_;
    logSize_ = other.logSize_;
    logCapacity_ = other.logCapacity_;
    return *this;
}

Entity::~Entity() { delete[] log_; }

void Entity::addLog(const std::string& message) {
    // Copy first in case message refers to an entry in the current array.
    std::string entry(message);
    if (logSize_ == logCapacity_) {
        if (logCapacity_ > std::numeric_limits<std::size_t>::max() / 2)
            throw std::length_error("Entity history is too large.");
        const std::size_t capacity = logCapacity_ * 2;
        auto bigger = std::make_unique<std::string[]>(capacity);
        for (std::size_t i = 0; i < logSize_; ++i) bigger[i] = log_[i];
        delete[] log_;
        log_ = bigger.release();
        logCapacity_ = capacity;
    }
    log_[logSize_] = std::move(entry);
    ++logSize_;
}

const std::string& Entity::logEntry(std::size_t index) const {
    if (index >= logSize_) throw std::out_of_range("Unknown history entry.");
    return log_[index];
}

int Entity::damage(int amount) {
    if (amount < 0) throw std::invalid_argument("Damage cannot be negative.");
    const int actual = std::min(amount, health_);
    health_ -= actual;
    if (health_ == 0) active_ = false;
    if (actual) addLog("Lost " + std::to_string(actual) + " health/integrity.");
    return actual;
}

int Entity::restoreHealth(int amount) {
    if (amount < 0) throw std::invalid_argument("Restoration cannot be negative.");
    const int actual = std::min(amount, maxHealth_ - health_);
    health_ += actual;
    if (actual) addLog("Restored " + std::to_string(actual) + " health/integrity.");
    return actual;
}

int Entity::receiveMedicalSupplies(int) {
    throw std::invalid_argument("This entity cannot receive medical supplies.");
}

void Entity::applyCommonSettings(const EntitySettings& settings) {
    settings.validate();
    name_ = settings.name;
    health_ = settings.health;
    maxHealth_ = settings.maxHealth;
    active_ = settings.active;
}

EntitySettings Entity::commonSettings() const {
    EntitySettings result;
    result.name = name_;
    result.health = health_;
    result.maxHealth = maxHealth_;
    result.active = active_;
    return result;
}

void Entity::printCommon(std::ostream& out) const {
    out << '#' << id_ << " " << name_ << " [" << kindName() << "]\n"
        << "  Health/integrity: " << health_ << '/' << maxHealth_
        << " | " << (active_ ? "active" : "inactive") << '\n';
    out << "  History (" << logSize_ << "):\n";
    if (logSize_ == 0) out << "    (empty)\n";
    for (std::size_t i = 0; i < logSize_; ++i)
        out << "    " << i + 1 << ". " << log_[i] << '\n';
}
