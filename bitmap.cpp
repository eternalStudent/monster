// returns 1 if the product is valid, 0 on overflow.
// negative factors are considered invalid.
int32 mul2sizes_valid(int32 a, int32 b){
    if (a < 0 || b < 0) return 0;
    if (b == 0) return 1; // mul-by-0 is always safe
    // portable way to check for no overflows in a*b
    return a <= INT32_MAX / b;
}

// returns 1 if "a*b*c + add" has no negative terms/factors and doesn't overflow
int32 mad3sizes_valid(int32 a, int32 b, int32 c){
    return mul2sizes_valid(a, b) && mul2sizes_valid(a * b, c) &&
        a * b * c <= INT32_MAX;
}

void* malloc_mad3(int32 a, int32 b, int32 c){
    if (!mad3sizes_valid(a, b, c)) return NULL;
    return Alloc(a * b * c);
}

//------------------
// Bit Operations 
//------------------

// returns 0..31 for the highest set bit
// I can use intrinsic BitScanReverse
int32 high_bit(uint32 z) {
    int32 n = 0;
    if (z == 0) return -1;
    if (z >= 0x10000) { n += 16; z >>= 16; }
    if (z >= 0x00100) { n += 8; z >>= 8; }
    if (z >= 0x00010) { n += 4; z >>= 4; }
    if (z >= 0x00004) { n += 2; z >>= 2; }
    if (z >= 0x00002) { n += 1;/* >>=  1;*/ }
    return n;
}

// I can use intrinsic popcnt
int32 bitcount(uint32 a) {
    a = (a & 0x55555555) + ((a >> 1) & 0x55555555); // max 2
    a = (a & 0x33333333) + ((a >> 2) & 0x33333333); // max 4
    a = (a + (a >> 4)) & 0x0f0f0f0f; // max 8 per 4, now 8 bits
    a = (a + (a >> 8)); // max 16 per 8 bits
    a = (a + (a >> 16)); // max 32 per 8 bits
    return a & 0xff;
}

#define BYTECAST(x)  ((byte) ((x) & 255))  // truncate int32 to byte without warnings

// extract an arbitrarily-aligned N-bit value (N=bits)
// from v, and then make it 8-bits long and fractionally
// extend it to full full range.
int32 shiftsigned(uint32 v, int32 shift, int32 bits) {
    static uint32 mul_table[9] = {
       0,
       0xff/*0b11111111*/, 0x55/*0b01010101*/, 0x49/*0b01001001*/, 0x11/*0b00010001*/,
       0x21/*0b00100001*/, 0x41/*0b01000001*/, 0x81/*0b10000001*/, 0x01/*0b00000001*/,
    };
    static int32 shift_table[9] = {
       0, 0,0,1,0,2,4,6,0,
    };
    if (shift < 0)
        v <<= -shift;
    else
        v >>= shift;
    Assert(v < 256);
    v >>= (8 - bits);
    Assert(bits >= 0 && bits <= 8);
    return (int32)(v * mul_table[bits]) >> shift_table[bits];
}

#define Skip(data, distance)    *(data)+=(distance)
#define ReadByte(data)          *(*data)++

int32 ReadUint16LittleEndian(byte** data) {
    int32 z = ReadByte(data);
    return z + (ReadByte(data) << 8);
}

uint32 ReadUint32LittleEndian(byte** data) {
    uint32 z = ReadUint16LittleEndian(data);
    return z + (ReadUint16LittleEndian(data) << 16);
}

// ------------
// end of utils
// ------------

struct BMPHeader {
    int32 bpp, offset, hsz;
    uint32 mr, mg, mb, ma, all_a;
    int32 extra_read;
};

