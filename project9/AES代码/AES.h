#pragma once
#include <bitset>
using namespace std;
typedef bitset<8> byte;
typedef bitset<32> word;
void StrToByte(const char* m, byte plain[]);
void initial_round(byte a[], byte b[], byte c[]);
void SubBytes(byte a[]);
void ShiftRows(byte a[]);
void MixColumns(byte a[]);
void AddRoundKey(byte a[16], word k[4]);
void KeyExpansion(byte key[16], word w[44]);


