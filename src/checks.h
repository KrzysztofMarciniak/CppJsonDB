#ifndef CHECKS_H
#define CHECKS_H

#include <string>
#include "logManager/logManager.h"

void checkUserLoggedIn(const std::string& currentUser);
void checkDatabaseSelected(const std::string& currentUser, const std::string& currentDatabase);

#endif
