MPI平行化 simluates heat conduction algorithm
===

MPI筆記：[連結](https://hackmd.io/Sf2gGL79RqigJHRQjMNTpw)  

`/home/PP-f19/MPI/bin/mpicc 0513404.c -o 0513404 `
`time /home/PP-f19/MPI/bin/mpiexec -npernode 1 --hostfile hostfile 0513404 10000 10000 53342`  

無平行化：60s  
平行化後：15~17s  
