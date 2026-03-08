#ifndef OV2640_INIT_H
#define OV2640_INIT_H
#include <stdint.h>

static const uint8_t ov2640_vga[][2] = {
	{0xff, 0x00},  /* Device control register list Table 12 */
	{0x2c, 0xff},  /* Reserved                              */
	{0x2e, 0xdf},  /* Reserved                              */

	{0xff, 0x01},  /* Device control register list Table 13 */
	{0x3c, 0x32},  /* Reserved                              */
	{0x11, 0x00},  /* Clock Rate Control
	// {0x11, 0x04},  /* Clock Rate Control
                      Bit[7]: Internal frequency doublers ON/OFF selection
                        0: OFF
                        1: ON
                      Bit[6]: Reserved
                      Bit[5:0]: Clock divider               */
	{0x09, 0x02},  /* Common control 2 | Output drive select - 2x capability */
	// {0x10, 0x01},  /* Automatic exposure control [2:9] bits */
	{0x04, 0xA8},  /* Mirror                                */
	{0x13, 0xe5},  /* Common control 8                      
	                  +Bit[5]: Banding filter selection - ON, set minimum exposure time to 1/120 s
	                  +Bit[2]: AGC auto/manual control selection - 1 (auto)
	                  +Bit[0]: Exposure control - 1 (auto) */
	{0x14, 0x48},  /* Common control 9 | AGC gain ceiling - 8x */
	// {0x15, 0x02},  /* Common control 10 (if bypass DSP is selected) */
	{0x2c, 0x0c},  /* Reserved                              */
	{0x33, 0x78},  /* Reserved                              */
	{0x3a, 0x33},  /* Reserved                              */
	{0x3b, 0xfB},  /* Reserved                              */
	{0x3e, 0x00},  /* Reserved                              */
	{0x43, 0x11},  /* Reserved                              */
	{0x16, 0x10},  /* Reserved                              */
	{0x4a, 0x81},  /* Reserved                              */
	{0x21, 0x99},  /* Reserved                              */
	{0x24, 0x40},  /* Luminance signal High range (64)      */
	{0x25, 0x38},  /* Luminance signal low range  (62)      */
	{0x26, 0x82},  /* Fast Mode Large Step Range Threshold - effective only in AEC/AGC fast mode */
	{0x5c, 0x00},  /* Reserved                              */
	{0x63, 0x00},  /* Reserved                              */
	{0x46, 0x3f},  /* Frame length adjustment | Each bit will add 1 horizontal line timing in frame
	                  (6)                                   */
	{0x0c, 0x3c},  /* Common control 3 | Set banding manually - 1 (50 Hz) */
	{0x61, 0x70},  /* Histogram algo low level (112)        */
	{0x62, 0x80},  /* Histogram algo high level (144)       */
	{0x7c, 0x05},  /* Reserved                              */
	{0x20, 0x80},  /* Reserved                              */
	{0x28, 0x30},  /* Reserved                              */
	{0x6c, 0x00},  /* Reserved                              */
	{0x6d, 0x80},  /* Reserved                              */
	{0x6e, 0x00},  /* Reserved                              */
	{0x70, 0x02},  /* Reserved                              */
	{0x71, 0x94},  /* Reserved                              */
	{0x73, 0xc1},  /* Reserved                              */
	{0x3d, 0x34},  /* Reserved                              */
	{0x5a, 0x57},  /* Reserved                              */
	{0x12, 0x00},  /* Common control 7 | Bit[6:4]: Resolution selection:
	                  +000: UXGA (full size) mode
					  010: CIF mode
					  100: SVGA mode                        */
	// {0x12, 0x20},  /* Common control 7, CIF mode            */
	{0x11, 0x00},  /* Clock Rate Control
	// {0x11, 0x04},  /* Clock Rate Control
                      Bit[7]: Internal frequency doublers ON/OFF selection
                        +0: OFF
                        1: ON
                      Bit[6]: Reserved
                      Bit[5:0]: Clock divider               */
	// {0x11, 0x80},  /* Clock Rate Control - double clock  */
	{0x17, 0x11},  /* Horiz window start Bit[10:3] (17 -> 136)
                        Bit[10:0]: Selects the start of the horizontal window, each LSB
                        represents two pixels               */
	{0x18, 0x75},  /* Horiz window end Bit[10:3] (117 -> 936) */
	{0x19, 0x01},  /* Vert window line start Bit[9:2] (1 -> 4)
                        Bit[9:0]: Selects the start of the vertical window, each LSB
                        represents two scan lines.          */
	{0x1a, 0x97},  /* Vert window line end Bit[9:2] (151 -> 604) */
	{0x32, 0x36},  /* Common control 32:
	                    Bit[7:6]: Pixel clock divide option
	                    Bit[5:3]: Horiz window end LSB Bit[2:0] (6)
	                    Bit[2:0]: Horiz window start LSB Bit[2:0] (6)
	                    UXGA default - 0x36              */
	// {0x32, 0x09},  /* CIF default - 0x09                    */
	{0x03, 0x0f},  /* Common control 1:
	                  Bit[3:2]: Vert window end line control Bit[1:0] (3)
	                  Bit[1:0]: Vert window start line control Bit[1:0] (3) */
	{0x37, 0x40},  /* Reserved                              */
	{0x4f, 0xbb},  /* 50Hz banding AEC Bit[7:0] (187)       */
	{0x50, 0x9c},  /* 60Hz banding AEC Bit[7:0] (156)       */
	{0x5a, 0x57},  /* Reserved                              */
	{0x6d, 0x80},  /* Reserved                              */
	{0x6d, 0x38},  /* Reserved                              */
	{0x39, 0x02},  /* Reserved                              */
	{0x35, 0x88},  /* Reserved                              */
	{0x22, 0x0a},  /* Reserved                              */
	{0x37, 0x40},  /* Reserved                              */
	{0x23, 0x00},  /* Reserved                              */
	{0x34, 0xa0},  /* Bit[2]: Zoom window horizontal start point (0) */
	{0x36, 0x1a},  /* Reserved                              */
	{0x06, 0x02},  /* Reserved                              */
	{0x07, 0xc0},  /* Reserved                              */
	{0x0d, 0xb7},  /* Reserved                              */
	{0x0e, 0x01},  /* Reserved                              */
	{0x4c, 0x00},  /* Reserved                              */

	{0xff, 0x00},  /* Device control register list Table 12 */
	{0xe5, 0x7f},  /* Reserved                              */
	{0xf9, 0xc0},  /* +Bit[7]: Microcontroller Reset
	                  +Bit[6]: Boot ROM select              */
	{0x41, 0x24},  /* Reserved                              */
	{0xe0, 0x14},  /* Reset:
	                  +Bit[4]: JPEG
					  +Bit[2]: DVP                          */
	{0x76, 0xff},  /* Reserved                              */
	{0x33, 0xa0},  /* Reserved                              */
	{0x42, 0x20},  /* Reserved                              */
	{0x43, 0x18},  /* Reserved                              */
	{0x4c, 0x00},  /* Reserved                              */
	{0x87, 0xd0},  /* Module Enable:
	                  +Bit[7]: BPC
					  +Bit[6]: WPC                          */
	{0x88, 0x3f},  /* Reserved                              */
	{0xd7, 0x03},  /* Reserved                              */
	{0xd9, 0x10},  /* Reserved                              */
	{0xd3, 0x82},  /* DVP output speed control:
	                  +Bit[7]: Auto mode
					  Bit[6:0]: DVP output speed control    
					      DVP PCLK = sysclk (48)/[6:0] (YUV0) 
						           = sysclk (48)/2*[6:0]) (RAW) */
	{0xc8, 0x08},  /* Reserved                              */
	{0xc9, 0x80},  /* Reserved                              */
	{0x7d, 0x00},  /* SDE indirect register access: address */
	{0x7c, 0x03},  /* SDE indirect register access: data    */
	{0x7d, 0x48},  /* SDE indirect register access: address */
	{0x7c, 0x08},  /* SDE indirect register access: data    */
	{0x7d, 0x20},  /* SDE indirect register access: address */
	{0x7d, 0x10},  /* SDE indirect register access: address */
	{0x7d, 0x0e},  /* SDE indirect register access: address */
	{0x90, 0x00},  /* Reserved                              */
	{0x91, 0x0e},  /* Reserved                              */
	{0x91, 0x1a},  /* Reserved                              */
	{0x91, 0x31},  /* Reserved                              */
	{0x91, 0x5a},  /* Reserved                              */
	{0x91, 0x69},  /* Reserved                              */
	{0x91, 0x75},  /* Reserved                              */
	{0x91, 0x7e},  /* Reserved                              */
	{0x91, 0x88},  /* Reserved                              */
	{0x91, 0x8f},  /* Reserved                              */
	{0x91, 0x96},  /* Reserved                              */
	{0x91, 0xa3},  /* Reserved                              */
	{0x91, 0xaf},  /* Reserved                              */
	{0x91, 0xc4},  /* Reserved                              */
	{0x91, 0xd7},  /* Reserved                              */
	{0x91, 0xe8},  /* Reserved                              */
	{0x91, 0x20},  /* Reserved                              */
	{0x92, 0x00},  /* Reserved                              */
	{0x93, 0x06},  /* Reserved                              */
	{0x93, 0xe3},  /* Reserved                              */
	{0x93, 0x02},  /* Reserved                              */
	{0x93, 0x02},  /* Reserved                              */
	{0x93, 0x00},  /* Reserved                              */
	{0x93, 0x04},  /* Reserved                              */
	{0x93, 0x00},  /* Reserved                              */
	{0x93, 0x03},  /* Reserved                              */
	{0x93, 0x00},  /* Reserved                              */
	{0x93, 0x00},  /* Reserved                              */
	{0x93, 0x00},  /* Reserved                              */
	{0x93, 0x00},  /* Reserved                              */
	{0x93, 0x00},  /* Reserved                              */
	{0x93, 0x00},  /* Reserved                              */
	{0x93, 0x00},  /* Reserved                              */
	{0x96, 0x00},  /* Reserved                              */
	{0x97, 0x08},  /* Reserved                              */
	{0x97, 0x19},  /* Reserved                              */
	{0x97, 0x02},  /* Reserved                              */
	{0x97, 0x0c},  /* Reserved                              */
	{0x97, 0x24},  /* Reserved                              */
	{0x97, 0x30},  /* Reserved                              */
	{0x97, 0x28},  /* Reserved                              */
	{0x97, 0x26},  /* Reserved                              */
	{0x97, 0x02},  /* Reserved                              */
	{0x97, 0x98},  /* Reserved                              */
	{0x97, 0x80},  /* Reserved                              */
	{0x97, 0x00},  /* Reserved                              */
	{0x97, 0x00},  /* Reserved                              */
	{0xc3, 0xef},  /* Module Enable:
	                  +Bit[7]: CIP
	                  +Bit[6]: DMY
	                  +Bit[5]: RAW_GMA
	                  Bit[4]: DG
	                  +Bit[3]: AWB
	                  +Bit[2]: AWB_GAIN
	                  +Bit[1]: LENC
					  +Bit[0]: PRE                           */

	{0xff, 0x00},  /* Device control register list Table 12 */
	{0xba, 0xdc},  /* Reserved                              */
	{0xbb, 0x08},  /* Reserved                              */
	{0xb6, 0x24},  /* Reserved                              */
	{0xb8, 0x33},  /* Reserved                              */
	{0xb7, 0x20},  /* Reserved                              */
	{0xb9, 0x30},  /* Reserved                              */
	{0xb3, 0xb4},  /* Reserved                              */
	{0xb4, 0xca},  /* Reserved                              */
	{0xb5, 0x43},  /* Reserved                              */
	{0xb0, 0x5c},  /* Reserved                              */
	{0xb1, 0x4f},  /* Reserved                              */
	{0xb2, 0x06},  /* Reserved                              */
	{0xc7, 0x00},  /* Reserved                              */
	{0xc6, 0x51},  /* Reserved                              */
	{0xc5, 0x11},  /* Reserved                              */
	{0xc4, 0x9c},  /* Reserved                              */
	{0xbf, 0x00},  /* Reserved                              */
	{0xbc, 0x64},  /* Reserved                              */
	{0xa6, 0x00},  /* Reserved                              */
	{0xa7, 0x1e},  /* Reserved                              */
	{0xa7, 0x6b},  /* Reserved                              */
	{0xa7, 0x47},  /* Reserved                              */
	{0xa7, 0x33},  /* Reserved                              */
	{0xa7, 0x00},  /* Reserved                              */
	{0xa7, 0x23},  /* Reserved                              */
	{0xa7, 0x2e},  /* Reserved                              */
	{0xa7, 0x85},  /* Reserved                              */
	{0xa7, 0x42},  /* Reserved                              */
	{0xa7, 0x33},  /* Reserved                              */
	{0xa7, 0x00},  /* Reserved                              */
	{0xa7, 0x23},  /* Reserved                              */
	{0xa7, 0x1b},  /* Reserved                              */
	{0xa7, 0x74},  /* Reserved                              */
	{0xa7, 0x42},  /* Reserved                              */
	{0xa7, 0x33},  /* Reserved                              */
	{0xa7, 0x00},  /* Reserved                              */
	{0xa7, 0x23},  /* Reserved                              */
	{0xc0, 0xc8},  /* Image horizontal size HSIZE[10:3] (200 -> 800) */
	{0xc1, 0x96},  /* Image vertical size VSIZE[10:3] (150 -> 600)   */
	{0x8c, 0x00},  /* Reserved                              */
	{0x86, 0x3d},  /* Module Enable:     
	                  +Bit[5]: DCW
	                  +Bit[4]: SDE
	                  +Bit[3]: UV_ADJ
	                  +Bit[2]: UV_AVG
	                  Bit[1]: Reserved
	                  +Bit[0]: CMX                          */
	{0x50, 0x89},  /* +Bit[7]: LP_DP
	                  Bit[6]: Round
					  Bit[5:3]: V_DIVIDER      (2)
					  Bit[2:0]: H_DIVIDER      (2)          */
	{0x51, 0x90},  /* H_SIZE[7:0] (real/4)     (144) (576)  */
	{0x52, 0x2c},  /* V_SIZE[7:0] (real/4)     (44)  (176)  */
	{0x53, 0x00},  /* OFFSET_X[7:0] (0)                     */
	{0x54, 0x00},  /* OFFSET_Y[7:0] (0)                     */
	{0x55, 0x88},  /* Bit[7]: V_SIZE[8] (real/4) (1 -> 1024)
                      Bit[6:4]: OFFSET_Y[10:8]   (0)
                      Bit[3]: H_SIZE[8] (real/4) (1 -> 1024)
                      Bit[2:0]: OFFSET_X[10:8]   (0)        */
	{0x5a, 0xa0},  /* OUTW[7:0] (real/4)       (160) (640)   */
	{0x5b, 0x78},  /* OUTH[7:0] (real/4)       (120) (480)   */
	{0x5c, 0x00},  /* Bit[7:4]: ZMSPD (zoom speed)
                      Bit[2]: OUTH[8]     (0)
                      Bit[1:0]: OUTW[9:8] (0)               */
	{0xd3, 0x04},  /* DVP output speed control:
	                  Bit[7]: Auto mode
					  Bit[6:0]: DVP output speed control    
					      DVP PCLK = sysclk (48)/[6:0] (YUV0) 
						           = sysclk (48)/2*[6:0]) (RAW) */
	{0x7f, 0x00},  /* Reserved                              */
	{0xda, 0x00},  /* Image mode (YUV422)                   */
	{0xe5, 0x1f},  /* Reserved                              */
	{0xe1, 0x67},  /* Reserved                              */
	{0xe0, 0x00},  /* Reset                                 */
	{0xdd, 0x7f},  /* Reserved                              */
	{0x05, 0x00},  /* Bypass DSP
                        Bit[7:1]: Reserved
                        Bit[0]: Bypass DSP select
                          +0: DSP
                          1: Bypass DSP, sensor out directly */

	{0xff, 0x00},  /* Device control register list Table 12 */
	{0xe0, 0x04},  /* Reset:
					  +Bit[2]: DVP                          */
	{0xc0, 0xc8},  /* Image horizontal size [10:3] bits (200 -> 800) */
	{0xc1, 0x96},  /* Image vertical size [10:3] bits (150 -> 600)   */
	{0x86, 0x3d},  /* Module Enable:     
	                  +Bit[5]: DCW
	                  +Bit[4]: SDE
	                  +Bit[3]: UV_ADJ
	                  +Bit[2]: UV_AVG
	                  Bit[1]: Reserved
	                  +Bit[0]: CMX                          */
	{0x50, 0x89},  /* +Bit[7]: LP_DP
	                  Bit[6]: Round
					  +Bit[5:3]: V_DIVIDER (2)
					  +Bit[2:0]: H_DIVIDER (2)              */
	{0x51, 0x90},  /* H_SIZE[7:0] (real/4) (144)  (576)     */
	{0x52, 0x2c},  /* V_SIZE[7:0] (real/4) (44)   (176)     */
	{0x53, 0x00},  /* OFFSET_X[7:0]        (0)              */
	{0x54, 0x00},  /* OFFSET_Y[7:0]        (0)              */
	{0x55, 0x88},  /* Bit[7]: V_SIZE[8] (real/4) (1 -> 1024)
                      Bit[6:4]: OFFSET_Y[10:8]   (0)
                      Bit[3]: H_SIZE[8]          (1 -> 1024)
                      Bit[2:0]: OFFSET_X[10:8]   (0)        */
	{0x57, 0x00},  /* Bit[7]: H_SIZE[9]        (0 -> 0)     */
	{0x5a, 0xa0},  /* OUTW[7:0] (real/4)       (160) (640)   */
	{0x5b, 0x78},  /* OUTH[7:0] (real/4)       (120) (480)   */
	{0x5c, 0x00},  /* Bit[7:4]: ZMSPD (zoom speed)
                      Bit[2]: OUTH[8]          (0)
                      Bit[1:0]: OUTW[9:8]      (0)          */
	{0xd3, 0x04},  /* DVP output speed control:
	                  Bit[7]: Auto mode
					  Bit[6:0]: DVP output speed control  (4)   
					      DVP PCLK = sysclk (48)/[6:0]    (YUV0) 
						           = sysclk (48)/2*[6:0]) (RAW) */
	{0xe0, 0x00},  /* Reset, none bits                      */

	{0xFF, 0x00},  /* Device control register list Table 12 */
	{0x05, 0x00},  /* Bypass DSP
                        Bit[7:1]: Reserved
                        Bit[0]: Bypass DSP select
                          +0: DSP
                          1: Bypass DSP, sensor out directly */
	//{0xDA, 0x08}, // Image mode (RGB565)
	{0xDA, 0x00}, /* Bit[3:2]: DVP output format
                       +00: YUV422
                       01: RAW10 (DVP)
                       10: RGB565
                       11: Reserved                         */
	// {0xda, 0x09},
	{0x98, 0x00},  /* Reserved                              */
	{0x99, 0x00},  /* Reserved                              */
	{0x00, 0x00},  /* Reserved                              */

	{0xff, 0x00},  /* Device control register list Table 12 */
	{0xe0, 0x04},  /* Reset:
					  +Bit[2]: DVP                           */
	{0xc0, 0xc8},  /* Image horizontal size HSIZE[10:3] bits (200 -> 800) */
	{0xc1, 0x96},  /* Image vertical size [10:3] bits (150 -> 600)   */
	{0x86, 0x3d},  /* Module Enable:     
	                  +Bit[5]: DCW
	                  +Bit[4]: SDE
	                  +Bit[3]: UV_ADJ
	                  +Bit[2]: UV_AVG
	                  Bit[1]: Reserved
	                  +Bit[0]: CMX                           */
	{0x50, 0x89},  /* +Bit[7]: LP_DP
	                  Bit[6]: Round
					  Bit[5:3]: V_DIVIDER   (1)
					  Bit[2:0]: H_DIVIDER   (1)              */
	{0x51, 0x90},  /* H_SIZE[7:0] (real/4)  (144) (576)      */
	{0x52, 0x2c},  /* V_SIZE[7:0] (real/4)  (44)  (176)      */
	{0x53, 0x00},  /* OFFSET_X[7:0]         (0)              */
	{0x54, 0x00},  /* OFFSET_Y[7:0]         (0)              */
	{0x55, 0x88},  /* Bit[7]: V_SIZE[8] (real/4) (1 -> 1024)
                      Bit[6:4]: OFFSET_Y[10:8]   (0)
                      Bit[3]: H_SIZE[8] (real/4) (1 -> 1024)
                      Bit[2:0]: OFFSET_X[10:8]   (0)        */
	{0x57, 0x00},  /* Bit[7]: H_SIZE[9] (real/4) (0)        */
	{0x5a, 0xa0},  /* OUTW[7:0] (real/4) (160)  (640)       */
	{0x5b, 0x78},  /* OUTH[7:0] (real/4) (120)  (480)       */
	{0x5c, 0x00},  /* Bit[7:4]: ZMSPD (zoom speed)
                      Bit[2]: OUTH[8]           (0 -> 0)
                      Bit[1:0]: OUTW[9:8]       (0 -> 0)    */
	{0xd3, 0x02},  /* DVP output speed control:
	                  Bit[7]: Auto mode
					  Bit[6:0]: DVP output speed control  (2)  
					      DVP PCLK = sysclk (48)/[6:0] (YUV0) 
						           = sysclk (48)/2*[6:0]) (RAW) */
	{0xe0, 0x00}  /* Reset, none bits                       */
};

