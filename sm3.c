
#include <string.h>
#include <stdint.h>

#define SM3_DIGEST_SIZE 32
#define SM3_BLOCK_SIZE 64
#define SM3_STATE_WORDS 8

typedef struct
{
	uint32_t digest[SM3_STATE_WORDS];
	uint64_t nblocks;
	uint8_t block[SM3_BLOCK_SIZE];
	size_t num;
} SM3_CTX;

void sm3_init(SM3_CTX *ctx);
void sm3_update(SM3_CTX *ctx, const uint8_t *data, size_t datalen);
void sm3_finish(SM3_CTX *ctx, uint8_t dgst[SM3_DIGEST_SIZE]);
void sm3_digest(const uint8_t *data, size_t datalen, uint8_t dgst[SM3_DIGEST_SIZE]);

void sm3_compress_blocks(uint32_t digest[8], const uint8_t *data, size_t blocks);

#define SM3_HMAC_SIZE (SM3_DIGEST_SIZE)

typedef struct
{
	SM3_CTX sm3_ctx;
	uint8_t key[SM3_BLOCK_SIZE];
} SM3_HMAC_CTX;

void sm3_hmac_init(SM3_HMAC_CTX *ctx, const uint8_t *key, size_t keylen);
void sm3_hmac_update(SM3_HMAC_CTX *ctx, const uint8_t *data, size_t datalen);
void sm3_hmac_finish(SM3_HMAC_CTX *ctx, uint8_t mac[SM3_HMAC_SIZE]);
void sm3_hmac(const uint8_t *key, size_t keylen,
			  const uint8_t *data, size_t datalen,
			  uint8_t mac[SM3_HMAC_SIZE]);

#define GETU32(ptr)             \
	((uint32_t)(ptr)[0] << 24 | \
	 (uint32_t)(ptr)[1] << 16 | \
	 (uint32_t)(ptr)[2] << 8 |  \
	 (uint32_t)(ptr)[3])

#define PUTU32(ptr, a)                \
	((ptr)[0] = (uint8_t)((a) >> 24), \
	 (ptr)[1] = (uint8_t)((a) >> 16), \
	 (ptr)[2] = (uint8_t)((a) >> 8),  \
	 (ptr)[3] = (uint8_t)(a))

#define ROTL(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

#define P0(x) ((x) ^ ROTL((x), 9) ^ ROTL((x), 17))
#define P1(x) ((x) ^ ROTL((x), 15) ^ ROTL((x), 23))

#define FF00(x, y, z) ((x) ^ (y) ^ (z))
#define FF16(x, y, z) (((x) & (y)) | ((x) & (z)) | ((y) & (z)))
#define GG00(x, y, z) ((x) ^ (y) ^ (z))
#define GG16(x, y, z) ((((y) ^ (z)) & (x)) ^ (z))

static uint32_t K[64] = {
	0x79cc4519U,
	0xf3988a32U,
	0xe7311465U,
	0xce6228cbU,
	0x9cc45197U,
	0x3988a32fU,
	0x7311465eU,
	0xe6228cbcU,
	0xcc451979U,
	0x988a32f3U,
	0x311465e7U,
	0x6228cbceU,
	0xc451979cU,
	0x88a32f39U,
	0x11465e73U,
	0x228cbce6U,
	0x9d8a7a87U,
	0x3b14f50fU,
	0x7629ea1eU,
	0xec53d43cU,
	0xd8a7a879U,
	0xb14f50f3U,
	0x629ea1e7U,
	0xc53d43ceU,
	0x8a7a879dU,
	0x14f50f3bU,
	0x29ea1e76U,
	0x53d43cecU,
	0xa7a879d8U,
	0x4f50f3b1U,
	0x9ea1e762U,
	0x3d43cec5U,
	0x7a879d8aU,
	0xf50f3b14U,
	0xea1e7629U,
	0xd43cec53U,
	0xa879d8a7U,
	0x50f3b14fU,
	0xa1e7629eU,
	0x43cec53dU,
	0x879d8a7aU,
	0x0f3b14f5U,
	0x1e7629eaU,
	0x3cec53d4U,
	0x79d8a7a8U,
	0xf3b14f50U,
	0xe7629ea1U,
	0xcec53d43U,
	0x9d8a7a87U,
	0x3b14f50fU,
	0x7629ea1eU,
	0xec53d43cU,
	0xd8a7a879U,
	0xb14f50f3U,
	0x629ea1e7U,
	0xc53d43ceU,
	0x8a7a879dU,
	0x14f50f3bU,
	0x29ea1e76U,
	0x53d43cecU,
	0xa7a879d8U,
	0x4f50f3b1U,
	0x9ea1e762U,
	0x3d43cec5U,
};