int32 ParseBMPHeader(byte** data, BMPHeader* info, Image* image){
    int32 hsz;
    if (ReadByte(data) != 'B' || ReadByte(data) != 'M') return 0;
    ReadUint32LittleEndian(data); // discard filesize
    ReadUint16LittleEndian(data); // discard reserved
    ReadUint16LittleEndian(data); // discard reserved
    info->offset = ReadUint32LittleEndian(data);
    info->hsz = hsz = ReadUint32LittleEndian(data); //size of info header = 40
    info->mr = info->mg = info->mb = info->ma = 0;
    info->extra_read = 14;

    if (info->offset < 0) return 0;

    if (hsz != 12 && hsz != 40 && hsz != 56 && hsz != 108 && hsz != 124) return 0;
    if (hsz == 12) {
        image->width = ReadUint16LittleEndian(data);
        image->height = ReadUint16LittleEndian(data);
    }
    else {
        image->width = ReadUint32LittleEndian(data);
        image->height = ReadUint32LittleEndian(data);
    }
    if (ReadUint16LittleEndian(data) != 1) return 0; //planes
    info->bpp = ReadUint16LittleEndian(data); //bits per pixel
    if (hsz != 12) {
        int32 compress = ReadUint32LittleEndian(data);
        if (compress == 1 || compress == 2) return 0; //not supported
        ReadUint32LittleEndian(data); // discard sizeof
        ReadUint32LittleEndian(data); // discard hres
        ReadUint32LittleEndian(data); // discard vres
        ReadUint32LittleEndian(data); // discard colorsused
        ReadUint32LittleEndian(data); // discard max important
        if (hsz == 40 || hsz == 56) {
            if (hsz == 56) {
                ReadUint32LittleEndian(data);
                ReadUint32LittleEndian(data);
                ReadUint32LittleEndian(data);
                ReadUint32LittleEndian(data);
            }
            if (info->bpp == 16 || info->bpp == 32) {
                if (compress == 0) {
                    if (info->bpp == 32) {
                        info->mr = 0xffu << 16;
                        info->mg = 0xffu << 8;
                        info->mb = 0xffu << 0;
                        info->ma = 0xffu << 24;
                        info->all_a = 0; // if all_a is 0 at end, then we loaded alpha channel but it was all 0
                    }
                    else {
                        info->mr = 31u << 10;
                        info->mg = 31u << 5;
                        info->mb = 31u << 0;
                    }
                }
                else if (compress == 3) {
                    info->mr = ReadUint32LittleEndian(data);
                    info->mg = ReadUint32LittleEndian(data);
                    info->mb = ReadUint32LittleEndian(data);
                    info->extra_read += 12;
                    // not documented, but generated by photoshop and handled by mspaint
                    if (info->mr == info->mg && info->mg == info->mb) {
                        // ?!?!?
                        return 0;
                    }
                }
                else
                    return 0;
            }
        }
        else {
            int32 i;
            if (hsz != 108 && hsz != 124)
                return 0;
            info->mr = ReadUint32LittleEndian(data);
            info->mg = ReadUint32LittleEndian(data);
            info->mb = ReadUint32LittleEndian(data);
            info->ma = ReadUint32LittleEndian(data);
            ReadUint32LittleEndian(data); // discard color space
            for (i = 0; i < 12; ++i)
                ReadUint32LittleEndian(data); // discard color space parameters
            if (hsz == 124) {
                ReadUint32LittleEndian(data); // discard rendering intent
                ReadUint32LittleEndian(data); // discard offset of profile data
                ReadUint32LittleEndian(data); // discard size of profile data
                ReadUint32LittleEndian(data); // discard reserved
            }
        }
    }
    return 1;
}

#define MAX_DIMENSIONS (1 << 24)

