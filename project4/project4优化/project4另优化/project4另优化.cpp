// project4优化.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <string>
#include <windows.h>
#include <ctime>
#include <immintrin.h>
#include <sstream>
using namespace std;

#define cpu_to_be32(v) (((v)>>24) | (((v)>>8)&0xff00) | (((v)<<8)&0xff0000) | ((v)<<24))
typedef struct {
	uint32_t digest[8];
	int nblocks;
	unsigned char block[64];
	int num;
} sm3_ctx_t;

string decToHex(int decimal) {
	if (decimal == 0) {
		return "0";
	}
	stringstream ss;
	ss << hex << decimal;
	string hex = ss.str();
	return hex;
}

#define P0(x) ((x) ^  ROTATELEFT((x),9)  ^ ROTATELEFT((x),17))
#define P1(x) ((x) ^  ROTATELEFT((x),15) ^ ROTATELEFT((x),23))
#define FF0(x,y,z) ( (x) ^ (y) ^ (z))
#define FF1(x,y,z) (((x) & (y)) | ( (x) & (z)) | ( (y) & (z)))
#define GG0(x,y,z) ( (x) ^ (y) ^ (z))
#define GG1(x,y,z) (((x) & (y)) | ( (~(x)) & (z)) )
#define ROTATELEFT(X,n)  (((X)<<(n)) | ((X)>>(32-(n))))

void sm3_compress(uint32_t digest[8], const unsigned char block[64]);

void sm3_init(sm3_ctx_t* ctx)
{
	ctx->digest[0] = 0x7380166F;
	ctx->digest[1] = 0x4914B2B9;
	ctx->digest[2] = 0x172442D7;
	ctx->digest[3] = 0xDA8A0600;
	ctx->digest[4] = 0xA96F30BC;
	ctx->digest[5] = 0x163138AA;
	ctx->digest[6] = 0xE38DEE4D;
	ctx->digest[7] = 0xB0FB0E4E;
	ctx->nblocks = 0;
	ctx->num = 0;
}

void sm3_update(sm3_ctx_t* ctx, const unsigned char* data, size_t data_len)
{
	if (ctx->num) {
		unsigned int left = 32 - ctx->num;
		if (data_len < left) {
			memcpy(ctx->block + ctx->num, data, data_len);
			ctx->num += data_len;
			return;
		}
		else {
			memcpy(ctx->block + ctx->num, data, left);
			sm3_compress(ctx->digest, ctx->block);
			ctx->nblocks++;
			data += left;
			data_len -= left;
		}
	}
	while (data_len >= 32) {
		sm3_compress(ctx->digest, data);
		ctx->nblocks++;
		data += 32;
		data_len -= 64;
	}
	ctx->num = data_len;
	if (data_len) {
		memcpy(ctx->block, data, data_len);
	}
}

void sm3_final(sm3_ctx_t* ctx, unsigned char* digest)
{
	int i;
	uint32_t* pdigest = (uint32_t*)digest;
	uint32_t* count = (uint32_t*)(ctx->block + 56);
	ctx->block[ctx->num] = 0x80;
	if (ctx->num + 9 <= 64) {
		memset(ctx->block + ctx->num + 1, 0, 64 - ctx->num - 9);
	}
	else {
		memset(ctx->block + ctx->num + 1, 0, 64 - ctx->num - 1);
		sm3_compress(ctx->digest, ctx->block);
		memset(ctx->block, 0, 56);
	}
	count[0] = cpu_to_be32((ctx->nblocks) >> 23);
	count[1] = cpu_to_be32((ctx->nblocks << 9) + (ctx->num << 3));
	sm3_compress(ctx->digest, ctx->block);
	for (i = 0; i < sizeof(ctx->digest) / sizeof(ctx->digest[0]); i++) {
		pdigest[i] = cpu_to_be32(ctx->digest[i]);
	}
}