void sm3_compress_blocks(uint32_t digest[8], const uint8_t *data, size_t blocks)
{
	uint32_t A;
	uint32_t B;
	uint32_t C;
	uint32_t D;
	uint32_t E;
	uint32_t F;
	uint32_t G;
	uint32_t H;
	uint32_t W[68];
	uint32_t SS1, SS2, TT1, TT2;
	int j;

	while (blocks--)
	{

		A = digest[0];
		B = digest[1];
		C = digest[2];
		D = digest[3];
		E = digest[4];
		F = digest[5];
		G = digest[6];
		H = digest[7];

		for (j = 0; j < 16; j++)
		{
			W[j] = GETU32(data + j * 4);
		}

		for (; j < 68; j++)
		{
			W[j] = P1(W[j - 16] ^ W[j - 9] ^ ROTL(W[j - 3], 15)) ^ ROTL(W[j - 13], 7) ^ W[j - 6];
		}

		for (j = 0; j < 16; j++)
		{
			SS1 = ROTL((ROTL(A, 12) + E + K[j]), 7);
			SS2 = SS1 ^ ROTL(A, 12);
			TT1 = FF00(A, B, C) + D + SS2 + (W[j] ^ W[j + 4]);
			TT2 = GG00(E, F, G) + H + SS1 + W[j];
			D = C;
			C = ROTL(B, 9);
			B = A;
			A = TT1;
			H = G;
			G = ROTL(F, 19);
			F = E;
			E = P0(TT2);
		}

		for (; j < 64; j++)
		{
			SS1 = ROTL((ROTL(A, 12) + E + K[j]), 7);
			SS2 = SS1 ^ ROTL(A, 12);
			TT1 = FF16(A, B, C) + D + SS2 + (W[j] ^ W[j + 4]);
			TT2 = GG16(E, F, G) + H + SS1 + W[j];
			D = C;
			C = ROTL(B, 9);
			B = A;
			A = TT1;
			H = G;
			G = ROTL(F, 19);
			F = E;
			E = P0(TT2);
		}

		digest[0] ^= A;
		digest[1] ^= B;
		digest[2] ^= C;
		digest[3] ^= D;
		digest[4] ^= E;
		digest[5] ^= F;
		digest[6] ^= G;
		digest[7] ^= H;

		data += 64;
	}
}

void sm3_init(SM3_CTX *ctx)
{
	memset(ctx, 0, sizeof(*ctx));
	ctx->digest[0] = 0x7380166F;
	ctx->digest[1] = 0x4914B2B9;
	ctx->digest[2] = 0x172442D7;
	ctx->digest[3] = 0xDA8A0600;
	ctx->digest[4] = 0xA96F30BC;
	ctx->digest[5] = 0x163138AA;
	ctx->digest[6] = 0xE38DEE4D;
	ctx->digest[7] = 0xB0FB0E4E;
}

void sm3_update(SM3_CTX *ctx, const uint8_t *data, size_t data_len)
{
	size_t blocks;

	ctx->num &= 0x3f;
	if (ctx->num)
	{
		size_t left = SM3_BLOCK_SIZE - ctx->num;
		if (data_len < left)
		{
			memcpy(ctx->block + ctx->num, data, data_len);
			ctx->num += data_len;
			return;
		}
		else
		{
			memcpy(ctx->block + ctx->num, data, left);
			sm3_compress_blocks(ctx->digest, ctx->block, 1);
			ctx->nblocks++;
			data += left;
			data_len -= left;
		}
	}

	blocks = data_len / SM3_BLOCK_SIZE;
	if (blocks)
	{
		sm3_compress_blocks(ctx->digest, data, blocks);
		ctx->nblocks += blocks;
		data += SM3_BLOCK_SIZE * blocks;
		data_len -= SM3_BLOCK_SIZE * blocks;
	}

	ctx->num = data_len;
	if (data_len)
	{
		memcpy(ctx->block, data, data_len);
	}
}

