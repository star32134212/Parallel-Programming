平行程式設計 108
===
一些會用到的資訊  
`gcc -pthread -std=gnu99 -O2 -s hw1.c -o pi`  
`time ./pi 2 1000000000` : 加time可以順便看時間  
`scp -P port_number FileName user@ip_address:/資料夾/檔案名` ： 可以從本地端傳檔案到遠端電腦  
`scp user@ip_address:檔案 .`：從遠端拉回來 `.`是目前資料夾  
上課提供的遠端機沒辦法處理zip，改傳tar過去  
`tar cvf FileName.tar DirName`壓縮  
`tar xvf FileName.tar`解壓縮  
該學GNU偵錯器(GDB)了！！  
