/*
MIT License

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

#ifndef TMC5160_REGISTERS_H
#define TMC5160_REGISTERS_H

#include <stdint.h>

/* Register addresses */
//enum TMC_Registers {
enum {
	GCONF             = 0x00,
    GSTAT             = 0x01,
    IFCNT             = 0x02,
    SLAVECONF         = 0x03,
    IO_INPUT_OUTPUT   = 0x04,
    X_COMPARE         = 0x05,
    OTP_PROG          = 0x06,
    OTP_READ          = 0x07,
    FACTORY_CONF      = 0x08,
    SHORT_CONF        = 0x09,
    DRV_CONF          = 0x0A,
    GLOBAL_SCALER     = 0x0B,
    OFFSET_READ       = 0x0C,
    IHOLD_IRUN        = 0x10,
    TPOWERDOWN        = 0x11,
    TSTEP             = 0x12,
    TPWMTHRS          = 0x13,
    TCOOLTHRS         = 0x14,
    THIGH             = 0x15,
    RAMPMODE          = 0x20,
    XACTUAL           = 0x21,
    VACTUAL           = 0x22,
    VSTART            = 0x23,
    A_1               = 0x24,
    V_1               = 0x25,
    AMAX              = 0x26,
    VMAX              = 0x27,
    DMAX              = 0x28,
    D_1               = 0x2A,
    VSTOP             = 0x2B,
    TZEROWAIT         = 0x2C,
    XTARGET           = 0x2D,
    VDCMIN            = 0x33,
    SW_MODE           = 0x34,
    RAMP_STAT         = 0x35,
    XLATCH            = 0x36,
    ENCMODE           = 0x38,
    X_ENC             = 0x39,
    ENC_CONST         = 0x3A,
    ENC_STATUS        = 0x3B,
    ENC_LATCH         = 0x3C,
    ENC_DEVIATION     = 0x3D,
    MSLUT_0_7         = 0x60,
    MSLUTSEL          = 0x68,
    MSLUTSTART        = 0x69,
    MSCNT             = 0x6A,
    MSCURACT          = 0x6B,
    CHOPCONF          = 0x6C,
    COOLCONF          = 0x6D,
    DCCTRL            = 0x6E,
    DRV_STATUS        = 0x6F,
    PWMCONF           = 0x70,
    PWM_SCALE         = 0x71,
    PWM_AUTO          = 0x72,
    LOST_STEPS        = 0x73
};

/* Register bit‑field unions */

/* General configuration */
typedef union {
    uint32_t value;
    struct {
        uint32_t recalibrate         : 1;
        uint32_t faststandstill      : 1;
        uint32_t en_pwm_mode         : 1;
        uint32_t multistep_filt      : 1;
        uint32_t shaft               : 1;
        uint32_t diag0_error         : 1;
        uint32_t diag0_otpw          : 1;
        uint32_t diag0_stall_step    : 1;
        uint32_t diag1_stall_dir     : 1;
        uint32_t diag1_index         : 1;
        uint32_t diag1_onstate       : 1;
        uint32_t diag1_steps_skipped : 1;
        uint32_t diag0_int_pushpull  : 1;
        uint32_t diag1_poscomp_pushpull : 1;
        uint32_t small_hysteresis    : 1;
        uint32_t stop_enable         : 1;
        uint32_t direct_mode         : 1;
        uint32_t test_mode           : 1;
        uint32_t reserved            : 14; // bits 18‑31
    };
} GCONF_Register_t;

/* Global status flags */
typedef union {
    uint32_t value;
    struct {
        uint32_t reset   : 1;
        uint32_t drv_err : 1;
        uint32_t uv_cp   : 1;
        uint32_t reserved: 29;
    };
} GSTAT_Register_t;

/* UART slave configuration */
typedef union {
    uint32_t value;
    struct {
        uint32_t slaveaddr : 8;  // bits 0‑7
        uint32_t senddelay : 4;  // bits 8‑11
        uint32_t reserved  : 20; // bits 12‑31
    };
} SLAVECONF_Register_t;