Image LoadBMP(byte* data){
    byte pal[256][4];
    int32 psize = 0, i, j, width;
    int32 flip_vertically, pad;

    BMPHeader info;
    info.all_a = 255;
    Image image;
    if (!ParseBMPHeader(&data, &info, &image))
        return {};

    // TODO: probably accept that as an argument
    flip_vertically = (image.height) > 0;
    image.height = abs(image.height);

    if (image.height > MAX_DIMENSIONS) return {};
    if (image.width > MAX_DIMENSIONS) return {};

    if (info.hsz == 12) {
        if (info.bpp < 24)
            psize = (info.offset - info.extra_read - 24) / 3;
    }
    else {
        if (info.bpp < 16)
            psize = (info.offset - info.extra_read - info.hsz) >> 2;
    }

    if (info.bpp == 24 && info.ma == 0xff000000)
        image.channels = 3;
    else
        image.channels = info.ma ? 4 : 3;

    image.data = (byte*)malloc_mad3(image.channels, image.width, image.height);
    if (!image.data) return {};
    if (info.bpp < 16) {
        int32 z = 0;
        if (psize == 0 || psize > 256) { Free(image.data); return {}; }
        for (i = 0; i < psize; ++i) {
            pal[i][2] = ReadByte(&data);
            pal[i][1] = ReadByte(&data);
            pal[i][0] = ReadByte(&data);
            if (info.hsz != 12) ReadByte(&data);
            pal[i][3] = 255;
        }
        Skip(&data, info.offset - info.extra_read - info.hsz - psize * (info.hsz == 12 ? 3 : 4));
        if (info.bpp == 1) width = (image.width + 7) >> 3;
        else if (info.bpp == 4) width = (image.width + 1) >> 1;
        else if (info.bpp == 8) width = image.width;
        else { Free(image.data); return {}; }
        pad = (-width) & 3;
        if (info.bpp == 1) {
            for (j = 0; j < image.height; ++j) {
                int32 bit_offset = 7, v = ReadByte(&data);
                for (i = 0; i < image.width; ++i) {
                    int32 color = (v >> bit_offset) & 0x1;
                    image.data[z++] = pal[color][0];
                    image.data[z++] = pal[color][1];
                    image.data[z++] = pal[color][2];
                    if (image.channels == 4) image.data[z++] = 255;
                    if (i + 1 == image.width) break;
                    if ((--bit_offset) < 0) {
                        bit_offset = 7;
                        v = ReadByte(&data);
                    }
                }
                Skip(&data, pad);
            }
        }
        else {
            for (j = 0; j < image.height; ++j) {
                for (i = 0; i < image.width; i += 2) {
                    int32 v = ReadByte(&data), v2 = 0;
                    if (info.bpp == 4) {
                        v2 = v & 15;
                        v >>= 4;
                    }
                    image.data[z++] = pal[v][0];
                    image.data[z++] = pal[v][1];
                    image.data[z++] = pal[v][2];
                    if (image.channels == 4) image.data[z++] = 255;
                    if (i + 1 == image.width) break;
                    v = (info.bpp == 8) ? ReadByte(&data) : v2;
                    image.data[z++] = pal[v][0];
                    image.data[z++] = pal[v][1];
                    image.data[z++] = pal[v][2];
                    if (image.channels == 4) image.data[z++] = 255;
                }
                Skip(&data, pad);
            }
        }
    }
    else {
        int32 rshift = 0, gshift = 0, bshift = 0, ashift = 0, rcount = 0, gcount = 0, bcount = 0, acount = 0;
        int32 z = 0;
        int32 easy = 0;
        Skip(&data, info.offset - info.extra_read - info.hsz);
        if (info.bpp == 24) width = 3 * image.width;
        else if (info.bpp == 16) width = 2 * image.width;
        else /* bpp = 32 and pad = 0 */ width = 0;
        pad = (-width) & 3;
        if (info.bpp == 24) {
            easy = 1;
        }
        else if (info.bpp == 32) {
            if (info.mb == 0xff && info.mg == 0xff00 && info.mr == 0x00ff0000 && info.ma == 0xff000000)
                easy = 2;
        }
        if (!easy) {
            if (!info.mr || !info.mg || !info.mb) { Free(image.data); return {}; }
            // right shift amt to put high bit in position #7
            rshift = high_bit(info.mr) - 7; rcount = bitcount(info.mr);
            gshift = high_bit(info.mg) - 7; gcount = bitcount(info.mg);
            bshift = high_bit(info.mb) - 7; bcount = bitcount(info.mb);
            ashift = high_bit(info.ma) - 7; acount = bitcount(info.ma);
            if (rcount > 8 || gcount > 8 || bcount > 8 || acount > 8) { Free(image.data); return {}; }
        }
        for (j = 0; j < image.height; ++j) {
            if (easy) {
                for (i = 0; i < image.width; ++i) {
                    byte a;
                    image.data[z + 2] = ReadByte(&data);
                    image.data[z + 1] = ReadByte(&data);
                    image.data[z + 0] = ReadByte(&data);
                    z += 3;
                    a = (easy == 2 ? ReadByte(&data) : 255);
                    info.all_a |= a;
                    if (image.channels == 4) image.data[z++] = a;
                }
            }
            else {
                int32 bpp = info.bpp;
                for (i = 0; i < image.width; ++i) {
                    uint32 v = (bpp == 16 ? (uint32)ReadUint16LittleEndian(&data) : ReadUint32LittleEndian(&data));
                    uint32 a;
                    image.data[z++] = BYTECAST(shiftsigned(v & info.mr, rshift, rcount));
                    image.data[z++] = BYTECAST(shiftsigned(v & info.mg, gshift, gcount));
                    image.data[z++] = BYTECAST(shiftsigned(v & info.mb, bshift, bcount));
                    a = (info.ma ? shiftsigned(v & info.ma, ashift, acount) : 255);
                    info.all_a |= a;
                    if (image.channels == 4) image.data[z++] = BYTECAST(a);
                }
            }
            Skip(&data, pad);
        }
    }

    // if alpha channel is all 0s, replace with all 255s
    if (image.channels == 4 && info.all_a == 0)
        for (i = 4 * image.width * image.height - 1; i >= 0; i -= 4)
            image.data[i] = 255;

    if (flip_vertically) {
        byte t;
        for (j = 0; j < image.height >> 1; ++j) {
            byte* p1 = image.data + j * image.width * image.channels;
            byte* p2 = image.data + (image.height - 1 - j) * image.width * image.channels;
            for (i = 0; i < image.width * image.channels; ++i) {
                t = p1[i]; p1[i] = p2[i]; p2[i] = t;
            }
        }
    }

    return image;
}

byte* ExpandChannels(byte* buffer, int32 length, uint32 rgb) {
    uint32* new_buffer = (uint32*)Alloc(length*4);
    if (new_buffer == NULL) return NULL;
    for (int32 i = 0; i < length; i++) {
        new_buffer[i] = (buffer[i] << 24) | rgb;
    }
    return (byte*)new_buffer;
}

byte* ExpandChannels(byte* buffer, int32 length) {
    return ExpandChannels(buffer, length, 0x00ffffff);
}