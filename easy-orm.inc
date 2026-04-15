#if !defined MAX_ORM
    #define MAX_ORM (16)
#endif


#if !defined EZMYSQL_MAX_TABLE_NAME_SIZE
    #define EZMYSQL_MAX_TABLE_NAME_SIZE (32)
#endif

static enum E_ORM_DATA {
    MySQL:E_ORM_MYSQL_HANDLE,
    E_ORM_TABLE[EZMYSQL_MAX_TABLE_NAME_SIZE char],
    bool:E_ORM_VALID
}
static s_OrmData[ORM:MAX_ORM][E_ORM_DATA];


stock ORM:ezorm_create(const table[], MySQL:handle)
{
    new
        ORM:id = orm_create(table, handle);
    
    if(id != MYSQL_INVALID_ORM) {
        s_OrmData[id][E_ORM_MYSQL_HANDLE] = handle;
        s_OrmData[id][E_ORM_VALID] = true;
        strpack(s_OrmData[id][E_ORM_TABLE], table);
    }

    return id;
}





stock ezorm_addcolumn_int(ORM:id, &var, const columnname[], const entry[])
{
    if(ezorm_addcolumn(id, columnname, entry, "qcb_ezorm_addcolumn_int")) {
        
    }
}






static ezorm_addcolumn(ORM:id, const columnname[], const entry[], const callback[])
{
    if(!s_OrmData[id][E_ORM_VALID]){
        return;
    }

    new tableName[EZMYSQL_MAX_TABLE_NAME_SIZE];
    strunpack(tableName, s_OrmData[id][E_ORM_TABLE]);

    mysql_addcolumn(s_OrmData[id][E_ORM_MYSQL_HANDLE], tableName, columnname, entry, "qcb_ezorm_addcolumn", _:id);
}


forward qcb_ezorm_addcolumn_int(MySQL:id, const tableName[], const columnname[], const entry[], const ORM:id);
public qcb_ezorm_addcolumn_int(MySQL:id, const tableName[], const columnname[], const entry[], const ORM:id)
{
    orm_addvar_int(id, var, columnname);
}



























static stock crash()
{
    new File:fileThatDoesNotExist = fopen("idonotexist3948");
    fwrite(fileThatDoesNotExist, "");
}