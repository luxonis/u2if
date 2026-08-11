#include "FsyncController.h"

#include "hardware/i2c.h"
#include "pico/stdlib.h"

#include <limits>
#include <cstring>
#include <math.h>

FsyncController::FsyncController()
{

}

FsyncController::~FsyncController()
{

}

CmdStatus FsyncController::process(uint8_t const *cmd, uint8_t response[64])
{
    CmdStatus status = CmdStatus::NOT_CONCERNED;

    if (Report::ID::FSYNC_GETPINCAPABILITIES <= cmd[0] && cmd[0] <= Report::ID::FSYNC_GETINPUTINFO && locked)
        unlockFsyncController(); 

    if (Report::ID::FSYNC_GETMODE <= cmd[0] && cmd[0] <= Report::ID::FSYNC_GETINPUTINFO && !initok)
        return CmdStatus::NOK;

    if (cmd[0] == Report::ID::FSYNC_INIT) {
        status = init(cmd, response);
    } else if (cmd[0] == Report::ID::FSYNC_PROBE) {
        status = probe(cmd, response);
    } else if (cmd[0] == Report::ID::FSYNC_GETPINCAPABILITIES) {
        status = getPinCapabilities(cmd, response);
    } else if (cmd[0] == Report::ID::FSYNC_SETPINCAPABILITIES) {
        status = setPinCapabilities(cmd, response);
    } else if (cmd[0] == Report::ID::FSYNC_GETMODE) {
        status = getMode(cmd, response);
    } else if (cmd[0] == Report::ID::FSYNC_SETMODE) {
        status = setMode(cmd, response);
    } else if (cmd[0] == Report::ID::FSYNC_GETFPS) {
        status = getFps(cmd, response);
    } else if (cmd[0] == Report::ID::FSYNC_SETFPS) {
        status = setFps(cmd, response);
    } else if (cmd[0] == Report::ID::FSYNC_GETDUTY) {
        status = getDuty(cmd, response);
    } else if (cmd[0] == Report::ID::FSYNC_SETDUTY) {
        status = setDuty(cmd, response);
    } else if (cmd[0] == Report::ID::FSYNC_GETPOLARITY) {
        status = getPolarity(cmd, response);
    } else if (cmd[0] == Report::ID::FSYNC_SETPOLARITY) {
        status = setPolarity(cmd, response);
    } else if (cmd[0] == Report::ID::FSYNC_GETINPUTINFO) {
        status = getInputInfo(cmd, response);
    }

    return status;
}

CmdStatus FsyncController::task(uint8_t response[64])
{
    (void)response;

    CmdStatus status = CmdStatus::NOT_CONCERNED;
    return status;
}

CmdStatus FsyncController::init(uint8_t const *cmd, uint8_t response[64])
{
    unlockFsyncController();

    FsyncI2cResult r = readReg(fsync_i2c, FSYNC_ADDRESS, PIN_CONFIG_VALID_REG);

    if (r.status < 0 || r.data != 0)
        return CmdStatus::NOK;

    initok = true;

    return CmdStatus::OK;
}

/*
 * response[2] = fsync i2c bus id (0xff - no stm found)
 * response[3] = fsync i2c address (0xff - no stm found)
 * response[4:8] = fsync fw version (0xffffffff means controller is in bootloader mode (no firmware))
 */
CmdStatus FsyncController::probe(uint8_t const *cmd, uint8_t response[64])
{
    uint8_t dummy;

    uint32_t neg1 = -1;
    memcpy(response + 4, &neg1, sizeof(uint32_t));

    FsyncI2cResult r1 = readReg(i2c1, FSYNC_ADDRESS, FW_VERSION_REG);
    response[2] = 1;
    fsync_i2c = i2c1;

    if (r1.status >= 0) {
        response[3] = FSYNC_ADDRESS;
        memcpy(response + 4, &r1.data, sizeof(r1.data));
        return CmdStatus::OK;
    }

    int b1 = i2c_read_timeout_us(
        i2c1,
        FSYNC_BOOT_ADDRESS,
        &dummy,
        1,
        false,
        1000
    );

    if (b1 == 1) {
        response[3] = FSYNC_BOOT_ADDRESS;
        return CmdStatus::OK;
    }

    FsyncI2cResult r0 = readReg(i2c0, FSYNC_ADDRESS, FW_VERSION_REG);
    response[2] = 0;
    fsync_i2c = i2c0;

    if (r0.status >= 0) {
        response[3] = FSYNC_ADDRESS;
        memcpy(response + 4, &r0.data, sizeof(r0.data));
        return CmdStatus::OK;
    }

    int b0 = i2c_read_timeout_us(
        i2c0,
        FSYNC_BOOT_ADDRESS,
        &dummy,
        1,
        false,
        1000
    );

    if (b0 == 1) {
        response[3] = FSYNC_BOOT_ADDRESS;
        return CmdStatus::OK;
    }

    response[2] = -1;
    response[3] = -1;

    return CmdStatus::NOK;
}

/*
 * cmd[1] = pin
 * response[2:6] = pin capability
 */
