#include "headers/listDatabasesCommand.h"
#include "lockManager/lockManager.h"


void ListDatabasesCommand::execute(const std::vector<std::string>& args) {
    INIT_DB_MANAGER;
    dbManager.loadDatabasesFromFile();
    if (IS_LOCKED("", LockFileType::DATABASE_MANAGER)){
        std::cout << "databases config file is currently locked... try again later\n";
        return;
    }
    LOCK("",LockFileType::DATABASE_MANAGER, "", "LIST DATABASES");
    UNLOCK("",LockFileType::DATABASE_MANAGER);
    dbManager.listDatabases();
}

