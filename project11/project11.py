import hashlib

def SHA256(str):
    sha256 = hashlib.sha256()
    sha256.update(str.encode('utf-8'))
    return sha256.hexdigest()

class SM3:
    def __init__(self, encoding='ascii'):  
        self.IV = [0x7380166f, 0x4914b2b9, 0x172442d7, 0xda8a0600, 0xa96f30bc, 0x163138aa, 0xe38dee4d, 0xb0fb0e4e]
        self.encoding = encoding
 
    def cshift_left(self, x, l):
        while l >= 32:
            l -= 32
        x = x & 0xffffffff
        bin_x = '{:032b}'.format(x)
        bin_x = bin_x[l:] + bin_x[:l]
        return int(bin_x, 2)
 
    def Tj(self, j):
        if (j < 16):
            return 0x79cc4519
        else:
            return 0x7a879d8a
 
    def FFj(self, x, y, z, j):
        if j < 16:
            return x ^ y ^ z
        else:
            return (x & y) | (x & z) | (y & z)
 
    def GGj(self, x, y, z, j):
        if j < 16:
            return x ^ y ^ z
        else:
            return (x & y) | (~x & z)
 
    def P0(self, x):
        return x ^ self.cshift_left(x, 9) ^ self.cshift_left(x, 17)
 
    def P1(self, x):
        return x ^ self.cshift_left(x, 15) ^ self.cshift_left(x, 23)
 
    def padding(self, msg):
        msg_len = len(msg)
        msg_blen = msg_len << 3
        m, n = msg_len >> 2, msg_len & 3
        block = []
        one_block = []
        if type(msg) == type(''):
            bt_msg = msg.encode(encoding=self.encoding, errors='strict')
        elif type(msg) == type(0):
            bt_msg = msg.to_bytes((msg.bit_length() + 7) // 8, "big")
        else:
            bt_msg = msg
        for i in range(m):
            wd = bt_msg[0] << 24 | bt_msg[1] << 16 | bt_msg[2] << 8 | bt_msg[3]
            one_block.append(wd)
            bt_msg = bt_msg[4:]
            if i & 15 == 15:
                block.append(one_block.copy())
                one_block.clear()
        if n == 0:
            new_wd = 0x80 << 24
        elif n == 1:
            new_wd = bt_msg[0] << 24 | 0x80 << 16
        elif n == 2:
            new_wd = bt_msg[0] << 24 | bt_msg[1] << 16 | 0x80 << 8
        else:
            new_wd = bt_msg[0] << 24 | bt_msg[1] << 16 | bt_msg[2] << 8 | 0x80
        one_block.append(new_wd)
        ob_len = len(one_block)
        if ob_len <= 14:
            for i in range(14 - ob_len):
                one_block.append(0)
            one_block.append(msg_blen >> 32)
            one_block.append(msg_blen & 0xffffffff)
            block.append(one_block.copy())
        else:
            for i in range(16 - ob_len):
                one_block.append(0)
            block.append(one_block.copy())
            one_block = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, msg_blen >> 32, msg_blen & 0xffffffff]
            block.append(one_block.copy())
        return block
 
    def CF(self, V, B):
        W0, W1 = B.copy(), []
        for i in range(16, 68):
            wd = self.P1(W0[i - 16] ^ W0[i - 9] ^ self.cshift_left(W0[i - 3], 15)) ^ self.cshift_left(W0[i - 13], 7) ^ \
                 W0[i - 6]
            W0.append(wd)
        for i in range(64):
            W1.append(W0[i] ^ W0[i + 4])
        A, B, C, D, E, F, G, H = V
        for i in range(64):
            SS1 = (self.cshift_left(self.cshift_left(A, 12) + E + self.cshift_left(self.Tj(i), i), 7)) & 0xffffffff
            SS2 = SS1 ^ self.cshift_left(A, 12)
            TT1 = (self.FFj(A, B, C, i) + D + SS2 + W1[i]) & 0xffffffff
            TT2 = (self.GGj(E, F, G, i) + H + SS1 + W0[i]) & 0xffffffff
            D = C
            C = self.cshift_left(B, 9)
            B = A
            A = TT1
            H = G
            G = self.cshift_left(F, 19)
            F = E
            E = self.P0(TT2)
        return A, B, C, D, E, F, G, H
 
    def compression(self, msg):
        block = self.padding(msg)
        V = self.IV
        for bi in block:
            res = self.CF(V, bi)
            for i in range(8):
                V[i] = V[i] ^ res[i]
        res = b''
        for v in V:
            res = res + v.to_bytes(4, "big")
        return res
 
 



