#include "FsyncController.h"

#include "hardware/i2c.h"
#include "pico/stdlib.h"

FsyncController::FsyncController()
{

}

FsyncController::~FsyncController()
{

}

CmdStatus FsyncController::process(uint8_t const *cmd, uint8_t response[64])
{
    CmdStatus status = CmdStatus::NOT_CONCERNED;

    if(cmd[0] == Report::ID::FSYNC_INIT) {
        status = Init(cmd, response);
    } else if(cmd[0] == Report::ID::FSYNC_PROBE) {
        status = probe(cmd, response);
    } else if(cmd[0] == Report::ID::FSYNC_GETPINCAPABILITIES) {
        status = getPinCapabilities(cmd, response);
    } else if(cmd[0] == Report::ID::FSYNC_SETPINCAPABILITIES) {
        status = setPinCapabilities(cmd, response);
    } else if(cmd[0] == Report::ID::FSYNC_GETMODE) {
        status = getMode(cmd, response);
    } else if(cmd[0] == Report::ID::FSYNC_SETMODE) {
        status = setMode(cmd, response);
    } else if(cmd[0] == Report::ID::FSYNC_GETFPS) {
        status = getFps(cmd, response);
    } else if(cmd[0] == Report::ID::FSYNC_SETFPS) {
        status = setFps(cmd, response);
    } else if(cmd[0] == Report::ID::FSYNC_SETDUTY) {
        status = getDuty(cmd, response);
    } else if(cmd[0] == Report::ID::FSYNC_GETDUTY) {
        status = setDuty(cmd, response);
    } else if(cmd[0] == Report::ID::FSYNC_SETPOLARITY) {
        status = getPolarity(cmd, response);
    } else if(cmd[0] == Report::ID::FSYNC_GETPOLARITY) {
        status = setPolarity(cmd, response);
    }

    return status;
}

CmdStatus FsyncController::task(uint8_t response[64])
{
    (void)response;

    CmdStatus status = CmdStatus::NOT_CONCERNED;
    return status;
}

CmdStatus FsyncController::Init(uint8_t const *cmd, uint8_t response[64])
{
    return CmdStatus::OK;
}

/*
 * response[2] = fsync i2c bus id
 * response[3] = fsync i2c address
 * response[4:8] = fsync fw version (0xffffffff means controller is in bootloader mode (no firmware))
 */
CmdStatus FsyncController::probe(uint8_t const *cmd, uint8_t response[64])
{
    uint8_t dummy = 0;

    FsyncI2cResult r0 = readReg(i2c0, FSYNC_CONTROLLER_ADDRESS, 0x00);
    response[2] = 0;

    *(int32_t *)(response + 4) = -1;

    if (r0.status >= 0) {
        response[3] = FSYNC_CONTROLLER_ADDRESS;
        *(int32_t *)(response + 4) = r0.data; 
        return CmdStatus::OK;
    }

    int b0 = i2c_read_timeout_us(
        i2c0,
        FSYNC_CONTROLLER_BOOT_ADDRESS,
        &dummy,
        1,
        false,
        1000
    );

    if (b0 != PICO_ERROR_GENERIC && b0 != PICO_ERROR_TIMEOUT) {
        response[3] = FSYNC_CONTROLLER_BOOT_ADDRESS;
        return CmdStatus::OK;
    }

    FsyncI2cResult r1 = readReg(i2c1, FSYNC_CONTROLLER_ADDRESS, 0x00);
    response[2] = 1;

    if (r1.status >= 0) {
        response[3] = FSYNC_CONTROLLER_ADDRESS;
        *(int32_t *)(response + 4) = r1.data; 
        return CmdStatus::OK;
    }

    int b1 = i2c_read_timeout_us(
        i2c1,
        FSYNC_CONTROLLER_BOOT_ADDRESS,
        &dummy,
        1,
        false,
        1000
    );

    if (b1 != PICO_ERROR_GENERIC && b1 != PICO_ERROR_TIMEOUT) {
        response[3] = FSYNC_CONTROLLER_BOOT_ADDRESS;
        return CmdStatus::OK;
    }

    response[2] = -1;
    response[3] = -1;

    return CmdStatus::OK;
}

CmdStatus FsyncController::getPinCapabilities(uint8_t const *cmd, uint8_t response[64])
{
    return CmdStatus::OK;
}

CmdStatus FsyncController::setPinCapabilities(uint8_t const *cmd, uint8_t response[64])
{
    return CmdStatus::OK;
}

CmdStatus FsyncController::getMode(uint8_t const *cmd, uint8_t response[64])
{
    return CmdStatus::OK;
}

CmdStatus FsyncController::setMode(uint8_t const *cmd, uint8_t response[64])
{
    return CmdStatus::OK;
}

CmdStatus FsyncController::getFps(uint8_t const *cmd, uint8_t response[64])
{
    return CmdStatus::OK;
}

CmdStatus FsyncController::setFps(uint8_t const *cmd, uint8_t response[64])
{
    return CmdStatus::OK;
}

CmdStatus FsyncController::getDuty(uint8_t const *cmd, uint8_t response[64])
{
    return CmdStatus::OK;
}

CmdStatus FsyncController::setDuty(uint8_t const *cmd, uint8_t response[64])
{
    return CmdStatus::OK;
}

CmdStatus FsyncController::getPolarity(uint8_t const *cmd, uint8_t response[64])
{
    return CmdStatus::OK;
}

CmdStatus FsyncController::setPolarity(uint8_t const *cmd, uint8_t response[64])
{
    return CmdStatus::OK;
}

int FsyncController::writeReg(i2c_inst_t *i2c, uint8_t addr, uint8_t reg, uint32_t data)
{
    int rc = 0;

    rc |= i2c_write_timeout_us(
        i2c,
        addr,
        &reg,
        sizeof(reg),
        true,
        1000
    );

    rc |= i2c_write_timeout_us(
        i2c,
        addr,
        (uint8_t *)&data,
        sizeof(data),
        false,
        1000
    );

    return rc;
}

FsyncI2cResult FsyncController::readReg(i2c_inst_t *i2c, uint8_t addr, uint8_t reg)
{
    FsyncI2cResult r = {0, 0};

    r.status |= i2c_write_timeout_us(
        i2c,
        addr,
        &reg,
        sizeof(r.status),
        true,
        1000
    );

    r.status |= i2c_read_timeout_us(
        i2c,
        addr,
        (uint8_t *)&r.data,
        sizeof(r.data),
        false,
        1000
    );

    return r;
}
