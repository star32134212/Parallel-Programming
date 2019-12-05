平行化Black-Scholes Model
===
`gcc final.c -o final -lm`   
`time ./final 100 1000000`  
第一個參數是分幾期 第二個參數是蒙地卡羅模擬次數  
`lm`是link libm，有用到`<math.h>`的話要用  