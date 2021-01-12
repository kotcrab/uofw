/* Copyright (C) The uOFW team
   See the file COPYING for copying permission.
*/

#ifndef POWER_KERNEL_H
#define	POWER_KERNEL_H

#include "common_header.h"
#include "power_error.h"

/** @defgroup Kernel Kernel
 *  @ingroup PowerLibrary
 *
 *  The Power service Kernel API.
 *  @{
 */

 /* Callbacks */

/** 
 * This constant specifies that a search for a slot number from among the empty slots is to be automatically performed
 * when a callback is registered with the ::scePowerRegisterCallback() function.
 */
#define SCE_POWER_CALLBACKSLOT_AUTO					(-1)

/**
 * Defines constants passed to the ::ScePowerCallback function specifying the reason for the callback.
 */
typedef enum {
	/** These bits represent the remaining battery capacity in [%].*/
	SCE_POWER_CALLBACKARG_BATTERY_CAP	= 0x0000007F,
	/** Indicates a battery has been equipped.  */
	SCE_POWER_CALLBACKARG_BATTERYEXIST	= 0x00000080,
	/** Indicates the battery is in a low battery state.  */
	SCE_POWER_CALLBACKARG_LOW_BATTERY	= 0x00000100,
	/** Indicates power is being supplied from an external power source (AC adapter). */
	SCE_POWER_CALLBACKARG_POWER_ONLINE	= 0x00001000,
	/** 
	 * Indicates the PSP's suspend process has begun. This happens for example when 
	 *	- the user quickly presses the HOLD switch 
	 *	- automatic sleep happens to save power
	 *	- forced suspension starts due to battery running out
	 */
	SCE_POWER_CALLBACKARG_SUSPENDING	= 0x00010000,
	/** Indicates the PSP's resume process has started. */
	SCE_POWER_CALLBACKARG_RESUMING		= 0x00020000,
	/** Indicates the PSP's resume process has completed. */
	SCE_POWER_CALLBACKARG_RESUME_COMP	= 0x00040000,
	/** 
	 * Indicates that standby operation has been started by pressing 
	 * and holding the POWER switch. 
	 */
	SCE_POWER_CALLBACKARG_STANDINGBY	= 0x00080000,
	SCE_POWER_CALLBACKARG_UNK_10000000	= 0x10000000,
	SCE_POWER_CALLBACKARG_UNK_20000000	= 0x20000000,
	/** Indicates whether or not the HOLD switch has been locked. */
	SCE_POWER_CALLBACKARG_HOLD_SWITCH	= 0x40000000,
	/** Indicates whether or not the POWER switch has been activated. */
	SCE_POWER_CALLBACKARG_POWER_SWITCH	= 0x80000000
} ScePowerCallbackArg;

/** 
 * Power service callback function prototype 
 * 
 * @param count Number of times the callback has been called.
 * @param arg The callback argument.
 * @param common Custom callback value. Specified by the same parameter in ::sceKernelCreateCallback().
 * 
 * @return Always SCE_ERROR_OK.
 */
typedef void (*ScePowerCallback)(s32 count, s32 arg, void *common);

/**
 * Registers a power service notification callback.
 * 
 * @param slot The callback registration slot to use. Specify value between 0 - 31.
 * @param cbid The callback UID obtained by ::sceKernelCreateCallback().
 * 
 * @return 0 - 31 on success when specific slot was used.
 * @return A value < 0 on failure.
 */
s32 scePowerRegisterCallback(s32 slot, SceUID cbid);

/**
 * Cancels a registered power service notification callback.
 * 
 * @param slot The slot of the callback to unregister. Specify a value between 0 - 31.
 * 
 * @return SCE_ERROR_OK on success, otherwise < 0.
 */
s32 scePowerUnregisterCallback(s32 slot);

s32 scePowerSetCallbackMode(s32 slot, s32 mode);

s32 scePowerGetCallbackMode(s32 slot, s32 *pMode);

/* Clock frequency functions */

/**
 * Gets the CPU clock frequency.
 * 
 * @return The CPU clock frequency in MHz.
 */
s32 scePowerGetCpuClockFrequencyInt(void);

/**
 * Gets the CPU clock frequency.
 * 
 * @remark The accuracy is identical to ::scePowerGetCpuClockFrequencyInt().
 * 
 * @return The CPU clock frequency in MHz.
 */
float scePowerGetCpuClockFrequencyFloat(void);

