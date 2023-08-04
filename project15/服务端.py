from random import randint
import socket
from Crypto.Util.number import inverse


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


client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
client.bind(('', 111))
print("等待建立连接...")
d2 = randint(1,N-1)
x,address = client.recvfrom(1024)
x = int(x.decode(),16)
y,address = client.recvfrom(1024)
y = int(y.decode(),16)
P1 = (x,y)
P1 = ECC_mul(inverse(d2,P),P1)
P1 = Point_Add(P1,(G_X,-G_Y))
x,address = client.recvfrom(1024)
x = int(x.decode(),16)
y,address = client.recvfrom(1024)
y = int(y.decode(),16)
Q1 = (x,y)
e,address = client.recvfrom(1024)
e = int(e.decode(),16)
k2 = randint(1,N-1)
k3 = randint(1,N-1)
Q2 = ECC_mul(k2,G)
x1,y1 = ECC_mul(k3,Q1)
x1,y1 = Point_Add((x1,y1),Q2)
r =(x1 + e)%N
s2 = (d2 * k3)%N
s3 = (d2 * (r+k2))%N
client.sendto(hex(r).encode(),address)
client.sendto(hex(s2).encode(),address)
client.sendto(hex(s3).encode(),address)
print("连接已关闭")
