/*
MIT License

Copyright (c) 2016 Mike Estee
Copyright (c) 2017 Tom Magnier
Copyright (c) 2026 Daniel Sasik

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

#include "TMC5160.h"
#include <math.h>
#include <string.h>

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define CONSTRAIN(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

#ifndef NAN
#define NAN(0.0f / 0.0f)
#endif

static float speedToHz(const TMC5160 *self, int32_t speedInternal)
{
	return((float)speedInternal *(float)self->fclk / (float)(1u << 24) / (float)TMC5160_USTEP_COUNT);
}

static int32_t speedFromHz(const TMC5160 *self, float speedHz)
{
	return (int32_t)(speedHz / ((float)self->fclk / (float)(1u << 24)) * (float)TMC5160_USTEP_COUNT);
}

static int32_t accelFromHz(const TMC5160 *self, float accelHz)
{
	return (int32_t)(accelHz /
		((float)self->fclk * (float)self->fclk / 
		(512.0 * 256.0) / (float)(1u << 24)) * (float)TMC5160_USTEP_COUNT);
}

static int32_t thrsSpeedToTstep(const TMC5160 *self, float thrsSpeed)
{
	return thrsSpeed != 0.0f ? (int32_t)CONSTRAIN((float)self->fclk / 
	(thrsSpeed * 256.0f), 0.0f, 1048575.0f) : 0;
}

// default params
PowerStageParams defaultPowerStageParams(void)
{
	PowerStageParams p;
	p.drvStr = 2;
	p.bbmTime = 0;
	p.bbmClks = 4;
	return p;
}

MotorParams defaultMotorParams(void)
{
	MotorParams m;
	m.globalScaler = 32;
	m.irun = 16;
	m.ihold = 0;
	m.freewheeling = FREEWHEEL_NORMAL;
	m.pwmGradInitial = 0;
	m.pwmOfsInitial = 30;
	return m; 
}

void TMC5160_init(TMC5160 *self, uint32_t fclk)
{
	self->fclk = fclk;
	self->lastRegisterReadSuccess = false;
	self->currentRampmode = RAMP_POSITIONING;
	self->chopConf.value = 0;
	self->readregister = NULL;
	self->writeregister = NULL;
}

// Core driver API
bool begin(TMC5160 *self, const PowerStageParams *powerParams,
			const MotorParams *motorParams, MotorDir stepperDirections)
{
	// clear the reset and charge pump undervoltage flags
	GSTAT_Register_t gstat = {0};
	gstat.reset = true;
	gstat.uv_cp = true;
	self->writeregister(self, GSTAT, gstat.value);

	DRV_CONF_Register_t drvConf = {0};
	drvConf.drvstrength = CONSTRAIN(powerParams->drvStr, 0, 3);
	drvConf.bbmtime = CONSTRAIN(powerParams->bbmTime, 0, 24);
	drvConf.bbmclks = CONSTRAIN(powerParams->bbmClks, 0, 25);
	self->writeregister(self, DRV_CONF, drvConf.value);

	self->writeregister(self, GLOBAL_SCALER, CONSTRAIN(motorParams->globalScaler, 32, 256));

	// set initial currents and delay
	IHOLD_IRUN_Register_t iholdrun = {0};
	iholdrun.ihold = CONSTRAIN(motorParams->ihold, 0, 31);
	iholdrun.irun = CONSTRAIN(motorParams->irun, 0, 31);
	iholdrun.iholddelay = 7;
	self->writeregister(self, IHOLD_IRUN, iholdrun.value);

	// todo set short detection / overcurrent protection levels 

	// set initial PWM vals
	PWMCONF_Register_t pwmconf = {0};
	pwmconf.value = 0xC40C001E; // reset default
	pwmconf.pwm_autoscale = false;
	if(self->fclk > TMC5160_DEFAULT_FCLK)
		pwmconf.pwm_freq = 0;
	else
		pwmconf.pwm_freq = 0x01; // recommended 35kHz with internal 12Mhz clock
	pwmconf.pwm_grad = motorParams->pwmGradInitial;
	pwmconf.pwm_ofs = motorParams->pwmOfsInitial;
	pwmconf.freewheel = motorParams->freewheeling;
	self->writeregister(self, PWMCONF, pwmconf.value);

	pwmconf.pwm_autoscale = true;
	pwmconf.pwm_autograd = true;
	self->writeregister(self, PWMCONF, pwmconf.value);

	// reccomended settings in config guide
	self->chopConf.toff = 5;
	self->chopConf.tbl = 2;
	self->chopConf.hstrt_tfd = 4;
	self->chopConf.hend_offset = 0;
	self->writeregister(self, CHOPCONF, self->chopConf.value);

	// use pos mode
	setRampMode(self, RAMP_POSITIONING);

	GCONF_Register_t gconf = {0};
	gconf.en_pwm_mode = true; //  enable stealthChop pwm mode
	gconf.shaft = stepperDirections;
	self->writeregister(self, GCONF, gconf.value);

	// set default start, stop, threshold speeds.
	setRampSpeeds(self, 0, 0.1f, 0);

	// set default D1 (cant be 0 in pos mode even with v1 = 0)
	self->writeregister(self, D_1, 100);

	return false;
}

void end(TMC5160 *self)
{
	(void)self;
	// nothign stop talking
}

bool isLastReadSuccesful(TMC5160 *self)
{
	return self->lastRegisterReadSuccess;
}

void setRampMode(TMC5160 *self, RampMode mode)
{
	switch(mode)
	{
		case RAMP_POSITIONING:
			self->writeregister(self, RAMPMODE, POSITIONING_MODE);
			break;
		case RAMP_VELOCITY:
			setMaxSpeed(self, 0); // no way to know dir => set speed to 0
			self->writeregister(self, RAMPMODE, VELOCITY_MODE_POS);
			break;
		case RAMP_HOLD:
			self->writeregister(self, RAMPMODE, HOLD_MODE);
	}
	
	self->currentRampmode = mode;
}

float getCurrentPos(TMC5160 *self)
{
	int32_t uStepPos = (int32_t)self->readregister(self, XACTUAL);

	if((uint32_t)(uStepPos) == 0xFFFFFFFF)
		return NAN;
	else
		return (float)uStepPos / (float)TMC5160_USTEP_COUNT;
}

float getEncoderPos(TMC5160 *self)
{
	int32_t uStepPos = (int32_t)self->readregister(self, X_ENC);

	if((uint32_t)(uStepPos) == 0xFFFFFFFF)
		return NAN;
	else
		return (float)uStepPos / (float)TMC5160_USTEP_COUNT;
}

float getLatchedPos(TMC5160 *self)
{
	int32_t uStepPos = (int32_t)self->readregister(self, XLATCH);

	if((uint32_t)(uStepPos) == 0xFFFFFFFF)
		return NAN;
	else
		return (float)uStepPos / (float)TMC5160_USTEP_COUNT;
}

float getLatchedEncoderPos(TMC5160 *self)
{
	int32_t uStepPos = (int32_t)self->readregister(self, ENC_LATCH);

	if((uint32_t)(uStepPos) == 0xFFFFFFFF)
		return NAN;
	else
		return (float)uStepPos / (float)TMC5160_USTEP_COUNT;
}

float getTargetPosition(TMC5160 *self)
{
	int32_t uStepPos = (int32_t)self->readregister(self, XTARGET);

	if((uint32_t)(uStepPos) == 0xFFFFFFFF)
		return NAN;
	else
		return (float)uStepPos / (float)TMC5160_USTEP_COUNT;
}

float getCurrentSpeed(TMC5160 *self)
{
	int32_t data = (int32_t)self->readregister(self, VACTUAL);

	if((uint32_t)(data) == 0xFFFFFFFF)
		return NAN;
	
	if(data & (1u << 23))
		data |= 0xFF000000;

	return (float)data / (float)TMC5160_USTEP_COUNT;
}

void setCurrentPos(TMC5160 *self, float postion, bool updateEncoderPos)
{
	self->writeregister(self, XACTUAL, (uint32_t)(int32_t)(postion * (float)TMC5160_USTEP_COUNT));

	if(updateEncoderPos)
	{
		self->writeregister(self, X_ENC, (uint32_t)(int32_t)(postion * (float)TMC5160_USTEP_COUNT));
		clearEncoderDeviationFlag(self);
	}
}

void setTargetPos(TMC5160 *self, float pos)
{
	self->writeregister(self, XTARGET, (uint32_t)(int32_t)(pos * (float)TMC5160_USTEP_COUNT));
}

void setMaxSpeed(TMC5160 *self, float speed)
{
	self->writeregister(self, VMAX, MIN(0x7FFFFF, speedFromHz(self, fabsf(speed)))); // VMAX : 23 bits

	if(self->currentRampmode == RAMP_VELOCITY)
	{
		self->writeregister(self, RAMPMODE, speed < 0.0f ? VELOCITY_MODE_NEG :VELOCITY_MODE_POS);
	}
}

void setRampSpeeds(TMC5160 *self, float startSpeed, float stopSpeed, float transitionSpeed)
{
	self->writeregister(self, VSTART, MIN(0x3FFFF, speedFromHz(self, fabsf(startSpeed))));
	self->writeregister(self, VSTOP, MIN(0x3FFFF, speedFromHz(self, fabsf(stopSpeed))));
	self->writeregister(self, V_1, MIN(0x3FFFF, speedFromHz(self, fabsf(transitionSpeed))));
}

void setAcceleration(TMC5160 *self, float maxAccel)
{
	self->writeregister(self, AMAX, MIN(0xFFFF, accelFromHz(self, fabsf(maxAccel))));
	self->writeregister(self, DMAX, MIN(0xFFFF, accelFromHz(self, fabsf(maxAccel))));
}

void setAccelerations(TMC5160 *self, float maxAccel, float maxDecel, float startAccel, float finalDecel)
{
	self->writeregister(self, AMAX, MIN(0xFFFF, accelFromHz(self, fabsf(maxAccel))));
	self->writeregister(self, DMAX, MIN(0xFFFF, accelFromHz(self, fabsf(maxAccel))));
	self->writeregister(self, A_1, MIN(0xFFFF, accelFromHz(self, fabsf(startAccel))));
	self->writeregister(self, D_1, MIN(0xFFFF, accelFromHz(self, fabsf(finalDecel))));
}

bool isTargetPosReached(TMC5160 *self)
{
	RAMP_STAT_Register_t rampStat = {0};
	rampStat.value = self->readregister(self, RAMP_STAT);
	return rampStat.position_reached ? true :false; // do i even need the true false here?
}

void stop(TMC5160 *self)
{
	self->writeregister(self, VSTART, 0);
	self->writeregister(self, VMAX, 0);
}

void disable(TMC5160 *self)
{
	CHOPCONF_Register_t chopconf = {0};
	chopconf.value = self->chopConf.value;
	chopconf.toff = 0;
	self->writeregister(self, CHOPCONF, chopconf.value);
}

void enable(TMC5160 *self)
{
	self->writeregister(self, CHOPCONF, self->chopConf.value);
}

DriverStatus getDriverStatus(TMC5160 *self)
{
	GSTAT_Register_t gstat = {0};
	gstat.value = self->readregister(self, GSTAT);
	DRV_STATUS_Register_t drvStatus = {0};
	drvStatus.value = self->readregister(self, DRV_STATUS);

	if(gstat.uv_cp)
		return CP_UV;
	if(drvStatus.s2sa)
		return S2SA;
	if(drvStatus.s2sb)
		return S2SB;
	if(drvStatus.s2ga)
		return S2GA;
	if(drvStatus.s2gb)
		return S2GB;
	if(drvStatus.ot)
		return OT;
	if(gstat.drv_err)
		return OTHER_ERR;
	if(drvStatus.otpw)
		return OTPW;
	
	return OK;
}

const char *getDriverStatusDescription(DriverStatus status)
{
	switch(status)
	{
		case OK		: return "OK";
		case CP_UV	: return "Charge pump undervoltage";
		case S2SA	: return "Short to supply phase A";
		case S2SB	: return "Short to supply phase B";
		case S2GA	: return "Short to ground phase A";
		case S2GB	: return "Short to ground phase B";
		case OT		: return "Overtemperature";
		case OTHER_ERR 	: return "Other driver error";
		case OTPW		: return "Overtemperature warning";
		default			: break;
	}
	return "Uknown status";
}

void setModeChangeSpeeds(TMC5160 *self, float pwmThrs, float coolThrs, float highThrs)
{
	self->writeregister(self, TPWMTHRS, MIN(0xFFFFF, thrsSpeedToTstep(self, pwmThrs)));
	self->writeregister(self, TCOOLTHRS, MIN(0xFFFFF, thrsSpeedToTstep(self, coolThrs)));
	self->writeregister(self, THIGH, MIN(0xFFFFF, thrsSpeedToTstep(self, highThrs)));
}

bool setEncoderResolution(TMC5160 *self, int32_t motorSteps, int32_t encResolution, bool inverted)
{ 
	float factor = (float)motorSteps * (float)TMC5160_USTEP_COUNT / (float)encResolution;

	// Check if the binary prescaler gives an exact match
	if((int32_t)(factor * 65536.0f) * encResolution == motorSteps * TMC5160_USTEP_COUNT * 65536)
	{
		ENCMODE_Register_t encmode = {0};
		encmode.value = self->readregister(self, ENCMODE);
		encmode.enc_sel_decimal = false;
		self->writeregister(self, ENCMODE, encmode.value);

		int32_t encCosnt = (int32_t)(factor * 65536.0f);
		if(inverted)
			encCosnt = -encCosnt;
		self->writeregister(self, ENC_CONST, (uint32_t)encCosnt);

		return true;
	}
	else
	{
		ENCMODE_Register_t encmode = {0};
		encmode.value = self->readregister(self, ENCMODE);
		encmode.enc_sel_decimal = true;
		self->writeregister(self, ENCMODE, encmode.value);

		int integerPart = (int)floorf(factor);
		int decimalPart = (int)((factor - (float)integerPart) * 10000.0f);
		if(inverted)
		{
			integerPart = 65535 - integerPart;
			decimalPart = 10000 - decimalPart;
		}
		int32_t encConst = integerPart * 65536 + decimalPart;
		self-> writeregister(self, ENC_CONST, (uint32_t)encConst);

		// check if the decimal prescaler gives exact match
		return ((int32_t)(factor * 10000.0f) * encResolution == motorSteps * (int32_t)TMC5160_USTEP_COUNT * 10000);
	}
}

void setEncoderIndexConfiguration(TMC5160 *self, enum ENCMODE_sensitivity_Values sensitivity,
									bool nActiveHigh, bool ignorePol, bool aActiveHigh, bool bActiveHigh)
{
	ENCMODE_Register_t encmode = {0};
	encmode.value = self->readregister(self, ENCMODE);

	encmode.sensitivity = sensitivity;
	encmode.pol_N = nActiveHigh;
	encmode.ignore_AB = ignorePol;
	encmode.pol_A = aActiveHigh;
	encmode.pol_B = bActiveHigh;

	self->writeregister(self, ENCMODE, encmode.value);
}

void setEncoderLatching(TMC5160 *self, bool enabled)
{
	ENCMODE_Register_t encmode = {0};
	encmode.value = self->readregister(self, ENCMODE);

	encmode.latch_x_act = true;
	encmode.clr_cont = enabled;

	self->writeregister(self, ENCMODE, encmode.value);
}

void setEncoderAllowedDeviation(TMC5160 *self, int steps)
{
	self->writeregister(self, ENC_DEVIATION, MIN(0xFFFFF, (uint32_t)(steps * TMC5160_USTEP_COUNT)));
}

bool isEncoderDeviationDetected(TMC5160 *self)
{
	ENC_STATUS_Register_t encstatus = {0};
	encstatus.value = self->readregister(self, ENC_STATUS);
	return isLastReadSuccesful(self) && encstatus.deviation_warn;
}

void clearEncoderDeviationFlag(TMC5160 *self)
{
	ENC_STATUS_Register_t encStatus = {0};
	encStatus.deviation_warn = true;
	self->writeregister(self, ENC_STATUS, encStatus.value);
}

void setShortProtectionLevels(TMC5160 *self, int s2slevel, int s2gLevel, int shortFilter, int shortDelay)
{
	SHORT_CONF_Register_t shortconf= {0};
	shortconf.s2s_level = CONSTRAIN(s2slevel, 4, 15);
	shortconf.s2g_level = CONSTRAIN(s2gLevel, 2, 15);
	shortconf.shortfilter = CONSTRAIN(shortFilter, 0, 3);
	shortconf.shortdelay = CONSTRAIN(shortDelay, 0, 1);

	self->writeregister(self, SHORT_CONF, shortconf.value);
}

// SPI 
static uint32_t spi_readRegister(TMC5160 *base, uint8_t address)
{
	SPI *self = (SPI *)base;
	uint8_t b3, b2, b1, b0;

	// first access. latch the adress. the data come backl on the next frame
	if(self->beginTransaction) 
		self->beginTransaction(self->context);
	
	self->transfer(self->context, address);
	self->transfer(self->context, 0);
	self->transfer(self->context, 0);
	self->transfer(self->context, 0);
	self->transfer(self->context, 0);

	// second access. read the latched data
	if(self->beginTransaction)
		self->beginTransaction(self->context);

	self->status = self->transfer(self->context, address);
	b3 = self->transfer(self->context, 0);
	b2 = self->transfer(self->context, 0);
	b1 = self->transfer(self->context, 0);
	b0 = self->transfer(self->context, 0);
	
	if(self->endTransaction) self->endTransaction(self->context);

	base->lastRegisterReadSuccess = true;

	return ((uint32_t)b3 << 24) | ((uint32_t) b2 << 16) | ((uint32_t)b1 << 8) | (uint32_t)b0;
}

static uint32_t spi_writeRegister(TMC5160 *base, uint8_t address, uint32_t data)
{
	SPI *self = (SPI *)base;

	if(self->beginTransaction) 
		self->beginTransaction(self->context);
	
 	self->status = self->transfer(self->context, address | TMC5160_WRITE_ACCESS);
	self->transfer(self->context, (uint8_t)(data >> 24));
	self->transfer(self->context, (uint8_t)(data >> 16));
	self->transfer(self->context, (uint8_t)(data >> 8));
	self->transfer(self->context, (uint8_t)(data));
	if(self->endTransaction) self->endTransaction(self->context);

	return self->status;
}

void SPI_init(SPI *self, uint32_t fclk, 
              uint8_t (*transfer)(void *context, uint8_t data),
              void (*beginTransaction)(void *context),
              void (*endTransaction)(void *context),
              void *context)
{
	TMC5160_init(&self->base, fclk);
	self->base.readregister = spi_readRegister;
	self->base.writeregister = spi_writeRegister;

	self->transfer = transfer;
	self->beginTransaction = beginTransaction;
	self->endTransaction = endTransaction;
	self->context = context;
	self->status = 0;
}

uint8_t SPI_readStatus(SPI *self)
{
	if(self->beginTransaction) 
		self->beginTransaction(self->context);
	
	self->status = self->transfer(self->context, 0);
	self->transfer(self->context, 0);
	self->transfer(self->context, 0);
	self->transfer(self->context, 0);
	self->transfer(self->context, 0);
	if(self->endTransaction) self->endTransaction(self->context);
	
	return self->status;
}

