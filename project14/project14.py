import binascii
from gmssl import sm2, func
from Crypto.Cipher import AES
import base64
import random
import string
import socket

private = 'c124889fb35a23cd3092a62f92ad9f3aafd07876c5bb08bf147ac27b43e15ec7'
public = '04aaf5c364472a7b26ab254a834f5b8104ef06387ea7cc9104dd183c6ace0a647a70f19e704721919bcf955a1be8c69c9a9ead286a7d1fdfe067145244c377e1ce'

#发送者
def K_random(length):
    characters = string.ascii_letters + string.digits
    random_string = ''.join(random.choice(characters) for _ in range(length))
    return random_string

def AES_enc(m,key):
    vi = '12345678abcdefgh'
    m = (lambda s: s + (16 - len(s) % 16) * chr(16 - len(s) % 16))(m)
    p = AES.new(key.encode('utf8'), AES.MODE_CBC, vi.encode('utf8'))
    e = p.encrypt(m.encode('utf8'))
    return base64.b64encode(e).decode('utf8')

def sm2_enc(k):
    sm2_crypt = sm2.CryptSM2(public_key=public, private_key=private)
    enc_data = sm2_crypt.encrypt(k)
    return enc_data

def sign(x):
    sm2_crypt = sm2.CryptSM2(public_key=public, private_key=private)
    random_hex_str = func.random_hex(sm2_crypt.para_len)
    sign = sm2_crypt.sign(x, random_hex_str)
    return sign

#接收者
def AES_dec(c,key):
    vi = '12345678abcdefgh'
    e = base64.decodebytes(c.encode('utf8'))
    p = AES.new(key.encode('utf8'), AES.MODE_CBC, vi.encode('utf8'))
    m = p.decrypt(e)
    m = (lambda s: s[0:-s[-1]])(m)
    return m.decode('utf8')

def sm2_dec(c):    
    sm2_crypt = sm2.CryptSM2(public_key=public, private_key=private)    
    dec_data = sm2_crypt.decrypt(c)
    #print(b"dec_data:%s" % dec_data)
    return dec_data

def sm2_ver(sign,k):
    sm2_crypt = sm2.CryptSM2(public_key=public, private_key=private) 
    verify = sm2_crypt.verify(sign, k)
    return verify

#开始
print("#############发送者部分###############")
m="202100150084"
print("要发送的消息：")
print(m)
k = K_random(16)   #随机生成一个AES加密的密钥
print("随机生成的AES加密的密钥为：")
print(k.encode('utf8'))
a=AES_enc(m,k)     #用对称加密AES加密消息m
b=sm2_enc(k.encode('utf8'))    #用sm2加密AES的密钥k
s=sign(k.encode('utf8'))    #签名，用于接收者验证
print("发送成功")   #把a,b,s发送给接收者

print("#############接收者部分###############")
key=sm2_dec(b)    #解密sm2得到AES密钥
print("接收到的AES加密的密钥为：")
print(key)        
p=sm2_ver(s,key)  #验证签名，确定AES密钥的正确性
if p:
    m=AES_dec(a,key.decode('utf8'))  #如果AES密钥正确，用其解密AES得到消息m
    print("接收成功，消息为：")
    print(m)


  
