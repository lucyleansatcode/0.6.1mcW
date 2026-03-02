#ifndef MEMORY_BUDGET_H
#define MEMORY_BUDGET_H

#include <cstddef>
#include "../platform/log.h"

class MemoryBudget
{
public:
    enum Subsystem
    {
        SUBSYS_CHUNK_MESH = 0,
        SUBSYS_TEXTURES,
        SUBSYS_AUDIO,
        SUBSYS_ENTITY_DATA,
        SUBSYS_UI,
        SUBSYS_COUNT
    };

    static void add(Subsystem subsystem, int deltaBytes) {
        if (subsystem < 0 || subsystem >= SUBSYS_COUNT)
            return;

        int* usage = getUsageStore();
        const int* budgets = getBudgetStore();

        usage[subsystem] += deltaBytes;
        if (usage[subsystem] < 0)
            usage[subsystem] = 0;

        if (usage[subsystem] > budgets[subsystem]) {
            LOGI("[mem] %s over budget: %d / %d bytes\n",
                getName(subsystem), usage[subsystem], budgets[subsystem]);
        }
    }

    static int getUsage(Subsystem subsystem) {
        return subsystem >= 0 && subsystem < SUBSYS_COUNT ? getUsageStore()[subsystem] : 0;
    }

    static int getBudget(Subsystem subsystem) {
        return subsystem >= 0 && subsystem < SUBSYS_COUNT ? getBudgetStore()[subsystem] : 0;
    }

    static bool hasBudget(Subsystem subsystem, int bytesToAdd) {
        return (getUsage(subsystem) + bytesToAdd) <= getBudget(subsystem);
    }

    static const char* getName(Subsystem subsystem) {
        switch (subsystem) {
            case SUBSYS_CHUNK_MESH: return "chunk_mesh";
            case SUBSYS_TEXTURES: return "textures";
            case SUBSYS_AUDIO: return "audio";
            case SUBSYS_ENTITY_DATA: return "entity_data";
            case SUBSYS_UI: return "ui";
            default: return "unknown";
        }
    }

private:
    static int* getUsageStore() {
        static int usage[SUBSYS_COUNT] = {0, 0, 0, 0, 0};
        return usage;
    }

    static const int* getBudgetStore() {
        // Conservative 96MB split to avoid OOM on low-memory handheld targets.
        static const int budgets[SUBSYS_COUNT] = {
            36 * 1024 * 1024, // chunks / meshes
            28 * 1024 * 1024, // textures
            12 * 1024 * 1024, // audio
            14 * 1024 * 1024, // entity data
            6  * 1024 * 1024  // UI
        };
        return budgets;
    }
};

#endif
