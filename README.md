![GitHub License](https://img.shields.io/github/license/ystepanoff/xbm2qmk?style=plastic)

# xbm2qmk

**xbm2qmk** is a command-line tool that reads a standard [XBM](https://en.wikipedia.org/wiki/X_BitMap) file, extracts the image data, 
and converts it into a page-packed bitmap format ready to display on SSD1306-based OLED screens (such as those used in 
[QMK Firmware](https://github.com/qmk/qmk_firmware)). 


## Features
- Supports typical 1-bit depth XBM files of various dimensions (e.g., 128×32, 128×64).
- Uses standard C libraries, making it simple to compile on most systems.

## Usage

1. Clone or download this repository.  
2. Open a terminal and navigate to the repository folder.  
3. Compile the tool:
```bash
gcc xbm2qmk.c -o xbm2qmk
```
or
```bash
clang xbm2qmk.c -o xbm2qmk
```
4. Run with
```bash 
./xbm2qmk /path/to/image.xbm > image_qmk.c
```
This generates a C source file containing a PROGMEM array (named after the XBM’s identifier) that you can include in your QMK firmware project.


## Incorporating into QMK

Include the generated .c file in your QMK keymap source. For example, if the output array is named `myimage_qmk`:
```c 
#include "myimage_qmk.c"

bool oled_task_user(void) {
    oled_write_raw_P(myimage_qmk, sizeof(myimage_qmk));
    return false;
}
```

Make sure that:
- Your board is configured for an OLED display of the same width/height as your bitmap.
- You have enabled `OLED_DRIVER_ENABLE = yes` in your `rules.mk`.

## Happy converting! 

Have fun displaying crisp, custom images on your QMK-powered mechanical keyboards. 
If you have any questions or suggestions, feel free to open an issue or create a PR!

## Credits 

The example XBM image was found in [this repo](https://github.com/tie/oneko/). 
