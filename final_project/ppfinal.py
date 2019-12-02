import math
import numpy as np
import matplotlib.pyplot as plt
from scipy.stats import norm

#定價模型公式
#call=期權初始價格
#S=交易的金融資產現價
#L=期權交割價格
#T=期權期效
#r=無風險利率
#vol^2=年度化方差(類似風險)
def blsprice(S,L,T,r,vol):
    d1 = (math.log(S/L)+(r+0.5*vol*vol)*T)/(vol*math.sqrt(T))
    d2 = (math.log(S/L)+(r-0.5*vol*vol)*T)/(vol*math.sqrt(T))
    call = S*norm.cdf(d1)-L*math.exp(-r*T)*norm.cdf(d2)
    return call

#蒙地卡羅模擬
#N=期 (想成是一個金融商品可以做交易的頻率)
#如果一年有100天可以做交易 N=100 則這個商品每365/100天就會有一個價格
def MCsim(S,T,r,vol,N):
    dt = T/N
    St = np.zeros((N+1))
    St[0] = S
    for i in range(N):
        St[i+1] = St[i]*math.exp((r-0.5*vol*vol)*dt+np.random.normal()*vol*math.sqrt(dt))
    return St

#假設有一個期權的資訊如下
S = 50
L = 40
T = 2
r = 0.08
vol = 0.2
N = 100
bls = blsprice(S,L,T,r,vol)
print("bls定價模型算出之價格： " + str(bls)) 

M = 20000 #模擬20000次，需要平行的地方
call = 0
for i in range(M):
    Sa = MCsim(S,T,r,vol,N) #Sa存每一期變動完的價格
    plt.plot(Sa)
    if(Sa[-1]-L>0): #有大於0才會執行(才有獲利)
        call += (Sa[-1]-L) ##算期望值

mp = call/M*math.exp(-r*T) #預計可以賺多少錢
print("蒙地卡羅預測可獲利： " + str(mp))
plt.show()

dif = abs(mp - bls)
print("誤差： " + str(dif))

