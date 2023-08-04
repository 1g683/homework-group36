#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

struct Pair {
    int first;
    int second;
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

struct Pair ecdsa_sign(int e, struct Pair G, int private_key, int k, int a, int mod_value, int n) {
    struct Pair R = fast_multiply(k, G, a, mod_value);  // R = kG
    int r = R.first % n;  // r = Rx mod n
    int s = (multiplicative_inverse(k, n) * (e + private_key * r)) % n;
    struct Pair signature = { r, s };
    return signature;
}

bool ecdsa_verify(int e, struct Pair G, int r, int s, struct Pair public_key, int a, int mod_value, int n) {
    int w = multiplicative_inverse(s, n);
    int ele1 = (e * w) % n;
    if (ele1 < 0) {
        ele1 += n;
    }
    int ele2 = (r * w) % n;
    if (ele1 < 0) {
        ele2 += n;
    }
    struct Pair R= point_add(point_multiply(ele1, G, a, mod_value), point_multiply(ele2, public_key, a, mod_value), a, mod_value);
    if (R.first == 0) {
        printf("false\n");
        return false;
    }
    else {
        if (R.first == r ) {
            printf("签名验证成功\n");
            return true;
        }
        if(abs(R.first - r) == n){
            printf("签名验证成功\n");
            return true;
        }
        else {
            printf("签名验证失败\n");
            return false;
        }
    }
}
///////////////////////////////////////////////////////////以上为之前的ECDSA实现//////////////////////////////////////////////////////

int rand_range(int min, int max) {
    return min + rand() % (max - min + 1);
}

int main() {
    int mod_value = 29; // 椭圆曲线域上的模数
    int a = 4;
    int b = 20;
    struct Pair G = {13,23};
    int n =37; // G点的基数
    char message[] = "Satoshi is here";
    int private_key = 5;
    int e = hash_string(message);
    int k = 6;
    struct Pair signature = ecdsa_sign(e, G, private_key, k, a, mod_value, n);
    struct Pair public_key = point_multiply(private_key, G, a, mod_value);
    printf("选取公钥为: (%d, %d)\n", public_key.first, public_key.second);
    printf("计算签名为: (%d, %d)\n", signature.first, signature.second);
    ecdsa_verify(e, G, signature.first, signature.second, public_key, a, mod_value, n);
    printf("////////////////////////////////////////////////////////////\n");
    printf("//////////////////伪造签名中……//////////////////////////////\n");
    printf("////////////////////////////////////////////////////////////\n");
    int u = rand_range(1, n - 1);
    int v = rand_range(1, n - 1);
    struct Pair R = point_add(
        point_multiply(u, G, a, mod_value),
        point_multiply(v, public_key, a, mod_value),
        a, mod_value
    );

    int r1 = R.first;
    int s1 = (r1* multiplicative_inverse(v, n)) % n;
    int e1 = (u * s1) % n;
    printf("伪造的签名为: (%d, %d)\n", r1, s1);
    ecdsa_verify(e1, G, r1, s1, public_key, a, mod_value, n);
    return 0;
}
  

