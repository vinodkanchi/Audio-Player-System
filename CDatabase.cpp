/**
 * CDatabase.h
 * Framework for DB access via MariaDb/C connector
 * on simple databases
 **/
// header
#include <iostream>	
#include <iomanip>
#include <cstring>
#include <stdlib.h>
#include <stdio.h>
using namespace std;

#include "CDatabase.h"

CDatabase::CDatabase() {
	m_State = DB_S_NOTREADY;
	m_ConnHandler = NULL;
	m_ResultSet = NULL;
	m_DataSet = NULL;
	m_ResultHeader = NULL;
	m_lastInsertId = 0;
	m_bPrint=true;
}

CDatabase::~CDatabase() {
	close();
}

bool CDatabase::setDatabase(const string dbname) {
	// build command string & execute
	string cmd("USE ");
	cmd += dbname;
	cmd += ";";
	return _executeSQLStmt(cmd, "CDatabase::setDatabase()", false);
}

void CDatabase::close() {
	if (m_State != DB_S_NOTREADY) {
		mysql_close(m_ConnHandler);
		m_State = DB_S_NOTREADY;
	}
}

bool CDatabase::fetch() {
	if (m_State != DB_S_FETCH) {
		return false;
	}

	if (!(m_DataSet = mysql_fetch_row(m_ResultSet))) {
		return false;
	}
	return true;
}

unsigned long long CDatabase::getLastInsertId(void) {
	return m_lastInsertId;
}

void CDatabase::closeQuery(void) {
	if(m_State != DB_S_FETCH)
		return;
	mysql_free_result(m_ResultSet);
	m_DataSet = NULL;
	m_ResultHeader = NULL;
	m_State = DB_S_CONNECTED;
}

void CDatabase::showState(void) {
	cout << "current state: ";
	switch (m_State) {
	case DB_S_NOTREADY:
		cout << "DB not ready" << endl;
		break;
	case DB_S_CONNECTED:
		cout << "DB connected" << endl;
		break;
	case DB_S_FETCH:
		cout << "fetching data" << endl;
		break;
	}
}

void CDatabase::printHeader() {
	if(m_State != DB_S_FETCH)
		return;

	for (unsigned int ColIdx = 0; ColIdx < m_ResultSet->field_count; ColIdx++) {
		/*
		 * if colname is longer than longest value => set width to length of
		 * colname
		 */
		m_ResultHeader[ColIdx].max_length =
				(strlen(m_ResultHeader[ColIdx].name) >= m_ResultHeader[ColIdx].max_length) ?
						strlen(m_ResultHeader[ColIdx].name) : m_ResultHeader[ColIdx].max_length;
		cout << setw(m_ResultHeader[ColIdx].max_length) << m_ResultHeader[ColIdx].name;

		if (ColIdx < m_ResultSet->field_count - 1)
			cout << " | ";
	}
	cout << endl;
	/*
	 *  print divider line
	 */
	for (unsigned int ColIdx = 0; ColIdx < m_ResultSet->field_count; ColIdx++) {
		cout << setw(m_ResultHeader[ColIdx].max_length) << setfill('-') << "-"
				<< setfill(' ');
		if (ColIdx < m_ResultSet->field_count - 1)
			cout << "---";
	}
	cout << endl;
}

void CDatabase::allowPrint(bool bpstmts)
{
	m_bPrint=bpstmts;
}

bool CDatabase::_open(const string host, const string DBUser,
		const string DBPassword, const string DBName)
{
	m_ConnHandler = mysql_init(NULL);
	if (mysql_real_connect(m_ConnHandler, host.data(), DBUser.data(),
			DBPassword.data(), DBName.data(), 3306, NULL, 0)) {
		m_State = DB_S_CONNECTED;
		return true;
	} else {
		return false;
	}
}

MYSQL_FIELD* CDatabase::_getHeader(void) {
	return mysql_fetch_fields(m_ResultSet);
}

string CDatabase::_getValueByIndex(const int idx) {
	return m_DataSet[idx];
}

string CDatabase::_getValueByName(const char* pname) {
	string name(pname);
	unsigned int idx = 0;
	while ((idx < mysql_field_count(m_ConnHandler))
			&& (m_ResultHeader[idx].name != name)) {
		idx++;
	}
	return m_DataSet[idx];
}

int CDatabase::_executeSQLStmt(const string SQLStmt, const string errText,
		const bool isSelectStmt) {

	if (m_bPrint)cout << SQLStmt << endl;

// return value 0 => OK, else error
	if (mysql_query(m_ConnHandler, SQLStmt.data())) {
		if (m_bPrint)_showSQLError(errText);
		return -1;
	} else {
		if (isSelectStmt) {
			m_ResultSet = mysql_store_result(m_ConnHandler);
			m_State = DB_S_FETCH;
			m_ResultHeader = _getHeader();
			// return number of rows
			return mysql_affected_rows(m_ConnHandler);
		} else {
			// store last insert id (only relevant for INSERT statements)
			m_lastInsertId = mysql_insert_id(m_ConnHandler);
			// return affected rows (INSERT, UPDATE, DELETE)
			return mysql_affected_rows(m_ConnHandler);
		}
	}
}

unsigned int CDatabase::_getNativeErrorCode(void) {
	return mysql_errno(m_ConnHandler);
}

void CDatabase::_showSQLError(const string infotxt) {
// ATTENTION !!!! cerr is not working in eclipse debug perspective
	cout << "MySQL error: " << infotxt.data();
	if (_getNativeErrorCode()) {
		cout << " - " << mysql_error(m_ConnHandler) << " ("
				<< _getNativeErrorCode() << ")";
	}
	cout << endl;
}

string CDatabase::_getSQLError(const string infotxt) {
	string dberr("No error occurred!");
	if (_getNativeErrorCode()) {
		dberr= infotxt + " - " + mysql_error(m_ConnHandler) + " (" + to_string(_getNativeErrorCode()) + ")";
	}
	return dberr;
}

string CDatabase::getSQLErrorMessage(const string infotxt) {
	return _getSQLError(infotxt);
}
