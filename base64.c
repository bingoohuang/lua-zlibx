// https://github.com/sniperHW/chuck/blob/master/src/luabind/base64.h#L22
// https://github.com/sniperHW/chuck/blob/master/src/util/base64.c#L47

#define base64_encoded_length(len) (((len + 2) / 3) * 4)
#define base64_decoded_length(len) (((len + 3) / 4) * 3)

static int base64_encode_internal(unsigned char *dst, const unsigned char *src, int len, const unsigned char *basis,
                                  int padding)
{
    unsigned char *d;
    const unsigned char *s;
    s = src;
    d = dst;

    while(len > 2) {
        *d++ = basis[(s[0] >> 2) & 0x3f];
        *d++ = basis[((s[0] & 3) << 4) | (s[1] >> 4)];
        *d++ = basis[((s[1] & 0x0f) << 2) | (s[2] >> 6)];
        *d++ = basis[s[2] & 0x3f];
        s += 3;
        len -= 3;
    }

    if(len) {
        *d++ = basis[(s[0] >> 2) & 0x3f];

        if(len == 1) {
            *d++ = basis[(s[0] & 3) << 4];
            *d++ = '=';

        } else {
            *d++ = basis[((s[0] & 3) << 4) | (s[1] >> 4)];
            *d++ = basis[(s[1] & 0x0f) << 2];
        }

        if(padding) {
            *d++ = '=';
        }
    }

    return d - dst;
}

int base64_encode(unsigned char *dst, const unsigned char *src, int len)
{
    static unsigned char basis64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    return base64_encode_internal(dst, src, len, basis64, 1);
}

int base64_encode_url(unsigned char *dst, const unsigned char *src, int len)
{
    static unsigned char basis64_url[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

    return base64_encode_internal(dst, src, len, basis64_url, 0);
}

static int base64_decode_internal(unsigned char *dst, const unsigned char *src, size_t slen, const unsigned char *basis)
{
    size_t len;
    unsigned char *d;
    const unsigned char *s;

    for(len = 0; len < slen; len++) {
        if(src[len] == '=') {
            break;
        }

        if(basis[src[len]] == 77) {
            return 0;
        }
    }

    if(len % 4 == 1) {
        return 0;
    }

    s = src;
    d = dst;

    while(len > 3) {
        *d++ = (char)(basis[s[0]] << 2 | basis[s[1]] >> 4);
        *d++ = (char)(basis[s[1]] << 4 | basis[s[2]] >> 2);
        *d++ = (char)(basis[s[2]] << 6 | basis[s[3]]);
        s += 4;
        len -= 4;
    }

    if(len > 1) {
        *d++ = (char)(basis[s[0]] << 2 | basis[s[1]] >> 4);
    }

    if(len > 2) {
        *d++ = (char)(basis[s[1]] << 4 | basis[s[2]] >> 2);
    }

    return d - dst;
}

int base64_decode(unsigned char *dst, const unsigned char *src, size_t slen)
{
    static unsigned char basis64[] = {
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 62, 77, 77, 77, 63,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 77, 77, 77, 77, 77, 77,
        77, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 77, 77, 77, 77, 77,
        77, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 77, 77, 77, 77, 77,

        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77
    };
    return base64_decode_internal(dst, src, slen, basis64);
}

int base64_decode_url(unsigned char *dst, const unsigned char *src, size_t slen)
{
    static unsigned char basis64[] = {
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 62, 77, 77,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 77, 77, 77, 77, 77, 77,
        77, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 77, 77, 77, 77, 63,
        77, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 77, 77, 77, 77, 77,

        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77
    };
    return base64_decode_internal(dst, src, slen, basis64);
}

int lua_f_base64_encode(lua_State *L)
{
    const unsigned char *src = NULL;
    size_t slen = 0;

    if(lua_isnil(L, 1)) {
        src = (const unsigned char *) "";

    } else {
        src = (const unsigned char *) luaL_checklstring(L, 1, &slen);
    }

    unsigned char *end = malloc(base64_encoded_length(slen));
    int nlen = base64_encode(end, src, slen);
    lua_pushlstring(L, (char *) end, nlen);
    free(end);
    return 1;
}

int lua_f_base64_decode(lua_State *L)
{
    const unsigned char *src = NULL;
    size_t slen = 0;

    if(lua_isnil(L, 1)) {
        src = (const unsigned char *) "";

    } else {
        src = (unsigned char *) luaL_checklstring(L, 1, &slen);
    }

    unsigned char *end = malloc(base64_decoded_length(slen));
    int nlen = base64_decode(end, src, slen);
    lua_pushlstring(L, (char *) end, nlen);
    free(end);
    return 1;
}

int lua_f_base64_encode_url(lua_State *L)
{
    const unsigned char *src = NULL;
    size_t slen = 0;

    if(lua_isnil(L, 1)) {
        src = (const unsigned char *) "";

    } else {
        src = (const unsigned char *) luaL_checklstring(L, 1, &slen);
    }

    unsigned char *end = malloc(base64_encoded_length(slen));
    int nlen = base64_encode_url(end, src, slen);
    lua_pushlstring(L, (char *) end, nlen);
    free(end);
    return 1;
}

int lua_f_base64_decode_url(lua_State *L)
{
    const unsigned char *src = NULL;
    size_t slen = 0;

    if(lua_isnil(L, 1)) {
        src = (const unsigned char *) "";

    } else {
        src = (unsigned char *) luaL_checklstring(L, 1, &slen);
    }

    unsigned char *end = malloc(base64_decoded_length(slen));
    int nlen = base64_decode_url(end, src, slen);
    lua_pushlstring(L, (char *) end, nlen);
    free(end);
    return 1;
}
