#ifndef SIMWORLD_ENTITY_H
#define SIMWORLD_ENTITY_H

#include "EntitySettings.h"
#include <cstddef>
#include <iosfwd>
#include <memory>
#include <string>

class World;

// Root of the three-level hierarchy. Identity, health, and an independently
// allocated history belong to every entity. Only World assigns IDs. The raw
// history array deliberately demonstrates the Rule of Three required by F2.
// Each concrete object's base subobject owns its own array; defaulted derived
// copy operations invoke this deep copy instead of sharing history storage.
class Entity {
    friend class World;
    int id_ = 0; // Zero means a newly constructed object not yet in a world.
    std::string name_;
    int health_;
    int maxHealth_;
    bool active_;
    std::string* log_ = nullptr;
    std::size_t logSize_ = 0;
    std::size_t logCapacity_ = 0;

    void assignId(int id) { id_ = id; }

protected:
    Entity();
    explicit Entity(const EntitySettings& settings);
    Entity(const Entity& other);
    Entity& operator=(const Entity& other);
    void applyCommonSettings(const EntitySettings& settings);
    EntitySettings commonSettings() const;
    void printCommon(std::ostream& out) const;

public:
    virtual ~Entity();
    int id() const { return id_; }
    const std::string& name() const { return name_; }
    int health() const { return health_; }
    int maxHealth() const { return maxHealth_; }
    bool active() const { return active_; }

    void addLog(const std::string& message);
    std::size_t logSize() const { return logSize_; }
    const std::string& logEntry(std::size_t index) const;
    int damage(int amount);
    int restoreHealth(int amount);

    virtual const char* kindName() const = 0;
    virtual bool isBiological() const = 0;
    virtual bool acceptsMedicalSupplies() const { return false; }
    virtual int medicalSupplyCount() const { return 0; }
    virtual int receiveMedicalSupplies(int amount);
    virtual EntitySettings settings() const = 0;
    virtual void configure(const EntitySettings& settings) = 0;
    virtual void takeTurn(World& world, std::ostream& out) = 0;
    virtual void printDetails(std::ostream& out) const = 0;
    virtual std::unique_ptr<Entity> clone() const = 0;
};

#endif
