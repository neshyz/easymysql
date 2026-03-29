#if defined _easymysql_included
    #endinput
#endif

#define _easymysql_included

#if !defined _logger_included
    #tryinclude <logger>
    #tryinclude "lib/easylogs/logger.inc"
#endif

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


#if !defined EZMYSQL_MAX_TABLE_COLUMNS
    #define EZMYSQL_MAX_TABLE_COLUMNS (256)
#endif


#if !defined EZMYSQL_MAX_TABLE_NAME_SIZE
    #define EZMYSQL_MAX_TABLE_NAME_SIZE (32)
#endif


#if !defined EZMYSQL_MAX_COLUMN_NAME_SIZE
    #define EZMYSQL_MAX_COLUMN_NAME_SIZE (32)
#endif


#if !defined EZMYSQL_MAX_TABLES
    #define EZMYSQL_MAX_TABLES (128)
#endif


static enum E_MYSQL_TABLE_DATA {
    E_MYSQL_TABLE_NAME[EZMYSQL_MAX_TABLE_NAME_SIZE char],
    DatabaseColumn:E_MYSQL_COLUMNIDX[EZMYSQL_MAX_TABLE_COLUMNS],
    bool:E_MYSQL_TABLE_VALID
}
static s_TableData[MySQL:EZMYSQL_MAX_CONNECTIONS][DatabaseTable:EZMYSQL_MAX_TABLES][E_MYSQL_TABLE_DATA];




enum e_EZMYSQL_DATATYPE {
    EZMYSQL_DATATYPE_INTEGER,
    EZMYSQL_DATATYPE_FLOAT,
    EZMYSQL_DATATYPE_STRING,
    EZMYSQL_DATATYPE_BOOLEAN,
    EZMYSQL_DATATYPE_OTHER
}

static enum E_MYSQL_COLUMN_DATA {
    E_MYSQL_COLUMN_NAME[EZMYSQL_MAX_COLUMN_NAME_SIZE char],
    e_EZMYSQL_DATATYPE:E_MYSQL_COLUMN_DATATYPE,
    DatabaseTable:E_MYSQL_COLUMN_TABLEIDX,
    bool:E_MYSQL_COLUMN_VALID
}
static s_ColumnData[MySQL:EZMYSQL_MAX_CONNECTIONS][DatabaseColumn:EZMYSQL_MAX_TABLE_COLUMNS][E_MYSQL_COLUMN_DATA];




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






static DatabaseTable:GetUnusedTableIdx(MySQL:handle)
{
    for(new DatabaseTable:tableidx; tableidx < DatabaseTable:EZMYSQL_MAX_TABLES; tableidx++) {
        if(!s_TableData[handle][tableidx][E_MYSQL_TABLE_VALID]) {
            return tableidx;
        }
    }

    return DatabaseTable:-1;
}



static DatabaseColumn:GetUnusedColumnIdx(MySQL:handle)
{
    for(new DatabaseColumn:columnidx; columnidx < DatabaseColumn:EZMYSQL_MAX_TABLE_COLUMNS; columnidx++) {
        if(!s_ColumnData[handle][columnidx][E_MYSQL_COLUMN_VALID]) {
            return columnidx;
        }
    }

    return DatabaseColumn:-1;
}

stock mysql_addtable(MySQL:handle, const tableName[], const entry[] = "ID INTEGER AUTO_INCREMENT, PRIMARY KEY(ID)")
{
    mysql_format(handle, s_QueryBuffer, sizeof(s_QueryBuffer),
        "CREATE TABLE IF NOT EXISTS `%s` (\
            %s);", tableName, entry
	);
        
    mysql_tquery(handle, s_QueryBuffer, "qcb_ezmysql_addtable", "iss", _:handle, tableName, entry);
}

forward qcb_ezmysql_addtable(MySQL:handle, const tableName[], const entry[]);
public qcb_ezmysql_addtable(MySQL:handle, const tableName[], const entry[])
{
    new
        DatabaseTable:tableidx = GetUnusedTableIdx(handle)
    ;

    if(mysql_errno(handle) != 0 || tableidx == DatabaseTable:-1) {

        mysql_error(s_ErrorBuffer);

        sendlog(s_Logger, LOG_ERROR,
            "table creation fail",
            logger_string("tableName", tableName),
            logger_string("entry", entry),
            logger_string("error", s_ErrorBuffer),
            logger_int("tableidx", _:tableidx)

        );

    }
    else {

        strpack(s_TableData[handle][tableidx][E_MYSQL_TABLE_NAME], tableName);
        s_TableData[handle][tableidx][E_MYSQL_TABLE_VALID] = true;
    
    }
}