/**
 * Gets the bus clock frequency.
 * 
 * @remark The bus clock frequency always operates at 1/2 the PLL clock frequency.
 * 
 * @return The current bus clock frequency in MHz.
 */
s32 scePowerGetBusClockFrequencyInt(void);

/**
 * Gets the bus clock frequency.
 *
 * @remark The bus clock frequency always operates at 1/2 the PLL clock frequency.
 * @remark The accuracy is identical to ::scePowerGetBusClockFrequencyInt().
 *
 * @return The current bus clock frequency in MHz.
 */
float scePowerGetBusClockFrequencyFloat(void);

/**
 * Gets the PLL output clock frequency.
 * 
 * @return The current PLL output clock frequency in MHz.
 */
s32 scePowerGetPllClockFrequencyInt(void);

/**
 * Gets the PLL output clock frequency.
 * 
 * @remark The accuracy is identical to ::scePowerGetPllClockFrequencyInt().
 *
 * @return The current PLL output clock frequency in MHz.
 */
float scePowerGetPllClockFrequencyFloat(void);

/**
 * Sets the CPU clock frequency.
 *
 * @param cpuFrequency The CPU clock frequency in MHz. Can bet to a value in range 1 to 333.
 * Note that it cannot be set to a value higher than the current PLL clock frequency.
 *
 * @return SCE_ERROR_OK on success, otherwise < 0.
 *
 * @remark Even in success case, the CPU might not actually operate the specified clock frequency.
 * To obtain the actual frequency in use, call either ::scePowerGetBusClockFrequencyInt() or
 * ::scePowerGetBusClockFrequencyFloat().
 */
s32 scePowerSetCpuClockFrequency(s32 cpuFrequency);

/**
 * Sets the bus clock frequency.
 * 
 * @param busFrequency The bus clock frequency in MHz.
 * 
 * @return SCE_ERROR_OK on success, otherwise < 0.
 * 
 * @remark This function has been removed from SDK 3.70 and later. If still called,
 * it will now always try to set the bus clock frequency to 1/2 PLL clock frequency.
 */
s32 scePowerSetBusClockFrequency(s32 busFrequency);

/**
 * Sets clock frequencies. 
 * 
 * @remark This is an alias for ::scePowerClockFrequency() on firmware 3.50.
 * 
 * @see ::scePowerClockFrequency() for more details.
 * 
 */
s32 scePowerSetClockFrequency350(s32 pllFrequency, s32 cpuFrequency, s32 busFrequency);

/**
 * Sets clock frequencies.
 * 
 * @param pllFrequency The PLL clock frequency in MHz. Specify either 190, 222, 266 or 333.
 * @param cpuFrequency The CPU clock frequency in MHz. Specify a value between 1 - 333. Valid values have
 * to be less than or equal to the value specified for @p pllFrequency.
 * @param busFrequency The bus clock frequency in MHz. Must be exactly 1/2 of @p pllFrequency. In case of 
 * 333 MHZ specified for @p pllFrequency, specify 166.
 * 
 * @return SCE_ERROR_OK on success, otherwise < 0.
 */
s32 scePowerSetClockFrequency(s32 pllFrequency, s32 cpuFrequency, s32 busFrequency);

#define SCE_POWER_PLL_CLOCK_ROUND_UP_37MHz		(1 << 0)
#define SCE_POWER_PLL_CLOCK_ROUND_UP_148MHz		(1 << 1)
#define SCE_POWER_PLL_CLOCK_ROUND_UP_190MHz		(1 << 2)
#define SCE_POWER_PLL_CLOCK_ROUND_UP_222MHz		(1 << 3)
#define SCE_POWER_PLL_CLOCK_ROUND_UP_266MHz		(1 << 4)
#define SCE_POWER_PLL_CLOCK_ROUND_UP_333MHz		(1 << 5)
#define SCE_POWER_PLL_CLOCK_ROUND_UP_19MHz		(1 << 8)
#define SCE_POWER_PLL_CLOCK_ROUND_UP_74MHz		(1 << 9)
#define SCE_POWER_PLL_CLOCK_ROUND_UP_96MHz		(1 << 10)
#define SCE_POWER_PLL_CLOCK_ROUND_UP_111MHz		(1 << 11)
#define SCE_POWER_PLL_CLOCK_ROUND_UP_133MHz		(1 << 12)
#define SCE_POWER_PLL_CLOCK_ROUND_UP_166MHz		(1 << 13)

