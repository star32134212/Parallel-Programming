平行程式設計 108
===
一些會用到的資訊  
IP：140.113.215.195  
Port：37031 ~ 37034  
`ssh -p 37031 0513404@140.113.215.195`  
`gcc -pthread -std=gnu99 -O2 -s hw1.c -o pi`  
`time ./pi 2 1000000000` : 加time可以順便看時間  
`scp -P port_number FileName user@ip_address:/資料夾/` ： 可以從本地端傳檔案到遠端電腦  
`scp user@ip_address:檔案 .`：從遠端拉回來 `.`是目前資料夾  
該學GNU偵錯器(GDB)了！！  