void sm3_finish(SM3_CTX *ctx, uint8_t *digest)
{
	int i;

	ctx->num &= 0x3f;
	ctx->block[ctx->num] = 0x80;

	if (ctx->num <= SM3_BLOCK_SIZE - 9)
	{
		memset(ctx->block + ctx->num + 1, 0, SM3_BLOCK_SIZE - ctx->num - 9);
	}
	else
	{
		memset(ctx->block + ctx->num + 1, 0, SM3_BLOCK_SIZE - ctx->num - 1);
		sm3_compress_blocks(ctx->digest, ctx->block, 1);
		memset(ctx->block, 0, SM3_BLOCK_SIZE - 8);
	}

	PUTU32(ctx->block + 56, ctx->nblocks >> 23);
	PUTU32(ctx->block + 60, (ctx->nblocks << 9) + (ctx->num << 3));
	sm3_compress_blocks(ctx->digest, ctx->block, 1);

	for (i = 0; i < 8; i++)
	{
		PUTU32(digest + i * 4, ctx->digest[i]);
	}
}

void sm3_digest(const uint8_t *msg, size_t msglen,
				uint8_t dgst[SM3_DIGEST_SIZE])
{
	SM3_CTX ctx;
	sm3_init(&ctx);
	sm3_update(&ctx, msg, msglen);
	sm3_finish(&ctx, dgst);
	memset(&ctx, 0, sizeof(ctx));
}

/**
 * HMAC_k(m) = H((k ^ opad) || H((k ^ ipad) || m))
 * pseudo-code:
 * function hmac(key, message)
 *	opad = [0x5c * blocksize]
 *	ipad = [0x36 * blocksize]
 *	if (length(key) > blocksize) then
 *		key = hash(key)
 *	end if
 *	for i from 0 to length(key) - 1 step 1
 *		ipad[i] = ipad[i] XOR key[i]
 *		opad[i] = opad[i] XOR key[i]
 *	end for
 *	return hash(opad || hash(ipad || message))
 * end function
 */

#define IPAD 0x36
#define OPAD 0x5C

void sm3_hmac_init(SM3_HMAC_CTX *ctx, const uint8_t *key, size_t key_len)
{
	int i;

	if (key_len <= SM3_BLOCK_SIZE)
	{
		memcpy(ctx->key, key, key_len);
		memset(ctx->key + key_len, 0, SM3_BLOCK_SIZE - key_len);
	}
	else
	{
		sm3_init(&ctx->sm3_ctx);
		sm3_update(&ctx->sm3_ctx, key, key_len);
		sm3_finish(&ctx->sm3_ctx, ctx->key);
		memset(ctx->key + SM3_DIGEST_SIZE, 0,
			   SM3_BLOCK_SIZE - SM3_DIGEST_SIZE);
	}
	for (i = 0; i < SM3_BLOCK_SIZE; i++)
	{
		ctx->key[i] ^= IPAD;
	}

	sm3_init(&ctx->sm3_ctx);
	sm3_update(&ctx->sm3_ctx, ctx->key, SM3_BLOCK_SIZE);
}

void sm3_hmac_update(SM3_HMAC_CTX *ctx, const uint8_t *data, size_t data_len)
{
	sm3_update(&ctx->sm3_ctx, data, data_len);
}

void sm3_hmac_finish(SM3_HMAC_CTX *ctx, uint8_t mac[SM3_HMAC_SIZE])
{
	int i;
	for (i = 0; i < SM3_BLOCK_SIZE; i++)
	{
		ctx->key[i] ^= (IPAD ^ OPAD);
	}
	sm3_finish(&ctx->sm3_ctx, mac);
	sm3_init(&ctx->sm3_ctx);
	sm3_update(&ctx->sm3_ctx, ctx->key, SM3_BLOCK_SIZE);
	sm3_update(&ctx->sm3_ctx, mac, SM3_DIGEST_SIZE);
	sm3_finish(&ctx->sm3_ctx, mac);
}

void sm3_hmac(const uint8_t *key, size_t key_len,
			  const uint8_t *data, size_t data_len,
			  uint8_t mac[SM3_HMAC_SIZE])
{
	SM3_HMAC_CTX ctx;
	sm3_hmac_init(&ctx, key, key_len);
	sm3_hmac_update(&ctx, data, data_len);
	sm3_hmac_finish(&ctx, mac);
	memset(&ctx, 0, sizeof(ctx));
}

#define SM3_DIGEST_LENGTH 32

