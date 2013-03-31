/* Copyright (C) 2011, 2012, 2013 The uOFW team
   See the file COPYING for copying permission.
*/

#include "clockgen_int.h"

#include <sysmem_sysevent.h>
#include <threadman_kernel.h>

SCE_MODULE_INFO(
    "sceClockgen_Driver",
    SCE_MODULE_KERNEL |
    SCE_MODULE_ATTR_CANT_STOP | SCE_MODULE_ATTR_EXCLUSIVE_LOAD | SCE_MODULE_ATTR_EXCLUSIVE_START,
    1, 9
);
SCE_MODULE_BOOTSTART("_sceClockgenModuleStart");
SCE_MODULE_REBOOT_BEFORE("_sceClockgenModuleRebootBefore");
SCE_SDK_VERSION(SDK_VERSION);

#define PSP_CY27040_I2C_ADDR        (0xD2)
#define PSP_CY27040_CMD_ALL_REGS    (0)
#define PSP_CY27040_CMD_REG(i)      ((i) - 128)
#define PSP_CY27040_REG_REVISION    (0)
#define PSP_CY27040_REG_CLOCK       (1)
#define PSP_CY27040_REG_SS          (2)
#define PSP_CY27040_REG_COUNT       (3)

#define PSP_CLOCK_AUDIO_FREQ    (1)
#define PSP_CLOCK_LEPTON        (8)
#define PSP_CLOCK_AUDIO         (16)

s32 sceI2cMasterTransmitReceive(u32, u8 *, s32, u32, u8 *, s32);
s32 sceI2cMasterTransmit(u32, u8 *, s32);
s32 sceI2cSetClock(s32, s32);

//0x000008F0
typedef struct {
    s32 mutex;
    u32 protocol;
    s8 curReg[PSP_CY27040_REG_COUNT];
    s8 oldReg[PSP_CY27040_REG_COUNT];
    u16 padding;
} ClockgenContext;

ClockgenContext g_Cy27040 = {
    .mutex = -1,
    .protocol = 0,
    .curReg = {0},
    .oldReg = {0}
};

//0x00000900
SceSysEventHandler g_ClockGenSysEv = {
    .size = sizeof(SceSysEventHandler),
    .name = "SceClockgen",
    .typeMask = 0x00FFFF00,
    .handler = _sceClockgenSysEventHandler,
    .gp = 0,
    .busy = 0,
    .next = NULL,
    .reserved = {0}
};

//0x00000000
s32 sceClockgenSetup() //sceClockgen_driver_50F22765
{
    u8 trsm[16];
    u8 recv[16];
    s32 ret;
    s32 i;

    if (g_Cy27040.protocol != 0) {
        for (i = 0; i < PSP_CY27040_REG_COUNT; i++) {
            trsm[0] = PSP_CY27040_CMD_REG(i);

            //sceI2c_driver_47BDEAAA
            ret = sceI2cMasterTransmitReceive(
                PSP_CY27040_I2C_ADDR, trsm, 1,
                PSP_CY27040_I2C_ADDR, recv, 1
            );

            if (ret < 0) {
                return ret;
            }

            g_Cy27040.curReg[i] = recv[0];
            g_Cy27040.oldReg[i] = recv[0];
        }
    }
    else {
        trsm[0] = PSP_CY27040_CMD_ALL_REGS;

        //sceI2c_driver_47BDEAAA
        ret = sceI2cMasterTransmitReceive(
            PSP_CY27040_I2C_ADDR, trsm, 1,
            PSP_CY27040_I2C_ADDR, recv, 16
        );

        if (ret < 0) {
            return ret;
        }

        for (i = 0; (i < PSP_CY27040_REG_COUNT) && (i < recv[0]); i++) {
            g_Cy27040.curReg[i] = recv[i + 1];
            g_Cy27040.oldReg[i] = recv[i + 1];
        }
    }

    return SCE_ERROR_OK;
}

