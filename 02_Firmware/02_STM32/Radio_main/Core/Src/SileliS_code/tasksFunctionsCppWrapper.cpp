/*
 * tasksFunctionsCppWrapper.cpp
 *
 *  Created on: Nov 8, 2025
 *      Author: dbank
 */

#include "SileliS_code/tasksFunctionsCppWrapper.h"

#include "SileliS_code/tasksFunctions.h"

void startUpTaskCPPWrapper(void *noThing) {
	startUpTask(noThing);
}
