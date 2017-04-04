/*
 * tasks_utils.h
 *
 *  Created on: 28.12.2016
 *      Author: Bartosz Bielawski
 */

#ifndef TASKS_UTILS_H_
#define TASKS_UTILS_H_

#include "task.hpp"

void setupTasks();
Tasks::Task* addTask(Tasks::Task* task);
void scheduleTasks();

#endif /* TASKS_UTILS_H_ */
