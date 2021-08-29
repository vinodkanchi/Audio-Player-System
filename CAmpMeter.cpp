/*
 * CAmpMeter.cpp
 *
 *  Created on: 21.10.2019
 *      Author: Wirth
 */
#include <iostream>
//#include <stdlib.h>
#include <math.h>
using namespace std;

#include "CPlayerIOCtrls.h"
#include "CAmpMeter.h"

CAmpMeter::CAmpMeter()
{
	m_scmode=SCALING_MODE_LIN;		// logarithmic or linear bar?
	m_scMax=0;						// maximum of the scale
	for(int i=0;i<8;i++)
		m_thresholds[i]=0;			// thresholds for the bar with 8 segments (simulates LEDs)
	m_vis=NULL;						// address of an object that may show binary patterns on the screen (visualizer)
}

void CAmpMeter::init(float min, float max, SCALING_MODES scmode, int logScaleMin, CPlayerIOCtrls* ppiocs)
{
	// todo: initialize the amplitude meter on the basis of the given parameter values
	// The maximum absolute value of min and max is taken for the maximum of the scale.
	// The thresholds are calculated for a simulated LED line with 8 LEDs in dependence of the given scaling mode (scmode).
		// Linear scaling: 0 ... maximum of the scale
		// Logarithmic scaling: logScaleMin ... 0 [dB], logScaleMin must be negative
	m_scMax=fmax(fabs(max), fabs(min));
	m_scmode=scmode;
	m_vis=ppiocs;

	for(int i=0;i<8;i++)
	{
		if(m_scmode==SCALING_MODE_LOG)
		{
			if(logScaleMin > 0)logScaleMin=-logScaleMin;
			//m_thresholds[i]=logScaleMin+(i+1)*abs(logScaleMin)/9.; // logScaleMin ... 0dB
			m_thresholds[i]=pow(10,(logScaleMin+(i+1)*abs(logScaleMin)/9.)/20); // logScaleMin ... 0dB
			//cout << "LOG threshold[" << i << "]: " << m_thresholds[i] << endl;
		}
		else
		{
			m_thresholds[i]=(i+1)*m_scMax/9.;
			//cout << "LIN threshold[" << i << "]: " << m_thresholds[i] << endl;
		}
	}
}

void CAmpMeter::write(float* databuf, unsigned long databufsize)
{
	if(NULL == databuf)throw CASDDException(SRC_AmpMeter, AMP_E_NOBUFFER, "Invalid data buffer.");
	return write(_getValueFromBuffer(databuf, databufsize));
}

void CAmpMeter::write(float data)
{
	if(NULL == m_vis)throw CASDDException(SRC_AmpMeter, AMP_E_NOVISUALIZER, "Can't do binary pattern output.");
	// todo:
	// writes the bar pattern to the binary pattern output
	char pat;
	pat= _getBarPattern(data);

	// if the PlayerIOControls object is not ready and throws an exception with PIOC_E_BPTHREADNOTREADY,
	// the method calls the PlayerIOControls open method
	try
	{
		m_vis->write(pat);
	}
	catch(CASDDException& e)
	{
		if(e.getErrorCode()==PIOC_E_BPTHREADNOTREADY)
		{
			m_vis->open();
			m_vis->write(pat);
		}
		else
			throw;
	}
}

unsigned char CAmpMeter::_getBarPattern(float data)
{
	char pat=0;
	// todo: Calculate appropriate bar pattern pat for data, which is a linear value in any case. The bar pattern is used
	// for visualization on the screen.
	// Example: pat is 0b11111111 if the absolute value of data is equal to the maximum data value (m_scMax)
	// In dependence of the scaling mode (m_scmode), the absolute value of data (linear scaling) or
	// the dB value of the absolute value of data (logarithmic scaling) has to be used for the bar pattern calculation.
	// Before calculating the dB value, the absolute value of data shall be divided by the linear scale maximum to
	// adjust the highest value to 0 dB (this is called peak normalization).
	if(m_scmode==SCALING_MODE_LOG)
		// if the thresholds represent a "linear" dB scale
		//data=20.*log10(fabs(data/m_scMax)+1e-16);
		// if the thresholds represent a logarithmic scale with linear values made from dB values
		data=fabs(data)/m_scMax;
	else
		data=fabs(data);

	int i=0;
	char mask=1;
	while(data > m_thresholds[i])
	{
		pat|=mask;
		mask <<= 1;
		i++;
	}
	return pat;
}


float CAmpMeter::_getValueFromBuffer(float* databuf, unsigned long databufsize)
{
//	float dmax=0., d;
//	unsigned long imax=0;
//	for(unsigned long i=0; i < databufsize; i++)
//	{
//		d=fabs(databuf[i]);
//		if(dmax < d)
//		{
//			dmax=d;
//			imax=i;
//		}
//	}
//	return databuf[imax];

	// better implementation, e.g. calculate the power of the buffered data
	float pbuf=0;
	for(unsigned long i=0; i < databufsize; i++)
	{
		pbuf+=(databuf[i]*databuf[i])/databufsize;
	}
	return pbuf;
}