static const uint8_t ov2640_uxga_cif[][2] = {   
	// {0xff, 0x00},  /* Device control register list Table 12 */  
	// {0xe0, 0x04},  /* Reset:
	// 				  +Bit[2]: DVP                          */
	// {0xc0, 0xc8},  /* Image horizontal size [10:3] bits (200 -> 800) */
	// {0xc1, 0x96},  /* Image vertical size [10:3] bits (150 -> 600)   */
	// // {0xc0, 0x64}, /* Image horizontal size [10:3] bits (100 -> 400) */ 
	// // {0xc1, 0x4C}, /* Image vertical size [10:3] bits (75 -> 300)  */  
	// // {0x8c, 0x00}, /* [6] bit - image horizontal size [11] bit
	// //                  [5-3] bits - image horizontal size [2:0] bits (0)
	// // 			        [2-0] bits - image vertical size [2:0] bits (0) */
	// {0x86, 0x35},  /* Module Enable:     
	//                   +Bit[5]: DCW
	//                   +Bit[4]: SDE
	//                   Bit[3]: UV_ADJ
	//                   +Bit[2]: UV_AVG
	//                   Bit[1]: Reserved
	//                   +Bit[0]: CMX                          */   
	// // {0x50, 0x92},  /* +Bit[7]: LP_DP
	// {0x50, 0x80},  /* +Bit[7]: LP_DP
	//                   Bit[6]: Round
	// 				  Bit[5:3]: V_DIVIDER  (2)
	// 				  Bit[2:0]: H_DIVIDER  (2)              */
	// {0x51, 0x90}, /* H_SIZE[7:0] (real/4)     (144)  (576)  */
	// {0x52, 0x2c}, /* V_SIZE[7:0] (real/4)     (44)   (176)  */
	// // {0x51, 0x20}, // H size [7:0] bits     (32) 
	// // {0x52, 0x20}, // V size [7:0] bits     (32)
	// {0x53, 0x00},  /* OFFSET_X[7:0]       (0)               */
	// {0x54, 0x00},  /* OFFSET_Y[7:0]       (0)               */
	// {0x55, 0x88},  /* Bit[7]: V_SIZE[8] (real/4) (1 -> 1024)
    //                   Bit[6:4]: OFFSET_Y[10:8]   (0)
    //                   Bit[3]: H_SIZE[8] (real/4) (1 -> 1024)
    //                   Bit[2:0]: OFFSET_X[10:8]   (0)        */
	// {0x57, 0x00},  /* Bit[7]: H_SIZE[9] (0 -> 0)            */
	// // {0x5a, 0x58},  /* OUTW[7:0] (real/4)  (88) (352)        */
	// // {0x5b, 0x48},  /* OUTH[7:0] (real/4)  (72) (288)        */
	// {0x5a, 0xA0},  /* OUTW[7:0] (real/4)  (88) (352)  (512)      */
	// {0x5b, 0x78},  /* OUTH[7:0] (real/4)  (72) (288)  (384)      */
	// {0x5c, 0x00},  /* Bit[7:4]: ZMSPD (zoom speed)
    //                   Bit[2]: OUTH[8]      (0 -> 0)
    //                   Bit[1:0]: OUTW[9:8]  (0 -> 0)         */
	// {0xd3, 0x08},  /* DVP output speed control:
	//                   Bit[7]: Auto mode
	// 				  Bit[6:0]: DVP output speed control (8)    
	// 				      DVP PCLK = sysclk (48)/[6:0] (YUV0) 
	// 					           = sysclk (48)/2*[6:0]) (RAW) */
	// {0xe0, 0x00}   /* Reset, none bits                      */
};   

#endif
