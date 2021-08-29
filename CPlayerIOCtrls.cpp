/*
 * CPlayerIOCtrls.cpp
 *
 *  Created on: 30.10.2020
 *      Author: Wirth
 */
/**
 * \file CPlayerIOCtrls.cpp
 *
 * \brief implementation of the IOWarrior control class (performs output only)
 */
#include "windows.h"
#include "ctype.h"
#include <iostream>
using namespace std;

#include "CPlayerIOCtrls.h"

CPlayerIOCtrls::CPlayerIOCtrls()
{
	m_bpThreadHandle=pthread_t{}; // initializes a struct with 0
	m_bpmut = PTHREAD_MUTEX_INITIALIZER;
	m_bpcond = PTHREAD_COND_INITIALIZER;
	m_binPattern=0;
	m_bpChanged=false;
	// keyboard monitoring thread
	m_kbmut = PTHREAD_MUTEX_INITIALIZER;
	m_kbcond = PTHREAD_COND_INITIALIZER;
	m_kbThreadHandle=pthread_t{};
	m_bPressed=false;
	m_bwait4enter=false;
	// Lab05 thread handles all input
	m_numInput=0;
	m_bNumInput=false;
	m_bStringInput=false;

	m_lastError=PIOC_E_OK;
	m_state=PIOC_S_NOTREADY;
}

CPlayerIOCtrls::~CPlayerIOCtrls()
{
	close();
}

void CPlayerIOCtrls::open()
{
	// evaluate state
	if(m_state == PIOC_S_READY)
		return;

	// set device state to ready
	m_state=PIOC_S_STARTING;

	// binary pattern output
	pthread_mutex_init(&m_bpmut,0);
	pthread_cond_init(&m_bpcond,0);
	int rc=pthread_create(&m_bpThreadHandle, NULL, bpThreadHandler, (void*) this);
	if(rc != 0)
	{
		// release resources
		pthread_mutex_destroy(&m_bpmut);
		pthread_cond_destroy(&m_bpcond);
		// put the object in a definite state
		m_lastError=PIOC_E_BPTHREADFAILED;	// set error value
		m_state=PIOC_S_NOTREADY;
		// throw an exception
		throw(CASDDException(SRC_PlayerIOControls,PIOC_E_BPTHREADFAILED,getErrorAsText(PIOC_E_BPTHREADFAILED)));
	}

	// keyboard monitoring
	m_state=PIOC_S_STARTING;
	pthread_mutex_init(&m_kbmut,0);
	pthread_cond_init(&m_kbcond,0);
	rc=pthread_create(&m_kbThreadHandle, NULL, kbThreadHandler, (void*) this);
	if(rc != 0)
	{
		// terminate binary pattern output thread
		pthread_kill(m_bpThreadHandle, 0); 		// signal number always 0 for windows
		// release resources
		pthread_mutex_destroy(&m_bpmut);
		pthread_cond_destroy(&m_bpcond);
		pthread_mutex_destroy(&m_kbmut);
		pthread_cond_destroy(&m_kbcond);
		m_lastError=PIOC_E_KBTHREADFAILED;	// set error value
		m_state=PIOC_S_NOTREADY;
		throw(CASDDException(SRC_PlayerIOControls,PIOC_E_KBTHREADFAILED,getErrorAsText(PIOC_E_KBTHREADFAILED)));
	}

	// busy waiting
	while(m_state != PIOC_S_READY);

	// set error to ok
	m_lastError=PIOC_E_OK;
	return;
}

void CPlayerIOCtrls::close()
{
	if(m_state == PIOC_S_NOTREADY)
		return;

	// terminate both keyboard monitoring thread
	pthread_kill(m_kbThreadHandle, 0);	// kills the kb thread while it is waiting for an user input (signal number always 0 for windows)

	m_state = PIOC_S_NOTREADY;			// will cause the thread to terminate

	pthread_mutex_lock(&m_bpmut);
	pthread_cond_signal(&m_bpcond);		// wake-up the bp thread to terminate
	pthread_mutex_unlock(&m_bpmut);

	pthread_join(m_bpThreadHandle, NULL); // waits for the bp thread to be terminated (no return value needed)
    pthread_mutex_destroy(&m_bpmut);
    pthread_cond_destroy(&m_bpcond);
    pthread_mutex_destroy(&m_kbmut);
    pthread_cond_destroy(&m_kbcond);
}

