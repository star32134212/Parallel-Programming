CUDA平行化 wave equation
===

CUDA筆記：[連結](https://hackmd.io/Sf2gGL79RqigJHRQjMNTpw)  

`gcc 0513404.c -o 0513404 -lm `  serial  
`nvcc 0513404.cu -o 0513404 ` 用CUDA  
`time ./0513404 1000000 10000 > output2`
無平行化：3~4m  
平行化後：1~2s  

結論：快好多啊啊啊!!!!!!  