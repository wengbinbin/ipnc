#include "alg_g711.h"


int ALG_ulawEncode(short *dst, short *src, short bufsize)
{

    unsigned short i;
    short data;
    unsigned short isNegative;
    short nOut;
    short lowByte = 1;
    int outputSize = bufsize / 2;

    for (i = 0; i < outputSize; i++) {
        data = *(src + i);
        data >>= 2;
        isNegative = (data < 0 ? 1 : 0);
        if (isNegative)
            data = -data;

        if (data <= 1) {
            nOut = (char) data;
        } else if (data <= 31) {
            nOut = ((data - 1) >> 1) + 1;
        } else if (data <= 95) {
            nOut = ((data - 31) >> 2) + 16;
        } else if (data <= 223) {
            nOut = ((data - 95) >> 3) + 32;
        } else if (data <= 479) {
            nOut = ((data - 223) >> 4) + 48;
        } else if (data <= 991) {
            nOut = ((data - 479) >> 5) + 64;
        } else if (data <= 2015) {
            nOut = ((data - 991) >> 6) + 80;
        } else if (data <= 4063) {
            nOut = ((data - 2015) >> 7) + 96;
        } else if (data <= 7903) {
            nOut = ((data - 4063) >> 8) + 112;
        } else {
            nOut = 127;
        }

        if (isNegative) {
            nOut = 127 - nOut;
        } else {
            nOut = 255 - nOut;
        }

        // Pack the bytes in a word
        if (lowByte)
            *(dst + (i >> 1)) = (nOut & 0x00FF);
        else
            *(dst + (i >> 1)) |= ((nOut << 8) & 0xFF00);
        lowByte ^= 0x1;
    }

		return (outputSize);

}


