/**
 * CDatabase.h
 * Framework for DB access via MariaDb/C connector
 * on simple databases
 *
 * is also wrapper for the
 *
 * for API-documentation of the
 *
 * \author Holger Frank - FB EIT h_da <holger.frank@h-da.de>
 */
#ifndef CDatabase_H
#define CDatabase_H

#include <iostream>
#include <iomanip>
#include <string>
#include <mysql.h>

using namespace std;

/**
 * possible states of db connection
 */
enum DB_CONN_STATE {
	DB_S_NOTREADY, DB_S_CONNECTED, DB_S_FETCH
};

/**
 * database class
 *
 * controls the connection to a mysql database
 * does not throw exceptions (derived class may throw or use traditional error handling)
 */
class CDatabase {
protected:
	/**
	 * current state of CDatabase object
	 */
	DB_CONN_STATE m_State;

	/**
	 * current row of data
	 *
	 * contains a row of data of the most recent select query and is updated
	 * with next row of the current resultset by every call of fetch() until
	 * end of data is reached
	 */
	MYSQL_ROW m_DataSet;

private:
	/**
	 * handler of mysql/mariadb connection
	 *
	 * has to be always initialized with mysql_init() before usable
	 * with mysql_real_connect()
	 */
	MYSQL* m_ConnHandler;

	/**
	 * pointer on current resultset
	 *
	 * is generated and filled when a select statement is performed by _executeSQLStmt()
	 * is cleared by call of closeQuery()
	 */
	MYSQL_RES* m_ResultSet;

	/**
	 * list of column names of the most recent select query
	 *
	 * \see mysql_fetch_fields()
	 */
	MYSQL_FIELD* m_ResultHeader;

	/**
	 * id of the last inserted row
	 *
	 * stores the value of a field with AUTOINCREMENT option in a table
	 * when one or more rows are inserted, in case of a multiple insert
	 * operation the id of the FIRST inserted row is stored
	 */
	my_ulonglong m_lastInsertId;

	/**
	 * if true _executeSQLStmt prints statement and error (default true)
	 */
	bool m_bPrint;

public:
	/*
	 * common db methods
	 */
	CDatabase();
	~CDatabase();

	/**
	 *	fetches a row of data from the current resultset
	 *
	 *	before any data can be fetched a select query has to be executed
	 *	successfully with an appropriate method of a derived class or with
	 *	_executeSQLStmt()
	 *
	 *	\return true: data fetched, false: no more data available or error
	 *	ocured (error is printed on console)
	 */
	bool fetch();

	/**
	 * closes query action
	 *
	 * must be called after select and the associated calls of fetch()
	 *
	 * frees the result set and sets db object to state DB_S_CONNECTED
	 */
	void closeQuery();

	/*
	 * closes DB connection
	 */
	void close();

	/*
	 * prints current state of db connection on stdout
	 */
	void showState(void);

	/**
	 * prints the names of columns of the most recent select query
	 */
	void printHeader();

	/**
	 * sets default database in opened connection via USE statement
	 *
	 * \see https://dev.mysql.com/doc/refman/5.7/en/use.html
	 *
	 * \param name name of the database
	 * \return operation successful/not successful
	 */
	bool setDatabase(const string name);

	/**
	 * returns the id of the last inserted row
	 *
	 * the table needs one numeric field with UNIQUE and AUTOINCREMENT option
	 *
	 * in case of inserting multiple rows, the id of the FIRST inserted
	 * row is returned
	 */
	unsigned long long getLastInsertId(void);

	/**
	 * shall Statements and SQL errors be printed?
	 * yes ... bpstmts==true
	 * no..... bpstmts==false
	 */
	void allowPrint(bool bpstmts=true);


	/**
	 * retrieve SQL error message including SQL error code
	 * infotxt: special text to be prefixed to the error message
	 */
	string getSQLErrorMessage(const string infotxt);

protected:
	/**
	 * opens the database on given server under given credentials
	 * is protected, because using databases with the base class does not make sense
	 * derived class may use this in an public open method with or without exception handling
	 *
	 * \param server FQDN or IP of DB-server
	 * \param DBUser username to be logged in
	 * \param DBPassword password of DBUser
	 * \param DBName name of the database on the server
	 * (can also be set by method setDb())
	 *
	 * \return operation successful/not successful (use _getSQLError and _getNativeErrorCode to get information about the reason)
	 */
	bool _open(const string host = "localhost", const string DBUser = "",
			const string DBPassword = "", const string DBName = "");

	/**
	 * returns field value of current dataset
	 *
	 * the field is determined by numerical index
	 *
	 * \param idx numerical index
	 * \return value as string
	 */
	string _getValueByIndex(const int idx);

	/**
	 * returns field value of current dataset
	 *
	 * the field is determined by the name of the column
	 *
	 * the name of the column can be either the value of the field
	 * determined by the definition of the table or by the alias
	 * defined via an AS statement in the query string
	 *
	 * e.g. SELECT id AS FilterID FROM ....
	 *
	 * \see https://www.w3schools.com/sql/sql_alias.asp
	 *
	 * \param idx numerical index
	 * \return value as string or NULL if name is not found in resultset
	 */
	string _getValueByName(const char* name);

	/**
	 * sends SQL query to db server
	 *
	 * sends the query string unmodified and unvalidated to the db server
	 *
	 * after call of this method a call of fetch() is necessary,
	 * otherwise error 'Commands out of sync' occurs
	 *
	 * \param SQLStmt string SQL statement
	 * \param errText preceding text of SQL error message
	 * \param isSelectStmt true if it is a SELECT statement which returns
	 * a result set
	 * \param printStmt true if statement should be printed
	 * \return -1 error (use _getSQLError and _getNativeErrorCode to get information about the reason)
	 * for select statements - returns result sets number of rows
	 * for insert, update, delete statements - returns the number of effected rows
	 */
	int _executeSQLStmt(const string SQLStmt, const string errText = "",
			const bool isSelectStmt = true);

	/**
	 * prints the SQL error text of the most recent mysql function call that can
	 * succeed or fail
	 *
	 * prints the SQL error text of the most recent mysql function call which
	 *
	 * for further details see the MySQL documentation (Version 5.7)
	 * \see https://dev.mysql.com/doc/mysql-errors/5.7/en/
	 *
	 * \param infotxt free text e.g. to include name of method in which the
	 * error occured
	 */
	void _showSQLError(const string infotxt);

	/**
	 * provides the SQL error text of the most recent mysql function call that can
	 * succeed or fail
	 *
	 * provides the SQL error text of the most recent mysql function call which
	 *
	 * for further details see the MySQL documentation (Version 5.7)
	 * \see https://dev.mysql.com/doc/mysql-errors/5.7/en/
	 *
	 * \param infotxt free text e.g. to include name of method in which the
	 * error occured
	 */
	string _getSQLError(const string infotxt);

	/**
	 * returns error code for the most recent function call that can
	 * succeed or fail. Zero means no error occurred.
	 *
	 * for further details see the MySQL documentation (Version 5.7)
	 * \see https://dev.mysql.com/doc/mysql-errors/5.7/en/
	 *
	 * \return mysql error code
	 */
	unsigned int _getNativeErrorCode(void);

private:
	/**
	 * reads name of columns of most recent select query
	 *
	 * \return pointer on MYSQL_FIELD struct
	 */
	MYSQL_FIELD* _getHeader(void);

};
#endif