CmdStatus FsyncController::getPinCapabilities(uint8_t const *cmd, uint8_t response[64])
{
    uint8_t pin; memcpy(&pin, cmd + 1, sizeof(pin));

    if (pin >= PIN_CONFIG_REG_END - PIN_CONFIG_REG_START)
        return CmdStatus::NOK;

    FsyncI2cResult r = readReg(fsync_i2c, FSYNC_ADDRESS, PIN_CONFIG_REG_START + pin);
    memcpy(response + 2, &r.data, sizeof(r.data));

    return r.status >= 0 ? CmdStatus::OK : CmdStatus::NOK;
}

/*
 * cmd[1] = pin
 * cmd[2:6] = pin capability
 * response[2] = error code if NOK (1 - i2c, 2 - controller already initialised, 3 - invalid capability or pin)
 */
CmdStatus FsyncController::setPinCapabilities(uint8_t const *cmd, uint8_t response[64])
{
    if (initok) {
        response[2] = 2;
        return CmdStatus::NOK;
    }

    uint8_t pin; memcpy(&pin, cmd + 1, sizeof(pin));
    uint32_t capability; memcpy(&capability, cmd + 2, sizeof(capability));

    // prevent inputing invalid capabilities or multiple at once
    if (capability & ~0b11111 || (capability & (capability - 1)) || capability == 0) {
        response[2] = 3;
        return CmdStatus::NOK;
    }

    if (pin >= PIN_CONFIG_REG_END - PIN_CONFIG_REG_START) {
        response[2] = 3;
        return CmdStatus::NOK;
    }

    int status = writeReg(fsync_i2c, FSYNC_ADDRESS, PIN_CONFIG_REG_START + pin, capability);

    if (status < 0) {
        response[2] = 1;
        return CmdStatus::NOK;
    }

    return CmdStatus::OK;
}

/*
 * response[2:6] = config register
 */
CmdStatus FsyncController::getMode(uint8_t const *cmd, uint8_t response[64])
{
    FsyncI2cResult r = readReg(fsync_i2c, FSYNC_ADDRESS, CONFIG_REG);
    memcpy(response + 2, &r.data, sizeof(r.data));

    return r.status >= 0 ? CmdStatus::OK : CmdStatus::NOK;
}

/*
 * cmd[1:5] = config register
 */
CmdStatus FsyncController::setMode(uint8_t const *cmd, uint8_t response[64])
{
    uint32_t cfg; memcpy(&cfg, cmd + 1, sizeof(cfg));

    if (cfg > 2)
        return CmdStatus::NOK;

    int status = writeReg(fsync_i2c, FSYNC_ADDRESS, CONFIG_REG, cfg);

    return status >= 0 ? CmdStatus::OK : CmdStatus::NOK;
}

/*
 * response[2:6] = internal source frequency
 * response[6:10] = actual output frequency
 */
CmdStatus FsyncController::getFps(uint8_t const *cmd, uint8_t response[64])
{
    FsyncI2cResult r = readReg(fsync_i2c, FSYNC_ADDRESS, INTERNAL_SOURCE_FREQ_REG);
    memcpy(response + 2, &r.data, sizeof(r.data));

    if (r.status < 0)
        return CmdStatus::NOK;

    r = readReg(fsync_i2c, FSYNC_ADDRESS, ACTUAL_OUTPUT_FREQ_REG);
    memcpy(response + 6, &r.data, sizeof(r.data));

    return r.status >= 0 ? CmdStatus::OK : CmdStatus::NOK;
}

/*
 * cmd[1:5] = requested frequency
 */
CmdStatus FsyncController::setFps(uint8_t const *cmd, uint8_t response[64])
{
    uint32_t fpsraw; memcpy(&fpsraw, cmd + 1, sizeof(fpsraw));

    // make sure we are actually casting to a 32-bit float in the IEEE754 standard
    static_assert(sizeof(float) == 4);
    static_assert(std::numeric_limits<float>::is_iec559);
    float fps; memcpy(&fps, cmd + 1, sizeof(fps));

    if (fps > FSYNC_STM_MAX_FPS_ALLOWED || fps < FSYNC_STM_MIN_FPS_ALLOWED || !isfinite(fps))
        return CmdStatus::NOK;

    int status = writeReg(fsync_i2c, FSYNC_ADDRESS, INTERNAL_SOURCE_FREQ_REG, fpsraw);

    return status >= 0 ? CmdStatus::OK : CmdStatus::NOK;
}

/*
 * cmd[1] = fsync channel
 * response[2:6] = channel duty [0, 2048]
 */
CmdStatus FsyncController::getDuty(uint8_t const *cmd, uint8_t response[64])
{
    uint8_t chn; memcpy(&chn, cmd + 1, sizeof(chn));

    if (chn >= (CHN_REGISTER_END - CHN_REGISTER_START) / 2)
        return CmdStatus::NOK;

    FsyncI2cResult r = readReg(fsync_i2c, FSYNC_ADDRESS, CHN_REGISTER_START + chn * REGS_PER_CHN);
    memcpy(response + 2, &r.data, sizeof(r.data));

    return r.status >= 0 ? CmdStatus::OK : CmdStatus::NOK;
}