void CPlayerIOCtrls::write(unsigned char data)
{
	// check the state
	if(m_state != PIOC_S_READY)
	{
		m_lastError=PIOC_E_BPTHREADNOTREADY;
		throw(CASDDException(SRC_PlayerIOControls,PIOC_E_BPTHREADNOTREADY,getErrorAsText(PIOC_E_BPTHREADNOTREADY)));
	}
	// set the new pattern to transfer to the thread
	// critical region start
	pthread_mutex_lock(&m_bpmut);
	m_binPattern=data;
	m_bpChanged=true;					// binary output has been changed
	pthread_cond_signal(&m_bpcond);		// wake-up the thread
	pthread_mutex_unlock(&m_bpmut);
	// end of critical region
	return;
}

void CPlayerIOCtrls::printData(unsigned char data) {
	write(data);
}

void* CPlayerIOCtrls::bpThreadHandler(void* Obj)
{
	CPlayerIOCtrls* pPIOC=(CPlayerIOCtrls*)Obj;
	cout << "Byte pattern thread has been started" << endl;
	pPIOC->m_state=PIOC_S_READY;

	while(1)
	{
		pthread_mutex_lock(&(pPIOC->m_bpmut));
		// put thread to sleep while there is nothing to do
		// if there is something to do, the main thread (in our case) wakes up the thread by setting a condition
		while ((pPIOC->m_state == PIOC_S_READY) && (pPIOC->m_bpChanged == false))
		{
			pthread_cond_wait(&(pPIOC->m_bpcond), &(pPIOC->m_bpmut)); // wait unlocks at entrance and re-locks at the end
		}

		if(pPIOC->m_state == PIOC_S_NOTREADY)
		{
			pthread_mutex_unlock(&(pPIOC->m_bpmut));
			break;
		}
		else
		{
			if(pPIOC->m_bpChanged == true)
			{
				unsigned char cByte= pPIOC->m_binPattern;
				for(int i=0;i<8;i++)
				{
					cByte & 0x80 ? cout << '1' : cout << '0';
					cByte <<= 1;
				}
				cout << '\r';
				pPIOC->m_bpChanged=false;
			}
			pthread_mutex_unlock(&pPIOC->m_bpmut);
		}
	}
	cout << "Byte pattern thread terminates. " << endl;
	return NULL;
}

//bool CPlayerIOCtrls::keyPressed(bool bSwitchOff)
//{
//	if(m_state!=PIOC_S_READY)
//	{
//		m_lastError=PIOC_E_KBTHREADNOTREADY;
//		throw(CASDDException(SRC_PlayerIOControls,PIOC_E_KBTHREADNOTREADY,getErrorAsText(PIOC_E_KBTHREADNOTREADY)));
//	}
//
//	// switch off key control
//	if(bSwitchOff)
//	{
////		pthread_mutex_lock(&m_kbmut);
////		cin.putback('\n');				// if thread waits for user input (it blocks)
//		pthread_kill(m_kbThreadHandle, 0);	// kills the kb thread while it is waiting for an user input (signal number always 0 for windows)
//		m_kbThreadHandle=pthread_t{};
//		m_bwait4enter=false;
//		m_bPressed=false;
//		m_state=PIOC_S_STARTING;
//		pthread_create(&m_kbThreadHandle, NULL, kbThreadHandler, (void*) this);
//		// busy waiting
//		while(m_state != PIOC_S_READY);
////		pthread_mutex_unlock(&m_kbmut);
//		return false;
//	}
//
//	// the attribute is set by the thread, a detected key press is cleared once it has been read
// 	if(m_bPressed == true)
//	{
//		m_bPressed=false;
//		return true;
//	}
//
//	pthread_mutex_lock(&m_kbmut);
//	if(m_bwait4enter==false){
//		m_bwait4enter=true;
//		pthread_cond_signal(&m_kbcond);		// wake-up the thread
//	}
//	pthread_mutex_unlock(&m_kbmut);
//
//	return false;
//}

