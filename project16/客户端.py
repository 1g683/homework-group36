from random import randint
import socket
from Crypto.Util.number import inverse
import sys
import binascii
from gmssl import sm3


# 参数
P = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF
N = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFF7203DF6B21C6052B53BBF40939D54123
a = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFC
b = 0x28E9FA9E9D9F5E344D5A9E4BCF6509A7F39789F515AB8F92DDBCBD414D940E93
G_X = 0x421DEBD61B62EAB6746434EBC3CC315E32220B3BADD50BDC4C4E6C147FEDD43D
G_Y = 0x0680512BCBB42C07D47349D2153B70C4E5D7FDFCBFA36EA1A85841B9E46E09A2
G = (G_X, G_Y)


# 辗转相除法求最大公约数
def get_gcd(a, b):
    re = a % b
    while re != 0:
        a = b
        b = re
        k = a // b
        re = a % b
    return b


# 改进欧几里得算法求线性方程的x与y
def get_(a, b):
    if b == 0:
        return 1, 0
    else:
        k = a // b
        remainder = a % b
        x1, y1 = get_(b, remainder)
        x, y = y1, x1 - k * y1
    return x, y


# 返回乘法逆元
def Multi_inverse(a, b):
    if b < 0:
        m = abs(b)
    else:
        m = b

    flag = get_gcd(a, b)
    if flag == 1:
        x, y = get_(a, b)
        x0 = x % m
        return x0
    else:
        print("无!")


# 椭圆曲线点加法运算
def Point_Add(a, b):
    if a == 0 and b == 0:
        return 0
    elif a == 0:
        return b
    elif b == 0:
        return a
    else:
        if a[0] > b[0]:
            a, b = b, a
        slope = (b[1] - a[1]) * inverse(b[0] - a[0], P) % P

        r = [(slope ** 2 - a[0] - b[0]) % P]
        r.append((slope * (a[0] - r[0]) - a[1]) % P)

        return (r[0], r[1])


def ECC_double(p):
    slope = (3 * p[0] ** 2 + a) * inverse(2 * p[1], P) % P
    r = [(slope ** 2 - 2 * p[0]) % P]
    r.append((slope * (p[0] - r[0]) - p[1]) % P)
    return (r[0], r[1])

def KDF(Z, klen):
    hlen = 256
    n = (klen // hlen) + 1
    if n >= 2 ** 32 - 1:
        raise ValueError("hash的长度太长!")
    K = ''
    for i in range(n):
        ct = (hex(i + 1)[2:]).rjust(32, '0')
        tmp_b = bytes.fromhex(Z + ct)
        Kct = sm3.sm3_hash(list(tmp_b))
        K += Kct
    bit_len = 256 * n
    K = (bin(int(K, 16))[2:]).rjust(bit_len, '0')
    K = K[:klen]
    return K

def ECC_mul(s, p):
    n = p
    r = 0
    s_binary = bin(s)[2:]
    s_length = len(s_binary)
    for i in reversed(range(s_length)):
        if s_binary[i] == '1':
            r = Point_Add(r, n)
        n = ECC_double(n)
    return r


address = ('127.0.0.1', 12345)
client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
try:
    client.connect(('127.0.0.1', 12345))
    print("连接建立")
except Exception:
    print('连接失败')
    sys.exit()
else:
    d1 = randint(1, N - 1)
    d2 = randint(1, N - 1)
    k = randint(1, N - 1)
    C_1 = ECC_mul(k, G)
    P_K = ECC_mul(inverse(d1 * d2, P) - 1, G)    
    G_2 = ECC_mul(k, P_K)
    t = KDF('{:064X}'.format(G_2[0]) + '{:064X}'.format(G_2[1]), 64)
    msg = "iloveyou"
    msg = hex(int(binascii.b2a_hex(msg.encode()).decode(), 16)).upper()[2:]
    print(msg)
    C_2 = hex(int(msg, 16) ^ int(t, 2))[2:].upper()
    C_3 = sm3.sm3_hash(list(bytes.fromhex('{:064X}'.format(G_2[0]) + msg + '{:064X}'.format(G_2[1]))))
    C = str(C_1[0]) + str(C_1[1]) + C_2 + C_3
    if C_1 != 0:
        T1 = ECC_mul(inverse(d1, P), C_1)
        x, y = hex(T1[0]), hex(T1[1])
        client.sendto(x.encode('utf-8'), address)
        client.sendto(y.encode('utf-8'), address)
    x2, address = client.recvfrom(1024)
    x2 = int(x2.decode(), 16)
    y2, address = client.recvfrom(1024)
    y2 = int(y2.decode(), 16)
    T2 = (x2, y2)
    T2 = ECC_mul(inverse(d2, P), T1)
    G_3 = Point_Add(T2, (C_1[0], -C_1[1]))
    t = KDF('{:064X}'.format(G_2[0]) + '{:064X}'.format(G_2[1]), 64)
    M = hex(int(C_2, 16) ^ int(t, 2))[2:].upper()
    print(M)