/*
 * cmd[1] = fsync channel
 * cmd[2:6] = channel duty [0, 2048]
 */
CmdStatus FsyncController::setDuty(uint8_t const *cmd, uint8_t response[64])
{
    uint8_t chn; memcpy(&chn, cmd + 1, sizeof(chn));
    uint32_t duty; memcpy(&duty, cmd + 2, sizeof(duty));

    if (chn >= (CHN_REGISTER_END - CHN_REGISTER_START) / 2)
        return CmdStatus::NOK;

    if (duty > DUTY_CYCLE_RANGE || duty < 0)
        return CmdStatus::NOK;

    int status = writeReg(fsync_i2c, FSYNC_ADDRESS, CHN_REGISTER_START + chn * REGS_PER_CHN, duty);

    return status >= 0 ? CmdStatus::OK : CmdStatus::NOK;
}

/*
 * cmd[1] = fsync channel
 * response[2:6] = channel polarity [0, 1]
 */
CmdStatus FsyncController::getPolarity(uint8_t const *cmd, uint8_t response[64])
{
    uint8_t chn; memcpy(&chn, cmd + 1, sizeof(chn));

    if (chn >= (CHN_REGISTER_END - CHN_REGISTER_START) / 2)
        return CmdStatus::NOK;

    FsyncI2cResult r = readReg(fsync_i2c, FSYNC_ADDRESS, CHN_REGISTER_START + chn * REGS_PER_CHN + 1);
    memcpy(response + 2, &r.data, sizeof(r.data));

    return r.status >= 0 ? CmdStatus::OK : CmdStatus::NOK;
}

/*
 * cmd[1] = fsync channel
 * cmd[2:6] = channel polarity [0, 1]
 */
CmdStatus FsyncController::setPolarity(uint8_t const *cmd, uint8_t response[64])
{
    uint8_t chn; memcpy(&chn, cmd + 1, sizeof(chn));
    uint32_t polarity; memcpy(&polarity, cmd + 2, sizeof(polarity));

    if (chn >= (CHN_REGISTER_END - CHN_REGISTER_START) / 2)
        return CmdStatus::NOK;

    if (polarity > 1 || polarity < 0)
        return CmdStatus::NOK;

    int status = writeReg(fsync_i2c, FSYNC_ADDRESS, CHN_REGISTER_START + chn * REGS_PER_CHN + 1, polarity);

    return status >= 0 ? CmdStatus::OK : CmdStatus::NOK;
}

/*
 * response[2] = input present [0, 1]
 * response[3:7] = input fps
 * response[7:11] = input duty
 */
CmdStatus FsyncController::getInputInfo(uint8_t const *cmd, uint8_t response[64])
{
    FsyncI2cResult r;
    int status = 0;

    r = readReg(fsync_i2c, FSYNC_ADDRESS, IN1_PRESENT_REG);
    status |= r.status;
    uint8_t in_present = (uint8_t)r.data;

    r = readReg(fsync_i2c, FSYNC_ADDRESS, IN1_FREQUENCY_REG);
    status |= r.status;
    uint32_t in_fps = r.data;

    r = readReg(fsync_i2c, FSYNC_ADDRESS, IN1_DUTY_CYCLE_REG);
    status |= r.status;
    uint32_t in_duty = r.data;

    memcpy(response + 2, &in_present, sizeof(in_present));
    memcpy(response + 2 + sizeof(in_present), &in_fps, sizeof(in_fps));
    memcpy(response + 2 + sizeof(in_present) + sizeof(in_fps), &in_duty, sizeof(in_duty));

    return status >= 0 ? CmdStatus::OK : CmdStatus::NOK;
}

FsyncI2cResult FsyncController::readReg(i2c_inst_t *i2c, uint8_t addr, uint8_t reg)
{
    FsyncI2cResult r = {0, 0};

    r.status |= i2c_write_timeout_us(
        i2c,
        addr,
        &reg,
        sizeof(reg),
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

int FsyncController::writeReg(i2c_inst_t *i2c, uint8_t addr, uint8_t reg, uint32_t data)
{
    int rc = 0;
    uint8_t msg[sizeof(reg) + sizeof(data)] = {reg, data & 0xFF, (data >> 8) & 0xFF, (data >> 16) & 0xFF, (data >> 24) & 0xFF};

    rc |= i2c_write_timeout_us(
        i2c,
        addr,
        (uint8_t *)&msg,
        sizeof(msg),
        false,
        1000
    );

    return rc;
}

void FsyncController::unlockFsyncController()
{
    if (!locked)
        return;

    uint8_t dummy[64];

    if (probe(dummy, dummy) != CmdStatus::OK)
        return;

    int status = writeReg(fsync_i2c, FSYNC_ADDRESS, FW_VERSION_REG, I2C_WRITE_UNLOCK_MAGIC_NUMBER);

    if (status >= 0)
        locked = false; 
}
