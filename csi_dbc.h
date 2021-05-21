#ifndef _CSI_DBC_H_
#define _CSI_DBC_H_

// types  
///////////////////////////////////////////////////////////
// DBErrorCode - status results of a csidb function call
///////////////////////////////////////////////////////////
typedef int DBErrorCode;

///////////////////////////////////////////////////////////
// DBQueryStatus - status results of a query executed
///////////////////////////////////////////////////////////
typedef int DBQueryStatus;

///////////////////////////////////////////////////////////
// DBConnectionHandle - handle returned from a data source connection
///////////////////////////////////////////////////////////
typedef unsigned int DBConnectionHandle;

///////////////////////////////////////////////////////////
// DBQueryHandle - handle returned from a query executed
///////////////////////////////////////////////////////////
typedef unsigned int DBQueryHandle;

///////////////////////////////////////////////////////////
// pTob1 - pointer to the tob1 formatted data structure
///////////////////////////////////////////////////////////
typedef void* pTob1;

///////////////////////////////////////////////////////////
// Tob1DataSize - size of the Tob1 data structure
///////////////////////////////////////////////////////////
typedef unsigned int Tob1DataSize;

///////////////////////////////////////////////////////////
// CsiTime - standard CsiTime
///////////////////////////////////////////////////////////
typedef long long CsiTime;

///////////////////////////////////////////////////////////
// CSI defined semicolon delimited connection string
// Format Below:
// DBType=<DataBaseType>;
// User ID="<user login name>";
// Password="<user's password>";
// Data Source="<user's data source name or IP address and database instance>";
// Initial Catalog="<database to connect to>";
// 
// Examples Below:
// Note: for passwords below i.e. Password="password", the actual password should be
//       the CSI encrypted hexadecimal string.
// (MySQL Connection (contains ODBC data source name)):
// DBType=1;Data Source="MySQL Connection"
// or (for user name and password not stored)
// DBType=1;Data Source="MySQL Connection";User ID="Ned";Password="password"
//
// (SQLServer Connection):
// (SQLServer Authentication)
// DBType=2;User ID="Ned";Password="password";Data Source="NED\LNDBSQLSERVER";Initial Catalog="LoggerNet"
// or
// DBType=2;User ID="Ned";Password="password";Data Source="192.168.5.01\LNDBSQLSERVER";Initial Catalog="LoggerNet"
// (Windows Authentication) Note: DO NOT send the User ID or Password
// DBType=2;Data Source="NED\LNDBSQLSERVER";Initial Catalog="LoggerNet"
//
// (SQLServerCompact Connection):
// DBType=3;Data Source="c:\Campbell Scientific\LNDB.sdf"
//
// (PostgreSQL Connection):
// DBType=4;User ID="Ned";Password="password";Data Source="somehostaddress";Initial Catalog="LoggerNet"
//
// (Oracle Connection):
// DBType=5;User ID="Ned";Password="password";Data Source="somehostaddress";Initial Catalog="LoggerNet"
// Note: the Initial Catalog for Oracle will be the service name.


///////////////////////////////////////////////////////////
typedef const char* ConnectionString;

///////////////////////////////////////////////////////////
// DataBaseType - CSI Supported database types
///////////////////////////////////////////////////////////
enum DataBaseType
{
   MySQL = 1, 
   SQLServer, 
   SQLServerCompact,
   PostgreSQL,
   Oracle
};


// Error Codes
///////////////////////////////////////////////////////////
// DB_SUCCESS - call was successful
///////////////////////////////////////////////////////////
DBErrorCode DB_SUCCESS = 0;

///////////////////////////////////////////////////////////
// DB_CONNECTED - connected to a given data source
///////////////////////////////////////////////////////////
DBErrorCode DB_CONNECTED = 1;

///////////////////////////////////////////////////////////
// DB_NOT_CONNECTED - not connected currently or lost connection
///////////////////////////////////////////////////////////
DBErrorCode DB_NOT_CONNECTED = 2;

///////////////////////////////////////////////////////////
// DB_QUERY_FAIL - query could not execute or contained no results
///////////////////////////////////////////////////////////
DBErrorCode DB_QUERY_FAIL = 3;

///////////////////////////////////////////////////////////
// DB_MISSING_HANDLE - could not find the given handle
///////////////////////////////////////////////////////////
DBErrorCode DB_MISSING_HANDLE = 4;