class ECC_2m:
    def __init__(self,poly,Gx,Gy,ecc_a=1,ecc_b=1):
        self.poly=poly
        self.ecc_a=ecc_a
        self.ecc_b=ecc_b
        self.Gx=Gx
        self.Gy=Gy
 
    def gf2_divmod(self,a, b):
        if b == 0:
            raise ZeroDivisionError
        ans = 0
        digit_a, digit_b = a.bit_length(), b.bit_length()
        while not a < b:
            rec = digit_a - digit_b
            a = a ^ (b << rec)
            ans = ans | (1 << rec)
            digit_a = a.bit_length()
        return ans, a
 
    def gf2_ex_gcd(self,a,b):
        x1, y1, x2, y2 = 1, 0, 0, 1
        while b:
            q, r = self.gf2_divmod(a, b)
            a, b = b, r
            x1, x2 = x2, x1 ^ self.gf2_mul(q, x2)
            y1, y2 = y2, y1 ^ self.gf2_mul(q, y2)
        return a, x1, y1
 
    '''res = a*b mod poly'''
    def gf2_mul(self,a,b):
        ans = 0
        digit_1 = self.poly.bit_length() - 1
        while b:
            if b & 1:
                ans = ans ^ a
            a, b = a << 1, b >> 1
            if a >> digit_1:
                a = a ^ self.poly
        return ans
 
    '''res = a^-1 mod poly'''
    def gf2_inverse(self,a):
        x1, x2 = 1, 0
        b = self.poly
        while b:
            q, r = self.gf2_divmod(a, b)
            a, b = b, r
            x1, x2 = x2, x1 ^ self.gf2_mul(q, x2)
        return x1
 
    '''res = a^k mod poly'''
    def gf2_quick_pow_mod(self,a,k):
        res = 1
        while k:
            if k & 1:
                res = self.gf2_mul(res, a)
            k = k // 2
            a = self.gf2_mul(a, a)
        return res
 
    '''仿射坐标点加运算'''
    def affine_pt_add(self,x1,y1,x2,y2):
        inv=self.gf2_inverse(x2^x1)
        nmd=self.gf2_mul(y2 ^ y1, inv)
        nmd2=self.gf2_mul(nmd, nmd)
        x3=nmd2^nmd^x1^x2^self.ecc_a
        t=self.gf2_mul(x1 ^ x3, nmd)
        y3=t^x3^y1
        return x3,y3
 
    '''仿射坐标倍点运算'''
    def affine_pt_double(self,x1,y1):
        inv=self.gf2_inverse(x1)
        nmd=x1^self.gf2_mul(y1, inv)
        nmd2=self.gf2_mul(nmd, nmd)
        x2=nmd2^nmd^self.ecc_a
        xx1=self.gf2_mul(x1, x1)
        t=self.gf2_mul(nmd ^ 1, x2)
        y2=xx1^t
        return x2,y2
 
    '''标准投影坐标点加运算'''
    def sproject_pt_add(self,x1,z1,x2,z2,Px):
        p=self.gf2_mul(x1, z2)
        q=self.gf2_mul(x2, z1)
        z3=self.gf2_mul(p ^ q, p ^ q)
        m=self.gf2_mul(p, q)
        n=self.gf2_mul(Px,z3)
        x3=m^n
        return x3,z3
 
    '''标准投影坐标倍点运算'''
    def sproject_pt_double(self, x1, z1):
        xx1=self.gf2_mul(x1, x1)
        zz1=self.gf2_mul(z1,z1)
        z2=self.gf2_mul(xx1,zz1)
        x14=self.gf2_mul(xx1,xx1)
        z14=self.gf2_mul(zz1,zz1)
        bz=self.gf2_mul(self.ecc_b,z14)
        x2=x14^bz
        return x2,z2
 
    '''仿射坐标下二进制展开法多倍点运算'''
    def affine_pt_mul(self,k,Px,Py):
        lbk=bin(k)[3:]
        Qx,Qy=Px,Py
        for i in lbk:
            Qx,Qy=self.affine_pt_double(Qx,Qy)
            if int(i):
                Qx, Qy=self.affine_pt_add(Qx,Qy,Px,Py)
        return Qx,Qy
 
    '''标准投影坐标下蒙格玛利多倍点运算'''
    def montgomery_pt_mul(self,k,Px,Py):
        lbk = bin(k)[3:]
        x1,z1=Px,1
        z2=self.gf2_mul(Px,Px)
        x2=self.gf2_mul(z2,z2)^self.ecc_b
        for i in lbk:
            if int(i):
                x1,z1=self.sproject_pt_add(x1,z1,x2,z2,Px)
                x2,z2=self.sproject_pt_double(x2,z2)
            else:
                x2,z2=self.sproject_pt_add(x2,z2,x1,z1,Px)
                x1,z1=self.sproject_pt_double(x1,z1)
        t0=self.gf2_mul(Px,z1)
        t1=self.gf2_mul(t0,z2)
        inv=self.gf2_inverse(t1)
        t2=self.gf2_mul(Px,z2)
        t3=self.gf2_mul(t2,x1)
        Qx=self.gf2_mul(t3,inv)
        t4=self.gf2_mul(t0,x2)
        m=self.gf2_mul(t4,inv)
        t5=self.gf2_mul(Qx^Px,m^Px)
        t6=self.gf2_mul(Px,Px)
        t7=t5^t6^Py
        t8=self.gf2_mul(t7,Qx^Px)
        t9=self.gf2_mul(z1,z2)
        t10=self.gf2_mul(t8,t9)
        Qy=self.gf2_mul(t10,inv)^Py
        return Qx,Qy
 
 


