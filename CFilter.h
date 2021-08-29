/*
 * CFilter.h
 *
 *  Created on: 11.09.2019
 *      Author: Wirth
 */

/**
 * \file CFilter.h
 *
 * \brief header file for filter class
 *
 * \see CFilter
 */
#ifndef CFILTER_H_
#define CFILTER_H_

class CFilterBase {
protected:
	/**
	 * \brief intermediate states from last sample or circular buffer (optimized delay filters)
	 */
	float *m_z;
	/**
	 * \brief filter order
	 */
	int m_order;
	/**
	 * \brief number of channels of the signals to be filtered
	 */
	int m_channels;

public:
	CFilterBase(int order, int channels);
	virtual ~CFilterBase();
	/**
	 * \brief pure virtual method defines the filter methods interface.
	 */
	virtual bool filter(float *x, float *y, int framesPerBuffer)=0;
	/**
	 * \brief Clears all intermediate values. May be used before filtering a new signal by the same filter object.
	 */
	void reset();

	/**
	 * \brief Getter methods (Lab05)
	 */
	int getOrder();
	int getNumChannels();
};
/**
 * \brief General filter class to calculate digital filter output via direct form II transposed
 * \brief implements operation filter, handles errors
 */
class CFilter : public CFilterBase{
private:
	/**
	 * \brief numerator filter coefficients
	 */
	float *m_b;
	/**
	 * \brief denominator filter coefficients
	 */
	float *m_a;

public:
	/**
	 * \brief Creates a filter object.
	 * \param ca, cb [in] pointers to the arrays of denominator and numerator filter coefficients
	 * \param order [in] filter order
	 * \param channels [in] number of channels of the signals to be filtered  (input signal)
	 */
	CFilter(float *ca, float *cb, int order, int channels = 2);
	virtual ~CFilter();
	/**
	 * \brief Filters a signal.
	 * \param x [in] pointer to the signal to be filtered (input signal)
	 * \param y [in] pointer to a buffer to store the filtered signal (output signal)
	 * \param framesPerBuffer [in] number of frames to be filtered (the signal length resp. buffer size of x and y must be framesPerBuffer*channels)
	 */
	bool filter(float *x, float *y, int framesPerBuffer);// straight forward difference equation
};

class CDelayFilter : public CFilterBase{
private:
	/**
	 * \brief Feed forward gain
	 */
	float m_gFF;
	/**
	 * \brief Feed back gain
	 */
	float m_gFB;
	/**
	 * \brief first element to read and write after block has been changed (circular buffer start position)
	 */
	int m_firstZ;

public:
	/**
	 * \brief to be implemented in task 2 - optimized delay filters
	 * \brief Creates a delay filter object (alternative constructor).
	 * \param gFF [in] feedforward gain of the delay filter (must not be 0!)
	 * \param gFB [in] feedback gain of the delay filter (is 0 for basic delay filters without feedback)
	 * \param delay_ms [in] delay time in ms
	 * \param fs [in] sampling frequency of the signal to be filtered  (input signal)
	 * \param channels [in] number of channels of the signal to be filtered  (input signal)
	 */
	CDelayFilter(float gff, float gFB, int delay_ms, int fs, int channels = 2);

	/**
	 * \brief Clears all intermediate values. May be used before filtering a new signal by the same filter object.
	 */
	void reset();

	/**
	 * \brief to be implemented in task 2 - optimized delay filters
	 * \brief Filters a signal (delay filter).
	 * \param x [in] pointer to the signal to be filtered (input signal)
	 * \param y [in] pointer to a buffer to store the filtered signal (output signal)
	 * \param framesPerBuffer [in] number of frames to be filtered (the signal length resp. buffer size of x and y must be framesPerBuffer*channels)
	 */
	bool filter(float *x, float *y, int framesPerBuffer);
};

#endif /* CFILTER_H_ */