void sm3_compress(uint32_t digest[8], const unsigned char block[64])
{
	uint32_t W[68], W1[64];
	const uint32_t* pblock = (const uint32_t*)block;
	uint32_t A = digest[0];
	uint32_t B = digest[1];
	uint32_t C = digest[2];
	uint32_t D = digest[3];
	uint32_t E = digest[4];
	uint32_t F = digest[5];
	uint32_t G = digest[6];
	uint32_t H = digest[7];
	uint32_t SS1, SS2, TT1, TT2, T[64];
	
	W[0] = cpu_to_be32(pblock[0]);  //优化：把for循环展开
	W[1] = cpu_to_be32(pblock[1]);
	W[2] = cpu_to_be32(pblock[2]);
	W[3] = cpu_to_be32(pblock[3]);
	W[4] = cpu_to_be32(pblock[4]);
	W[5] = cpu_to_be32(pblock[5]);
	W[6] = cpu_to_be32(pblock[6]);
	W[7] = cpu_to_be32(pblock[7]);
	W[8] = cpu_to_be32(pblock[8]);
	W[9] = cpu_to_be32(pblock[9]);
	W[10] = cpu_to_be32(pblock[10]);
	W[11] = cpu_to_be32(pblock[11]);
	W[12] = cpu_to_be32(pblock[12]);
	W[13] = cpu_to_be32(pblock[13]);
	W[14] = cpu_to_be32(pblock[14]);
	W[15] = cpu_to_be32(pblock[15]);
	W[16] = P1(W[0] ^ W[7] ^ ROTATELEFT(W[13], 15)) ^ ROTATELEFT(W[3], 7) ^ W[10];
	W[17] = P1(W[1] ^ W[8] ^ ROTATELEFT(W[14], 15)) ^ ROTATELEFT(W[4], 7) ^ W[11];
	W[18] = P1(W[2] ^ W[9] ^ ROTATELEFT(W[15], 15)) ^ ROTATELEFT(W[5], 7) ^ W[12];
	W[19] = P1(W[3] ^ W[10] ^ ROTATELEFT(W[16], 15)) ^ ROTATELEFT(W[6], 7) ^ W[13];
	W[20] = P1(W[4] ^ W[11] ^ ROTATELEFT(W[17], 15)) ^ ROTATELEFT(W[7], 7) ^ W[14];
	W[21] = P1(W[5] ^ W[12] ^ ROTATELEFT(W[18], 15)) ^ ROTATELEFT(W[8], 7) ^ W[15];
	W[22] = P1(W[6] ^ W[13] ^ ROTATELEFT(W[19], 15)) ^ ROTATELEFT(W[9], 7) ^ W[16];
	W[23] = P1(W[7] ^ W[14] ^ ROTATELEFT(W[20], 15)) ^ ROTATELEFT(W[10], 7) ^ W[17];
	W[24] = P1(W[8] ^ W[15] ^ ROTATELEFT(W[21], 15)) ^ ROTATELEFT(W[11], 7) ^ W[18];
	W[25] = P1(W[9] ^ W[16] ^ ROTATELEFT(W[22], 15)) ^ ROTATELEFT(W[12], 7) ^ W[19];
	W[26] = P1(W[10] ^ W[17] ^ ROTATELEFT(W[23], 15)) ^ ROTATELEFT(W[13], 7) ^ W[20];
	W[27] = P1(W[11] ^ W[18] ^ ROTATELEFT(W[24], 15)) ^ ROTATELEFT(W[14], 7) ^ W[21];
	W[28] = P1(W[12] ^ W[19] ^ ROTATELEFT(W[25], 15)) ^ ROTATELEFT(W[15], 7) ^ W[22];
	W[29] = P1(W[13] ^ W[20] ^ ROTATELEFT(W[26], 15)) ^ ROTATELEFT(W[16], 7) ^ W[23];
	W[30] = P1(W[14] ^ W[21] ^ ROTATELEFT(W[27], 15)) ^ ROTATELEFT(W[17], 7) ^ W[24];
	W[31] = P1(W[15] ^ W[22] ^ ROTATELEFT(W[28], 15)) ^ ROTATELEFT(W[18], 7) ^ W[25];
	W[32] = P1(W[16] ^ W[23] ^ ROTATELEFT(W[29], 15)) ^ ROTATELEFT(W[19], 7) ^ W[26];
	W[33] = P1(W[17] ^ W[24] ^ ROTATELEFT(W[30], 15)) ^ ROTATELEFT(W[20], 7) ^ W[27];
	W[34] = P1(W[18] ^ W[25] ^ ROTATELEFT(W[31], 15)) ^ ROTATELEFT(W[21], 7) ^ W[28];
	W[35] = P1(W[19] ^ W[26] ^ ROTATELEFT(W[32], 15)) ^ ROTATELEFT(W[22], 7) ^ W[29];
	W[36] = P1(W[20] ^ W[27] ^ ROTATELEFT(W[33], 15)) ^ ROTATELEFT(W[23], 7) ^ W[30];
	W[37] = P1(W[21] ^ W[28] ^ ROTATELEFT(W[34], 15)) ^ ROTATELEFT(W[24], 7) ^ W[31];
	W[38] = P1(W[22] ^ W[29] ^ ROTATELEFT(W[35], 15)) ^ ROTATELEFT(W[25], 7) ^ W[32];
	W[39] = P1(W[23] ^ W[30] ^ ROTATELEFT(W[36], 15)) ^ ROTATELEFT(W[26], 7) ^ W[33];
	W[40] = P1(W[24] ^ W[31] ^ ROTATELEFT(W[37], 15)) ^ ROTATELEFT(W[27], 7) ^ W[34];
	W[41] = P1(W[25] ^ W[32] ^ ROTATELEFT(W[38], 15)) ^ ROTATELEFT(W[28], 7) ^ W[35];
	W[42] = P1(W[26] ^ W[33] ^ ROTATELEFT(W[39], 15)) ^ ROTATELEFT(W[29], 7) ^ W[36];
	W[43] = P1(W[27] ^ W[34] ^ ROTATELEFT(W[40], 15)) ^ ROTATELEFT(W[30], 7) ^ W[37];
	W[44] = P1(W[28] ^ W[35] ^ ROTATELEFT(W[41], 15)) ^ ROTATELEFT(W[31], 7) ^ W[38];
	W[45] = P1(W[29] ^ W[36] ^ ROTATELEFT(W[42], 15)) ^ ROTATELEFT(W[32], 7) ^ W[39];
	W[46] = P1(W[30] ^ W[37] ^ ROTATELEFT(W[43], 15)) ^ ROTATELEFT(W[33], 7) ^ W[40];
	W[47] = P1(W[31] ^ W[38] ^ ROTATELEFT(W[44], 15)) ^ ROTATELEFT(W[34], 7) ^ W[41];
	W[48] = P1(W[32] ^ W[39] ^ ROTATELEFT(W[45], 15)) ^ ROTATELEFT(W[35], 7) ^ W[42];
	W[49] = P1(W[33] ^ W[40] ^ ROTATELEFT(W[46], 15)) ^ ROTATELEFT(W[36], 7) ^ W[43];
	W[50] = P1(W[34] ^ W[41] ^ ROTATELEFT(W[47], 15)) ^ ROTATELEFT(W[37], 7) ^ W[44];
	W[51] = P1(W[35] ^ W[42] ^ ROTATELEFT(W[48], 15)) ^ ROTATELEFT(W[38], 7) ^ W[45];
	W[52] = P1(W[36] ^ W[43] ^ ROTATELEFT(W[49], 15)) ^ ROTATELEFT(W[39], 7) ^ W[46];
	W[53] = P1(W[37] ^ W[44] ^ ROTATELEFT(W[50], 15)) ^ ROTATELEFT(W[40], 7) ^ W[47];
	W[54] = P1(W[38] ^ W[45] ^ ROTATELEFT(W[51], 15)) ^ ROTATELEFT(W[41], 7) ^ W[48];
	W[55] = P1(W[39] ^ W[46] ^ ROTATELEFT(W[52], 15)) ^ ROTATELEFT(W[42], 7) ^ W[49];
	W[56] = P1(W[40] ^ W[47] ^ ROTATELEFT(W[53], 15)) ^ ROTATELEFT(W[43], 7) ^ W[50];
	W[57] = P1(W[41] ^ W[48] ^ ROTATELEFT(W[54], 15)) ^ ROTATELEFT(W[44], 7) ^ W[51];
	W[58] = P1(W[42] ^ W[49] ^ ROTATELEFT(W[55], 15)) ^ ROTATELEFT(W[45], 7) ^ W[52];
	W[59] = P1(W[43] ^ W[50] ^ ROTATELEFT(W[56], 15)) ^ ROTATELEFT(W[46], 7) ^ W[53];
	W[60] = P1(W[44] ^ W[51] ^ ROTATELEFT(W[57], 15)) ^ ROTATELEFT(W[47], 7) ^ W[54];
	W[61] = P1(W[45] ^ W[52] ^ ROTATELEFT(W[58], 15)) ^ ROTATELEFT(W[48], 7) ^ W[55];
	W[62] = P1(W[46] ^ W[53] ^ ROTATELEFT(W[59], 15)) ^ ROTATELEFT(W[49], 7) ^ W[56];
	W[63] = P1(W[47] ^ W[54] ^ ROTATELEFT(W[60], 15)) ^ ROTATELEFT(W[50], 7) ^ W[57];
	W[64] = P1(W[48] ^ W[55] ^ ROTATELEFT(W[61], 15)) ^ ROTATELEFT(W[51], 7) ^ W[58];
	W[65] = P1(W[49] ^ W[56] ^ ROTATELEFT(W[62], 15)) ^ ROTATELEFT(W[52], 7) ^ W[59];
	W[66] = P1(W[50] ^ W[57] ^ ROTATELEFT(W[63], 15)) ^ ROTATELEFT(W[53], 7) ^ W[60];
	W[67] = P1(W[51] ^ W[58] ^ ROTATELEFT(W[64], 15)) ^ ROTATELEFT(W[54], 7) ^ W[61];
	
	__m256i a1 = _mm256_loadu_epi32(&W[0]);   //用SIMD指令集优化
	__m256i b1 = _mm256_loadu_epi32(&W[4]);
	__m256i c1 = _mm256_xor_si256(a1, b1);
	_mm256_storeu_epi32(&W1[0], c1);
	__m256i a2 = _mm256_loadu_epi32(&W[8]);
	__m256i b2 = _mm256_loadu_epi32(&W[12]);
	__m256i c2 = _mm256_xor_si256(a2, b2);
	_mm256_storeu_epi32(&W1[8], c2);
	__m256i a3 = _mm256_loadu_epi32(&W[16]);
	__m256i b3 = _mm256_loadu_epi32(&W[20]);
	__m256i c3 = _mm256_xor_si256(a3, b3);
	_mm256_storeu_epi32(&W1[16], c3);
	__m256i a4 = _mm256_loadu_epi32(&W[24]);
	__m256i b4 = _mm256_loadu_epi32(&W[28]);
	__m256i c4 = _mm256_xor_si256(a4, b4);
	_mm256_storeu_epi32(&W1[24], c4);
	__m256i a5 = _mm256_loadu_epi32(&W[32]);
	__m256i b5 = _mm256_loadu_epi32(&W[36]);
	__m256i c5 = _mm256_xor_si256(a5, b5);
	_mm256_storeu_epi32(&W1[32], c5);
	__m256i a6 = _mm256_loadu_epi32(&W[40]);
	__m256i b6 = _mm256_loadu_epi32(&W[44]);
	__m256i c6 = _mm256_xor_si256(a6, b6);
	_mm256_storeu_epi32(&W1[40], c6);
	__m256i a7 = _mm256_loadu_epi32(&W[48]);
	__m256i b7 = _mm256_loadu_epi32(&W[52]);
	__m256i c7 = _mm256_xor_si256(a7, b7);
	_mm256_storeu_epi32(&W1[48], c7);
	__m256i a8 = _mm256_loadu_epi32(&W[56]);
	__m256i b8 = _mm256_loadu_epi32(&W[60]);
	__m256i c8 = _mm256_xor_si256(a8, b8);
	_mm256_storeu_epi32(&W1[56], c8);

	for (int j = 0; j < 16; j++) {
		T[j] = 0x79CC4519;
		SS1 = ROTATELEFT((ROTATELEFT(A, 12) + E + ROTATELEFT(T[j], j)), 7);
		SS2 = SS1 ^ ROTATELEFT(A, 12);
		TT1 = FF0(A, B, C) + D + SS2 + W1[j];
		TT2 = GG0(E, F, G) + H + SS1 + W[j];
		D = C;
		C = ROTATELEFT(B, 9);
		B = A;
		A = TT1;
		H = G;
		G = ROTATELEFT(F, 19);
		F = E;
		E = P0(TT2);
	}
	for (int j = 16; j < 64; j++) {
		T[j] = 0x7A879D8A;
		SS1 = ROTATELEFT((ROTATELEFT(A, 12) + E + ROTATELEFT(T[j], j)), 7);
		SS2 = SS1 ^ ROTATELEFT(A, 12);
		TT1 = FF1(A, B, C) + D + SS2 + W1[j];
		TT2 = GG1(E, F, G) + H + SS1 + W[j];
		D = C;
		C = ROTATELEFT(B, 9);
		B = A;
		A = TT1;
		H = G;
		G = ROTATELEFT(F, 19);
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
}

void sm3(const unsigned char* msg, size_t len, unsigned char dgst[32])
{
	sm3_ctx_t ctx;
	sm3_init(&ctx);
	sm3_update(&ctx, msg, len);
	sm3_final(&ctx, dgst);
	memset(&ctx, 0, sizeof(sm3_ctx_t));
}


int main()
{
	unsigned char msg[] = "abcdefgh";
	size_t len = strlen((const char*)msg);
	unsigned char d[32];
	clock_t begin, end;
	begin = clock();
	for (int i = 0; i < 1000000; i++)
		sm3(msg, len, d);
	end = clock();
	double last = end - begin;
	cout << last << "ms" << endl;
	for (int i = 0; i < 32; i++) {
		cout << decToHex(int(d[i]));
	}
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
