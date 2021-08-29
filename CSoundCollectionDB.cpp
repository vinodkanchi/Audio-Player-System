/*
 * CSoundCollectionDB.cpp
 *
 *  Created on: 03.01.2020
 *      Author: Wirth
 */

#include <iostream>
#include "CSoundCollectionDB.h"
#include "CASDDException.h"


CSoundCollectionDB::CSoundCollectionDB() {
// nothing to do
}

CSoundCollectionDB::~CSoundCollectionDB() {
	close();
}

void CSoundCollectionDB::open(const string host, const string DBUser,
		const string DBPassword) {
	if(false == _open("localhost","ASDDuser","ASDDuser","soundfiles"))
		throw CASDDException(SRC_Database,_getNativeErrorCode(), _getSQLError("Could not open database audiofilters"));
}


int CSoundCollectionDB::selectAllSounds() {
	if(m_State!=DB_S_CONNECTED)
		return -1;
	string sSQL="select * from sounds;";
	int queryResult = _executeSQLStmt(sSQL, "selectAllSounds ", true);
	if(queryResult < 0)
		throw CASDDException(SRC_Database, _getNativeErrorCode(), _getSQLError("selectAllSounds"));
	return queryResult;
}

int CSoundCollectionDB::selectNumSounds() {
	if(m_State!=DB_S_CONNECTED)
		return -1;
	int num;
	string sSQL="select count(*) from sounds;";
	int queryResult = _executeSQLStmt(sSQL, "selectNumSounds ", true);
	if(queryResult < 0)
		throw CASDDException(SRC_Database, _getNativeErrorCode(), _getSQLError("selectAllSounds"));
	bool bfetch=fetch(); // get the number of available sounds
	if(true==bfetch)
		num=stoi(m_DataSet[0]);
	closeQuery();
	return bfetch ? num : -1;
}

int CSoundCollectionDB::selectFs(int soundID) {
	if(m_State!=DB_S_CONNECTED)
		return -1;

	int fs;
	string sSQL="select fs from sounds where sid=" + to_string(soundID)+";";
	int queryResult = _executeSQLStmt(sSQL, "selectAllSounds ", true);
	if(queryResult > 0)
	{
		if(false == fetch())	//Daten holen
		{
			closeQuery();
			return -1;
		}
		else
		{
			fs= stoi(m_DataSet[0]);
			closeQuery();
			return fs;
		}
	}
	return -1;
}

//Lab 1b)
bool CSoundCollectionDB::selectSoundData(int soundID)
{
	if(m_State!=DB_S_CONNECTED)
		return false;

	string sSQL;
	sSQL= "SELECT * FROM sounds WHERE sid=" + to_string(soundID);
	int queryResult = _executeSQLStmt(sSQL, "selectAllSounds ", true);
	if(queryResult > 0)
	{
		bool bret= fetch(); // no closeQuery here, because the getter methods will not work otherwise!
		return bret;
	}
	else
		return false;
}

bool CSoundCollectionDB::insertSound(string path, string name, int fs,int numChan)
{
	if(m_State!=DB_S_CONNECTED)
		return false;

	string modpath=_modifyPath(path);
	string sSQL="insert into sounds(path,name,fs,numchan) values(\'" + modpath+"\',\'"+name+"\',"+to_string(fs)+","+to_string(numChan)+");";
	return _executeSQLStmt(sSQL, "insertSound", false);
}

string CSoundCollectionDB::getPath() {
	string sval("No sound data available: at first select data!");
	if(m_State!=DB_S_FETCH)
		return sval;
	sval=_getValueByName("path");
	return sval;
}

string CSoundCollectionDB::getName() {
	string sval("No sound data available: at first select data!");
	if(m_State!=DB_S_FETCH)
		return sval;
	sval=_getValueByName("name");
	return sval;
}

int CSoundCollectionDB::getFs() {
	if(m_State!=DB_S_FETCH)
		return -1;
	return stoi(_getValueByName("fs"));
}

int CSoundCollectionDB::getID() {
	if(m_State!=DB_S_FETCH)
		return -1;
	return stoi(_getValueByName("sid"));
}

int CSoundCollectionDB::getNumChan() {
	if(m_State!=DB_S_FETCH)
		return -1;
	return stoi(_getValueByName("numchan"));
}

string CSoundCollectionDB::_modifyPath(string path) {
	unsigned int pos=0,pos1;
	string modpath;
	unsigned int pathlen=path.length();
	while((pos1=path.find("\\",pos))<pathlen)
	{
		modpath=modpath+path.substr(pos,pos1-pos)+"\\\\";
		pos=pos1+1;
	}
//	modpath=modpath+path.substr(pos)+"\\\\";
	modpath=modpath+path.substr(pos); // NEW: removed the backslashes at the end
	return modpath;
}
