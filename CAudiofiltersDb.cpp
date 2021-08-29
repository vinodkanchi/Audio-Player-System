/*
 * CAudiofiltersDb.cpp
 *
 *  Created on: Nov 9, 2020
 *      Author: h.frank
 */

#include <cstring>
#include <iomanip>
#include "CASDDException.h"
#include "CAudiofiltersDb.h"

CAudiofiltersDb::CAudiofiltersDb() {
	// noting TODO
}

CAudiofiltersDb::~CAudiofiltersDb() {
	close();
}

void CAudiofiltersDb::open(const string host, const string DBUser,
		const string DBPassword) {
	if(false == _open(host,"ASDDuser","ASDDuser","audiofilters"))
		throw CASDDException(SRC_Database,_getNativeErrorCode(), _getSQLError("Could not open database audiofilters"));
}

int CAudiofiltersDb::selectAllFilters()
{
	if(m_State!=DB_S_CONNECTED)
		return 0;

	string sSQL="SELECT * FROM filterview;";
	int queryResult = _executeSQLStmt(sSQL, "selectAllFilters ", true);
	if(queryResult < 0)
		throw CASDDException(SRC_Database, _getNativeErrorCode(), _getSQLError("selectAllFilters"));
	return queryResult;
}

int CAudiofiltersDb::selectFilters(int fs) {
	if(m_State!=DB_S_CONNECTED)
		return false;

	string sSQL= "SELECT FilterID, Type, Subtype, Filterorder, Fs, Delay, Description FROM filterview WHERE Fs=0 OR Fs=" + to_string(fs);
	int queryResult = _executeSQLStmt(sSQL, "selectFilters for given fs", true);
	if(queryResult < 0)
		throw CASDDException(SRC_Database, _getNativeErrorCode(), _getSQLError("selectAllFilters"));
	return queryResult;
}

//NEW:
bool CAudiofiltersDb::selectFilter(int fid)
{
	string sSQL;
	sSQL= "SELECT FilterID, Type, Subtype, Filterorder, Fs, Delay, Description FROM filterview WHERE FilterID=" + to_string(fid);
	if( 1 == _executeSQLStmt(sSQL, "_selectFilter data for a given filter id", true))
		return fetch();
	else
		return false;
}


float* CAudiofiltersDb::getBCoeffs(int fid, int &numBCoeffs) {
	numBCoeffs=0;
	if(m_State!=DB_S_CONNECTED)
		return NULL;

	if(false == _checkFilter(fid)) // CHANGED
		return NULL;

	return _selectFilterCoefficients(fid, "NUMERATOR", numBCoeffs);
}

float* CAudiofiltersDb::getACoeffs(int fid, int &numACoeffs) {
	numACoeffs=0;
	if(m_State!=DB_S_CONNECTED)
		return NULL;

	if(false == _checkFilter(fid)) // CHANGED
		return NULL;

	return _selectFilterCoefficients(fid, "DENOMINATOR", numACoeffs);
}

int CAudiofiltersDb::getOrder() {
	if(m_State!=DB_S_FETCH)
		return -1;
	return stoi(_getValueByName("Filterorder"));
}

unsigned long long CAudiofiltersDb::getFilterID() {
	if(m_State!=DB_S_FETCH)
		return -1;
	return stoull(_getValueByName("FilterID"));
}

int CAudiofiltersDb::getDelay() {
	if(m_State!=DB_S_FETCH)
		return -1;
	return stoi(_getValueByName("Delay"));
}

string CAudiofiltersDb::getFilterType()
{
	string sval("No filter type data available: at first select data!");
	if(m_State!=DB_S_FETCH)
		return sval;
	sval=_getValueByName("Type");
	return sval;
}

string CAudiofiltersDb::getFilterSubType()
{
	string sval("No filter subtype data available: at first select data!");
	if(m_State!=DB_S_FETCH)
		return sval;
	sval=_getValueByName("Subtype");
	return sval;
}

string CAudiofiltersDb::getFilterInfo()
{
	string sval("No filter info data available: at first select data!");
	if(m_State!=DB_S_FETCH)
		return sval;
	sval=_getValueByName("Description");
	return sval;
}

int CAudiofiltersDb::getFs()
{
	if(m_State!=DB_S_FETCH)
		return -1;
	return stoi(_getValueByName("Fs"));
}

int CAudiofiltersDb::deleteFilterType(string type, string subtype) {
	if(m_State!=DB_S_CONNECTED)
		return false;

	string sSQL="DELETE FROM filtertypes WHERE f_type=\'" + type + "\' AND f_subtype=\'" + subtype + "\';";
	return _executeSQLStmt(sSQL, "deleteFilterType ", false);
}