/**
 * Sets a use mask for the PLL clock. Allows to set fine-grained 'round up' steps for PLL clock frequencies
 * set via ::scePowerSetClockFrequency(). The actual PLL clock frequency will be round up to the smallest frequency
 * which is greater than or equal to the specified PLL clock frequency.
 * 
 * @param useMask The mask containing the different round up steps.
 * 
 * @par Example: Given the following PLL use mask and custom PLL frequency of 166 MHz
 * @code
 * // Pressing the select will reset the idle timer. No other button will reset it.
 * scePowerSetPllUseMask(SCE_POWER_PLL_CLOCK_ROUND_UP_148MHz | SCE_POWER_PLL_CLOCK_ROUND_UP_190MHz | SCE_POWER_PLL_CLOCK_ROUND_UP_333MHz);
 * scePowerSetClockFrequency(166, 166, 83);
 * @endcode
 * the specified PLL frequency will be round up to 190MHz by the power service.
 * 
 * @return Always SCE_ERROR_OK.
 */
s32 scePowerSetPllUseMask(s32 useMask);

/* Hardware component power settings */

/**
 * Gets the current Tachyon voltage.
 * 
 * @return The current Tachyon voltage set by the power service.
 */
s16 scePowerGetCurrentTachyonVoltage(void);

/**
 * Gets the possible Tachyon volatge values used by the power service.
 * 
 * @param pMaxVoltage Pointer to the s16 variable which is to receive the maximum Tachyon voltage.
 * @param pDefaultVoltage Pointer to the s16 variable which is to receive the default Tachyon voltage.
 * 
 * @return Always SCE_ERROR_OK.
 */
s32 scePowerGetTachyonVoltage(s16* pMaxVoltage, s16* pDefaultVoltage);

/**
 * Sets the possible Tachyon voltages used by the power service.
 * 
 * @param maxVoltage The voltage used for the Tachyon SoC IC when the PSP's PLL operates at clock frequencies 
 * 266 Mhz or 333 MHz.
 * @param defaultVoltage The voltage used for the Tachyon SoC IC when the PSP's PLL operates at clock 
 * frequencies 222 MHz or below.
 * 
 * @return Always SCE_ERROR_OK.
 */
s32 scePowerSetTachyonVoltage(s16 maxVoltage, s16 defaultVoltage);

/**
 * Gets the current DDR memory voltage. 
 * 
 * @return The current DDR memory voltage set by the power service.
 */
s16 scePowerGetCurrentDdrVoltage(void);

/**
 * Gets the possible Tachyon voltage values used by the power service.
 * 
 * @param pMaxVoltage Pointer to the s16 variable which is to receive the maximum DDR memory voltage.
 * @param pDefaultVoltage Pointer to the s16 variable which is to receive the default DDR memory voltage.
 * 
 * @return Always SCE_ERROR_OK.
 */
s32 scePowerGetDdrVoltage(s16* pMaxVoltage, s32* pDefaultVoltage);

/**
 * Sets the possible DDR memory voltages used by the power service.
 * 
 * @param maxVoltage The voltage used for the DDR memory component when the PSP's PLL operates at clock frequency 
 * 333 MHz.
 * @param defaultVoltage The voltage used for the DDR memory component when the PSP's PLL operates at clock 
 * frequencies 266 MHz or below.
 * 
 * @return Always SCE_ERROR_OK.
 */
s32 scePowerSetDdrVoltage(s16 maxVoltage, s32 defaultVoltage);

/**
 * Gets the current DDR memory strength. 
 * 
 * @return The current DDR memory strength set by the power service.
 */
s16 scePowerGetCurrentDdrStrength(void);

/**
 * Gets the possible DDR memory strength values used by the power service.
 * 
 * @param pMaxStrength Pointer to the s16 variable which is to receive the maximum DDR memory strength value.
 * @param pDefaultStrength Pointer to the s16 variable which is to receive the default DDR memory voltage.
 * 
 * @return Always SCE_ERROR_OK.
 */
s32 scePowerGetDdrStrength(s16* pMaxStrength, s16* pDefaultStrength);

/**
 * Sets the possible DDR memory strength values used by the power service.
 * 
 * @param maxStrength The strength value used for the DDR memory component when the PSP's PLL operates at 
 * clock frequency 333 MHz.
 * @param defaultStrength The strength value used for the DDR memory component when the PSP's PLL operates at 
 * clock frequencies 266 MHz and below.
 * 
 * @return Always SCE_ERROR_OK.
 */
s32 scePowerSetDdrStrength(s16 maxStrength, s16 defaultStrength);

/* POWER switch manipulation lock / unlock */

