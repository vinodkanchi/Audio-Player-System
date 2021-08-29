/*
 * CUserInterface.h
 *
 *  Created on: 09.01.2020
 *      Author: Wirth
 */

#ifndef SRC_CUSERINTERFACE_H_
#define SRC_CUSERINTERFACE_H_
#include <string>
using namespace std;

#include "CPlayerIOCtrls.h"
#include "CAmpMeter.h"

#define CUI_UNKNOWN 0xffff // error value (maximum valid is CUI_UNKNOWN-1)

// Strategy interface (pure virtual class) for user interface class
class CUserInterface
{
public:
	CUserInterface(){};
	virtual ~CUserInterface(){};
	virtual void init()=0;
	virtual int getListSelection(string* items, int* idItems=NULL)=0;
	virtual string getUserInputPath()=0;
	virtual void showMessage(string msg)=0;
	virtual bool wait4Key(bool bBlock=true)=0;
	virtual void showAmplitude(float* databuf, int bufsize)=0;
	virtual void setAmplitudeScaling(SCALING_MODES mode)=0;
};


// concrete strategy for a user interface class with command-line only
class CUserInterfaceCmdLine : public CUserInterface
{
private:
	CPlayerIOCtrls m_pioc;
	CAmpMeter m_ampMeter;

public:
	CUserInterfaceCmdLine();
	void init();
	// displays a menu and returns the user's choice
	// Parameters:
	// items: pointer to array with the items of the list, the last item MUST be an empty string
	// idItems: pointer to array with one ID for each item (to choose, the user enters one of these IDs),
	//          if idItems is NULL, the IDs will be generated automatically (starting at 0)
	int getListSelection(string* items, int* idItems=NULL);
	// returns a user provided path (expands the backslashes in the right manner)
	string getUserInputPath();
	// displays msg
	void showMessage(string msg);
	// returns true, if the user presses ENTER (no IOWarrior connected) or the IOWarrior's button (IOWarrior connected)
	// if bBlock is true, it does not return before the user had pressed ENTER or the button (it blocks)
	// if bBlock is false, it returns immediately (true...button/ENTER pressed, else false)
	bool wait4Key(bool bBlock=true);

	// Visualizes the amplitude of a data buffer on the LED line. Returns true if the visualization was successfully, otherwise false.
	// databuf The address of the first data buffer element.
	// bufsize The number of elements of the data buffer.
	void showAmplitude(float* databuf, int bufsize);
	// changes the amplitude meter's scaling mode
	// mode The scaling of the amplitude meter (may be SCALING_MODE_LIN or SCALING_MODE_LOG)
	void setAmplitudeScaling(SCALING_MODES mode);
};


#endif /* SRC_CUSERINTERFACE_H_ */