bool CAudiofiltersDb::insertFilter(string type, string subtype, int fs,
		int order, int delay_ms, string info, float *b, int blen, float *a,
		int alen)
{
	if(m_State!=DB_S_CONNECTED)
		return false;

	string sSQL, id;
	bool bDelOldCoeffs=false;
	int rowsEffected=0;

	// insert type and subtype into filtertypes (because the unique constraint an error will occur if the type/subtype pair already exists)
	sSQL= "INSERT INTO filtertypes (f_type,f_subtype) VALUES (\'" + type + "\',\'" + subtype + "\');";
	_executeSQLStmt(sSQL, "insertFilter: type", false);

	// get ftid
	sSQL= "SELECT ftid FROM filtertypes WHERE f_type=\'" + type + "\' AND f_subtype=\'" + subtype + "\';";
	if( 0 > _executeSQLStmt(sSQL, "insertFilter: get type id", true))
		throw CASDDException(SRC_Database, _getNativeErrorCode(), _getSQLError("insertFilter: get type id"));

	if(false == fetch())	//Daten holen
	{
		closeQuery();
		return false;
	}
	else
	{
		id= m_DataSet[0];
		closeQuery();
	}

	// insert filter into filters (with ftid_fk=ftid, because the unique constraint an error will occur if the filter already exists)
	sSQL= "INSERT INTO filters (ftid_fk,fs_Hz,f_order,delay_ms,f_info) VALUES (" + id + "," + to_string(fs) + "," + to_string(order) + "," + to_string(delay_ms) + ",\'" + info + "\');";
	rowsEffected= _executeSQLStmt(sSQL, "insertFilter: insert filter ", false);
	if (0 > rowsEffected)
	{
		// the filter already exists: delete its filter coefficients (no define for Native error codes?)
		if(1062 == _getNativeErrorCode())
			bDelOldCoeffs=true;
	}

	// get fid
	sSQL= "SELECT fid FROM filters WHERE ftid_fk=" + id + " AND fs_Hz=" + to_string(fs) + " AND f_order=" + to_string(order) + " AND delay_ms=" + to_string(delay_ms) + ";";
	if( 0 > _executeSQLStmt(sSQL, "insertFilter: get filter id", true))
		throw CASDDException(SRC_Database, _getNativeErrorCode(), _getSQLError("insertFilter: get filter id"));

	if(false == fetch())	//Daten holen
	{
		closeQuery();
		return false;
	}
	id= m_DataSet[0];
	closeQuery();

	// insert coefficients into filtercoefficients (with fid_fk=fid)
	// delete existing filter coefficients for this filter (to update will cause a lot of programming effort)
	if(bDelOldCoeffs == true)
	{
		sSQL= "DELETE FROM filtercoefficients WHERE fid_fk=" + id + ";";
		_executeSQLStmt(sSQL, "insertFilter: delete old filter coefficients", false);
	}

	// insert denominator coefficients
	for(int k=0; k < alen;k++)
	{
		sSQL= "INSERT INTO filtercoefficients (fid_fk,coefficient,position,polynom) VALUES (" + id + "," + to_string(a[k]) + "," + to_string(k) + "," + "\'DENOMINATOR\');";
		_executeSQLStmt(sSQL, "insertFilter: insert filter coefficients (denominator)", false);
	}

	// insert numerator coefficients
	for(int k=0; k < blen;k++)
	{
		sSQL= "INSERT INTO filtercoefficients (fid_fk,coefficient,position,polynom) VALUES (" + id + "," + to_string(b[k]) + "," + to_string(k) + "," + "\'NUMERATOR\');";
		_executeSQLStmt(sSQL, "insertFilter: insert filter coefficients (numerator)", false);
	}
	//cout << "here" << endl;
	return true;
}

bool CAudiofiltersDb::_checkFilter(int fid)
{
	string sSQL;
	sSQL= "SELECT COUNT(*) FROM filterview WHERE FilterID=" + to_string(fid);
	_executeSQLStmt(sSQL, "_selectFilter data for a given filter id", true);
	if(true==fetch())
	{
		int cnt=stoi(m_DataSet[0]);
		closeQuery();
		return (cnt==1)?true:false;
	}
	else
		return false;
}

// private methods
float* CAudiofiltersDb::_selectFilterCoefficients(int fid, string polynom, int& numCoeffs)
{

	string sSQL= "SELECT coefficient FROM filtercoefficients WHERE fid_fk=" + to_string(fid) + " AND polynom=\'" + polynom + "\'";
	numCoeffs=_executeSQLStmt(sSQL, "_selectFilterCoefficients for a given filter id and polynom", true);
	if( 0 <= numCoeffs)
	{
		int i=0;
		float* coeffs=new float[numCoeffs];
		while(true==fetch())
		{
			coeffs[i]=stof(m_DataSet[0]);
			i++;
		}
		closeQuery();
		return coeffs;
	}
	return NULL;
}