/* Read input pins */
typedef union {
    uint32_t value;
    struct {
        uint32_t refl_step          : 1;
        uint32_t refr_dir           : 1;
        uint32_t encb_dcen_cfg4     : 1;
        uint32_t enca_dcin_cfg5     : 1;
        uint32_t drv_enn            : 1;
        uint32_t enc_n_dco_cfg6     : 1;
        uint32_t sd_mode            : 1;
        uint32_t swcomp_in          : 1;
        uint32_t reserved0          : 16; // bits 8‑23
        uint32_t version            : 8;  // bits 24‑31
    };
} IOIN_Register_t;

/* OTP programming */
typedef union {
    uint32_t value;
    struct {
        uint32_t otpbit   : 3;  // bits 0‑2
        uint32_t reserved0: 1;  // bit 3
        uint32_t otpbyte  : 2;  // bits 4‑5
        uint32_t reserved1: 2;  // bits 6‑7
        uint32_t otpmagic : 8;  // bits 8‑15
        uint32_t reserved2: 16; // bits 16‑31
    };
} OTP_PROG_Register_t;

/* OTP configuration memory */
typedef union {
    uint32_t value;
    struct {
        uint32_t otp_fclktrim : 5;  // bits 0‑4
        uint32_t otp_S2_level : 1;  // bit 5
        uint32_t otp_bbm      : 1;  // bit 6
        uint32_t otp_tbl      : 1;  // bit 7
        uint32_t reserved     : 24; // bits 8‑31
    };
} OTP_READ_Register_t;

/* Short detector configuration */
typedef union {
    uint32_t value;
    struct {
        uint32_t s2s_level   : 4;  // bits 0‑3
        uint32_t reserved0    : 4;  // bits 4‑7
        uint32_t s2g_level    : 4;  // bits 8‑11
        uint32_t reserved1    : 4;  // bits 12‑15
        uint32_t shortfilter  : 2;  // bits 16‑17
        uint32_t shortdelay   : 1;  // bit 18
        uint32_t reserved2    : 13; // bits 19‑31
    };
} SHORT_CONF_Register_t;

/* Driver configuration */
typedef union {
    uint32_t value;
    struct {
        uint32_t bbmtime    : 5;  // bits 0‑4
        uint32_t reserved0  : 3;  // bits 5‑7
        uint32_t bbmclks    : 4;  // bits 8‑11
        uint32_t reserved1  : 4;  // bits 12‑15
        uint32_t otselect   : 2;  // bits 16‑17
        uint32_t drvstrength: 2;  // bits 18‑19
        uint32_t filt_isense: 2;  // bits 20‑21
        uint32_t reserved2  : 10; // bits 22‑31
    };
} DRV_CONF_Register_t;

/* Offset calibration result */
typedef union {
    uint32_t value;
    struct {
        uint32_t phase_b : 8;  // bits 0‑7
        uint32_t phase_a : 8;  // bits 8‑15
        uint32_t reserved: 16; // bits 16‑31
    };
} OFFSET_READ_Register_t;

/* Driver current control */
typedef union {
    uint32_t value;
    struct {
        uint32_t ihold      : 5;  // bits 0‑4
        uint32_t reserved0  : 3;  // bits 5‑7
        uint32_t irun       : 5;  // bits 8‑12
        uint32_t reserved1  : 3;  // bits 13‑15
        uint32_t iholddelay : 4;  // bits 16‑19
        uint32_t reserved2  : 12; // bits 20‑31
    };
} IHOLD_IRUN_Register_t;