/**
 * @brief Locks POWER switch manipulation (sets a power off lock).
 * 
 * This function delays a power off request until the lock is canceled. It is used in a critical section 
 * to protect timing when a power interruption would cause a problem. For example, it might be used to prevent 
 * a power interruption while data is being written to the Memory Stick.
 * 
 * @param lockType The power processing lock type. Specify ::SCE_KERNEL_POWER_LOCK_DEFAULT.
 * 
 * @return SCE_ERROR_OK on success, otherwise < 0.
 * 
 * @remark Since the user will not be able to turn the power off by operating the POWER/HOLD switch 
 * during a power locked state, if the ::scePowerLockForKernel() function is used carelessly, it can 
 * significantly decrease the usability of the PSP�.
 */
s32 scePowerLockForKernel(s32 lockType);

/**
 * @brief Cancels a power off lock.
 * 
 * This function cancels a power lock that was previously set with the ::scePowerLockForKernel() API.
 * 
 * @param lockType Power processing lock type. Specify ::SCE_KERNEL_POWER_LOCK_DEFAULT.
 * 
 * @return The remaining existing power locks (>= 0).
 */
s32 scePowerUnlockForKernel(s32 lockType);


/* WLAN functions */

/**
 * Attempts to activate WLAN.
 * 
 * @remark Battery charging might be suppressed while WLAN is active.
 * 
 * @return SCE_ERROR_OK on success, otherwise < 0.
 */
s32 scePowerWlanActivate(void);

/**
 * Deactivates WLAN. 
 * 
 * @remark If battery charging was disabled previously due to an active WLAN, charging will be enabled again.
 * 
 * @return Always SCE_ERROR_OK.
 */
s32 scePowerWlanDeactivate(void);

/** Specifies a device type which permits a maximum PLL clock frequency of 222 MHz when WLAN is active. */
#define SCE_POWER_WLAN_COEXISTENCE_CLOCK_222MHz		0
/** Specifies a device type which supports PLL clock frequencies of 266 MHz and 333 MHz when WLAN is active. */
#define SCE_POWER_WLAN_COEXISTENCE_CLOCK_333MHz		1

/**
 * Gets the maximum clock frequency allowed when WLAN is active.
 * 
 * @remark For the PSP-100X, the PLL clock frequency cannot be set to 266 MHz or 333 MHz while WLAN is actice. 
 * However, for the PSP-2000 and later devices, the PLL clock frequency can be set to 266 MHz or 333 MHz while 
 * WLAN is active.
 * 
 * @return ::SCE_POWER_WLAN_COEXISTENCE_CLOCK_222MHz or ::SCE_POWER_WLAN_COEXISTENCE_CLOCK_333MHz on success, 
 * otherwise < 0.
 */
s32 scePowerCheckWlanCoexistenceClock(void);

/* Specifies that WLAN can be operated at the maximum PLL clock frequency. */
#define SCE_POWER_WLAN_EXCLUSIVE_CLOCK_LIMIT_NONE		0
/** Specifies that WLAN can only be operated at a PLL clock frequency up to 222 MHz. */
#define SCE_POWER_WLAN_EXCLUSIVE_CLOCK_LIMIT_222Mhz		1
/** Specifies that WLAN can only be operated at a PLL clock frequency up to 266 MHz. */
#define SCE_POWER_WLAN_EXCLUSIVE_CLOCK_LIMIT_266Mhz		2

/**
 * Sets the clock frequency limit at which WLAN can be operated. If WLAN is active this limits the allowed
 * clock frequencies. If WLAN is OFF, this limit determines whether the PSP currently runs at a clock frequency 
 * slow enough so that WLAN can be successfully activated.
 * 
 * @param clockLimit Specifies the clock frequency limit.
 *
 * @return Always SCE_ERROR_OK.
 */
s32 scePowerSetExclusiveWlan(u8 clockLimit);

s32 scePowerCheckWlanCondition(u32 freq);

u8 scePowerGetWlanActivity(void);

/**
 * Gets the Grahpic Engine's current eDRAM refresh mode.
 * 
 * @return The refresh mode.
 */
s32 scePowerGetGeEdramRefreshMode(void);

/**
 * Sets the Graphic Engine's eDRAM refresh mode.
 * 
 * @param geEdramRefreshMode The refresh mode. Pass either 0 or 1.
 * 
 * @return SCE_ERROR_OK on success, otherwise < 0.
 */
s32 scePowerSetGeEdramRefreshMode(s32 geEdramRefreshMode);


 /** @} */

#endif	/* POWER_KERNEL_H */