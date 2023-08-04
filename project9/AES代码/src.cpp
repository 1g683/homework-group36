#include <iostream>
#include <bitset>
#include <string>
using namespace std;
typedef bitset<8> byte;//创建了一个新类型byte，长度为8的二进制数组

void StrToByte(const char* m, byte plain[]) {//将字符串根据ASCII码转换成char[]型二进制串
	char mAscii[128];
	int mLen = strlen(m);
	for (int i = 0; i < mLen; ++i) {
		for (int j = 0; j < 8; ++j) {
			mAscii[i * 8 + j] = (m[i] >> (7 - j)) & 1;
		}
	}
	// Pad plaintext with zeros
	for (int i = mLen * 8;i < 128;i++) {
		mAscii[i] = 0;
	}
	int count = 0;
	for (int i = 0;i < 16;i++) {
		for (int j = 7;j >=0;j--) {
			plain[i][j] = mAscii[count];
			count++;
		}
	}

}
typedef bitset<32> word;
 




void initial_round(byte a[], byte b[], byte c[]) {
	for (int i = 0;i < 16;i++) {
		c[i] = a[i] ^ b[i];
	}

}
byte S_Box[16][16] =  //S盒
{
	{0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76},
	{0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0},
	{0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15},
	{0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75},
	{0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84},
	{0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF},
	{0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8},
	{0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2},
	{0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73},
	{0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB},
	{0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79},
	{0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08},
	{0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A},
	{0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E},
	{0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF},
	{0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16}
};
byte s[16] =      //列混合要乘的矩阵
{   
	0x02,0x03,0x01,0x01,
	0x01,0x02,0x03,0x01,
	0x01,0x01,0x02,0x03,
	0x03,0x01,0x01,0x02
};
word Rc[10] =   //轮常量
{ 
	0x01000000, 0x02000000, 0x04000000, 0x08000000, 0x10000000,
    0x20000000, 0x40000000, 0x80000000, 0x1b000000, 0x36000000
};
void SubBytes(byte a[]) //字节替换
{
	for (int i = 0; i < 16; i++)    //低4bit确定列；高4bit确定行
	{
		int row = a[i][7]*8 + a[i][6]*4 + a[i][5]*2 +a[i][4];
		int col = a[i][3]*8 + a[i][2]*4 + a[i][1]*2 + a[i][0];
		a[i] = S_Box[row][col];//进行字节替换 
	}
}
void ShiftRows(byte a[])     //行位移
{
	byte temp[2];
	//第二行循环左移一位
	temp[0] = a[4];
	a[4] = a[5];
	a[5] = a[6];
	a[6] = a[7];
	a[7] = temp[0];
	// 第三行循环左移两位
	temp[0] = a[8];
	temp[1] = a[9];
	a[8] = a[10];
	a[9] = a[11];
	a[10] = temp[0];
	a[11] = temp[1];
	// 第四行循环左移三位
	temp[0] = a[15];
	a[15] = a[14];
	a[14] = a[13];
	a[13] = a[12];
	a[12] = temp[0];
}
byte GF28Mul(byte a, byte b)                //在有限域GF（2^8）上的乘法实现
{
	byte m = 0, c[8];
	int n, i;
	c[0] = b;
	for (i = 1; i < 8; i++) {
		n = c[i - 1][7];
		c[i] = (c[i - 1] << 1) ^ (byte)(n ? 0x1b : 0x00);
	}
	for (i = 0; i < 8; i++) {
		if (a[i] == 1)
			m ^= c[i];
	}
	return m;
}
void MixColumns(byte a[])             //列混合
{
	byte l[4];
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			l[j] = a[i + j * 4];
		}
		a[i] = GF28Mul(s[0], l[0]) ^ GF28Mul(s[1], l[1]) ^ GF28Mul(s[2], l[2]) ^ GF28Mul(s[3], l[3]);
		a[i + 4] = GF28Mul(s[4], l[0]) ^ GF28Mul(s[5], l[1]) ^ GF28Mul(s[6], l[2]) ^ GF28Mul(s[7], l[3]);
		a[i + 8] = GF28Mul(s[8], l[0]) ^ GF28Mul(s[9], l[1]) ^ GF28Mul(s[10], l[2]) ^ GF28Mul(s[11], l[3]);
		a[i + 12] = GF28Mul(s[12], l[0]) ^ GF28Mul(s[13], l[1]) ^ GF28Mul(s[14], l[2]) ^ GF28Mul(s[15], l[3]);
	}
}
void AddRoundKey(byte a[16], word k[4])  //轮密钥加
{
	for (int i = 0; i < 4; ++i)
	{
		word key1 = k[i] >> 24;
		word key2 = (k[i] << 8) >> 24;
		word key3 = (k[i] << 16) >> 24;
		word key4 = (k[i] << 24) >> 24;
		a[i] = a[i] ^ byte(key1.to_ulong());
		a[i + 4] = a[i + 4] ^ byte(key2.to_ulong());
		a[i + 8] = a[i + 8] ^ byte(key3.to_ulong());
		a[i + 12] = a[i + 12] ^ byte(key4.to_ulong());
	}
}
void KeyExpansion(byte key[16], word w[44])//密钥扩展得到4*11个word
{
	word temp;
	int i = 0;
	while (i < 4)
	{
		word step;
		step = key[4 * i].to_ulong();
		step <<= 24;
		w[i] |= step;
		step = key[4 * i + 1].to_ulong();
		step <<= 16;
		w[i] |= step;
		step = key[4 * i + 2].to_ulong();
		step <<= 8;
		w[i] |= step;
		step = key[4 * i + 3].to_ulong();
		w[i] |= step;//将4个byte合成一个word
		i++;
	}
	i = 4;
	while (i < 44)
	{
		temp = w[i - 1]; 
		if (i % 4 == 0)
		{
			word high = temp << 8;
			word low = temp >> 24;
			word temp1 = high | low;//将字节左移一位
			word temp2;
			for (int i = 0; i < 32; i += 8)
			{
				int row = temp1[i + 7] * 8 + temp1[i + 6] * 4 + temp1[i + 5] * 2 + temp1[i + 4];
				int col = temp1[i + 3] * 8 + temp1[i + 2] * 4 + temp1[i + 1] * 2 + temp1[i];
				byte val = S_Box[row][col];
				for (int j = 0; j < 8; ++j)
				{
					temp2[i + j] = val[j];
				}//对word中的字节进行S-盒变换
			}
			w[i] = w[i - 4] ^ temp2 ^ Rc[i / 4 - 1];//将最左侧的字节与已知的RC进行异或相加；
		}
		else
		{
			w[i] = w[i - 4] ^ temp;//将不被4整除的进行与对应的位异或
		}
		i++;
	}
}


