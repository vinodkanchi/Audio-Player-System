/*
 * CFilter.cpp
 *
 *  Created on: 11.09.2019
 *      Author: Wirth
 */
#include <math.h>
#include "CASDDException.h"
#include "CFilter.h"

CFilterBase::CFilterBase(int order, int channels) {
	m_order = abs(order);
	m_channels = abs(channels);
	if ((m_order != 0) && (m_channels != 0)) {
		// intermediate buffer: for the intermediate filter states from last sample
		m_z = new float[m_channels * (m_order + 1)];
		for (int i = 0; i < m_channels * (m_order + 1); i++) {
			m_z[i] = 0.;
		}
	} else
		throw CASDDException(SRC_Filter, -1,
				"Filter order and channels must not be zero!");
}

CFilterBase::~CFilterBase() {
	if (m_z != NULL)
		delete[] m_z;
}

void CFilterBase::reset() {
	for (int i = 0; i < m_channels * (m_order + 1); i++) {
		m_z[i] = 0.;
	}
}

int CFilterBase::getOrder() {
	return m_order;
}

int CFilterBase::getNumChannels() {
	return m_channels;
}

CFilter::CFilter(float *ca, float *cb, int order, int channels)
:CFilterBase(order,channels)
{
	if ((ca != NULL) && (cb != NULL)) {
		m_a = new float[m_order + 1];
		m_b = new float[m_order + 1];
		for (int i = 0; i <= m_order; i++) {
			m_a[i] = ca[i] / ca[0];
			m_b[i] = cb[i] / ca[0];
		}
	} else
		throw CASDDException(SRC_Filter, -1,
				"Filter coefficients not available!");
}


CFilter::~CFilter() {
	if (m_a != NULL)
		delete[] m_a;
	if (m_b != NULL)
		delete[] m_b;
}

bool CFilter::filter(float *x, float *y, int framesPerBuffer) {
	if ((framesPerBuffer < m_order) || (x == NULL) || (y == NULL))
		return false;

	int bufsize = framesPerBuffer * m_channels;
	for (int k = 0; k < bufsize; k += m_channels) {
		for (int c = 0; c < m_channels; c++) {
			y[k + c] = m_b[0] * x[k + c] + m_z[0 + c];
		}
		for (int n = 1; n <= m_order; n++) {
			for (int c = 0; c < m_channels; c++)
				m_z[m_channels * (n - 1) + c] = m_b[n] * x[k + c]
						+ m_z[m_channels * n + c] - m_a[n] * y[k + c];
		}
	}
	return true;
}

/**
 * Creates an echo filter
 *
 * y[n]=x[n]+wight*x[n-delay*fs] - simple echo
 * y[n]=x[n]+(wight-feedback)*x[n-delay*fs]+feedback*y[n-delay*fs] - simple echo with feedback
 */
CDelayFilter::CDelayFilter(float gFF, float gFB, int delay_ms, int fs, int channels)
:CFilterBase((int) (abs(delay_ms) * abs(fs) / 1000.),channels){
	m_firstZ = 0;
	m_gFB = gFB;
	m_gFF = gFF;
}

void CDelayFilter::reset() {
	m_firstZ = 0;
	CFilterBase::reset();
}

bool CDelayFilter::filter(float *x, float *y, int framesPerBuffer) {
	if ((framesPerBuffer < m_order) || (x == NULL) || (y == NULL))
		return false;

	int bufsize = framesPerBuffer * m_channels;
	for (int k = 0; k < bufsize; k+=m_channels) {
		for(int c=0; c < m_channels; c++)
		{
			y[k+c] = x[k+c] + m_z[m_firstZ+c];
			m_z[m_firstZ+c] = (m_gFF-m_gFB) * x[k+c] + m_gFB * y[k+c];
		}
		m_firstZ = (m_firstZ + m_channels) % (m_channels * (m_order+1));
	}

	return true;
}