///////////////////////////////////////////////////////////
// DB_UNKNOWN_ERROR - unexpected behavior occurred
///////////////////////////////////////////////////////////
DBErrorCode DB_UNKNOWN_ERROR = 5;


// status results from a query executed

///////////////////////////////////////////////////////////
// QUERY_NO_ID - query was not assigned a handle - possible DB_QUERY_FAIL
///////////////////////////////////////////////////////////
DBQueryStatus QUERY_NO_ID = 1;

///////////////////////////////////////////////////////////
// QUERY_BUSY - query still executing (retrieving records)
///////////////////////////////////////////////////////////
DBQueryStatus QUERY_BUSY = 2;

///////////////////////////////////////////////////////////
// QUERY_FINISHED - query has fully executed (all results are delivered)
///////////////////////////////////////////////////////////
DBQueryStatus QUERY_FINISHED = 3;

///////////////////////////////////////////////////////////
// QUERY_FINISHED - query has executed successfully (NO results are delivered)
///////////////////////////////////////////////////////////
DBQueryStatus QUERY_FINISHED_NO_RESULTS = 4;


// dynamic linking

/////////////////////////////////////////////////////////////////////
// ConnectByString - Connects to the user defined database
// params:
//  DBConnectionHandle* handle - returned handle for datasource connection
//  const char* connectionString - semicolon delimited connection string
// returns:
//  DBErrorCode - Was the connection successful
///////////////////////////////////////////////////////////////////// 
typedef DBErrorCode (__stdcall*Connect)(DBConnectionHandle* handle, ConnectionString connectionString); 

/////////////////////////////////////////////////////////////////////
// Disconnect - Disconnects from database
// params:
//	DBConnectionHandle handle - handle for a datasource connection	
// returns:
//  DBErrorCode - Did we disconnect?
///////////////////////////////////////////////////////////////////// 
typedef DBErrorCode (__stdcall*Disconnect)(DBConnectionHandle handle);


/////////////////////////////////////////////////////////////////////
// LogToTob1File - Logs results from a function that returns Tob1DataPtr
//				   Note: Used for debugging
//                       Each time a query is made, this file is overwritten
//                       Also, for each time GetQueryResults is called
// params:
//  char* fileName - Name and path of file to log to
// returns:
//  DBErrorCode
///////////////////////////////////////////////////////////////////// 
typedef DBErrorCode (__stdcall*LogToTob1File)(DBConnectionHandle handle, const char* fileName);


/////////////////////////////////////////////////////////////////////
// IsStillConnected - Tests to make sure the connection is still valid.
// This should be used if a long period of time has gone by
// or if it is suspect connection may have been broken.
// params:
//	DBConnectionHandle handle - handle for a datasource connection
// returns:
//  bool - Are we still connected? - true = 1; false = 0;
/////////////////////////////////////////////////////////////////////
typedef int (__stdcall*IsStillConnected)(DBConnectionHandle handle);

/////////////////////////////////////////////////////////////////////
// GetSystemDataSources - gets a list of available data sources 
// params:
//	DBErrorCode* errorCode - returns the DBErrorCode status
//  Tob1DataSize* tob1DataSize - returns the size of tob1 memory
// returns:
//   pointer to Tob1 memory location with the ASCII string list of system datasources
///////////////////////////////////////////////////////////////////// 
typedef pTob1 (__stdcall*GetSystemDataSources)(DBErrorCode* DBErrorCode, Tob1DataSize* tob1DataSize);

/////////////////////////////////////////////////////////////////////
// GetSystemDataSources - gets a list of available data sources by type 
// params:
//	DBErrorCode* errorCode - returns the DBErrorCode status
//  Tob1DataSize* tob1DataSize - returns the size of tob1 memory
//  DataBaseType dbType - The datasource type you would like to retrieve
// returns:
//   pointer to Tob1 memory location with the ASCII string list of (DataBaseType) datasources
///////////////////////////////////////////////////////////////////// 
typedef pTob1 (__stdcall*GetSystemDataSourcesByType)(DBErrorCode* DBErrorCode, Tob1DataSize* tob1DataSize, DataBaseType dbType);


/////////////////////////////////////////////////////////////////////
// GetLastError - gets the last error produced by a database 
//	connection attempt
// returns:
//   char* lastError - a string representation of the error
///////////////////////////////////////////////////////////////////// 
typedef char* (__stdcall*GetLastErrorMsg)();

