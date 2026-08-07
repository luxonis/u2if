#ifndef _INTERFACE_FSYNCCONTROLLER_H
#define _INTERFACE_FSYNCCONTROLLER_H

#define FSYNC_CONTROLLER_ADDRESS 0x12
#define FSYNC_CONTROLLER_BOOT_ADDRESS 0x56

#include "PicoInterfacesBoard.h"
#include "BaseInterface.h"
#include "pico/sync.h"
#include "hardware/i2c.h"

struct FsyncI2cResult {
    int status;
    uint32_t data;
};

class FsyncController : public BaseInterface {
public:
    FsyncController();
    virtual ~FsyncController();

    CmdStatus process(uint8_t const *cmd, uint8_t response[64]);
    CmdStatus task(uint8_t response[64]);

    CmdStatus Init(uint8_t const *cmd, uint8_t response[64]);
    CmdStatus probe(uint8_t const *cmd, uint8_t response[64]);
    CmdStatus getPinCapabilities(uint8_t const *cmd, uint8_t response[64]);
    CmdStatus setPinCapabilities(uint8_t const *cmd, uint8_t response[64]);

    CmdStatus getMode(uint8_t const *cmd, uint8_t response[64]);
    CmdStatus setMode(uint8_t const *cmd, uint8_t response[64]);
    CmdStatus getFps(uint8_t const *cmd, uint8_t response[64]);
    CmdStatus setFps(uint8_t const *cmd, uint8_t response[64]);
    CmdStatus getDuty(uint8_t const *cmd, uint8_t response[64]);
    CmdStatus setDuty(uint8_t const *cmd, uint8_t response[64]);
    CmdStatus getPolarity(uint8_t const *cmd, uint8_t response[64]);
    CmdStatus setPolarity(uint8_t const *cmd, uint8_t response[64]);

private:
    int writeReg(i2c_inst_t *i2c, uint8_t addr, uint8_t reg, uint32_t data);
    FsyncI2cResult readReg(i2c_inst_t *i2c, uint8_t addr, uint8_t reg);
};

#endif