/* Switch mode configuration */
typedef union {
    uint32_t value;
    struct {
        uint32_t stop_l_enable     : 1;
        uint32_t stop_r_enable     : 1;
        uint32_t pol_stop_l        : 1;
        uint32_t pol_stop_r        : 1;
        uint32_t swap_lr           : 1;
        uint32_t latch_l_active    : 1;
        uint32_t latch_l_inactive  : 1;
        uint32_t latch_r_active    : 1;
        uint32_t latch_r_inactive  : 1;
        uint32_t en_latch_encoder  : 1;
        uint32_t sg_stop           : 1;
        uint32_t en_softstop       : 1;
        uint32_t reserved          : 20; // bits 12‑31
    };
} SW_MODE_Register_t;

/* Ramp status and switch event status */
typedef union {
    uint32_t value;
    struct {
        uint32_t status_stop_l       : 1;
        uint32_t status_stop_r       : 1;
        uint32_t status_latch_l      : 1;
        uint32_t status_latch_r      : 1;
        uint32_t event_stop_l        : 1;
        uint32_t event_stop_r        : 1;
        uint32_t event_stop_sg       : 1;
        uint32_t event_pos_reached   : 1;
        uint32_t velocity_reached    : 1;
        uint32_t position_reached    : 1;
        uint32_t vzero               : 1;
        uint32_t t_zerowait_active   : 1;
        uint32_t second_move         : 1;
        uint32_t status_sg           : 1;
        uint32_t reserved            : 18; // bits 14‑31
    };
} RAMP_STAT_Register_t;

/* Encoder configuration */
typedef union {
    uint32_t value;
    struct {
        uint32_t pol_A          : 1;
        uint32_t pol_B          : 1;
        uint32_t pol_N          : 1;
        uint32_t ignore_AB      : 1;
        uint32_t clr_cont       : 1;
        uint32_t clr_once       : 1;
        uint32_t sensitivity    : 2;  // bits 6‑7
        uint32_t clr_enc_x      : 1;  // bit 8
        uint32_t latch_x_act    : 1;  // bit 9
        uint32_t enc_sel_decimal: 1;  // bit 10
        uint32_t reserved       : 21; // bits 11‑31
    };
} ENCMODE_Register_t;

/* Encoder status */
typedef union {
    uint32_t value;
    struct {
        uint32_t n_event        : 1;
        uint32_t deviation_warn : 1;
        uint32_t reserved       : 30;
    };
} ENC_STATUS_Register_t;

/* Chopper and driver configuration */
typedef union {
    uint32_t value;
    struct {
        uint32_t toff           : 4;  // bits 0‑3
        uint32_t hstrt_tfd      : 3;  // bits 4‑6
        uint32_t hend_offset    : 4;  // bits 7‑10
        uint32_t tfd_3          : 1;  // bit 11
        uint32_t disfdcc        : 1;  // bit 12
        uint32_t rndtf          : 1;  // bit 13
        uint32_t chm            : 1;  // bit 14
        uint32_t tbl            : 2;  // bits 15‑16
        uint32_t vsense         : 1;  // bit 17
        uint32_t vhighfs        : 1;  // bit 18
        uint32_t vhighchm       : 1;  // bit 19
        uint32_t tpfd           : 4;  // bits 20‑23
        uint32_t mres           : 4;  // bits 24‑27
        uint32_t intpol         : 1;  // bit 28
        uint32_t dedge          : 1;  // bit 29
        uint32_t diss2g         : 1;  // bit 30
        uint32_t diss2vs        : 1;  // bit 31
    };
} CHOPCONF_Register_t;

/* coolStep and stallGuard2 configuration */
typedef union {
    uint32_t value;
    struct {
        uint32_t semin          : 4;  // bits 0‑3
        uint32_t seup           : 2;  // bits 4‑5
        uint32_t reserved0      : 2;  // bits 6‑7
        uint32_t semax          : 4;  // bits 8‑11
        uint32_t reserved1      : 1;  // bit 12
        uint32_t sedn           : 2;  // bits 13‑14
        uint32_t seimin         : 1;  // bit 15
        uint32_t sgt            : 7;  // bits 16‑22
        uint32_t reserved2      : 1;  // bit 23
        uint32_t sfilt          : 1;  // bit 24
        uint32_t reserved3      : 7;  // bits 25‑31
    };
} COOLCONF_Register_t;