/////////////////////////////////////////////////////////////////////
// GetLastErrorForConn - gets the last error produced by the given database connection 
// returns:
//   char* lastError - a string representation of the error
///////////////////////////////////////////////////////////////////// 
typedef char* (__stdcall*GetLastErrorMsgForConn)(DBConnectionHandle handle);

/////////////////////////////////////////////////////////////////////
// GetInitialCatalog - get a list of databases for the
// Note: You MUST connect first to the datasource to get the
//       initial catalog(database list)
// Example: (notice Initial Catalog= is missing)
// (SQLServer Connection):
// DBType=2;User ID="Ned";Password="password";Data Source="NED\LNDBSQLSERVER""
// params:
//	DBConnectionHandle handle - handle for a datasource connection
//  DBErrorCode* errorCode - returns the DBErrorCode status
// returns:
//  DBQueryHandle - the handle for this query
/////////////////////////////////////////////////////////////////////
typedef DBQueryHandle (__stdcall*GetInitialCatalog)(DBConnectionHandle handle, DBErrorCode* errorCode);

/////////////////////////////////////////////////////////////////////
// GetTables - gets a list of database tables
// params:
//	DBConnectionHandle handle - handle for a datasource connection	
//  DBErrorCode* errorCode - returns the DBErrorCode status
// returns:
//  DBQueryHandle - the handle for this query
///////////////////////////////////////////////////////////////////// 
typedef DBQueryHandle (__stdcall*GetTables)(DBConnectionHandle handle, DBErrorCode* errorCode);

/////////////////////////////////////////////////////////////////////
// GetColumns - gets a list of columns is a table
// params:
//	DBConnectionHandle handle - handle for a datasource connection	
//  DBErrorCode* errorCode - returns the DBErrorCode status
//  char* tableName - name of table in database to get columns
// returns:
//   DBQueryHandle - the handle for this query
///////////////////////////////////////////////////////////////////// 
typedef DBQueryHandle (__stdcall*GetColumns)(DBConnectionHandle handle, DBErrorCode* errorCode, const char* tableName);


/////////////////////////////////////////////////////////////////////
// GetNumRecordsInTable - gets a total number of records in a table
// params:
//	DBConnectionHandle handle - handle for a datasource connection	
//  DBErrorCode* errorCode - returns the DBErrorCode status
//  char* tableName - name of table in database to query
// returns:
//   unsigned int - the total number of records in the given table
///////////////////////////////////////////////////////////////////// 
typedef unsigned int (__stdcall*GetNumRecordsInTable)(DBConnectionHandle handle, DBErrorCode* errorCode, const char* tableName);

/////////////////////////////////////////////////////////////////////
// GetMetaData - gets the full contents of the metatable or only contents of tableName specified
// params:
//	DBConnectionHandle handle - handle for a datasource connection
//  DBErrorCode* errorCode - returns the DBErrorCode status
//  char* tableName - name of table to retrieve metadata for
//                    Note: no name for table will retrieve whole MetaData Table
// returns:
//  DBQueryHandle - the handle for this query
///////////////////////////////////////////////////////////////////// 
typedef DBQueryHandle (__stdcall*GetMetaData)(DBConnectionHandle handle, DBErrorCode* errorCode, const char* tableName);

/////////////////////////////////////////////////////////////////////
// GetDataBetweenTime - gets data between begin and end times
//  Note: Includes the begin time, but not the end time records.
// params:
//	DBConnectionHandle handle - handle for a datasource connection
//  DBErrorCode* errorCode - returns the DBErrorCode status
//  char* tableName - name of table in database to query
//  const char* columnList - list of columns to query in given tableName
//                           list should be formatted as comma separated 
//                           data with null terminating char.
//                           i.e.: ColA, ColB, ColC\0

//                           Note: TimeStamp and RecordNumber will be included automatically
//                                 * character will include all columns
//	CsiTime beginTime - the timestamp at which to start the query
//	CsiTime endTime - the timestamp at which to end the query
// returns:
//	DBQueryHandle - the handle for this query
/////////////////////////////////////////////////////////////////////
typedef DBQueryHandle (__stdcall*GetDataBetweenTime)
	(DBConnectionHandle handle, DBErrorCode* errorCode, const char* tableName, const char* columnList, 
	 CsiTime beginTime, CsiTime endTime);

