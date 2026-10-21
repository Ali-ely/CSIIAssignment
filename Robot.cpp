#include "Robot.h"
#include <algorithm>
#include <ostream>
#include <stdexcept>

Robot::Robot() : Robot(EntitySettings{}) {}
Robot::Robot(const EntitySettings& settings)
    : Entity(settings), battery_(settings.energy), maxBattery_(settings.maxEnergy) {}

bool Robot::spendBattery(int amount) {
    if (amount < 0) throw std::invalid_argument("Battery cost cannot be negative.");
    if (battery_ < amount) return false;
    battery_ -= amount;
    return true;
}

void Robot::charge(std::ostream& out) {
    const int gain = std::min(30, maxBattery_ - battery_);
    battery_ += gain;
    addLog("Charged; recovered " + std::to_string(gain) + " battery.");
    out << "  #" << id() << ' ' << name() << " charges (+" << gain << " battery).\n";
}

void Robot::applyRobotSettings(const EntitySettings& settings) {
    applyCommonSettings(settings);
    battery_ = settings.energy;
    maxBattery_ = settings.maxEnergy;
}

EntitySettings Robot::robotSettings() const {
    auto result = commonSettings();
    result.energy = battery_;
    result.maxEnergy = maxBattery_;
    return result;
}

void Robot::printRobot(std::ostream& out) const {
    printCommon(out);
    out << "  Battery: " << battery_ << '/' << maxBattery_ << '\n';
}
