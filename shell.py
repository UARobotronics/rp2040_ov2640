import atexit
import IPython
import math
from PIL import Image
import serial
import struct
import sys
import time

CMD_REG_WRITE = 0xAA
CMD_REG_READ = 0xBB
CMD_CAPTURE = 0xCC

WIDTH = 352
HEIGHT = 288
CHUNKS_NUM = 20
CHUNK_HEIGHT = 20
CHUNK_LENGTH = 5
BYTES_PER_PIXEL = 2

ser = serial.Serial(sys.argv[1], 460800)
atexit.register(lambda: ser.close())

# ======================
# Actual device commands
# ======================
def reg_write(reg, value):
    ser.reset_output_buffer()
    ser.reset_input_buffer()

    assert isinstance(reg, int)
    assert 0x00 <= reg <= 0xFF
    assert isinstance(value, int)
    assert 0x00 <= value <= 0xFF

    ser.write(struct.pack('BBB', CMD_REG_WRITE, reg, value))

def reg_read(reg):
    ser.reset_output_buffer()
    ser.reset_input_buffer()

    assert isinstance(reg, int)
    assert 0x00 <= reg <= 0xFF

    ser.write(struct.pack('BB', CMD_REG_READ, reg))

    return ser.read(1)[0]

def read_buf():
    ser.reset_output_buffer()
    ser.reset_input_buffer()

    print(f"Incoming data:\n")

    # time.sleep(5)

    ser.write(struct.pack('B', CMD_CAPTURE))

    data = ser.read(200)
    # data = ser.read_all()

    print(f"Received {len(data)} bytes\n");

    for i in range (0, len(data)):
        # data = ser.read(1)
        print(f"{data[i]:02x} ", end='')

def capture(filename):
    ser.reset_output_buffer()
    ser.reset_input_buffer()

    ser.write(struct.pack('B', CMD_CAPTURE))

    print(f"Reading image data...")

    # # RGB565 format
    # total_bytes = 352*288*2
    # raw = ser.read(352*288*2)
    
    # # Read frame count
    # raw = ser.read(1)
    # print(f"Frame count: {raw[0]}")

    # raw = ser.read(1)
    # print(f"Chunk count: {raw[0]}")

    # YUV422 format
    # total_bytes = WIDTH * CHUNK_HEIGHT * BYTES_PER_PIXEL
    total_bytes = WIDTH * HEIGHT * BYTES_PER_PIXEL
    print_bytes = WIDTH * 3

    print(f"Data format: YUV422")
    print(f"Raw data (first {print_bytes} bytes):")

    img = Image.new('RGB', (WIDTH, HEIGHT))
    width, height = img.size
    data = img.load()

    raw = ser.read(total_bytes)

    # # RGB565 format
    # for y in range(height):
    #     for x in range(width):
    #         idx = y * width + x
    #         v = struct.unpack('<H', raw[2*idx:2*(idx+1)])[0]
    #         # Reverse binary representation
    #         # v = int(bin(v)[2:].zfill(16)[::-1], 2)

    #         r, g, b = v >> (5 + 6), (v >> 5) & 0b111111, v & 0b11111 

    #         r = math.floor(r / 0x1f * 0xff)
    #         g = math.floor(g / 0x3f * 0xff)
    #         b = math.floor(b / 0x1f * 0xff)
    #         data[x, y] = (r, g, b)

    # YUV422 format
    for y in range(height):
        for x in range(width):
            idx = y * width + x
            data_ind = 2*idx + 1
            if (data_ind < total_bytes):
                v = raw[data_ind]

                data[x, y] = (v, v, v)

                if idx < print_bytes:
                    print(f"{v:02x} ", end='')

    # # Receive image data by chunks
    # for y in range(150):
    #     raw = ser.read(CHUNK_LENGTH * BYTES_PER_PIXEL)
    #     print(f"Read {y} line");
    #     for x in range(CHUNK_LENGTH):
    #         data_ind = 2*x + 1
    #         if (data_ind < total_bytes):
    #             v = raw[data_ind]
    # 
    #             data[x, y] = (v, v, v)
    # 
    #             # if idx < print_bytes:
    #             #     print(f"{v:02x} ", end='')

    img.save(filename)

# =======
# Helpers
# =======
def reg_write_list(l):
    for reg, value in l:
        reg_write(reg, value)

def reg_set_bit(reg, bit):
    value = reg_read(reg)
    value |= 1 << bit
    reg_write(reg, value)

def reg_clear_bit(reg, bit):
    value = reg_read(reg)
    value &= 0xFF ^ (1 << bit)
    reg_write(reg, value)

def reg_get_bit(reg, bit):
    value = reg_read(reg)
    return (value >> bit) & 1

# ====================
# Camera configuration
# ====================
def set_hflip(hflip):
    reg_write(0xff, 0x01)
    if hflip:
        reg_set_bit(0x04, 7)
    else:
        reg_clear_bit(0x04, 7)

def set_vflip(vflip):
    reg_write(0xff, 0x01)
    if vflip:
        reg_set_bit(0x04, 6)
    else:
        reg_clear_bit(0x04, 6)

IPython.embed()
