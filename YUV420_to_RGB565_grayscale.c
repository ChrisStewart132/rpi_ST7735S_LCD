/**
 * Author: Christopher Stewart (Christopher.ray.stewart@gmail.com)
 * Date: 07062024
 * Description: program to convert an incoming stream of 128x160 yuv420 images to grayscale RGB565 (16bit per pixel) format
 * 
 * gcc -o YUV420_to_RGB565_grayscale YUV420_to_RGB565_grayscale.c -lgpiod
 * rpicam-vid -t 0 -n --framerate 30 --width 128 --height 160 --codec yuv420 -o - | ./YUV420_to_RGB565_grayscale | ./ST7735S_LCD_stdin_stream
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#define WIDTH 128
#define HEIGHT 160

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
        for(int i = 0; i < HEIGHT; i++){
            for(int j = 0; j < WIDTH; j++){
                uint8_t gray = y[i][j];
                uint8_t r = gray/8;//0-31
                uint8_t g = gray/4;//0-63
                uint8_t b = gray/8;//0-31
                // RGB565 format: 5 bits for red, 6 bits for green, 5 bits for blue
                rgb565[i][j] = (r << 11) | (g << 5) | b;
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