/////////////////////////////////////////////////////////////////////
// GetNumRecordsBetweenTime - gets the number of records in a table between begin and end times
//  Note: Includes the begin time, but not the end time records.
// params:
//	DBConnectionHandle handle - handle for a datasource connection
//  DBErrorCode* errorCode - returns the DBErrorCode status
//  char* tableName - name of table in database to query
//	CsiTime beginTime - the timestamp at which to start the query
//	CsiTime endTime - the timestamp at which to end the query
// returns:
//	DBQueryHandle - the handle for this query
/////////////////////////////////////////////////////////////////////
typedef DBQueryHandle (__stdcall*GetNumRecordsBetweenTime)
	(DBConnectionHandle handle, DBErrorCode* errorCode, const char* tableName, CsiTime beginTime, CsiTime endTime);
	 
/////////////////////////////////////////////////////////////////////
// GetDataBeginEnd - gets data between begin and end times
//  Note: Includes the begin time, but not the end time records.
// params:
//	DBConnectionHandle handle - handle for a datasource connection
//  DBErrorCode* errorCode - returns the DBErrorCode status
//  char* tableName - name of table in database to query
//  const char* columnList - list of columns to query in given tableName
//                           list should be formatted as comma separated 
//                           data with null terminating char.
//                           i.e.: ColA, ColB, ColC\0
//                           Note: TimeStamp and RecordNumber will be included automatically
//                                 * character will include all columns 
//	CsiTime beginTime - the timestamp at which to start the query
//	CsiTime endTime - the timestamp at which to end the query
//	int recordCount - number of records to retrieve
// returns:
//	DBQueryHandle - the handle for this query
/////////////////////////////////////////////////////////////////////
typedef DBQueryHandle (__stdcall*GetDataBeginEnd)
	(DBConnectionHandle handle, DBErrorCode* errorCode, const char* tableName, const char* columnList, 
	 CsiTime beginTime, CsiTime endTime, int recordCount);


/////////////////////////////////////////////////////////////////////
// GetDataFromTime - gets data(recordCount records) from beginTime  
// params:
//	DBConnectionHandle handle - handle for a datasource connection
//  DBErrorCode* errorCode - returns the DBErrorCode status
//  char* tableName - name of table in database to query
//  const char* columnList - list of columns to query in given tableName
//                           list should be formatted as comma separated 
//                           data with null terminating char.
//                           i.e.: ColA, ColB, ColC\0
//                           Note: TimeStamp and RecordNumber will be included automatically
//                                 * character will include all columns
//	CsiTime beginTime - the timestamp at which to start the query
//	int recordCount - number of records to retrieve
// returns:
//  DBQueryHandle - the handle for this query
/////////////////////////////////////////////////////////////////////
typedef DBQueryHandle (__stdcall*GetDataFromTime)
	(DBConnectionHandle handle, DBErrorCode* errorCode, const char* tableName, const char* columnList,
	CsiTime beginTime, int recordCount);


/////////////////////////////////////////////////////////////////////
// GetDataTimeRecordNum - gets data(recordCount records) from beginTime  
// params:
//	DBConnectionHandle handle - handle for a datasource connection
//  DBErrorCode* errorCode - returns the DBErrorCode status
//  char* tableName - name of table in database to query
//  const char* columnList - list of columns to query in given tableName
//                           list should be formatted as comma separated 
//                           data with null terminating char.
//                           i.e.: ColA, ColB, ColC\0
//                           Note: TimeStamp and RecordNumber will be included automatically
//                                 * character will include all columns
//	CsiTime beginTime - the timestamp at which to start the query
//  unsigned int beginRecordNumber - record number to start from
//	int recordCount - number of records to retrieve
//  bool includeFirstRecord - include the first record with data? true = yes
// returns:
//  DBQueryHandle - the handle for this query
/////////////////////////////////////////////////////////////////////
typedef DBQueryHandle (__stdcall*GetDataTimeRecordNum)
	(DBConnectionHandle handle, DBErrorCode* errorCode, const char* tableName, const char* columnList, 
	 CsiTime beginTime,unsigned int beginRecordNumber, int recordCount, bool includeFirstRecord);

