#ifndef _INTERFACE_FSYNCCONTROLLER_H
#define _INTERFACE_FSYNCCONTROLLER_H

#define FSYNC_ADDRESS 0x12
#define FSYNC_BOOT_ADDRESS 0x56

#define I2C_WRITE_UNLOCK_MAGIC_NUMBER 42

#define FW_VERSION_REG 0
#define CONFIG_REG 1
#define INTERNAL_SOURCE_FREQ_REG 2
#define ACTUAL_OUTPUT_FREQ_REG 3
#define IN1_PRESENT_REG 4
#define IN1_FREQUENCY_REG 5
#define IN1_DUTY_CYCLE_REG 6
#define CLOSEST_FREQUENCY_REG 7

#define CHN_REGISTER_START	8 // = first register
#define OUT1_DUTY_CYCLE_REG	8
#define OUT1_POLARITY_REG	9
#define OUT2_DUTY_CYCLE_REG	10
#define OUT2_POLARITY_REG	11
#define OUT3_DUTY_CYCLE_REG	12
#define OUT3_POLARITY_REG	13
#define OUT4_DUTY_CYCLE_REG	14
#define OUT4_POLARITY_REG	15
#define OUT5_DUTY_CYCLE_REG	16
#define OUT5_POLARITY_REG	17
#define OUT6_DUTY_CYCLE_REG	18
#define OUT6_POLARITY_REG	19
#define OUT7_DUTY_CYCLE_REG	20
#define OUT7_POLARITY_REG	21
#define OUT8_DUTY_CYCLE_REG	22
#define OUT8_POLARITY_REG	23
#define OUT9_DUTY_CYCLE_REG	24
#define OUT9_POLARITY_REG	25
#define OUT10_DUTY_CYCLE_REG	26
#define OUT10_POLARITY_REG	27
#define OUT11_DUTY_CYCLE_REG	28
#define OUT11_POLARITY_REG	29
#define OUT12_DUTY_CYCLE_REG	30
#define OUT12_POLARITY_REG	31
#define OUT13_DUTY_CYCLE_REG	32
#define OUT13_POLARITY_REG	33

#define CHN_REGISTER_END	34 // = last register + 1

#define REGS_PER_CHN 2

#define PIN_PA0_ID 0
#define PIN_PA1_ID 1
#define PIN_PA2_ID 2
#define PIN_PA3_ID 3
#define PIN_PA4_ID 4
#define PIN_PA5_ID 5
#define PIN_PA6_ID 6
#define PIN_PA7_ID 7
#define PIN_PA8_ID 8
#define PIN_PA11_ID 9
#define PIN_PA12_ID 10
#define PIN_PA13_ID 11
#define PIN_PA14_ID 12
#define PIN_PA15_ID 13
#define PIN_PB0_ID 14
#define PIN_PB1_ID 15
#define PIN_PB3_ID 16
#define PIN_PB4_ID 17
#define PIN_PB5_ID 18
#define PIN_PB8_ID 19
#define PIN_PC6_ID 20

// Device Hardware Configuration Registers
#define PIN_CONFIG_REG_START 34 // = first register
#define PIN_PA0_CONFIG_REG PIN_CONFIG_REG_START + PIN_PA0_ID
#define PIN_PA1_CONFIG_REG PIN_CONFIG_REG_START + PIN_PA1_ID
#define PIN_PA2_CONFIG_REG PIN_CONFIG_REG_START + PIN_PA2_ID
#define PIN_PA3_CONFIG_REG PIN_CONFIG_REG_START + PIN_PA3_ID
#define PIN_PA4_CONFIG_REG PIN_CONFIG_REG_START + PIN_PA4_ID
#define PIN_PA5_CONFIG_REG PIN_CONFIG_REG_START + PIN_PA5_ID
#define PIN_PA6_CONFIG_REG PIN_CONFIG_REG_START + PIN_PA6_ID
#define PIN_PA7_CONFIG_REG PIN_CONFIG_REG_START + PIN_PA7_ID
#define PIN_PA8_CONFIG_REG PIN_CONFIG_REG_START + PIN_PA8_ID
#define PIN_PA11_CONFIG_REG PIN_CONFIG_REG_START + PIN_PA11_ID
#define PIN_PA12_CONFIG_REG PIN_CONFIG_REG_START + PIN_PA12_ID
#define PIN_PA13_CONFIG_REG PIN_CONFIG_REG_START + PIN_PA13_ID
#define PIN_PA14_CONFIG_REG PIN_CONFIG_REG_START + PIN_PA14_ID
#define PIN_PA15_CONFIG_REG PIN_CONFIG_REG_START + PIN_PA15_ID
#define PIN_PB0_CONFIG_REG PIN_CONFIG_REG_START + PIN_PB0_ID
#define PIN_PB1_CONFIG_REG PIN_CONFIG_REG_START + PIN_PB1_ID
#define PIN_PB3_CONFIG_REG PIN_CONFIG_REG_START + PIN_PB3_ID
#define PIN_PB4_CONFIG_REG PIN_CONFIG_REG_START + PIN_PB4_ID
#define PIN_PB5_CONFIG_REG PIN_CONFIG_REG_START + PIN_PB5_ID
#define PIN_PB8_CONFIG_REG PIN_CONFIG_REG_START + PIN_PB8_ID
#define PIN_PC6_CONFIG_REG PIN_CONFIG_REG_START + PIN_PC6_ID

// should be PIN_PC6_CONFIG_REG + 1
#define PIN_CONFIG_REG_END 55

#define PIN_CONFIG_VALID_REG 55
#define ADC1_DATA_REG 56

#define PIN_CONFIG_TYPE_HIGH_Z_Pos 0
#define PIN_CONFIG_TYPE_HIGH_Z (1 << PIN_CONFIG_TYPE_HIGH_Z_Pos)
#define PIN_CONFIG_TYPE_PWM_Pos 1
#define PIN_CONFIG_TYPE_PWM (1 << PIN_CONFIG_TYPE_PWM_Pos)
#define PIN_CONFIG_TYPE_ADC_Pos 2
#define PIN_CONFIG_TYPE_ADC (1 << PIN_CONFIG_TYPE_ADC_Pos)
#define PIN_CONFIG_TYPE_PWM_KEEPAWAKE_Pos 3
#define PIN_CONFIG_TYPE_PWM_KEEPAWAKE (1 << PIN_CONFIG_TYPE_PWM_KEEPAWAKE_Pos)
#define PIN_CONFIG_TYPE_PWM_HFSTROBE_Pos 4
#define PIN_CONFIG_TYPE_PWM_HFSTROBE (1 << PIN_CONFIG_TYPE_PWM_HFSTROBE_Pos)

#define FSYNC_STM_MIN_FPS_ALLOWED   0.1
#define FSYNC_STM_MAX_FPS_ALLOWED   600.0

#define DUTY_CYCLE_RANGE 2048

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

    CmdStatus init(uint8_t const *cmd, uint8_t response[64]);
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
    FsyncI2cResult readReg(i2c_inst_t *i2c, uint8_t addr, uint8_t reg);
    int writeReg(i2c_inst_t *i2c, uint8_t addr, uint8_t reg, uint32_t data);
    void unlockFsyncController();

    bool initok = false;
    bool locked = true;

    i2c_inst_t *fsync_i2c = NULL;
};

#endif
