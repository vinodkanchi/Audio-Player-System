/*
 * CAmpMeter.h
 *
 *  Created on: 21.10.2019
 *      Author: Wirth
 */

#ifndef CAMPMETER_H_
#define CAMPMETER_H_

enum SCALING_MODES{SCALING_MODE_LIN,SCALING_MODE_LOG};
enum AMP_ERRORS{AMP_E_NOBUFFER,AMP_E_NOVISUALIZER};

class CPlayerIOCtrls;

class CAmpMeter
{
private:
	SCALING_MODES m_scmode;				// logarithmic or linear scaling
	float m_scMax;						// maximum of the scale
	float m_thresholds[8];				// thresholds for the bar with 8 segments (simulates LEDs)
	CPlayerIOCtrls* m_vis;				// address of an object that may show binary patterns on the screen (visualizer)

public:
	/**
	 * Constructor
	 * initializes the attributes with initial values (see UML class diagram)
	 */
	CAmpMeter();

	/**
	 * \brief Initializes the amplitude meter.
	 * \param min, max [in] The range of the data to visualize. The maximum absolute value of min and max is taken for the maximum of the scale.
	 * \param scmode [in] The scaling of the amplitude meter (may be SCALING_MODE_LIN or SCALING_MODE_LOG).
	 * \param logScaleMin [in] The minimum of the logarithmic scale (used for logarithmic scaling only, may be set to 0 for linear scaling).
	 * \param ppiocs [in] The address of an object that may show binary patterns on the screen.
	 */
	void init(float min, float max, SCALING_MODES scmod, int logScaleMin, CPlayerIOCtrls* ppiocs);

	/**
	 * \brief Visualizes the amplitude of a data buffer on the screen, simulating an LED line.
	 * \param databuf [in] The address of the first data buffer element.
	 * \param databufsize [in] The number of elements of the data buffer.
	 */
	void write(float* databuf, unsigned long databufsize);

	/**
	 * \brief Visualizes the amplitude of one single data value on the on the screen, simulating an LED line.
	 * \param data [in] The data value.
	 */
	void write(float data);

private:
	/**
	 * \brief Returns an appropriate bar pattern for the data value (e.g. returns 0b11111111 if the absolute value of data is equal to the maximum data value).
	 * \brief The bar pattern may be used for visualization on the screen or the LED line.
	 * \param data [in] The data value.
	 */
	unsigned char _getBarPattern(float data);

	/**
	 * \brief Returns an representative amplitude value for the data buffer. This value is taken for the visualization of the buffer.
	 * \param databuf [in] The address of the first data buffer element.
	 * \param databufsize [in] The number of elements of the data buffer.
	 */
	float _getValueFromBuffer(float* databuf, unsigned long databufsize);
};

#endif /* CAMPMETER_H_ */