//0x000000F8
s32 sceClockgenSetSpectrumSpreading(s32 arg) //sceClockgen_driver_C9AF3102
{
    s32 regSS;
    s32 ret;
    s32 res;

    if (arg < 0) {
        regSS = g_Cy27040.oldReg[PSP_CY27040_REG_SS] & 0x7;
        ret = arg;

        if (regSS == (g_Cy27040.curReg[PSP_CY27040_REG_SS] & 0x7)) {
            return ret;
        }
    }
    else {
        switch(g_Cy27040.curReg[PSP_CY27040_REG_REVISION] & 0xF) {
        case 0x8:
        case 0xF:
        case 0x7:
        case 0x3:
        case 0x9:
        case 0xA:
            if (arg < 2) {
                regSS = 0;
                ret = 0;
            }
            else if (arg < 7) {
                regSS = 1;
                ret = 5;
            }
            else if (arg < 15) {
                regSS = 2;
                ret = 10;
            }
            else if (arg < 25) {
                regSS = 4;
                ret = 20;
            }
            else {
                regSS = 6;
                ret = 30;
            }
            break;
        case 0x4:
            if (arg < 2) {
                regSS = 0;
                ret = 0;
            }
            else if (arg < 7) {
                regSS = 1;
                ret = 5;
            }
            else if (arg < 12) {
                regSS = 2;
                ret = 10;
            }
            else if (arg < 17) {
                regSS = 3;
                ret = 15;
            }
            else if (arg < 22) {
                regSS = 4;
                ret = 20;
            }
            else if (arg < 27) {
                regSS = 5;
                ret = 25;
            }
            else {
                regSS = 6;
                ret = 30;
            }
            break;
        default:
            return SCE_ERROR_NOT_SUPPORTED;
        }
    }

    res = sceKernelLockMutex(g_Cy27040.mutex, 1, 0);
    if (res < 0) {
        return res;
    }

    g_Cy27040.curReg[PSP_CY27040_REG_SS] = (regSS & 7) | (g_Cy27040.curReg[PSP_CY27040_REG_SS] & ~7);
    res = _cy27040_write_register(PSP_CY27040_REG_SS, g_Cy27040.curReg[PSP_CY27040_REG_SS]);

    sceKernelUnlockMutex(g_Cy27040.mutex, 1);
    if (res < 0) {
        return res;
    }

    return ret;
}

//0x000002B4
s32 _sceClockgenModuleStart(SceSize args __attribute__((unused)), void *argp __attribute__((unused)))
{
    s32 mutexId;

    //sceI2c_driver_62C7E1E4
    sceI2cSetClock(4, 4);

    //ThreadManForKernel_B7D098C6
    mutexId = sceKernelCreateMutex("SceClockgen", 1, 0, 0);

    if (mutexId >= 0) {
        g_Cy27040.mutex = mutexId;

        //sceSysEventForKernel_CD9E4BB5
        sceKernelRegisterSysEventHandler(&g_ClockGenSysEv);
    }

    g_Cy27040.protocol = 1;
    sceClockgenSetup();

    return SCE_ERROR_OK;
}

//0x00000320
s32 _sceClockgenModuleRebootBefore(SceSize args __attribute__((unused)), void *argp __attribute__((unused)))
{
    s32 oldRegSS;
    s32 curRegSS;

    oldRegSS = g_Cy27040.oldReg[PSP_CY27040_REG_SS];
    curRegSS = g_Cy27040.curReg[PSP_CY27040_REG_SS];

    if ((curRegSS & 7) != (oldRegSS & 7)) {
        _cy27040_write_register(PSP_CY27040_REG_SS, (curRegSS & ~7) | (oldRegSS & 7));
    }

    if (g_Cy27040.mutex >= 0) {
        //ThreadManForKernel_F8170FBE
        sceKernelDeleteMutex(g_Cy27040.mutex);

        g_Cy27040.mutex = -1;
    }

    //sceSysEventForKernel_D7D3FDCD
    sceKernelUnregisterSysEventHandler(&g_ClockGenSysEv);

    return SCE_ERROR_OK;
}

//0x00000398
s32 sceClockgenInit() //sceClockgen_driver_29160F5D
{
    s32 mutexId;

    sceI2cSetClock(4, 4);
    mutexId = sceKernelCreateMutex("SceClockgen", 1, 0, 0);

    if (mutexId < 0) {
        return mutexId;
    }

    g_Cy27040.mutex = mutexId;

    sceKernelRegisterSysEventHandler(&g_ClockGenSysEv);

    return SCE_ERROR_OK;
}

//0x000003EC
s32 sceClockgenEnd() //sceClockgen_driver_36F9B49D
{
    if (g_Cy27040.mutex >= 0) {
        sceKernelDeleteMutex(g_Cy27040.mutex);

        g_Cy27040.mutex = -1;
    }

    sceKernelUnregisterSysEventHandler(&g_ClockGenSysEv);

    return SCE_ERROR_OK;
}

//0x00000438
s32 sceClockgenSetProtocol(u32 prot) //sceClockgen_driver_3F6B7C6B
{
    g_Cy27040.protocol = prot;

    return SCE_ERROR_OK;
}

