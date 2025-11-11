#pragma once
#include <stdio.h>
#include <stdint.h>

#define HID_DESCRIPTOR_LENGTH (30) // always 30
#define HID_DESCRIPTOR_VERSION (0x0100) // always 0x0100

//trigger line
#define HID_TL_HAS_DATA (0)
#define HID_TL_NO_DATA (1)

#define HID_I2C_HID_DESCRIPTOR_REGISTER (0x20) //defined in device tree
#define HID_I2C_REPORT_DESCRIPTOR_REGISTER (0x30)
#define HID_I2C_INPUT_REGISTER (0x34)
#define HID_I2C_OUTPUT_REGISTER (0x40)
#define HID_I2C_COMMAND_REGISTER (0x50)
#define HID_I2C_DATA_REGISTER (0x60)

typedef struct __attribute__((__packed__)) 
{
    uint16_t wHIDDescLength;
    uint16_t bcdVersion;
    uint16_t wReportDescLength;
    uint16_t wReportDescRegister;
    uint16_t wInputRegister;
    uint16_t wMaxInputLength;
    uint16_t wOutputRegister;
    uint16_t wMaxOutputLength;
    uint16_t wCommandRegister;
    uint16_t wDataRegister;
    uint16_t wVendorID;
    uint16_t wProductID;
    uint16_t wVersionID;
    uint32_t RESERVED;
} i2c_hid_descriptor_t;

//OpCode
typedef enum I2C_HID_CMD
{
    I2C_HID_CMD_RESERVED = 0,
    I2C_HID_CMD_RESET,
    I2C_HID_CMD_GET_REPORT,
    I2C_HID_CMD_SET_REPORT,
    I2C_HID_CMD_GET_IDLE,
    I2C_HID_CMD_SET_IDLE,
    I2C_HID_CMD_GET_PROTOCOL,
    I2C_HID_CMD_SET_PROTOCOL,
    I2C_HID_CMD_SET_POWER,
    I2C_HID_CMD_START_VENDOR_RESERVED,
    I2C_HID_CMD_END_VENDOR_RESERVED = 0x0b1110,
    I2C_HID_CMD_RESERVED_END,
}I2C_HID_CMD;

typedef enum I2C_HID_REPORT_TYPE
{
    I2C_HID_REPORT_TYPE_RESERVED = 0,
    I2C_HID_REPORT_TYPE_INPUT,
    I2C_HID_REPORT_TYPE_OUTPUT,
    I2C_HID_REPORT_TYPE_FEATURE,
}I2C_HID_REPORT_TYPE;


typedef struct __attribute__((__packed__)) 
{
    unsigned short length;
    unsigned char button;
    char x;
    char y;
    char wheel;
} i2c_hid_mouse_t;

typedef struct __attribute__((__packed__)) 
{
    unsigned short length;
    unsigned char mod_key;
    char reserved;
    unsigned char keypress[6];
} i2c_hid_keyboard_t;