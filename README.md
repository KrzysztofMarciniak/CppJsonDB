# C-JSON-DBMS

A lightweight SQL database management system written in C++ that stores data in JSON files. Written for a school project.

## Overview

C-JSON-DBMS is a custom SQL database engine built from scratch in C++. It provides a relational database system with data stored in JSON format, offering a balance between the flexibility of JSON and the structured query capabilities of SQL. The system implements a comprehensive set of database features including user authentication (hashed passwords), permissions management, transaction support, and SQL query capabilities.

### Requirement libraries
* readline
* nlohmann_json
* openssl
* fmt

## Features

### Core Database Features
- **JSON-Based Storage**: All database data is stored in JSON files for easy inspection and portability
- **SQL Query Support**: Implements common SQL commands (SELECT, INSERT, UPDATE, DELETE)
- **Table Joins**: Support for INNER, LEFT, RIGHT, and FULL joins
- **WHERE Clauses**: Filter data with conditional expressions
- **ORDER BY**: Sort query results
- **LIMIT**: Restrict the number of returned rows
- **Data Types**: Support for various data types
- **Auto-increment**: Support for auto-incrementing primary keys

### Security Features
- **User Authentication**: Secure user creation with SHA-256 encrypted passwords
- **Permission System**: Granular access control for databases and operations
- **Locking Mechanism**: Prevents concurrent modifications with database-level and table-level locks

### System Features
- **Modular Architecture**: Easily extendable command system
- **REPL Interface**: Interactive command-line interface with colored prompts
- **Performance Metrics**: Built-in query execution time calculation
- **Logging System**: Comprehensive activity logging with different log levels
- **Transaction Support**: Basic transaction capabilities

## Getting Started

### Building the Project

```bash
make -j8
```

### Running the Application

Start the interactive REPL:
```bash
./dbms
```

Or run with commands directly:
```bash
./dbms .login username password// .use database_name// your_sql_query
```

Multiple commands can be chained using the `//` separator.

## Usage Examples

### Login and Query Example

```bash
./dbms .login krzysztof password// .use baza// select name FROM users
```

Output:
```
Logged in as: krzysztof
Switched to database: baza
Table: users
+----------------+
|name            |
+----------------+
|"nfenfun"       |
|"mfimfeimfemf"  |
|"fjefeifmeim"   |
+----------------+
```

## Command Reference

### Meta Commands

#### User Authentication
- `.login <username> <password>` - Authenticate as a user
- `.logout` - Log out the current user

#### Database Selection
- `.use <database>` - Select a database to work with

#### System Commands
- `.exit` - Exit the application
- `.help` - Display help information

### Database Management

#### Database Operations
- `.create database <name>` - Create a new database
- `.delete database <name>` - Delete a database
- `.show databases` - List all available databases

#### Permission Management
- `.add permission <permission> on <database> to <username>` - Grant permissions (alias: `.add p`)
- `.remove permission <permission> on <database> from <username>` - Revoke permissions (alias: `.rm p`)
- `.view database permissions <database>` - View all permissions for a database (alias: `.ls db p`)

#### Testing Commands
- `.set dummy user <username>` - Set a dummy user for testing
- `.set dummy database <database_name>` - Set a dummy database for testing

### SQL Commands

#### Data Definition Language (DDL)
- `CREATE TABLE <name> (<column_definitions>)` - Create a new table (aliases: `create tbl`, `create table`)
  - Column definition format: `column_name type [PRIMARY KEY] [AUTO_INCREMENT] [FOREIGN KEY REFERENCES table(column)]`
- `DROP TABLE <name>` - Delete a table (aliases: `drop tbl`, `drop table`)
- `SHOW TABLES` - List all tables in the current database (alias: `show tables`)
- `DESCRIBE <table>` - Show table structure (aliases: `desc`, `describe`)

#### Data Manipulation Language (DML)
- `SELECT <columns> FROM <table> [JOIN <table2> ON <condition>] [WHERE <condition>] [ORDER BY <column>] [LIMIT <n>]` - Query data (alias: `select`) Join types: INNER JOIN, LEFT JOIN, RIGHT JOIN, FULL JOIN
- `INSERT INTO <table> (<columns>) VALUES (<values>)` - Insert data (alias: `insert`)
- `UPDATE <table> SET <column>=<value> [WHERE <condition>]` - Update data (alias: `update`)
- `DELETE FROM <table> [WHERE <condition>]` - Delete data (alias: `delete`)
### Supported Data Types
- int - Integer values
- string - String values
### Supported Permissions
- READ - Permission to read data from a database
- WRITE - Permission to write data to a database
- CREATE - Permission to create tables in a database
- DELETE - Permission to delete tables from a database
- MANAGE - Permission to manage database permissions