//0x00000448
s32 sceClockgenGetRevision() //sceClockgen_driver_CE36529C
{
    return g_Cy27040.curReg[PSP_CY27040_REG_REVISION];
}

//0x00000454
s32 sceClockgenGetRegValue(u32 idx) //sceClockgen_driver_0FD28D8B
{
    if (idx >= PSP_CY27040_REG_COUNT) {
        return SCE_ERROR_INVALID_INDEX;
    }

    return g_Cy27040.curReg[idx];
}

//0x0000047C
s32 sceClockgenAudioClkSetFreq(u32 freq) //sceClockgen_driver_DAB6E612
{
    if (freq == 44100) {
        return _sceClockgenSetControl1(PSP_CLOCK_AUDIO_FREQ, 0);
    }
    else if (freq == 48000) {
        return _sceClockgenSetControl1(PSP_CLOCK_AUDIO_FREQ, 1);
    }

    return SCE_ERROR_INVALID_VALUE;
}

//0x000004BC
s32 sceClockgenAudioClkEnable() //sceClockgen_driver_A1D23B2C
{
    return _sceClockgenSetControl1(PSP_CLOCK_AUDIO, 1);
}

//0x000004DC
s32 sceClockgenAudioClkDisable() //sceClockgen_driver_DED4C698
{
    return _sceClockgenSetControl1(PSP_CLOCK_AUDIO, 0);
}

//0x000004FC
s32 sceClockgenLeptonClkEnable()  //sceClockgen_driver_7FF82F6F
{
    return _sceClockgenSetControl1(PSP_CLOCK_LEPTON, 1);
}

//0x0000051C
s32 sceClockgenLeptonClkDisable() //sceClockgen_driver_DBE5F283
{
    return _sceClockgenSetControl1(PSP_CLOCK_LEPTON, 0);
}

//0x0000053C
s32 _sceClockgenSysEventHandler(
    s32 ev_id,
    char *ev_name __attribute__((unused)),
    void *param __attribute__((unused)),
    s32 *result __attribute__((unused)))
{
    if (ev_id == 0x10000) {
        //ThreadManForKernel_0DDCD2C9
        sceKernelTryLockMutex(g_Cy27040.mutex, 1);

        return SCE_ERROR_OK;
    }

    if (ev_id < 0x10000) {
        return SCE_ERROR_OK;
    }

    if (ev_id == 0x100000) {
        g_Cy27040.curReg[PSP_CY27040_REG_CLOCK] &= ~PSP_CLOCK_LEPTON;

        _cy27040_write_register(PSP_CY27040_REG_CLOCK, g_Cy27040.curReg[PSP_CY27040_REG_CLOCK] & 0xF7);
        _cy27040_write_register(PSP_CY27040_REG_SS, g_Cy27040.curReg[PSP_CY27040_REG_SS]);

        sceKernelUnlockMutex(g_Cy27040.mutex, 1);
    }

    return SCE_ERROR_OK;
}

//0x000005DC
s32 _sceClockgenSetControl1(s32 bus, s32 mode)
{
    s32 ret;
    s32 regClk;

    //ThreadManForKernel_B011B11F
    ret = sceKernelLockMutex(g_Cy27040.mutex, 1, 0);

    if (ret < 0) {
        return ret;
    }

    regClk = g_Cy27040.curReg[PSP_CY27040_REG_CLOCK];
    ret = (regClk & bus) > 0;

    if (!mode) {
        regClk &= ~bus;
    }
    else {
        regClk |= bus;
        regClk &= 0xFF;
    }

    if (regClk != g_Cy27040.curReg[PSP_CY27040_REG_CLOCK]) {
        g_Cy27040.curReg[PSP_CY27040_REG_CLOCK] = regClk;

        _cy27040_write_register(PSP_CY27040_REG_CLOCK, regClk);
    }

    //ThreadManForKernel_6B30100F
    sceKernelUnlockMutex(g_Cy27040.mutex, 1);

    return ret;
}

//0x00000680
s32 _cy27040_write_register(u8 idx, u8 val)
{
    u8 trsm[16];
    s32 ret;

    trsm[0] = PSP_CY27040_CMD_REG(idx);
    trsm[1] = val;

    if (idx >= PSP_CY27040_REG_COUNT) {
        return SCE_ERROR_INVALID_INDEX;
    }

    //sceI2c_driver_8CBD8CCF
    ret = sceI2cMasterTransmit(
        PSP_CY27040_I2C_ADDR, trsm, 2
    );

    if (ret < 0) {
        return ret;
    }

    return SCE_ERROR_OK;
}