//void* CPlayerIOCtrls::kbThreadHandler(void* Obj)
//{
//	CPlayerIOCtrls* pPIOC=(CPlayerIOCtrls*)Obj;
//	cout << "keyboard monitoring thread has been started" << endl;
//	pPIOC->m_state=PIOC_S_READY;
//	while(pPIOC->m_state == PIOC_S_READY)
//	{
//		// Lab05: send thread to sleep is necessary if other user input is needed within the program
//		pthread_mutex_lock(&(pPIOC->m_kbmut));
//		// put thread to sleep while there is nothing to do
//		// if there is something to do, the main thread (in our case) wakes up the thread by setting a condition
//		while ((pPIOC->m_state == PIOC_S_READY) && (pPIOC->m_bwait4enter == false))
//		{
//			pthread_cond_wait(&(pPIOC->m_kbcond), &(pPIOC->m_kbmut)); // wait unlocks at entrance and re-locks at the end
//		}
//
//		if(pPIOC->m_state == PIOC_S_NOTREADY)
//		{
//			pthread_mutex_unlock(&(pPIOC->m_kbmut));
//			break;
//		}
//		else
//		{
//			pthread_mutex_unlock(&pPIOC->m_kbmut);
//			cout << "kb thread is waiting for a key press..." << endl;
//			cin.ignore(2,'\n');
//			cout << "go to sleep..." << endl;
//			pthread_mutex_lock(&(pPIOC->m_kbmut));
//			pPIOC->m_bPressed=true;
//			pPIOC->m_bwait4enter=false;
//			pthread_mutex_unlock(&pPIOC->m_kbmut);
//		}
//	}
//	return NULL;
//}

bool CPlayerIOCtrls::keyPressed()
{
	if(m_state!=PIOC_S_READY)
	{
		m_lastError=PIOC_E_KBTHREADNOTREADY;
		throw(CASDDException(SRC_PlayerIOControls,PIOC_E_KBTHREADNOTREADY,getErrorAsText(PIOC_E_KBTHREADNOTREADY)));
	}

	// the attribute is set by the thread, a detected key press is cleared once it has been read
 	if(m_bPressed == true)
	{
		pthread_mutex_lock(&m_kbmut);
		m_bPressed=false;
		m_bwait4enter=true;
		pthread_mutex_unlock(&m_kbmut);
		return true;
	}
 	else
	{
		pthread_mutex_lock(&m_kbmut);
		m_bwait4enter=true;
		pthread_mutex_unlock(&m_kbmut);
		return false;
	}
}

int CPlayerIOCtrls::getUserInputNum()
{
	if(m_state!=PIOC_S_READY)
	{
		m_lastError=PIOC_E_KBTHREADNOTREADY;
		throw(CASDDException(SRC_PlayerIOControls,PIOC_E_KBTHREADNOTREADY,getErrorAsText(PIOC_E_KBTHREADNOTREADY)));
	}

	pthread_mutex_lock(&m_kbmut);
	m_bNumInput=false;
	pthread_mutex_unlock(&m_kbmut);
	while(!m_bNumInput);
	return m_numInput;
}

string CPlayerIOCtrls::getUserInputString()
{
	if(m_state!=PIOC_S_READY)
	{
		m_lastError=PIOC_E_KBTHREADNOTREADY;
		throw(CASDDException(SRC_PlayerIOControls,PIOC_E_KBTHREADNOTREADY,getErrorAsText(PIOC_E_KBTHREADNOTREADY)));
	}
	pthread_mutex_lock(&m_kbmut);
	m_bStringInput=false;
	pthread_mutex_unlock(&m_kbmut);
	while(!m_bStringInput);
	return m_sInput;
}