/* dcStep configuration */
typedef union {
    uint32_t value;
    struct {
        uint32_t dc_time        : 10; // bits 0‑9
        uint32_t reserved0      : 6;  // bits 10‑15
        uint32_t dc_sg          : 8;  // bits 16‑23
        uint32_t reserved1      : 8;  // bits 24‑31
    };
} DCCTRL_Register_t;

/* Driver status */
typedef union {
    uint32_t value;
    struct {
        uint32_t sg_result      : 9;  // bits 0‑8
        uint32_t reserved0      : 3;  // bits 9‑11
        uint32_t s2sa           : 1;  // bit 12
        uint32_t s2sb           : 1;  // bit 13
        uint32_t stealth        : 1;  // bit 14
        uint32_t fsactive       : 1;  // bit 15
        uint32_t cs_actual      : 5;  // bits 16‑20
        uint32_t reserved1      : 3;  // bits 21‑23
        uint32_t stallguard     : 1;  // bit 24
        uint32_t ot             : 1;  // bit 25
        uint32_t otpw           : 1;  // bit 26
        uint32_t s2ga           : 1;  // bit 27
        uint32_t s2gb           : 1;  // bit 28
        uint32_t ola            : 1;  // bit 29
        uint32_t olb            : 1;  // bit 30
        uint32_t stst           : 1;  // bit 31
    };
} DRV_STATUS_Register_t;

/* stealthChop PWM configuration */
typedef union {
    uint32_t value;
    struct {
        uint32_t pwm_ofs        : 8;  // bits 0‑7
        uint32_t pwm_grad       : 8;  // bits 8‑15
        uint32_t pwm_freq       : 2;  // bits 16‑17
        uint32_t pwm_autoscale  : 1;  // bit 18
        uint32_t pwm_autograd   : 1;  // bit 19
        uint32_t freewheel      : 2;  // bits 20‑21
        uint32_t reserved0      : 2;  // bits 22‑23
        uint32_t pwm_reg        : 4;  // bits 24‑27
        uint32_t pwm_lim        : 4;  // bits 28‑31
    };
} PWMCONF_Register_t;

/* PWM scale results */
typedef union {
    uint32_t value;
    struct {
        uint32_t pwm_scale_sum  : 8;  // bits 0‑7
        uint32_t reserved0      : 8;  // bits 8‑15
        uint32_t pwm_scale_auto : 9;  // bits 16‑24
        uint32_t reserved1      : 7;  // bits 25‑31
    };
} PWM_SCALE_Register_t;

/* PWM auto values */
typedef union {
    uint32_t value;
    struct {
        uint32_t pwm_ofs_auto   : 8;  // bits 0‑7
        uint32_t reserved0      : 8;  // bits 8‑15
        uint32_t pwm_grad_auto  : 8;  // bits 16‑23
        uint32_t reserved1      : 8;  // bits 24‑31
    };
} PWM_AUTO_Register_t;

/* Additional field value enumerations */
enum RAMPMODE_Values {
    POSITIONING_MODE  = 0x00,
    VELOCITY_MODE_POS = 0x01,
    VELOCITY_MODE_NEG = 0x02,
    HOLD_MODE         = 0x03
};

enum PWMCONF_freewheel_Values {
    FREEWHEEL_NORMAL   = 0x00,
    FREEWHEEL_ENABLED  = 0x01,
    FREEWHEEL_SHORT_LS = 0x02,
    FREEWHEEL_SHORT_HS = 0x03
};

enum ENCMODE_sensitivity_Values {
    ENCODER_N_NO_EDGE       = 0x00,
    ENCODER_N_RISING_EDGE   = 0x01,
    ENCODER_N_FALLING_EDGE  = 0x02,
    ENCODER_N_BOTH_EDGES    = 0x03
};

#endif // TMC5160_REGISTERS_H