/*
sm3算法,算HASH值
@api sm.sm3(data)
@string 待计算的数据,必选
@return string 对应的hash值
@usage
local encodeStr = gmssl.sm3("lqlq666lqlq946")
log.info("testsm.sm3update",string.toHex(encodeStr))
*/
static int l_sm3_update(lua_State *L)
{
	size_t inputLen = 0;
	uint8_t dgst[SM3_DIGEST_LENGTH];
	const char *inputData = lua_tolstring(L, 1, &inputLen);
	sm3_digest((uint8_t *)inputData, inputLen, dgst);

	lua_pushlstring(L, (char *)dgst, SM3_DIGEST_LENGTH);
	return 1;
}

#define large_malloc(s) (malloc(((int)(s / 4096) + 1) * 4096))

static int l_sm3_update_base64(lua_State *L)
{
	size_t inputLen = 0;
	uint8_t dgst[SM3_DIGEST_LENGTH];
	const char *inputData = lua_tolstring(L, 1, &inputLen);
	sm3_digest((uint8_t *)inputData, inputLen, dgst);

	size_t slen = SM3_DIGEST_LENGTH;
	unsigned char *end = large_malloc(base64_encoded_length(slen));
	int nlen = base64_encode(end, dgst, slen);
	lua_pushlstring(L, (char *)end, nlen);
	free(end);

	return 1;
}

static int l_sm3_update_base64_url(lua_State *L)
{
	size_t inputLen = 0;
	uint8_t dgst[SM3_DIGEST_LENGTH];
	const char *inputData = lua_tolstring(L, 1, &inputLen);
	sm3_digest((uint8_t *)inputData, inputLen, dgst);

	size_t slen = SM3_DIGEST_LENGTH;
	unsigned char *end = large_malloc(base64_encoded_length(slen));
	int nlen = base64_encode_url(end, dgst, slen);
	lua_pushlstring(L, (char *)end, nlen);
	free(end);

	return 1;
}

/*
sm3算法,算HASH值,但带HMAC
@api sm.sm3hmac(data, key)
@string 待计算的数据,必选
@string 密钥
@return string 对应的hash值
@usage
local encodeStr = gmssl.sm3hmac("lqlq666lqlq946", "123")
log.info("testsm.sm3update",string.toHex(encodeStr))
*/
static int l_sm3hmac_update(lua_State *L)
{
	size_t inputLen = 0;
	size_t keyLen = 0;
	uint8_t dgst[SM3_DIGEST_LENGTH];
	const char *inputData = lua_tolstring(L, 1, &inputLen);
	const char *keyData = lua_tolstring(L, 2, &keyLen);
	sm3_hmac((uint8_t *)keyData, keyLen, (uint8_t *)inputData, inputLen, dgst);

	lua_pushlstring(L, (char *)dgst, SM3_DIGEST_LENGTH);
	return 1;
}

static int l_sm3hmac_update_base64(lua_State *L)
{
	size_t inputLen = 0;
	size_t keyLen = 0;
	uint8_t dgst[SM3_DIGEST_LENGTH];
	const char *inputData = lua_tolstring(L, 1, &inputLen);
	const char *keyData = lua_tolstring(L, 2, &keyLen);
	sm3_hmac((uint8_t *)keyData, keyLen, (uint8_t *)inputData, inputLen, dgst);

	size_t slen = SM3_DIGEST_LENGTH;
	unsigned char *end = large_malloc(base64_encoded_length(slen));
	int nlen = base64_encode(end, dgst, slen);
	lua_pushlstring(L, (char *)end, nlen);
	free(end);
	return 1;
}

static int l_sm3hmac_update_base64_url(lua_State *L)
{
	size_t inputLen = 0;
	size_t keyLen = 0;
	uint8_t dgst[SM3_DIGEST_LENGTH];
	const char *inputData = lua_tolstring(L, 1, &inputLen);
	const char *keyData = lua_tolstring(L, 2, &keyLen);
	sm3_hmac((uint8_t *)keyData, keyLen, (uint8_t *)inputData, inputLen, dgst);

	size_t slen = SM3_DIGEST_LENGTH;
	unsigned char *end = large_malloc(base64_encoded_length(slen));
	int nlen = base64_encode_url(end, dgst, slen);
	lua_pushlstring(L, (char *)end, nlen);
	free(end);
	return 1;
}
