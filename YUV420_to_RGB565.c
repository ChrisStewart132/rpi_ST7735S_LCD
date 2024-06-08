/**
 * Author: Christopher Stewart (Christopher.ray.stewart@gmail.com)
 * Date: 08062024
 * Description: program to convert an incoming stream of 128x160 yuv420 images to RGB565 (16bit per pixel) format
 * 
 * gcc -o YUV420_to_RGB565 YUV420_to_RGB565.c -lgpiod
 * rpicam-vid -t 0 -n --framerate 30 --width 128 --height 160 --codec yuv420 -o - | ./YUV420_to_RGB565 | ./ST7735S_LCD_stdin_stream
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#define WIDTH 128
#define HEIGHT 160

uint16_t yuv_to_rgb565(uint8_t y, uint8_t u, uint8_t v) {
    int r = y + 1.402 * (v - 128);
    int g = y - 0.344136 * (u - 128) - 0.714136 * (v - 128);
    int b = y + 1.772 * (u - 128);
    r = r < 0 ? 0 : (r > 255 ? 255 : r);
    g = g < 0 ? 0 : (g > 255 ? 255 : g);
    b = b < 0 ? 0 : (b > 255 ? 255 : b);
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

int main(int argc, char** argv) {
    uint8_t y[HEIGHT][WIDTH];
    uint8_t u[HEIGHT/2][WIDTH/2];
    uint8_t v[HEIGHT/2][WIDTH/2];
    uint16_t rgb565[HEIGHT][WIDTH];

    while(1){
        // read from stdin a YUV420 frame
        for(int i = 0; i < HEIGHT; i++){
            read(0, y[i], sizeof(uint8_t)*WIDTH);
        }
        for(int i = 0; i < HEIGHT/2; i++){
            read(0, u[i], sizeof(uint8_t)*WIDTH/2);
        }
        for(int i = 0; i < HEIGHT/2; i++){
            read(0, v[i], sizeof(uint8_t)*WIDTH/2);
        }

        // iterate each 4x4 YUV420 block in the frame and add rgb565 pixels for stdout
        for(int i = 0; i < HEIGHT; i+=2){
            for(int j = 0; j < WIDTH; j+=2){
                uint8_t u_val = u[i/2][j/2];
                uint8_t v_val = v[i/2][j/2];
                rgb565[i][j] = yuv_to_rgb565(y[i][j], u_val, v_val);
                rgb565[i][j+1] = yuv_to_rgb565(y[i][j+1], u_val, v_val);
                rgb565[i+1][j] = yuv_to_rgb565(y[i+1][j], u_val, v_val);
                rgb565[i+1][j+1] = yuv_to_rgb565(y[i+1][j+1], u_val, v_val);
            }
        }

        // write rgb565 frame to stdout
        for(int i = 0; i < HEIGHT; i++){
            write(1, rgb565[i], sizeof(uint16_t)*WIDTH);
        }
        usleep(1000);
    }
	return EXIT_SUCCESS;
}