## Architecture

The system follows a modular design with the following key components:

### Core Components

- **Command Processor**: Parses and executes SQL and system commands
- **SQL Manager**: Handles SQL operations and query execution
- **Database Manager**: Manages database files and metadata
- **User Manager**: Handles user authentication and password encryption
- **Lock Manager**: Provides concurrency control through file locks
- **Log Manager**: Records system activities with different log levels
- **REPL**: Provides the interactive interface with command history

### Design Patterns

- **Singleton Pattern**: Used for managers (LockManager, LogManager)
- **Command Pattern**: Each command is encapsulated in its own class
- **Factory Pattern**: Command creation is handled by factories

### Command Registration
Commands are registered in the CommandFactory class in src/commandFactory/commandFactory.cpp. The registration process is split into two main methods:

* registerMetaCommands - Registers system/meta commands
* registerSQLCommands - Registers SQL commands

#### Commands are registered with their name, implementation, description, and aliases:

```cpp
void CommandFactory::registerMetaCommands(std::string& currentUser, std::string& currentDatabase) {
    std::vector<CommandInfo> commands = {
        {".exit", std::make_shared<ExitCommand>(), "Exit the application", {".quit", ".q"}},
        {".set dummy user", std::make_shared<SetDummyUserCommand>(currentUser), "Set a dummy user, .set dummy user <user>"},
        {".set dummy database", std::make_shared<SetDummyDatabaseCommand>(currentDatabase), "Set a dummy database, .set dummy database <database>"},
        {".help", std::make_shared<HelpCommand>(*this), "Display help information", {".h"}},
        {".login", std::make_shared<LoginCommand>(currentUser), "Login command, .login <user> <password>"},
        {".logout", std::make_shared<LogoutCommand>(currentUser, currentDatabase), "Logout command, .logout"},
        {".create user", std::make_shared<CreateUserCommand>(), "Create user command, .create user <username> <password>"},
        {".create database", std::make_shared<CreateDatabaseCommand>(currentUser), "Create database command, .create database <databaseName>", {".c db"}},
        {".add permission", std::make_shared<AddPermissionCommand>(currentUser), "Add permission to database, .add permission <databaseName> <username> <permission>", {".add p"}},
        {".list databases", std::make_shared<ListDatabasesCommand>(), "List all databases", {".ls dbs"}},
        {".remove permission", std::make_shared<RemovePermissionCommand>(currentUser), "Remove permission from database, .remove permission <databaseName> <username> <permission>", {".rm p"}},
        {".view database permissions", std::make_shared<ViewAllDatabasePermissionsCommand>(currentUser), "View permissions for database, .view database permissions <databaseName>", {".ls db p"}},
        {".delete database", std::make_shared<DeleteDatabaseCommand>(currentUser), "Delete a database, .delete database <databaseName>", {".rm db"}},
        {".use", std::make_shared<UseDatabaseCommand>(currentUser, currentDatabase), "use database command, .use <database name>"}
    };

    REGISTER_COMMANDS_ASYNC(commands, this);
}
void CommandFactory::registerSQLCommands(std::string& currentUser, std::string& currentDatabase) {
    std::vector<CommandInfo> commands = {
        {"SELECT", std::make_shared<SelectCommand>(currentUser, currentDatabase), "Select command", {"select"}},
        {"INSERT", std::make_shared<InsertCommand>(currentUser, currentDatabase), "Insert command", {"insert"}},
        {"UPDATE", std::make_shared<UpdateCommand>(currentUser, currentDatabase), "Update command", {"update"}},
        {"DELETE", std::make_shared<DeleteCommand>(currentUser, currentDatabase), "Delete command", {"delete"}},
        {"CREATE TABLE", std::make_shared<CreateTableCommand>(currentUser, currentDatabase), "Create table command", {"create tbl", "create table"}},
        {"DROP TABLE", std::make_shared<DropTableCommand>(currentUser, currentDatabase), "Drop table command", {"drop tbl", "drop table"}},
        {"SHOW TABLES", std::make_shared<ShowTablesCommand>(currentUser, currentDatabase), "Show tables command", {"show tables"}},
        {"DESCRIBE", std::make_shared<DescribeCommand>(currentUser, currentDatabase), "Describe command", {"desc", "describe"}}
    };

    REGISTER_COMMANDS_ASYNC(commands, this);
}
```
## Project Structure

