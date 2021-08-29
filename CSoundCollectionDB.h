/*
 * CSoundCollectionDB.h
 *
 *  Created on: 03.01.2020
 *      Author: Wirth
 */

#ifndef SRC_CSOUNDCOLLECTIONDB_H_
#define SRC_CSOUNDCOLLECTIONDB_H_
#include <string>
using namespace std;
#include "CDatabase.h"

class CSoundCollectionDB: public CDatabase
{
public:
	CSoundCollectionDB();
	~CSoundCollectionDB();
	void open(const string host = "localhost", const string DBUser = "",
			const string DBPassword = "");
	int selectAllSounds();
	int selectFs(int soundID);
	//Lab 1b)
	bool selectSoundData(int soundID); // Lab04 Task 1
	int selectNumSounds(); 				// Lab05 prep task 1 e
	bool insertSound(string path,string name,int fs,int numChan);
	string getPath();
	string getName();
	int getID();
	int getFs();
	int getNumChan();
private:
	string _modifyPath(string path);
};

#endif /* SRC_CSOUNDCOLLECTIONDB_H_ */
