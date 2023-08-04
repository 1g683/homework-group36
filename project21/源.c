#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
struct Pair {
    int first;
    int second;
};
struct signature {
    struct Pair R;
    int s;

};
// 辗转相除法求最大公约数
int get_gcd(int a, int b) {
    int remainder = a % b;
    while (remainder != 0) {
        a = b;
        b = remainder;
        int k = a / b;
        remainder = a % b;
    }
    return b;
}

// 改进欧几里得算法求线性方程的x与y
struct Pair get_extended_gcd(int a, int b) {
    if (b == 0) {
        struct Pair p = { 1, 0 };
        return p;
    }
    else {
        int k = a / b;
        int remainder = a % b;
        struct Pair p = get_extended_gcd(b, remainder);
        int x1 = p.first;
        int y1 = p.second;
        int x = y1;
        int y = x1 - k * y1;
        struct Pair result = { x, y };
        return result;
    }
}

// 返回乘法逆元
int multiplicative_inverse(int a, int b) {
    int m = (b < 0) ? abs(b) : b;
    int flag = get_gcd(a, b);
    if (abs(flag) == 1) {
        struct Pair p = get_extended_gcd(a, b);
        int x = p.first;
        int y = p.second;
        int x0 = x % m;
        return x0;
    }
    else {
        printf("Do not have!\n");
        return 0;
    }
}

// 所用的椭圆曲线
// y^2 = x^3 + ax + by mod (mod_value)

struct Pair point_add(struct Pair P, struct Pair Q, int a, int mod_value) {
    if (P.first == Q.first) {
        int numerator = (3 * P.first * P.first + a) % mod_value;
        int denominator = (2 * P.second) % mod_value;
        int val = multiplicative_inverse(denominator, mod_value);
        int y = (numerator * val) % mod_value;
        int Rx = (y * y - P.first - Q.first + mod_value) % mod_value;
        int Ry = (y * (P.first - Rx) - P.second + mod_value) % mod_value;
        struct Pair result = { Rx, Ry };
        return result;
    }
    else {
        int numerator = (Q.second - P.second + mod_value) % mod_value;
        int denominator = (Q.first - P.first + mod_value) % mod_value;
        int val = multiplicative_inverse(denominator, mod_value);
        int y = (numerator * val) % mod_value;
        int Rx = (y * y - P.first - Q.first + mod_value) % mod_value;
        int Ry = (y * (P.first - Rx) - P.second + mod_value) % mod_value;
        struct Pair result = { Rx, Ry };
        return result;
    }
}

struct Pair point_multiply(int n, struct Pair point, int a, int mod_value) {
    if (n == 0) {
        struct Pair result = { 0, 0 };
        return result;
    }
    else if (n == 1) {
        return point;
    }
    struct Pair t = point;
    while (n >= 2) {
        t = point_add(t, point, a, mod_value);
        n = n - 1;
    }
    return t;
}

struct Pair double_point(struct Pair point, int a, int mod_value) {
    return point_add(point, point, a, mod_value);
}

struct Pair fast_multiply(int n, struct Pair point, int a, int mod_value) {
    if (n == 0) {
        struct Pair result = { 0, 0 };
        return result;
    }
    else if (n == 1) {
        return point;
    }
    else if (n % 2 == 0) {
        return point_multiply(n / 2, double_point(point, a, mod_value), a, mod_value);
    }
    else {
        return point_add(point_multiply((n - 1) / 2, double_point(point, a, mod_value), a, mod_value), point, a, mod_value);
    }
}

int hash_string(const char* str) {
    int h = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        h = (h * 31 + str[i])%37;
    }
    return h;
}

int rand_range(int min, int max) {
    return min + rand() % (max - min + 1);
}