```
src/
├── commands/                  # Command implementations
│   ├── metacommands/          # System command implementations
│   │   ├── addPermissionCommand/
│   │   ├── createDatabaseCommand/
│   │   ├── deleteDatabaseCommand/
│   │   ├── removePermissionCommand/
│   │   ├── useDatabaseCommand/
│   │   └── [...]
│   └── sqlcommands/           # SQL command implementations
│       ├── createTableCommand/
│       ├── deleteCommand/
│       ├── describeCommand/
│       ├── insertCommand/
│       ├── selectCommand/
│       ├── showTablesCommand/
│       ├── updateCommand/
│       └── [...]
├── databaseManager/           # Database file management
├── userManager/               # User authentication and management
├── lockManager/               # Concurrency control
├── logManager/                # Logging system
├── repl/                      # Interactive shell
├── sqlManager/                # SQL query processing
└── main.cpp                   # Entry point
```

## Locking Mechanism

The system uses a file-based locking mechanism to prevent concurrent modifications:

- **Database-level locks**: Prevent concurrent modifications to database metadata
- **Table-level locks**: Prevent concurrent modifications to table data
- **User-level locks**: Prevent concurrent modifications to user information

Lock files are created in the data directory and contain information about the user and operation that acquired the lock.

The system uses the following lock types:

- **DATABASE**: Locks a specific database file
- **DATABASE_MANAGER**: Locks the database configuration file
- **USER_INFO**: Locks the user information file
- **TABLE**: Locks a specific table within a database

Locking and unlocking operations are handled through macros:

- **IS_LOCKED(resource, type)**: Checks if a resource is locked
- **LOCK(resource, type, user, operation)**: Locks a resource
- **UNLOCK(resource, type)**: Unlocks a resource

## Logging System

The logging system records all operations with different log levels:

- **INFO**: Normal operations
- **WARNING**: Potential issues
- **ERROR**: Operation failures
- **DEBUG**: Detailed information for debugging

Logs are stored in `data/logs.txt` and include timestamps, user information, and database context.
Logs are stored in `data/logs.txt` and include timestamps, user information, and database context.

## Extending the System

### Adding a New Command

The modular command architecture makes it easy to add new commands:

1. Create a new command class that inherits from the `Command` base class
2. Implement the `execute()` method
3. Register the command in the command factory

Example of a command implementation:

```cpp
class MyNewCommand : public Command {
public:
    MyNewCommand(std::string& currentUser, std::string& currentDatabase)
        : currentUser(currentUser), currentDatabase(currentDatabase) {}
    
    void execute(const std::vector<std::string>& args) override {
        // Check user is logged in
        checkUserLoggedIn(currentUser);
        
        // Check database is selected
        checkDatabaseSelected(currentUser, currentDatabase);
        
        // Check permissions
        INIT_DB_MANAGER;
        if (!dbManager.canUseDatabase(currentDatabase, currentUser)) {
            DISPLAY_MESSAGE("Error: You do not have permission to use this database.");
            return;
        }
        
        // Implement command logic
        // ...
        
        // Log the operation
        LOG(LogLevel::INFO, currentDatabase, currentUser, "MyNewCommand executed");
    }
    
private:
    std::string& currentUser;
    std::string& currentDatabase;
};
```

## Performance Considerations

- **JSON Storage**: While convenient for development and debugging, JSON storage may not be optimal for large datasets
- **Locking Mechanism**: The file-based locking system provides basic concurrency control but may become a bottleneck with many users
- **Query Optimization**: The current implementation focuses on correctness rather than optimization

## Security Considerations

- **Password Storage**: Passwords are hashed using SHA-256 before storage
- **Permission System**: Granular permissions control access to databases and operations
- **Input Validation**: Commands validate input to prevent injection attacks

## Limitations

- This is a test/educational system and not intended for production use
- Performance may be limited for large datasets due to the JSON storage format
- Concurrent access is managed through file locks but has limitations
- Complex SQL features like subqueries or window functions are not supported

## License

This project is available for educational and personal use.
