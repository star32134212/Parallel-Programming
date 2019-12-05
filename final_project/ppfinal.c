#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int M;//模擬次數
int N;//期

double normalCDF(double value)
{
   return 0.5 * erfc(-value * M_SQRT1_2);
}

float mean = 0;
float std = 1;

float normal(){
    float u,v,x;
    u = rand() / (double)RAND_MAX;
    v = rand() / (double)RAND_MAX;
    x = sqrt(-2 * log(u)) * cos(2 * M_PI * v) * std + mean;
    return x;
}


//定價模型公式
//call=期權初始價格
//S=交易的金融資產現價
//L=期權交割價格
//T=期權期效
//r=無風險利率
//vol^2=年度化方差(類似風險)
float blsprice(float S,float L,float T,float r,float vol){
    float d1,d2,call;
    d1 = (log(S/L)+(r+0.5*vol*vol)*T)/(vol*sqrt(T));
    d2 = (log(S/L)+(r-0.5*vol*vol)*T)/(vol*sqrt(T));
    call = S*normalCDF(d1)-L*exp(-r*T)*normalCDF(d2);
    return call;
}


//蒙地卡羅模擬
//N=期 (想成是一個金融商品可以做交易的頻率)
//如果一年有100天可以做交易 N=100 則這個商品每365/100天就會有一個價格
float MCsim(float S,float T,float r,float vol,int N){
    float dt = T/N;
    float St[N+1];
    St[0] = S;
    for(int i = 0; i < N; i++){
        St[i+1] = St[i]*exp((r-0.5*vol*vol)*dt+normal()*vol*sqrt(dt));
    }
    return St[N];
}


//假設有一個期權的資訊如下
float S = 50.0;
float L = 40.0;
float T = 2.0;
float r = 0.08;
float vol = 0.2;
float call = 0;

int main(int argc , char *argv []) { 
    sscanf(argv[1],"%d" ,&N); 
    sscanf(argv[2],"%d" ,&M); 
    float bls,mp;
    double dif;
    float Sa;
    bls = blsprice(S,L,T,r,vol);
    printf("bls定價模型算出之價格 %f\n",bls);

    for(int j = 0; j < M; j++){
        Sa = MCsim(S,T,r,vol,N); //Sa存每一期變動完的價格
        if(Sa-L>0){ //有大於0才會執行(才有獲利)
            call += (Sa-L); //算期望值
        } 
    }

    mp = call/M*exp(-r*T); 
    printf("蒙地卡羅預測可獲利 %f\n",mp);
    //dif = fabs(mp - bls);
    //printf("誤差 ％lf\n",dif);
    return 0;
}