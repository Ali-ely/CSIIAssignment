#ifndef SIMWORLD_ROBOT_H
#define SIMWORLD_ROBOT_H

#include "Entity.h"

// The mechanical middle level shares battery accounting and charging across
// both robot types while leaving their normal jobs to virtual overrides.
class Robot : public Entity {
    int battery_;
    int maxBattery_;

protected:
    Robot();
    explicit Robot(const EntitySettings& settings);
    Robot(const Robot& other);
    Robot& operator=(const Robot& other);
    bool spendBattery(int amount);
    void charge(std::ostream& out);
    void applyRobotSettings(const EntitySettings& settings);
    EntitySettings robotSettings() const;
    void printRobot(std::ostream& out) const;

public:
    ~Robot() override;
    bool isBiological() const final { return false; }
    int battery() const { return battery_; }
};

#endif
