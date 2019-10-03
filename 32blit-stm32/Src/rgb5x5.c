#include "rgb5x5.h"
void _i2c_send_8(I2C_HandleTypeDef *i2c_port, uint8_t address, uint8_t reg, uint8_t data);
uint8_t _i2c_recv_8(I2C_HandleTypeDef *i2c_port, uint8_t address, uint8_t reg);
void _bank(I2C_HandleTypeDef *i2c_port, uint8_t bank);
void _frame(I2C_HandleTypeDef *i2c_port, uint8_t frame);
void _i2c_send_buf(I2C_HandleTypeDef *i2c_port, uint8_t address, uint8_t reg, uint8_t *data, uint8_t len);
void rgb5x5_show(I2C_HandleTypeDef *i2c_port);

uint8_t rgb5x5_buf[144] = {0};

bool rgb5x5_init(I2C_HandleTypeDef *i2c_port) {

    _bank(i2c_port, RGB5X5_CONFIG_BANK); // 232 253 11
    _i2c_send_8(i2c_port, RGB5X5_DEVICE_ADDRESS, RGB5X5_SHUTDOWN_REGISTER, 1); // Disable shutdown
    _i2c_send_8(i2c_port, RGB5X5_DEVICE_ADDRESS, RGB5X5_MODE_REGISTER, RGB5X5_PICTURE_MODE);
    _i2c_send_8(i2c_port, RGB5X5_DEVICE_ADDRESS, RGB5X5_AUDIOSYNC_REGISTER, 0);
    _frame(i2c_port, 0);

    uint8_t enable_leds[] = {
        RGB5X5_ENABLE_OFFSET,
        0b00000000, 0b10111111,
        0b00111110, 0b00111110,
        0b00111111, 0b10111110,
        0b00000111, 0b10000110,
        0b00110000, 0b00110000,
        0b00111111, 0b10111110,
        0b00111111, 0b10111110,
        0b01111111, 0b11111110,
        0b01111111, 0b00000000,
    };

    _bank(i2c_port, 0);
    HAL_I2C_Master_Transmit(i2c_port, RGB5X5_DEVICE_ADDRESS, &enable_leds[0], 19, HAL_TIMEOUT);
}

typedef struct pixel {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} pixel;

void rgb5x5_set_pixel(I2C_HandleTypeDef *i2c_port, uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
        pixel lookup[] = {
            {118, 69, 85},
            {117, 68, 101},
            {116, 84, 100},
            {115, 83, 99},
            {114, 82, 98},
            {113, 81, 97},
            {112, 80, 96},
            {134, 21, 37},
            {133, 20, 36},
            {132, 19, 35},
            {131, 18, 34},
            {130, 17, 50},
            {129, 33, 49},
            {128, 32, 48},

            {127, 47, 63},
            {121, 41, 57},
            {122, 25, 58},
            {123, 26, 42},
            {124, 27, 43},
            {125, 28, 44},
            {126, 29, 45},
            {15, 95, 111},
            {8, 89, 105},
            {9, 90, 106},
            {10, 91, 107},
            {11, 92, 108},
            {12, 76, 109},
            {13, 77, 93},
        };

        pixel current = lookup[index];

        rgb5x5_buf[current.r] = r;
        rgb5x5_buf[current.g] = g;
        rgb5x5_buf[current.b] = b;
}

void rgb5x5_rgb(I2C_HandleTypeDef *i2c_port, uint8_t r, uint8_t g, uint8_t b) {
    for(auto x = 0; x < 5*5; x++){
        rgb5x5_set_pixel(i2c_port, x, r, g, b);
    }
    rgb5x5_show(i2c_port);
}

void rgb5x5_show(I2C_HandleTypeDef *i2c_port) {
    uint8_t rgb[4] = {0};

    for(auto x = 0; x < 48; x++){
        rgb[0] = RGB5X5_PWM_OFFSET + (x * 3);
        rgb[1] = rgb5x5_buf[(x * 3)];
        rgb[2] = rgb5x5_buf[(x * 3) + 1];
        rgb[3] = rgb5x5_buf[(x * 3) + 2];
        HAL_I2C_Master_Transmit(i2c_port, RGB5X5_DEVICE_ADDRESS, &rgb[0], 4, HAL_TIMEOUT);
    }
}

/*
    uint8_t light_leds[145] = {0};


    for(auto x = 0; x < 145; x++){
        light_leds[x] = 0;
    }

    for(auto x = 0; x < 5; x++){
        _i2c_send_buf(i2c_port, RGB5X5_DEVICE_ADDRESS, RGB5X5_COLOR_OFFSET + (x * 29), &light_leds[(x * 29)], 29);
    }
*/


void _bank(I2C_HandleTypeDef *i2c_port, uint8_t bank) {
    _i2c_send_8(i2c_port, RGB5X5_DEVICE_ADDRESS, RGB5X5_BANK_ADDRESS, bank);
}

void _frame(I2C_HandleTypeDef *i2c_port, uint8_t frame) {
    _i2c_send_8(i2c_port, RGB5X5_DEVICE_ADDRESS, RGB5X5_FRAME_REGISTER, frame);
}


void _i2c_send_buf(I2C_HandleTypeDef *i2c_port, uint8_t address, uint8_t reg, uint8_t *data, uint8_t len){
    uint8_t data_buffer[30];
    data_buffer[0] = reg;
    for(auto x = 0; x < len; x++){
        data_buffer[x+1] = data[x];
    }
    HAL_I2C_Master_Transmit(i2c_port, address, &data_buffer[0], len + 1, HAL_TIMEOUT);
}