/////////////////////////////////////////////////////////////////////
// GetDataFirst - gets data(recordCount records) at front of a table 
// params:
//	DBConnectionHandle handle - handle for a datasource connection
//  DBErrorCode* errorCode - returns the DBErrorCode status
//  char* tableName - name of table in database to query
//  const char* columnList - list of columns to query in given tableName
//                           list should be formatted as comma separated 
//                           data with null terminating char.
//                           i.e.: ColA, ColB, ColC\0
//                           Note: TimeStamp and RecordNumber will be included automatically
//                                 * character will include all columns
//	int recordCount - number of records to retrieve
// returns:
//  DBQueryHandle - the handle for this query
/////////////////////////////////////////////////////////////////////
typedef DBQueryHandle (__stdcall*GetDataFirst)
	(DBConnectionHandle handle, DBErrorCode* DBErrorCode, const char* tableName, 
	const char* columnList, int recordCount);

/////////////////////////////////////////////////////////////////////
// GetDataLast - gets data(recordCount records) at end of table  
// params:
//	DBConnectionHandle handle - handle for a datasource connection
//  DBErrorCode* errorCode - returns the DBErrorCode status
//  char* tableName - name of table in database to query
//  const char* columnList - list of columns to query in given tableName
//                           list should be formatted as comma separated 
//                           data with null terminating char.
//                           i.e.: ColA, ColB, ColC\0
//                           Note: TimeStamp and RecordNumber will be included automatically
//                                 * character will include all columns
//	int recordCount - number of records to retrieve
// returns:
//  DBQueryHandle - the handle for this query
/////////////////////////////////////////////////////////////////////
typedef DBQueryHandle (__stdcall*GetDataLast)
	(DBConnectionHandle handle, DBErrorCode* DBErrorCode, const char* tableName, 
	const char* columnList, int recordCount);

/////////////////////////////////////////////////////////////////////
// GetStationTableFromDBTablename - gets the associated station name and table name
//  for the given dbTableName
// params:
//	DBConnectionHandle handle - handle for a datasource connection
//  DBErrorCode* errorCode - returns the DBErrorCode status
//  char* dbTableName - name of dbTableName in LNDBTableMeta table to query
//    for the result
// returns:
//  DBQueryHandle - the handle for this query
/////////////////////////////////////////////////////////////////////
typedef DBQueryHandle(__stdcall*GetStationTableFromDBTablename)
   (DBConnectionHandle handle, DBErrorCode* DBErrorCode, const char* dbTableName);


/////////////////////////////////////////////////////////////////////
// GetQueryResults - gets from the query with the given handle
// params:
//	DBQueryHandle queryHandle - the handle for a query to get data from
//  int recordCount - The MAXIMUM number of tob1 records to return
//                    
//  DBQueryStatus* queryStatus - returns the current status of the query
//  Tob1DataSize* tob1DataSize - total Size of all data returned
// returns:
//	pointer to Tob1 memory location with next results of the query
///////////////////////////////////////////////////////////////////// 
typedef pTob1 (__stdcall*GetQueryResults)(DBQueryHandle queryHandle, int recordCount, DBQueryStatus* queryStatus, 
											  Tob1DataSize* tob1DataSize);

/////////////////////////////////////////////////////////////////////
// CloseQueryResults - Closes a query and releases its handle 
//                    (this should be done upon a GetQueryResults - queryStatus of QUERY_FINISHED
//                     or to a abort the query completely)
// params:
//	DBQueryHandle - the handle for a query to close
// returns:
//	DBErrorCode - did the query close?
///////////////////////////////////////////////////////////////////// 
typedef DBErrorCode (__stdcall*CloseQueryResults)(DBQueryHandle queryHandle);

/////////////////////////////////////////////////////////////////////
// BackupDatabase - Backs up the current connected database.
// params:
//  DBConnectionHandle* handle - returned handle for datasource connection
//  const char* filePath - the path/location with the file name to backup to.
// returns:
//  DBErrorCode - Was the attempt to backup successful
///////////////////////////////////////////////////////////////////// 
typedef DBErrorCode(__stdcall*BackupDatabase)(DBConnectionHandle handle, const char* filePath);

/////////////////////////////////////////////////////////////////////
// RestoreDatabase - Restores, from a file, the given database.
// params:
//  DBConnectionHandle* handle - returned handle for datasource connection
//  const char* filePath - the path/location with the file name to restore from.
// returns:
//  DBErrorCode - Was the attempt to restore successful
///////////////////////////////////////////////////////////////////// 
typedef DBErrorCode(__stdcall*RestoreDatabase)(DBConnectionHandle handle, const char* filePath);


#endif