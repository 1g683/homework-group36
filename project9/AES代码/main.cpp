#include <iostream>
#include <bitset>
#include <string>
#include"AES.h"
using namespace std;
typedef bitset<8> byte;
typedef bitset<32> word;
int main()
{

	byte key[16];
	byte initial[16];
	const char* plain = "202100150084";
	const char* key_ = "202100150084";



	StrToByte(plain, initial);
	StrToByte(key_, key);//编码
	cout << "所要加密的明文(编码后)为："<<endl;
	for (int i = 0; i < 16; ++i)
	{
		cout << hex << initial[i].to_ulong() << " ";   // 将bitset转换成unsigned long类型
		if ((i + 1) % 4 == 0)     //换行
			cout << endl;
	}
	cout << "所要加密的密钥(编码后)为:"<< endl;
	for (int i = 0; i < 16; ++i)
	{
		cout << hex << key[i].to_ulong() << " ";   // 将bitset转换成unsigned long类型
		if ((i + 1) % 4 == 0)     //换行
			cout << endl;
	}

	word w[44];
	word k1[4];

	KeyExpansion(key, w);
	/*
	cout << "密钥拓展后为：" << endl;
	for (int i = 0; i < 44; ++i)
		cout  << hex<<w[i].to_ulong() << endl;
	for (int i = 0; i < 4; ++i)
		k1[i] = w[i];
		*/



	AddRoundKey(initial, k1);
	
	for (int i = 0;i < 9;++i) {		
		SubBytes(initial);
		ShiftRows(initial);
		MixColumns(initial);
		for (int j = 0; j < 4; ++j)
			k1[j] = w[4 * (i + 1) + j];
		AddRoundKey(initial, k1);
		
	}
	SubBytes(initial);
	ShiftRows(initial);
	for (int i = 0; i < 4; ++i)
		k1[i] = w[40 + i];

	AddRoundKey(initial,k1);
	
	
	
	// printer 
	cout << endl << "加密结果为：" << endl;
	for (int i = 0; i < 16; ++i)
	{
		cout << hex << initial[i].to_ulong() << " ";   // 将bitset转换成unsigned long类型
		if ((i + 1) % 4 == 0)     //换行
			cout << endl;
	}


	return 0;
	
}