stock mysql_columnexists(MySQL:handle, const table[], const columnName[])
{
    new unpackedColumnIdx[EZMYSQL_MAX_COLUMN_NAME_SIZE];
    for(new DatabaseColumn:columnidx; columnidx < DatabaseColumn:EZMYSQL_MAX_TABLE_COLUMNS; columnidx++) {
        if(!s_ColumnData[handle][columnidx][E_MYSQL_COLUMN_VALID]) {
            continue;
        }
        strunpack(unpackedColumnIdx, s_ColumnData[handle][columnidx][E_MYSQL_COLUMN_NAME]);

        if(strcmp(unpackedColumnIdx, columnName, true) == 0) {
            return 1;
        }
    }


    new dbname[EZMYSQL_MAX_DBNAME_SIZE];
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

stock mysql_addcolumn(MySQL:handle, const DatabaseTable:tableidx, const columnName[], e_EZMYSQL_DATATYPE:datatype)
{
    new tableName[EZMYSQL_MAX_TABLE_NAME_SIZE];
    strunpack(tableName, s_TableData[handle][tableidx][E_MYSQL_TABLE_NAME]);


    if(mysql_columnexists(handle, tableName, columnName)) {

        return;
        
    }

    format(s_QueryBuffer, sizeof(s_QueryBuffer),

        "ALTER TABLE `%s` ADD COLUMN `%s` %s;\n",
        tableName, columnName, typeData

    );

    mysql_tquery(handle, s_QueryBuffer, "qcb_ezmysql_addcolumn", "iisi", _:handle, _:tableidx, columnName, _:datatype);
}

forward qcb_ezmysql_addcolumn(MySQL:handle, const DatabaseTable:tableidx, const columnName[], e_EZMYSQL_DATATYPE:datatype);
public qcb_ezmysql_addcolumn(MySQL:handle, const DatabaseTable:tableidx, const columnName[], e_EZMYSQL_DATATYPE:datatype)
{
    new
        DatabaseColumn:columnidx = GetUnusedColumnIdx(handle)
    ;

    if(mysql_errno(handle) != 0) {

        mysql_error(s_ErrorBuffer);

        sendlog(s_Logger, LOG_ERROR,
            "mysql column creation failed",
            logger_int("handle", _:handle),
            logger_string("table", table),
            logger_string("columnName", columnName),
            logger_string("typeData", typeData),
            logger_string("error", s_ErrorBuffer),
            logger_int("columnidx", _:columnidx)
        );

    }
    else {

        strpack(s_ColumnData[handle][columnidx][E_MYSQL_COLUMN_NAME], columnName);
        s_ColumnData[handle][columnidx][E_MYSQL_COLUMN_VALID] = true;

    }
}




stock mysql_removecolumn(MySQL:handle, DatabaseTable:tableidx, DatabaseColumn:columnidx)
{
    if(!mysql_columnexists(handle, tableidx, columnidx)) {

        return;
        
    }



    new
        tableName[EZMYSQL_MAX_TABLE_NAME_SIZE],
        columnName[EZMYSQL_MAX_COLUMN_SIZE]
    ;
    strunpack(tableName, s_TableData[handle][E_MYSQL_TABLE_NAME]);
    strunpack(columnName, s_ColumnData[handle][E_MYSQL_COLUMN_NAME]);

    format(s_QueryBuffer, sizeof(s_QueryBuffer),

        "ALTER TABLE `%s` DROP COLUMN `%s`;\n",
        tableName, columnName

    );

    mysql_tquery(handle, s_QueryBuffer, "qcb_ezmysql_removecolumn", "iiiss", _:handle, _:tableidx, _:columnidx, tableName, columnName);
}



forward qcb_ezmysql_removecolumn(MySQL:handle, DatabaseTable:tableidx, DatabaseColumn:columnidx, const tableName[], const columnName[]);
public qcb_ezmysql_removecolumn(MySQL:handle, DatabaseTable:tableidx, DatabaseColumn:columnidx, const tableName[], const columnName[])
{
    if(mysql_errno(handle) != 0) {

        mysql_error(s_ErrorBuffer);

        sendlog(s_Logger, LOG_ERROR,
            "mysql column removal failed",
            logger_int("handle", _:handle),
            logger_string("table", tableName),
            logger_string("columnName", columnName),
            logger_string("error", s_ErrorBuffer),
            logger_int("tableidx", _:tableidx),
            logger_int("columnidx", _:columnidx),

        );

    }
    else {
        s_ColumnData[handle][columnidx][E_MYSQL_COLUMN_VALID] = false;
        strpack(s_ColumnData[handle][columnidx][E_MYSQL_COLUMN_NAME], "");
    }
}