// 函数将字符串和两个整数拼接成新的字符串
char* concatStringInts(const char* str, int num1, int num2) {
    // 计算所需的总长度
    int totalLength = snprintf(NULL, 0, "%s%d%d", str, num1, num2) + 1;
    // 分配足够的内存来存储拼接后的字符串
    char* result = (char*)malloc(totalLength);
    if (result == NULL) {
        return NULL; // 内存分配失败
    }
    // 使用 snprintf 将字符串和两个整数拼接到 result 中
    snprintf(result, totalLength, "%s%d%d", str, num1, num2);
    return result;
}




   


struct signature Schnorr_Sign(int mod_value, const char* m, int prikey, struct Pair G, int a, int n) {
    int k = rand_range(1, mod_value);

    struct Pair R = point_multiply(k, G, a, mod_value);
    char* concatenated = concatStringInts(m, R.first, R.second);
    int e = hash_string(concatenated) % n;
    int s = (k + e * prikey) % n;
    struct signature ret;
    ret.R = R;
    ret.s = s;
    return ret;
}

bool sign_Verify(struct signature sign, const char* m, struct Pair G, struct Pair pubkey, int a, int mod_value) {

    char* concatenated = concatStringInts(m, sign.R.first, sign.R.second);
    int e = hash_string(concatenated) % 19;
    struct Pair A = point_multiply(sign.s, G, a, mod_value);
    struct Pair B = point_add(point_multiply(e, pubkey, a, mod_value), sign.R, a, mod_value);
    if ((A.first % mod_value) == (B.first % mod_value)) {
        printf("签名验证成功！\n");
        return true;
    }
    else {
        printf("签名失败\n");
        return false;
    }
}

bool sign_Verify_Batch(struct signature sign[4], const char* m[4][100], struct Pair G, struct Pair pubkey, int a, int mod_value, int n) {
    int bisi = sign[0].s;
    struct Pair biRi = sign[0].R;
    char* concatenated = concatStringInts(m[0], sign[0].R.first, sign[0].R.second);
    int e = hash_string(concatenated) % n;
    int eibi = e;
    struct Pair B;
    struct Pair A;
    int b;
    for (int i = 1; i < 4; i++) {
        b = rand() % (mod_value - 1) + 1;
        bisi = bisi + b * sign[i].s;
        biRi = point_add(point_multiply(b, sign[i].R, a, mod_value), biRi, a, mod_value);
        char* mm = concatStringInts(m[i], sign[i].R.first, sign[i].R.second);
        e = hash_string(mm) % n;
        eibi = eibi + e * b;
    }
    printf("对以上4个签名批量验签：\n");
    A = point_multiply(bisi % n, G, a, mod_value);
    B = point_add(point_multiply(eibi % n, pubkey, a, mod_value), biRi, a, mod_value);

    if ((A.first % mod_value) == (B.first % mod_value)) {
        printf("签名验证成功！\n");
        return true;
    }
    else {
        printf("签名失败\n");
        return false;
    }
}
int main() {
    int mod_value = 17; // 椭圆曲线域上的模数
    int a = 2;
    int b = 2;
    struct Pair G = {7,1};
    int n =19; // G点的基数
    char strings[4][100]; //4条要加密的消息
    strcpy(strings[0], "Hello");
    strcpy(strings[1], "World");
    strcpy(strings[2], "Gao");
    strcpy(strings[3], "Chang");
    
    int private_key = 5;
    struct signature sign[4];
    clock_t start, end;
    double cpu_time_used;
   
    for (int i = 0;i < 4;i++) {
        sign[i]= Schnorr_Sign(mod_value, strings[i], private_key, G, a, n);
        printf("对消息的签名为: [(%d, %d)，%d]\n", sign[i].R.first, sign[i].R.second, sign[i].s);
    }
  
    struct Pair public_key = point_multiply(private_key, G, a, mod_value);

    printf("对以上4个签名分别进行验证……\n");
    for (int i = 0;i < 4;i++) {
        sign_Verify(sign[i], strings[i], G, public_key, a, mod_value);
    }
    sign_Verify_Batch(sign, strings, G, public_key, a, mod_value, n);
    return 0;
}

 