/*
MIT License

Copyright (c) 2016 Mike Estee (Estee_TMC5160)
Copyright (c) 2017 Tom Magnier

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef TMC5160_H
#define TMC5160_H

//#include "stm32f4xx_hal.h"
#include "TMC5160_registers.h"
#include <stdbool.h>
#include <stdint.h>

#define TMC5160_IC_VERSION		0x30u
#define TMC5160_DEFAULT_FCLK	12000000u 	// typical interal clock freq in Hz
#define TMC5160_USTEP_COUNT		256			// number of microsteps per full step
#define TMC5160_WRITE_ACCESS	0X80u		// write access for SPI / UART

typedef struct
{

} TMC5160_MOTOR;

typedef enum
{
	NORMAL_MOTOR_DIR = 0x00,
	INVERSE_MOTOR_DIR = 0x01
} MotorDir;

typedef enum
{
    RAMP_POSITIONING,
    RAMP_VELOCITY,
    RAMP_HOLD
} RampMode;

typedef enum
{
	OK,			// no error detected
	CP_UV,		// charge pump undervoltage
	S2SA,		// short to supply phase A
	S2SB,		// short to supply phase B
	S2GA,		// short to ground phase A
	S2GB,		// short to ground phase B
	OT,			// overtemperature (error)
	OTHER_ERR,	// gstat drv error is set but none of the above found (errors can be extended in the future)
	OTPW		// overtemperature
} DriverStatus;

typedef struct
{
	uint8_t drvStr;		// MOSFET gate driver current (0-3)
	uint8_t bbmTime;	// "Break before make" duration in ns (0-24)
	uint8_t bbmClks;	// "Break before make" duration in clock cycles (0-15)
} PowerStageParams;

typedef struct
{
	uint8_t globalScaler; 	// global current scaling (32-256)
	uint8_t irun;			// motor run current (0-31)
	uint8_t ihold;			// standstill current (0-31)
	enum PWMCONF_freewheel_Values freewheeling;	// freewheeling / pasive braking when ihold = 0
	uint8_t pwmOfsInitial; 	// initial stealhChop PWM aplitude offset (0-255)
	uint8_t pwmGradInitial;	// initial stealhChop PWM velocity dependent gradient
} MotorParams;

// Base "object". The first member of every transport struct must be a TMC5160
// so that the transport can be used through a tmc5160 pointer.
typedef struct TMC5160 TMC5160;
struct TMC5160
{
	uint32_t fclk;
	bool lastRegisterReadSuccess;
	RampMode currentRampmode;
	CHOPCONF_Register_t chopConf; // save to restore on enable / disable

	// "virtual" transport methods, set by the transport init functions
	uint32_t (*readregister)(TMC5160 *self, uint8_t adress);
	uint32_t (*writeregister)(TMC5160 *self, uint8_t adress, uint32_t data);
};

// Return parameter structs pre-filled with the library default values. 
PowerStageParams defaultPowerStageParams(void);
MotorParams defaultMotorParams(void);

// Initialise the common base state. Called by the transport init functions;
// normally not used directly.
void init(TMC5160 *self, uint32_t fclk);

// Core driver API (transport agnostic
bool begin(TMC5160 *self, const PowerStageParams *powerParams,
			const MotorParams *motorParams, MotorDir stepperDirections);

void end(TMC5160 *self);

bool isLastReadSuccesful(TMC5160 *self);

void setRampMode(TMC5160 *self, RampMode mode);

float getCurrentPos(TMC5160 *self);
float getEncoderPos(TMC5160 *self);
float getLatchedPos(TMC5160 *self);			// latched position on  last event (steps)
float getLatchedEncoderPos(TMC5160 *self);
float getTargetPosition(TMC5160 *self);		
float getCurrentSpeed(TMC5160 *self);		// (steps / s)

void setCurrentPos(TMC5160 *self, float pos, bool updateEncoderPos);
void setTargetPos(TMC5160 *self, float pos);
void setMaxSpeed(TMC5160 *self, float speed);
void setRampSpeeds(TMC5160 *self, float startSpeed, float stopSpeed, float transitionSpeed);
void setAcceleration(TMC5160 *self, float maxAccel);
void setAccelerations(TMC5160 *self, float maxAccel, float maxDecel, float startAccel, float finalDecel);

bool isTargetPosReached(TMC5160 *self);
bool isTargetVelReached(TMC5160 *self);

void stop(TMC5160 *self);;
void disable(TMC5160 *self);
void enable(TMC5160 *self);

DriverStatus getDriverStatus(TMC5160 *self);
const char *getDriverStatusDescription(DriverStatus st);

void setModeChangeSpeeds(TMC5160 *self, float pwmThrs, float coolThrs, float highThrs);

bool setEncoderResolution(TMC5160 *self, int32_t motorSteps, int32_t encResolution, bool inverted);
void setEncoderIndexConfiguration(TMC5160 *self, enum ENCMODE_sensitivity_Values sensitivity,
									bool nActiveHigh, bool ignorePol, bool aActiveHigh, bool bActiveHigh);
void setEncoderLatching(TMC5160 *self, bool enabled);
void setEncoderAllowedDeviation(TMC5160 *self, int steps);
bool isEncoderDeviationDetected(TMC5160 *self);
void clearEncoderDeviationFlag(TMC5160 *self);

void setShortProtectionLevels(TMC5160 *self, int s2sLevel, int s2gLevel, int shortFilter, int shortDelay);

// SPI
typedef struct 
{
	TMC5160 base;
	void (*beginTransaction)(void *context); // optional may be null
	void (*endTransaction)(void *context); // optional may be null
	uint8_t (*transfer)(void *context, uint8_t data); // required full duplex byte transfer
	void *context;
	uint8_t status; // SPI status byte from last access
} SPI;

void SPI_init(SPI *self, uint32_t fclk, 
              uint8_t (*transfer)(void *context, uint8_t data),
              void (*beginTransaction)(void *context),
              void (*endTransaction)(void *context),
              void *context);
uint8_t SPI_readStatus(SPI *self);

#endif
