/*
 * CAudiofiltersDb.h
 *
 *  Created on: Jan, 2020
 *      Author: a.wirth
 */

#ifndef CAUDIOFILTERSDB_H_
#define CAUDIOFILTERSDB_H_
#include "CDatabase.h"

class CAudiofiltersDb: public CDatabase {
public:
	CAudiofiltersDb();
	virtual ~CAudiofiltersDb();

	/**
	 * \brief opens the database audiofilters
	 * throws an exception in case of an error (uses _getSQLError and _getNativeErrorCode to get information about the reason)
	 * \param mariadb host ip, username, password
	 */
 	void open(const string host = "localhost", const string DBUser = "",
			const string DBPassword = "");

	/**
	 * \brief select all existing filters from the filterview
	 * \return -1 error (use _getSQLError and _getNativeErrorCode to get information about the reason)
	 *  returns result sets number of rows
	 */
 	int selectAllFilters();

 	/**
	 * \brief select all existing filters for a specified sampling frequency,
	 * throws an ASDDException if an error occurred
	 * \param fs sampling frequency
	 *  returns result sets number of rows
	 */
	int selectFilters(int fs);

 	/**
	 * \brief NEW: selects the data of one specific filter
	 * \param fs filter id (primary key)
	 *  returns true ... data are available otherwise false (call closeQuery
	 *  after you have finished working with the filter data)
	 */
	bool selectFilter(int fid);

	/**
	 * \brief record query methods requires a previous call to selectFilters or selectAllFilters
	 * and a fetch
	 */
	int getOrder();						// filter order
	int getDelay();						// NEW: delay time [ms]
	int getFs();						// sampling frequency
	string getFilterType();				// type of the filter (e.g. lowpass, shelving, delay, ...)
	string getFilterSubType();			// subtype of the filter (e.g. butter, yulewalk, feedback, ...)
	string getFilterInfo();				// free textual filter description
	unsigned long long getFilterID();	// FilterID from Database

	/**
	 * \brief get the filter coefficients of the filter with the id fid
	 * \param fid filter id
	 * \param (out) numBCoeffs provides the number of coefficients
	 * \return NULL if no coefficients are available
	 *  returns the array with the filter coefficients, the caller is responsible to release the arrays memory
	 */
	float* getBCoeffs(int fid, int& numBCoeffs);
	float* getACoeffs(int fid, int& numACoeffs);

	/**
	 * \brief delete a filter type and subtype
	 * \param filter type and subtype
	 * \return number of effected rows (shall be 1!)
	 */
	int deleteFilterType(string type, string subtype);

	/**
	 * \brief insert a filter into the database
	 * \param
	 */
	bool insertFilter(string type, string subtype, int fs, int order, int delay_ms, string info, float* b, int blen, float*a, int alen);

	/**
	 * \brief private methods
	 */
private:
	bool _checkFilter(int fid); // NEW: checks if the filter with fid exists
	float* _selectFilterCoefficients(int fid, string polynom, int& numCoeffs);
};
#endif /* CAUDIOFILTERSDB_H_ */
