import random
import string

T = [0x79cc4519, 0x7a879d8a]
w=[0] * 68
w_=[0] * 64
IV = [0x7380166f, 0x4914b2b9, 0x172442d7, 0xda8a0600, 0xa96f30bc, 0x163138aa, 0xe38dee4d, 0xb0fb0e4e]
    
def FF(x,y,z,j):
    if 0 <= j <= 15:
        return x ^ y ^ z
    elif 16 <= j <= 63:
        return (x & y) | (x & z) | (y & z)
    return 0
    
def GG(x,y,z,j):
    if 0 <= j <= 15:
        return x ^ y ^ z
    elif 16 <= j <= 63:
        return (x & y) | (~x & z)
    return 0

def left_move(x, num):
    num = num % 32
    left = (x << num) % (2 ** 32)
    right = (x >> (32 - num)) % (2 ** 32)
    m = left ^ right
    return m

def p0(x): 
    return x^(left_move(x,9))^(left_move(x,17))
def p1(x):
    return x^(left_move(x,15))^(left_move(x,23))

def fill(m):
    length = len(m)   
    l = length * 8      
    num = length // 64
    rbyte = length % 64
    mrbin = ""
    new = bytearray((num + 1) * 64)  
    for i in range(length):
        new[i] = m[i]
    rbit = rbyte * 8   
    for i in range(rbyte):
        mrbin += "{:08b}".format(m[num * 64 + i])
    k = (448 - l - 1) % 512
    while k < 0:
        k += 512
    p = str(bin(l)[2:])
    p = "0" * (64 - len(p)) + p
    mrbin += "1" + "0" * k + p
    for i in range(0, 64 - rbyte):
        a = mrbin[i * 8 + rbit: (i + 1) * 8 + rbit]
        b = length + i
        new[b] = int(a, 2) 
    return new

def extend(m):
    for i in range(0, 16):
        w[i] = int.from_bytes(m[i * 4:(i + 1) * 4], byteorder="big")
    for i in range(16, 68):
        w[i] = p1(w[i-16] ^ w[i-9] ^ left_move(w[i-3], 15)) ^ left_move(w[i-13], 7) ^ w[i-6]
    for i in range(64):
        w_[i] = w[i] ^ w[i+4]

def compress(m,IV):
    extend(m)
    ss1 = 0
    A = IV[0]
    B = IV[1]
    C = IV[2]
    D = IV[3]
    E = IV[4]
    F = IV[5]
    G = IV[6]
    H = IV[7]
    for j in range(64):
        if j < 16:
            ss1 = left_move((left_move(A, 12) + E + left_move(T[0], j)) % (2 ** 32), 7)
        elif j >= 16:
            ss1 = left_move((left_move(A, 12) + E + left_move(T[1], j)) % (2 ** 32), 7)
        ss2 = ss1 ^ left_move(A, 12)
        tt1 = (FF(A, B, C, j) + D + ss2 + w_[j]) % (2 ** 32)
        tt2 = (GG(E, F, G, j) + H + ss1 + w[j]) % (2 ** 32)
        D = C
        C = left_move(B, 9)
        B = A
        A = tt1
        H = G
        G = left_move(F, 19)
        F = E
        E = p0(tt2)
    IV[0] ^= A
    IV[1] ^= B
    IV[2] ^= C
    IV[3] ^= D
    IV[4] ^= E
    IV[5] ^= F
    IV[6] ^= G
    IV[7] ^= H

def SM3(m,IV,flag):
    new = fill(m)
    n = len(new) // 64
    if flag:
        B = []
        for i in range(0, n-1):
            B.append(new[(i + 1)*64:(i+2)*64])
        for i in range(0, n-1):
            compress(B[i],IV)
    else:
        for i in range(0, n):
            compress(new[i * 64:(i + 1) * 64],IV)
    s = ""
    for i in range(len(IV)):
        s += hex(IV[i])[2:]
    return s

def generate_random_string(length):
    letters = string.ascii_lowercase + string.digits  
    random_string = ''.join(random.choice(letters) for _ in range(length))
    return random_string

def len_attack(m,length):
    x = ""
    if length > 64:
        for i in range(0, int(length / 64) * 64):
            x += '1'
    for i in range(0, length % 64):
        x += '1'
    x=bytearray(x, encoding='utf-8')
    x=fill(x)
    print("附加消息：",'202100150084')
    s=bytearray('202100150084', encoding='utf-8')
    s=x+s
    c=SM3(s,m,1)
    return c

length=8   #可以改变随机生成的消息长度
a=generate_random_string(length)
a=bytearray(a, encoding='utf-8')
print("生成的随机消息：")
print(a)
y=SM3(a,IV,0)
print("随机消息的hash值：")
print(y)
m=[]
for i in range(0, len(y), 8):
    m.append(int(y[i:i+8],16))
b=fill(a)
x=len_attack(m,length)
print("长度攻击得到的hash值：")
print(x)
IV = [0x7380166f, 0x4914b2b9, 0x172442d7, 0xda8a0600, 0xa96f30bc, 0x163138aa, 0xe38dee4d, 0xb0fb0e4e]
p=SM3(b+bytearray('202100150084', encoding='utf-8'),IV,0)
print("生成的随机消息+填充+附加消息：")
print(b+bytearray('202100150084', encoding='utf-8'))
print("它的hash值：")
print(p)





