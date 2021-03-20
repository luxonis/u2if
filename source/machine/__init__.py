from .i2c import I2C
from .pin import Pin
from .signal import Signal
from .pwm import PWM
from .adc import ADC
from .uart import UART
from .spi import SPI
from .u2if_const import u2if
from .u2if import Device


def select_interface(sn):
    Device(serial_number_str=sn)


def unique_id():
    device = Device()
    return device.get_serial_number()


def firmware_version():
    device = Device()
    return device.get_firmware_version()
