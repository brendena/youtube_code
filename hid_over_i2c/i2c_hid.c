/*
 * Copyright (c) 2021 Valentin Milea <valentin.milea@gmail.com>
 * Copyright (c) 2023 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <hardware/i2c.h>
#include <pico/i2c_slave.h>
#include <pico/stdlib.h>
#include <stdio.h>
#include <string.h>
#include "i2c_hid_types.h"
#include "i2c_hid_descriptors.h"

static const uint I2C_SLAVE_ADDRESS = 0x2c;
static const uint I2C_BAUDRATE = 100000; // 100 kHz

static const uint I2C_SLAVE_SDA_PIN = 4; // 4
static const uint I2C_SLAVE_SCL_PIN = 5; // 5

static const uint I2C_HID_INTERRUPT_PIN = 3;

static struct
{
    unsigned char write_buffer[24];
    uint16_t mem_address;
    bool i2c_write;
    bool fully_setup;


    I2C_HID_REPORT_TYPE report_type;
    I2C_HID_CMD i2c_cmd;
} context = {0};


i2c_hid_descriptor_t i2c_hid_descriptor = {
    wHIDDescLength : HID_DESCRIPTOR_LENGTH, //always 
    bcdVersion : HID_DESCRIPTOR_VERSION,//1.0 
    wReportDescLength : sizeof(keyboard_hid_descriptor), 
    wReportDescRegister : HID_I2C_REPORT_DESCRIPTOR_REGISTER,
    wInputRegister : HID_I2C_INPUT_REGISTER, 
    wMaxInputLength : sizeof(i2c_hid_keyboard_t), 
    wOutputRegister : HID_I2C_OUTPUT_REGISTER,
    wMaxOutputLength : 4, //we don't have a output register
    wCommandRegister : HID_I2C_COMMAND_REGISTER,
    wDataRegister : HID_I2C_DATA_REGISTER,
    wVendorID : 0,
    wProductID : 0,
    wVersionID : 0,
    RESERVED : 0
} ;

bool got_descriptor = false;

i2c_hid_mouse_t mouse_state = {
    .length = 0x6,
    .button = 0,
    .x = 0x5,
    .y = 0x5,
    .wheel = 0
};

i2c_hid_keyboard_t keyboard_state = {
    .length = sizeof(i2c_hid_keyboard_t),
    .mod_key = 0,
    .reserved = 0,
    .keypress = {0}
};

// Our handler is called from the I2C ISR, so it must complete quickly. Blocking calls /
// printing to stdio may interfere with interrupt handling.
static void i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event) {
    static unsigned int count = 0;
    switch (event) {
    //write
    case I2C_SLAVE_RECEIVE: // master has written some data
        context.i2c_write = 1;
        if(count == 0)
        {
            context.mem_address = 0;
            context.mem_address = i2c_read_byte_raw(i2c);
        }
        else if(count == 1)
        {
            context.mem_address += i2c_read_byte_raw(i2c) << 8;
        }
        else
        {
            context.write_buffer[count - 2] = i2c_read_byte_raw(i2c);
        }
        count++;
        break;
    //read
    case I2C_SLAVE_REQUEST: // master is requesting data
        // load from memory
        unsigned char return_data = 0x00;
        if(context.mem_address == HID_I2C_HID_DESCRIPTOR_REGISTER)
        {
            unsigned char * hid_descriptor_ptr = &i2c_hid_descriptor;
            return_data = hid_descriptor_ptr[count];
        }
        else if(context.mem_address == HID_I2C_REPORT_DESCRIPTOR_REGISTER)
        {
            return_data = keyboard_hid_descriptor[count]; 
            context.fully_setup = true;
        }
        else if(context.i2c_cmd)
        {
            /*On reset you need to respond with all 0's*/
            if(context.i2c_cmd == I2C_HID_CMD_RESET)
            {
                return_data = 0; 
            }
        }
        else if(context.mem_address == 0 &&
                context.i2c_cmd == 0 &&
                context.fully_setup == true)
        {
            unsigned char * mouse_ptr = &keyboard_state;
            return_data = mouse_ptr[count]; 
        }

        i2c_write_byte_raw(i2c, return_data);
        count++;

        break;
    case I2C_SLAVE_FINISH: // master has signalled Stop / Restart
        if(context.i2c_write == 0){
            context.i2c_cmd = 0;
            context.mem_address  = 0;
            //after read happened interrupt high
            gpio_put(I2C_HID_INTERRUPT_PIN, HID_TL_NO_DATA);
        }

        if(context.mem_address == HID_I2C_COMMAND_REGISTER)
        {
            context.i2c_cmd = context.write_buffer[1] & 0xf;
            context.report_type = context.write_buffer[0] & 0b11 >> 4;
            
            if(context.i2c_cmd != I2C_HID_CMD_SET_POWER)
            {
                gpio_put(I2C_HID_INTERRUPT_PIN, HID_TL_HAS_DATA);
            }
            else
            {                
                if(context.report_type == 0 ){
                    //power is turned on
                    // i don't support turning power off
                }
                context.i2c_cmd = 0;
            }
            context.mem_address = 0;
        }
        context.i2c_write = false;
        count = 0;
        break;
    default:
        break;
    }
}

static void setup_slave() {
    gpio_init(I2C_SLAVE_SDA_PIN);
    gpio_set_function(I2C_SLAVE_SDA_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SLAVE_SDA_PIN);

    gpio_init(I2C_SLAVE_SCL_PIN);
    gpio_set_function(I2C_SLAVE_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SLAVE_SCL_PIN);

    i2c_init(i2c0, I2C_BAUDRATE);
    // configure I2C0 for slave mode
    i2c_slave_init(i2c0, I2C_SLAVE_ADDRESS, &i2c_slave_handler);
}


int main() {
    stdio_init_all();
    setup_slave();

    gpio_init(I2C_HID_INTERRUPT_PIN);
    gpio_set_dir(I2C_HID_INTERRUPT_PIN, GPIO_OUT);
    gpio_put(I2C_HID_INTERRUPT_PIN, 1);

    while(true){
        sleep_ms(1000);
        if(context.fully_setup)
        {
            keyboard_state.keypress[0] = 0x04;
            gpio_put(I2C_HID_INTERRUPT_PIN, 0);
            sleep_ms(50);
            keyboard_state.keypress[0] = 0;
            gpio_put(I2C_HID_INTERRUPT_PIN, 0);
        }
    }
}