void* CPlayerIOCtrls::kbThreadHandler(void* Obj)
{
	CPlayerIOCtrls* pPIOC=(CPlayerIOCtrls*)Obj;
	char line[256];
	cout << "keyboard monitoring thread has been started" << endl;
	pPIOC->m_state=PIOC_S_READY;
	while(pPIOC->m_state == PIOC_S_READY)
	{
		cin.getline(line,256);
		if(!(*line) && pPIOC->m_bwait4enter == true)
		{
			pthread_mutex_lock(&(pPIOC->m_kbmut));
			pPIOC->m_bPressed=true;
			pPIOC->m_bwait4enter=false;
			pthread_mutex_unlock(&pPIOC->m_kbmut);
		}
		else if(!(*line))		// user pressed ENTER, but not for key control
		{
			pthread_mutex_lock(&(pPIOC->m_kbmut));
			pPIOC->m_sInput.empty();
			pPIOC->m_bStringInput=true;
			pPIOC->m_bNumInput=false;
			pthread_mutex_unlock(&pPIOC->m_kbmut);
		}
		else if(*line)
		{
			int num=atoi(line);
			if(num==0)
			{
				char* pos=line;
				while(*pos && isblank(*pos))pos++;
				if(*pos == 0x30)		// user entered 0
				{
					pthread_mutex_lock(&(pPIOC->m_kbmut));
					pPIOC->m_bNumInput=true;
					pPIOC->m_numInput=0;
					pPIOC->m_bStringInput=false;
					pthread_mutex_unlock(&pPIOC->m_kbmut);
				}
				else	// user entered a string
				{
					pthread_mutex_lock(&(pPIOC->m_kbmut));
					pPIOC->m_bStringInput=true;
					pPIOC->m_sInput=line;
					pPIOC->m_bNumInput=false;
					pthread_mutex_unlock(&pPIOC->m_kbmut);
				}
			}
			else	// user entered a number
			{
				pthread_mutex_lock(&(pPIOC->m_kbmut));
				pPIOC->m_bNumInput=true;
				pPIOC->m_numInput=num;
				pPIOC->m_bStringInput=false;
				pthread_mutex_unlock(&pPIOC->m_kbmut);
			}
		}
	}
	return NULL;
}

void CPlayerIOCtrls::printState()
{
	if(m_state != PIOC_S_NOTREADY)
	{
		while(m_bpChanged!=false);
		// start of critical region
		pthread_mutex_lock(&m_bpmut);
	}
	cout << endl;
	switch(m_state)
	{
	case PIOC_S_NOTREADY: cout << "player io-controls are not ready" << endl; break;
	case PIOC_S_READY: cout << "player io-controls are ready" << endl; break;
	default: cout << "Unknown State" << endl; break;
	}
	switch(m_lastError)
	{
	case PIOC_E_BPTHREADNOTREADY: cout << "Error: binary pattern output thread is not active - please call the open method first" << endl; break;
	case PIOC_E_BPTHREADFAILED: cout << "Error: binary pattern output thread could not start. " << endl; break;
	case PIOC_E_KBTHREADNOTREADY: cout << "Error: keyboard monitoring thread is not active - please call the open method first" << endl; break;
	case PIOC_E_KBTHREADFAILED: cout << "Error: keyboard monitoring thread could not start. " << endl; break;
	case PIOC_E_OK: cout << "No Error :-)" << endl; break;
	default: cout << "Unknown Error :-(" << endl; break;
	}
	if(m_state != PIOC_S_NOTREADY)
		pthread_mutex_unlock(&m_bpmut);
		// end of critical region
}

PIOC_STATES CPlayerIOCtrls::getState()
{
	return m_state;
}


string CPlayerIOCtrls::getStateAsText(PIOC_STATES stat)
{
	switch(stat)
	{
	case PIOC_S_NOTREADY: return string("player io-controls are not ready");
	case PIOC_S_READY: return string("player io-controls are ready");
	default: return string("Unknown State");
	}
}

PIOC_ERRORS CPlayerIOCtrls::getLastError()
{
	return m_lastError;
}

string CPlayerIOCtrls::getErrorAsText(PIOC_ERRORS err)
{
	switch(err)
	{
	case PIOC_E_BPTHREADNOTREADY: return string("Error: binary pattern output thread is not active - please call the open method first");
	case PIOC_E_BPTHREADFAILED: return string("Error: binary pattern output thread could not start. ");
	case PIOC_E_KBTHREADNOTREADY: return string("Error: keyboard monitoring thread is not active - please call the open method first");
	case PIOC_E_KBTHREADFAILED: return string("Error: keyboard monitoring thread could not start. ");
	case PIOC_E_OK: return string("No Error :-)");
	default: return string("Unknown Error :-(");
	}
}

