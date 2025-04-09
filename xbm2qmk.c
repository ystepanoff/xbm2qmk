/*
 * A simple utility to convert a standard XBM file (which has #defines for width and height,
 * then a static array of hex bytes storing row-packed data) into a QMK-friendly array of bytes
 * where each byte represents 8 vertical pixels (i.e., page-packed).
 *
 * Usage:
 *   ./xbm2qmk input.xbm > output.c
 *
 * Note: this code does minimal error checking and assumes the input is well-formed.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void xbm_to_page_packed(unsigned char *dst, const unsigned char *src, int width, int height) {
    /*
     * XBM standard: each byte = 8 horizontal pixels, left-to-right,
     * then the next byte continues that same row, until 'width' bits are covered.
     * Then we move to the next row, etc. So the total size = (width * height) / 8 bytes.
     *
     * QMK/SSD1306 page-packed: each byte = 8 vertical pixels for a single column. We have
     * width columns, and each column is split into (height/8) "pages". Typically for 128x32,
     * we have 128 columns and 4 pages. The total size is again (width * height)/8 bytes,
     * but the arrangement is different.
     *
     * We'll read bits from 'src' row by row, and write them into 'dst' page by page.
     */
    int bytes_per_row = width / 8;
    int pages = height / 8;
    int total_bytes = bytes_per_row * height;

    for(int i = 0; i < total_bytes; i++){
        dst[i] = 0x00;
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int src_byte_index = (y * bytes_per_row) + (x / 8);
            int src_bit = x % 8;
            unsigned char pixel_on = (src[src_byte_index] >> src_bit) & 1U;

            int page = y / 8;
            int bit_within_page = y % 8;
            int dst_byte_index = (page * width) + x;
            if (pixel_on) {
                dst[dst_byte_index] |= (1 << bit_within_page);
            }
        }
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s input.xbm\n", argv[0]);
        return 1;
    }

    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        perror("fopen");
        return 1;
    }

    int width = 0, height = 0;
    char line[1024];
    char name[256] = {0};

    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "#define %[^_]_width %d", name, &width) == 2) {
            // found the width
        }
        else if (sscanf(line, "#define %*[^_]_height %d", &height) == 1) {
            // found the height
        }
        else if (strstr(line, "static") && strstr(line, "[]")) {
            // done reading definitions
            break;
        }
    }

    if (width == 0 || height == 0) {
        fprintf(stderr, "Failed to find width or height in %s\n", argv[1]);
        fclose(fp);
        return 1;
    }

    int total_bytes = (width * height) / 8;
    unsigned char *row_packed_data = (unsigned char*)calloc(total_bytes, 1);
    if (!row_packed_data) {
        fprintf(stderr, "Memory allocation failed.\n");
        fclose(fp);
        return 1;
    }

    int count = 0;
    while (fgets(line, sizeof(line), fp) && count < total_bytes) {
        char *p = line;
        while (*p && count < total_bytes) {
            while (*p && isspace((unsigned char)*p)) {
                p++;
            }
            if (!*p) break;

            if (*p == '0' && *(p+1) == 'x') {
                unsigned int val;
                if (sscanf(p, "0x%x", &val) == 1) {
                    row_packed_data[count++] = (unsigned char)val;
                }
            }
            while (*p && *p != ',' && *p != '}') {
                p++;
            }
            if (*p == ',') p++;
        }

        if (strstr(line, "};")) {
            break;
        }
    }

    fclose(fp);

    if (count < total_bytes) {
        fprintf(stderr, "Warning: did not read enough bytes (%d of %d)\n", count, total_bytes);
    }

    unsigned char *page_packed_data = (unsigned char*)calloc(total_bytes, 1);
    if (!page_packed_data) {
        fprintf(stderr, "Memory allocation failed.\n");
        free(row_packed_data);
        return 1;
    }

    xbm_to_page_packed(page_packed_data, row_packed_data, width, height);
    free(row_packed_data);

    printf("const char PROGMEM %s_qmk[] = {\n", name);

    for (int i = 0; i < total_bytes; i++) {
        if ((i % 8) == 0) {
            printf("    ");
        }
        printf("0x%02X", page_packed_data[i]);
        if (i < total_bytes - 1) {
            printf(",");
        } else {
            printf(" ");
        }
        if ((i % 8) == 7) {
            printf("\n");
        } else {
            printf(" ");
        }
    }
    printf("};\n\n");

    free(page_packed_data);

    return 0;
}
