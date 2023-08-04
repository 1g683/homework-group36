from random import randint
import socket
from Crypto.Util.number import inverse
import sys
import binascii
from gmssl import sm3, func



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


def EC_double(p):
    slope = (3 * p[0] ** 2 + a) * inverse(2 * p[1], P) % P
    r = [(slope ** 2 - 2 * p[0]) % P]
    r.append((slope * (p[0] - r[0]) - p[1]) % P)
    return (r[0], r[1])


def EC_mul(s, p):
    n = p
    r = 0
    s_binary = bin(s)[2:]
    s_length = len(s_binary)
    for i in reversed(range(s_length)):
        if s_binary[i] == '1':
            r = Point_Add(r, n)
        n = EC_double(n)
    return r


address = ('127.0.0.1', 111)
client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
try:
    client.connect(('127.0.0.1', 111))
    print("连接建立")
except Exception:
    print('连接失败')
    sys.exit()
else:
    d1 = randint(1, N - 1)
    P1 = EC_mul(inverse(d1, P), G)
    x, y = hex(P1[0]), hex(P1[1])
    client.sendto(x.encode('utf-8'), address)
    client.sendto(y.encode('utf-8'), address)
    msg = "202100150084"
    msg = hex(int(binascii.b2a_hex(msg.encode()).decode(), 16)).upper()[2:]
    ID_user = "sdu"
    ID_user = hex(int(binascii.b2a_hex(ID_user.encode()).decode(), 16)).upper()[2:]
    ENTL_A = '{:04X}'.format(len(ID_user) * 4)
    ide = ENTL_A + ID_user + '{:064X}'.format(a) + '{:064X}'.format(b) + '{:064X}'.format(G_X) + '{:064X}'.format(G_Y)
    Z = sm3.sm3_hash(func.bytes_to_list(ide.encode()))
    M = Z + msg
    e = sm3.sm3_hash(func.bytes_to_list(M.encode()))
    k1 = randint(1, N - 1)
    Q1 = EC_mul(k1, G)
    x, y = hex(Q1[0]), hex(Q1[1])
    client.sendto(x.encode('utf-8'), address)
    client.sendto(y.encode('utf-8'), address)
    client.sendto(e.encode('utf-8'), address)
    r, _ = client.recvfrom(1024)
    r = int(r.decode(), 16)
    s2, _ = client.recvfrom(1024)
    s2 = int(s2.decode(), 16)
    s3, _ = client.recvfrom(1024)
    s3 = int(s3.decode(), 16)
    s = ((d1 * k1) * s2 + d1 * s3 - r) % N
    print(f"Signature : {hex(r)} \n{hex(s)}")
    client.close()
