file = open("16bit_rgb_128_160.bin", 'wb')

WIDTH = 128
HEIGHT = 160
for i in range(WIDTH):
    for j in range(HEIGHT):
        r = bytes([0xF8, 0x00])  # Red
        g = bytes([0x07, 0xE0])  # Green
        b = bytes([0x00, 0x1F])  # Blue
        black = bytes([0x00, 0x00]) 
        if i < 10:# top 10 rows
            file.write(black)
        elif i < WIDTH // 3:
            file.write(r)
        elif i < (WIDTH // 3) * 2:
            file.write(g)
        else:
            file.write(b)
file.close()
