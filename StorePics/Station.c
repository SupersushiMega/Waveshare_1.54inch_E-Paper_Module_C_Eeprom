
/*******************************************************************************
* image
* filename: unsaved
* name: Station
*
* preset name: Monochrome
* data block size: 8 bit(s), uint8_t
* RLE compression enabled: no
* conversion type: Monochrome, Edge 206
* bits per pixel: 1
*
* preprocess:
*  main scan direction: top_to_bottom
*  line scan direction: forward
*  inverse: no
*******************************************************************************/

/*
 typedef struct {
     const uint8_t *data;
     uint16_t width;
     uint16_t height;
     uint8_t dataSize;
     } tImage;
*/
#include <stdint.h>



static const uint8_t image_data_Station[32] = {
    0x80, 0x01, 
    0x7b, 0xc0, 
    0x77, 0xe0, 
    0x6f, 0xf0, 
    0x5f, 0xf8, 
    0x3f, 0xfc, 
    0x78, 0x1e, 
    0x77, 0xee, 
    0x77, 0xee, 
    0x78, 0x1e, 
    0x3f, 0xfc, 
    0x5f, 0xf8, 
    0x6f, 0xf0, 
    0x77, 0xe0, 
    0x7b, 0xc0, 
    0x80, 0x01
};
const tImage Station = { image_data_Station, 16, 16,
    8 };