class SM2_2m:
    def __init__(self, poly, Gx=0, Gy=0, n=0, ecc_a=1, ecc_b=1, dA=0, pA_x=0, pA_y=0, ID='', encoding='ascii'):
        self.poly = poly
        self.encoding = encoding
        self.set_ecc(Gx, Gy, n, ecc_a, ecc_b)
        self.set_key(dA, pA_x, pA_y)
        self.set_ID(ID)
 
    def set_ecc(self, Gx, Gy, n, ecc_a, ecc_b):
        self.Gx = Gx
        self.Gy = Gy
        self.n = n
        self.ecc_a = ecc_a
        self.ecc_b = ecc_b
 
    def set_key(self, dA, pA_x, pA_y):
        self.dA = dA
        self.pA_x = pA_x
        self.pA_y = pA_y
 
    def set_ID(self, ID):
        if type(ID) == type(''):
            self.ID = ID.encode(encoding=self.encoding, errors='strict')
        elif type(ID) == type(0):
            self.ID = ID.to_bytes((ID.bit_length() + 7) // 8, "big")
        else:
            self.ID = ID
        blen_ID = (len(self.ID) << 3) & 0xffff
        h = (blen_ID >> 8).to_bytes(1, "big")
        l = (blen_ID & 0xff).to_bytes(1, "big")
        self.ENTLA = h + l
 
    def pf_pow(self, a, k):
        e = k % (self.n - 1)
        if e == 0: return 1
        lbe = bin(e)[3:]
        x = a
        for i in lbe:
            x = x * x % self.n
            if int(i): x = a * x % self.n
        return x
 
    def pf_inverse(self, a):
        return self.pf_pow(a, self.n - 2)
 
    def get_ZA(self):
        msg = self.ENTLA + self.ID
        bytes_len = (self.poly.bit_length() + 6) // 8
        bytes_a = self.ecc_a.to_bytes(bytes_len, "big")
        bytes_b = self.ecc_b.to_bytes(bytes_len, "big")
        bytes_Gx = self.Gx.to_bytes(bytes_len, "big")
        bytes_Gy = self.Gy.to_bytes(bytes_len, "big")
        bytes_Ax = self.pA_x.to_bytes(bytes_len, "big")
        bytes_Ay = self.pA_y.to_bytes(bytes_len, "big")
        msg = msg + bytes_a + bytes_b + bytes_Gx + bytes_Gy + bytes_Ax + bytes_Ay
        sm3 = SM3()
        ZA = sm3.compression(msg)
        return ZA
 
    def get_e(self, msg):
        if type(msg) == type(''):
            bt_msg = msg.encode(encoding=self.encoding, errors='strict')
        elif type(msg) == type(0):
            bt_msg = msg.to_bytes((msg.bit_length() + 7) // 8, "big")
        else:
            bt_msg = msg
        M = self.get_ZA() + bt_msg
        sm3 = SM3()
        bytes_e = sm3.compression(M)
        e = int.from_bytes(bytes_e, 'big')
        return e
 
    def signature(self, k, msg):
        e = self.get_e(msg)
        ECC = ECC_2m(self.poly, self.Gx, self.Gy, self.ecc_a, self.ecc_b)
        x1, y1 = ECC.montgomery_pt_mul(k, self.Gx, self.Gy)
        r = (e + x1) % self.n
        inv_dA = self.pf_inverse(1 + self.dA)
        rdA = (k - r * self.dA) % self.n
        s = inv_dA * rdA % n
        bytes_r = r.to_bytes(32, "big")
        bytes_s = s.to_bytes(32, "big")
        return bytes_r, bytes_s
 
    def verify(self, msg, r, s):
        e = self.get_e(msg)
        r = int.from_bytes(r, 'big')
        s = int.from_bytes(s, 'big')
        t = (r + s) % self.n
        ECC = ECC_2m(self.poly, self.Gx, self.Gy, self.ecc_a, self.ecc_b)
        xx1, yy1 = ECC.montgomery_pt_mul(s, self.Gx, self.Gy)
        xx2, yy2 = ECC.montgomery_pt_mul(t, self.pA_x, self.pA_y)
        x1, y1 = ECC.affine_pt_add(xx1, yy1, xx2, yy2)
        R = (e + x1) % self.n
        if R == r:
            print("****验证通过****")
        else:
            print("####验证失败####")
 
 
if __name__ == '__main__':
    poly = 0x20000000000000000000000000000000000000000000000000000000000001001
    Gx = 0xCDB9CA7F1E6B0441F658343F4B10297C0EF9B6491082400A62E7A7485735FADD
    Gy = 0x13DE74DA65951C4D76DC89220D5F7777A611B1C38BAE260B175951DC8060C2B3E
    n = 0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBC972CF7E6B6F900945B3C6A0CF6161D
    b = 0xE78BCD09746C202378A7E72B12BCE00266B9627ECB0B5A25367AD1AD4CC6242B
    a = 0x0
    dA = 0x771EF3DBFF5F1CDC32B9C572930476191998B2BF7CB981D7F5B39202645F0931
    pA_x = 0x165961645281A8626607B917F657D7E9382F1EA5CD931F40F6627F357542653B2
    pA_y = 0x1686522130D590FB8DE635D8FCA715CC6BF3D05BEF3F75DA5D543454448166612
    ID = 0x414C494345313233405941484F4F2E434F4D
    msg = "message digest"
    #为了遵循RFC6979改变的地方
    sm3 = SM3()
    k=SHA256(str(dA)+str(sm3.compression(msg)))
    k=int(k,16)
    #即k = SHA256(d + HASH(m))
    sm2 = SM2_2m(poly, Gx, Gy, n, a, b, dA, pA_x, pA_y, ID)
    r, s = sm2.signature(k, msg)
    print("开始签名：\nr:", end="")
    i = 0
    for rr in r:
        print("%02x" % rr, end="")
    print("\ns:", end="")
    for ss in s:
        print("%02x" % ss, end="")
    print("\n开始验签：")
    sm2.verify(msg, r, s)
