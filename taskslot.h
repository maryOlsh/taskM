#ifndef TASKSLOT_H
#define TASKSLOT_H

#include "task.h"

struct TaskSlot {
    Task task;
    double start;
    double end;

    // Default constructor
    TaskSlot() : start(0.0), end(0.0) {}

    // Constructor with parameters
    TaskSlot(const Task &t, double s, double e) : task(t), start(s), end(e) {}
};

#endif // TASKSLOT_H 