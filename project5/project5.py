import hashlib
from random import randint
from math import ceil,floor

def SHA256(str):
    sha256 = hashlib.sha256()
    sha256.update(str.encode('utf-8'))
    return sha256.hexdigest()

class child:
    def __init__(self):
        self.hash = None
        self.value = None
        self.parent = None
        self.left = None
        self.right = None
        
    def add(self, value, flag):
        self.value = str(value)
        if flag==0:
            self.hash = SHA256('0x00'+self.value)
        else:
            self.hash = SHA256('0x01'+self.value)
        
class Merkle_tree:
    def __init__(self):
        self.root = None
        self.children = []

    def create(self, children):
        a = []
        for i in children:
            c = child()
            c.add(i,0)
            a.append(c)
            self.children.append(c)
        while (len(a) > 1):
            b = []
            if(len(a) % 2 != 0):
                s = a.pop(-1)
                while(len(a) > 1):
                    d = child()
                    l = a.pop(0)
                    r = a.pop(0)
                    l.parent = d
                    r.parent = d
                    d.left = l
                    d.right = r
                    d.add(l.hash +r.hash,1)
                    b.append(d)
                    self.children.append(d)
                b.append(s)
            else:
                while(len(a) > 1):
                    d = child()
                    l = a.pop(0)
                    r = a.pop(0)
                    l.parent = d
                    r.parent = d
                    d.left = l
                    d.right = r
                    d.add(l.hash +r.hash,1)
                    b.append(d)
                    self.children.append(d)
            a = b
        self.root = self.children[-1]
        print("创造成功！")

    def inclusion(self, x):
        path = []
        flag = []
        for i in self.children:
            if str(x) == i.value:
                p = i
                break
        while p.parent is not None:
            if p.parent.left == p:
                q=p.parent.right.hash
                path.append(q)
                flag.append(0)
            else:
                q=p.parent.left.hash
                path.append(q)
                flag.append(1)
            p = p.parent
        s = SHA256('0x00'+str(x))
        tmp=0
        for i in path:
            if flag[tmp]==0:
                s = SHA256('0x01'+s+i)
            if flag[tmp]==1:
                s = SHA256('0x01'+i+s)
            tmp+=1
        if s == self.root.hash:
            print("证明成功！"+str(x)+" 在Merkle tree里")
        else:
            print("证明失败。")

    def exclusion(self, x):
        small=floor(x)
        big=ceil(x)
        path1 = []
        flag1 = []
        for i in self.children:
            if str(small) == i.value:
                p1 = i
                break
        while p1.parent is not None:
            if p1.parent.left == p1:
                q1=p1.parent.right.hash
                path1.append(q1)
                flag1.append(0)
            else:
                q1=p1.parent.left.hash
                path1.append(q1)
                flag1.append(1)
            p1 = p1.parent
        s1 = SHA256('0x00'+str(small))
        tmp1=0
        for i in path1:
            if flag1[tmp1]==0:
                s1 = SHA256('0x01'+s1+i)
            if flag1[tmp1]==1:
                s1 = SHA256('0x01'+i+s1)
            tmp1+=1
        path2 = []
        flag2 = []
        for i in self.children:
            if str(big) == i.value:
                p2 = i
                break
        while p2.parent is not None:
            if p2.parent.left == p2:
                q2=p2.parent.right.hash
                path2.append(q2)
                flag2.append(0)
            else:
                q2=p2.parent.left.hash
                path2.append(q2)
                flag2.append(1)
            p2 = p2.parent
        s2 = SHA256('0x00'+str(big))
        tmp2=0
        for i in path2:
            if flag2[tmp2]==0:
                s2 = SHA256('0x01'+s2+i)
            if flag2[tmp2]==1:
                s2 = SHA256('0x01'+i+s2)
            tmp2+=1
        if s1 == self.root.hash and s2 == self.root.hash:
            print("证明成功！"+str(x)+"不在Merkle tree里")

print("创造一个有100000个叶节点的Merkle tree")
lst = []
for i in range(100000):
    lst.append(i)
tree = Merkle_tree()
tree.create(lst)
print("Merkle tree的根的hash值：")
print(tree.root.hash)

'''
print("Merkle tree的结点：")#结果太长，这里注释掉了，若想查看这个有100000个叶节点的Merkle tree的结点，解注释这段即可
for i in tree.children:
    print(i)
    print(i.value)
    print(i.hash)
    print("$$$$$$$$$$$$$$$$$$")
'''
    
print("证明6在Merkle tree中：")
tree.inclusion(6)  #可改成0——99999中的任意一个整数，证明其在Merkle tree里

print("证明12.5不在Merkle tree中：")
tree.exclusion(12.5)#可改成0——99999中的任意一个非整数，证明其不在Merkle tree里



