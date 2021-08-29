/*
 * CPlayerIOCtrls.h
 *
 *  Created on: 30.10.2020
 *      Author: Wirth
 */

/**
 * \file CPlayerIOCtrls.h
 *
 * \brief header file of the PlayerIO control class (thread controlled key input and binary pattern output)
 */
#ifndef CPlayerIOCtrls_H_
#define CPlayerIOCtrls_H_

#include "CASDDException.h"

/**
 * \brief CAUTION!!!! MinGW contains an incompatible version of this .h file! Forgetting to set the include path
 * \brief may cause a curious behavior or crash!!!!
 */
#include <pthread.h>

/**
 * \brief Error Codes
 */
enum PIOC_ERRORS{PIOC_E_OK,PIOC_E_KBTHREADNOTREADY, PIOC_E_KBTHREADFAILED, PIOC_E_BPTHREADNOTREADY, PIOC_E_BPTHREADFAILED};
/**
 * \brief Device States (used to implement a small state machine)
 */
enum PIOC_STATES{PIOC_S_NOTREADY,PIOC_S_STARTING,PIOC_S_READY};

/**
 * \brief Player IO control class with exception handling
 */
class CPlayerIOCtrls
{
	/**
	 * Binary pattern output thread
	 * ----------------------------
	 */
	/**
	 * handle of the binary pattern output thread (NULL if no thread has been started at program start)
	 */
	pthread_t m_bpThreadHandle;
	/**
	 * mutex for the condition of the binary pattern output thread
	 */
	pthread_mutex_t m_bpmut;
	/**
	 * condition of the binary pattern output thread to wakeup an do its job
	 */
	pthread_cond_t m_bpcond;
	/**
	 * variable for the binary pattern output thread to get the pattern byte to show on the screen
	 */
	unsigned char m_binPattern;
	/**
	 * variable signalizes that the binary pattern output has been changed
	 */
	bool m_bpChanged;

	/**
	 * key monitoring thread
	 * ----------------------------
	 */
	pthread_t m_kbThreadHandle;
	/**
	 * mutex for the condition of the key monitoring thread
	 */
	pthread_mutex_t m_kbmut;
	/**
	 * condition of the key monitoring thread to wakeup an do its job
	 */
	pthread_cond_t m_kbcond;
	bool m_bPressed;				// shared attribute to recognize the return key press
	bool m_bwait4enter;				// shared attribute to signalize kbthread to do something

	// Lab05 thread handles all input
	int m_numInput;
	bool m_bNumInput;
	string m_sInput;
	bool m_bStringInput;

	/**
	 * saves the last error occurred (PIOC_E_OK if no error occurred)
	 */
	PIOC_ERRORS m_lastError;
	/**
	 * saves the current state of the player IO control (PIOC_S_READY if all threads are started)
	 */
	PIOC_STATES m_state;

public:
	/**
	 * Constructor
	 * initializes the attributes with initial values
	 */
	CPlayerIOCtrls();

	/**
	 * Destructor
	 * stops the threads (calling close)
	 */
	virtual ~CPlayerIOCtrls();

	/**
	 * \brief Starts both of the threads.
	 * \brief throws an exception if it failed to start the threads.
	 */
	void open();

	/**
	 * \brief stops the threads
	 */
	void close();

	/**
	 * \brief Prints a binary formatted byte on the screen.
	 * \brief throws an exception if the object is not in ready state
	 * \param data [in] to show as binary 8-bit pattern on the screen
	 */
	void write(unsigned char data);

	/**
	 * \brief same like write (for compatibility with IOWarrior class).
	 */
	void printData(unsigned char data);

	/**
	 * \brief monitors the return key. Returns true, if the user pressed the return key, otherwise false.
	 * \brief throws an exception if the object is not in ready state
	 */
	bool keyPressed();

	/**
	 * \brief waits for the user to enter a number (blocking)
	 * \brief throws an exception if the object is not in ready state
	 */
	int getUserInputNum();

	/**
	 * \brief waits for the user to enter a string (blocking)
	 * \brief throws an exception if the object is not in ready state
	 */
	string getUserInputString();

	/**
	 * \brief Prints the current state of the player IO control.
	 */
	void printState();

	/**
	 * \brief Queries the current state of the player IO control.
	 */
	PIOC_STATES getState();
	string getStateAsText(PIOC_STATES stat);
	PIOC_ERRORS getLastError();
	string getErrorAsText(PIOC_ERRORS err);

private:
	/*
	 * \brief controls the behavior of binary pattern output thread
	 */
	static void* bpThreadHandler(void *Obj);
	/*
	 * \brief controls the behavior of keyboard monitoring thread
	 */
	static void* kbThreadHandler(void *Obj);
};

#endif /* CPlayerIOCtrls_H_ */
