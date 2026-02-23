#if defined _easymysql_included
    #endinput
#endif

#define _easymysql_included


// instalar "logger.inc"
#tryinclude "lib/easylogs/logger.inc"
#tryinclude <logger>

#if !defined _logger_included
    #error "install easylogs.inc ->   neshyz/easylogs.git"
#endif





// crear un buffer de queries global
#if !defined EZMYSQL_MAX_QUERY_BUFFER_SIZE
    #define EZMYSQL_MAX_QUERY_BUFFER_SIZE (2048)
#endif

#if !defined EZMYSQL_MAX_ERROR_BUFFER_SIZE
    #define EZMYSQL_MAX_ERROR_BUFFER_SIZE (512)
#endif

static s_QueryBuffer[EZMYSQL_MAX_QUERY_BUFFER_SIZE];
static s_ErrorBuffer[EZMYSQL_MAX_ERROR_BUFFER_SIZE];


// tamaño buffer de conexiones mysql
#if !defined EZMYSQL_MAX_CONNECTIONS
    #define EZMYSQL_MAX_CONNECTIONS (3)
#endif


// tamaños de buffer parametros de conexion mysql
#if !defined EZMYSQL_MAX_PASS_SIZE
    #define EZMYSQL_MAX_PASS_SIZE (32)
#endif

#if !defined EZMYSQL_MAX_DBNAME_SIZE
    #define EZMYSQL_MAX_DBNAME_SIZE (16)
#endif

#if !defined EZMYSQL_MAX_USER_SIZE
    #define EZMYSQL_MAX_USER_SIZE (16)
#endif

#if !defined EZMYSQL_MAX_HOST_SIZE
    #define EZMYSQL_MAX_HOST_SIZE (16)
#endif



// definir loglevel de ezmysql
#if !defined EZMYSQL_LOG_LEVEL
    #define EZMYSQL_LOG_LEVEL (LOG_WARN)
#endif


static enum E_MYSQL_CONNECTION_DATA {
    E_MYSQL_CONNECTION_DBNAME[EZMYSQL_MAX_DBNAME_SIZE char],
    E_MYSQL_CONNECTION_PASSWORD[EZMYSQL_MAX_PASS_SIZE char],
    E_MYSQL_CONNECTION_USER[EZMYSQL_MAX_USER_SIZE char],
    E_MYSQL_CONNECTION_HOST[EZMYSQL_MAX_HOST_SIZE char]
}

static s_ConnectionData[MySQL:EZMYSQL_MAX_CONNECTIONS][E_MYSQL_CONNECTION_DATA];

static Logger:s_Logger = INVALID_LOGGER_ID;














stock ezmysql_init()
{
    s_Logger = GetLogger("ezmysql", .fileAppend = true, .consoleAppend = false, .level = EZMYSQL_LOG_LEVEL);

    if(!IsValidLogger(s_Logger)) {
        return 0;
    }
    else {
        return 1;
    }
}







stock MySQL:ezmysql_connect(const host[], const user[], const password[], const dbname[], MySQLOpt:option_id = MySQLOpt:0)
{
    new MySQL:handle = mysql_connect(host, user, password, dbname, option_id);

    if(handle != MYSQL_INVALID_HANDLE && mysql_errno(handle) == 0) {
        strpack(s_ConnectionData[handle][E_MYSQL_CONNECTION_HOST], host);
        strpack(s_ConnectionData[handle][E_MYSQL_CONNECTION_USER], user);
        strpack(s_ConnectionData[handle][E_MYSQL_CONNECTION_PASSWORD], password);
        strpack(s_ConnectionData[handle][E_MYSQL_CONNECTION_DBNAME], dbname);

        return handle;
    }
    else {
        mysql_error(s_ErrorBuffer, .handle = handle);
        
        sendlog(s_Logger, LOG_FATAL, "failed to connect",
            logger_int("handle", _:handle),
            logger_string("host", host),
            logger_string("user", user),
            logger_string("password", password),
            logger_string("host", host),
            logger_string("error", s_ErrorBuffer)
        );

        return MYSQL_INVALID_HANDLE;
    }
}






stock mysql_addtable(MySQL:handle, const tableName[], const entry[] = "ID INTEGER AUTO_INCREMENT, PRIMARY KEY(ID)")
{
    mysql_format(handle, s_QueryBuffer, sizeof(s_QueryBuffer),
        "CREATE TABLE IF NOT EXISTS `%s` (\
            %s);", tableName, entry
	);
        
    mysql_query(handle, s_QueryBuffer, true);

    if(mysql_errno(handle) != 0) {

        mysql_error(s_ErrorBuffer);

        sendlog(s_Logger, LOG_ERROR,
            "table creation fail",
            logger_string("tableName", tableName),
            logger_string("entry", entry),
            logger_string("error", s_ErrorBuffer)

        );

    }
}









stock mysql_columnexists(MySQL:handle, const table[], const columnName[])
{
    new dbName[EZMYSQL_MAX_DBNAME_SIZE];
    strunpack(dbname, s_ConnectionData[handle][E_MYSQL_CONNECTION_DBNAME]);

    mysql_format(handle, s_QueryBuffer, sizeof(s_QueryBuffer),
        "SELECT COUNT(*) AS column_exists \
        FROM information_schema.COLUMNS \
        WHERE TABLE_NAME = '%e' \
        AND TABLE_SCHEMA = '%e' \
        AND COLUMN_NAME = '%e';",
        table,
        dbname,
        columnName
    );

    mysql_query(handle, s_QueryBuffer, true);

    new int;
    cache_get_value_name_int(0, "column_exists", int);

    return int;
}

stock mysql_addcolumn(MySQL:handle, const table[], const columnName[], const typeData[])
{
    if(mysql_columnexists(handle, table, columnName)) {

        return;
        
    }

    format(s_QueryBuffer, sizeof(s_QueryBuffer),

        "ALTER TABLE `%s` ADD COLUMN `%s` %s;\n",
        table, columnName, typeData

    );

    mysql_query(handle, s_QueryBuffer, true);

    if(mysql_errno(handle) != 0) {

        mysql_error(s_ErrorBuffer);

        sendlog(s_Logger, LOG_ERROR,
            "mysql column creation failed",
            logger_int("handle", _:handle),
            logger_string("table", table),
            logger_string("columnName", columnName),
            logger_string("typeData", typeData),
            logger_string("error", s_ErrorBuffer)
        );

    }
}