#include "ConsoleUI.h"
#include <exception>
#include <iostream>
#include <limits>
#include <sstream>

bool ConsoleUI::readInt(const std::string& prompt, int minimum, int maximum, int& result) {
    std::string line;
    while (true) {
        out_ << prompt;
        if (!std::getline(in_, line)) return false;
        std::istringstream parser(line);
        long long value = 0;
        if (parser >> value) {
            parser >> std::ws;
            if (parser.eof() && value >= minimum && value <= maximum) {
                result = static_cast<int>(value);
                return true;
            }
        }
        out_ << "Enter a whole number from " << minimum << " to " << maximum << ".\n";
    }
}

bool ConsoleUI::readId(const std::string& prompt, int& result) {
    return readInt(prompt, 1, std::numeric_limits<int>::max() - 1, result);
}

bool ConsoleUI::readName(std::string& result) {
    while (true) {
        out_ << "Name (1-60 characters): ";
        if (!std::getline(in_, result)) return false;
        EntitySettings check;
        check.name = result;
        try { check.validate(); return true; }
        catch (const std::exception& error) { out_ << error.what() << '\n'; }
    }
}

bool ConsoleUI::readSettings(EntitySettings& settings) {
    if (!readName(settings.name)) return false;
    if (!readInt("Maximum health/integrity (1-1000): ", 1, 1000, settings.maxHealth)) return false;
    if (!readInt("Current health/integrity: ", 0, settings.maxHealth, settings.health)) return false;
    if (settings.health == 0) {
        settings.active = false;
        out_ << "Zero health makes this entity inactive.\n";
    } else {
        int active = 1;
        if (!readInt("Active? (1=yes, 0=no): ", 0, 1, active)) return false;
        settings.active = active != 0;
    }
    if (!readInt("Maximum stamina/battery (1-1000): ", 1, 1000, settings.maxEnergy)) return false;
    if (!readInt("Current stamina/battery: ", 0, settings.maxEnergy, settings.energy)) return false;
    if (!readInt("Role skill/efficiency (1-10): ", 1, 10, settings.skill)) return false;
    return readInt("Role supplies (parts, medical supplies, or doses; 0-10000): ",
                   0, 10000, settings.supplies);
}

void ConsoleUI::menu() const {
    out_ << "\n=== SPACE COLONY: SIMWORLD ===\n"
         << "1. Create entity (defaults)     2. Create entity (custom)\n"
         << "3. List entities                4. Inspect entity\n"
         << "5. Modify entity                6. Duplicate entity\n"
         << "7. Delete entity                8. Emergency repair\n"
         << "9. Emergency treatment         10. Enter event\n"
         << "11. View pending events        12. Run one turn\n"
         << "13. Run multiple turns         14. Undo\n"
         << "15. Colony status and rules     0. Exit\n";
}

void ConsoleUI::run() {
    out_ << "Build a colony and survive 20 turns. Create at least one colonist before running.\n"
         << "Engineers support the habitat; medics treat colonists; maintenance bots repair\n"
         << "robots; medical drones deliver medicine. All successful changes support undo.\n";
    while (true) {
        menu();
        int choice = 0;
        if (!readInt("Choice: ", 0, 15, choice) || choice == 0) break;
        int id = 0;
        int target = 0;
        int kind = 0;
        int amount = 0;
        switch (choice) {
        case 1:
        case 2: {
            if (!readInt("Kind (1=Engineer, 2=Medic, 3=MaintenanceBot, 4=MedicalDrone): ", 1, 4, kind)) return;
            if (choice == 1) simulation_.createDefault(static_cast<EntityKind>(kind), out_);
            else {
                EntitySettings settings;
                if (!readSettings(settings)) return;
                simulation_.create(static_cast<EntityKind>(kind), settings, out_);
            }
            break;
        }
        case 3: simulation_.world().listEntities(out_); break;
        case 4:
            if (!readId("Entity ID: ", id)) return;
            if (const Entity* entity = simulation_.world().find(id)) entity->printDetails(out_);
            else out_ << "Unknown entity ID.\n";
            break;
        case 5: {
            if (!readId("Entity ID to modify: ", id)) return;
            const Entity* entity = simulation_.world().find(id);
            if (!entity) { out_ << "Unknown entity ID.\n"; break; }
            entity->printDetails(out_);
            out_ << "Enter replacement attributes (ID, concrete kind, and existing history are preserved).\n";
            EntitySettings settings = entity->settings();
            if (!readSettings(settings)) return;
            simulation_.modify(id, settings, out_);
            break;
        }
        case 6:
            if (!readId("Entity ID to duplicate: ", id)) return;
            simulation_.duplicate(id, out_);
            break;
        case 7:
            if (!readId("Entity ID to delete: ", id)) return;
            simulation_.remove(id, out_);
            break;
        case 8:
        case 9:
            if (!readId("Actor ID: ", id) || !readId("Target ID: ", target)) return;
            if (choice == 8) simulation_.repair(id, target, out_);
            else simulation_.heal(id, target, out_);
            break;
        case 10:
            if (!readInt("Event (1=Repair, 2=Medical, 3=Supply delivery): ", 1, 3, kind) ||
                !readId("Target ID: ", target) ||
                !readInt("Amount (1-1000): ", 1, 1000, amount)) return;
            simulation_.enterEvent(static_cast<EventKind>(kind), target, amount, out_);
            break;
        case 11: simulation_.world().showEvents(out_); break;
        case 12: simulation_.runTurns(1, out_); break;
        case 13:
            if (!readInt("Number of turns (1-1000): ", 1, 1000, amount)) return;
            simulation_.runTurns(amount, out_);
            break;
        case 14: simulation_.undo(out_); break;
        case 15:
            simulation_.world().showStatus(out_);
            out_ << "Undo levels: " << simulation_.undoDepth() << "\n"
                 << "Upkeep costs 6 habitat integrity/turn. Below 30, active entities take 10 damage.\n"
                 << "Win: finish turn 20 with a surviving colonist and positive habitat integrity.\n"
                 << "Lose: habitat reaches zero or no active colonists remain at turn end.\n"
                 << "Events process after entity actions. Equal-priority targets use creation order.\n"
                 << "Emergency requests cost 10 energy and 1 supply; costs are paid when queued.\n"
                 << "A deleted/inactive event target causes the request to be discarded without refund.\n"
                 << "An entire Run N command is one undo action. Undo can reverse a mission outcome.\n";
            break;
        default: break;
        }
    }
    out_ << "Colony session ended.\n";
}
