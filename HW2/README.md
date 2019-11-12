OpenMP平行化CG共軛梯度法
===

[作業要求](https://people.cs.nctu.edu.tw/~ypyou/courses/PP-f19/assignments/HW2.pdf)

執行結果：  

| core\執行次數|    有平行化   |   無平行化   | 
|  --------   | --------  | -------- |
| 1           | 72s    | 70s   |
| 2           | 46s    | 71s   | 
| 4           | 37s    | 86s   | 

#### CG共軛演算法
[CG Algorithm]](https://ccjou.wordpress.com/2015/12/10/krylov-%E5%AD%90%E7%A9%BA%E9%96%93%E6%B3%95%E2%94%80%E2%94%80%E7%B7%9A%E6%80%A7%E6%96%B9%E7%A8%8B%E7%9A%84%E6%95%B8%E5%80%BC%E8%A7%A3%E6%B3%95-%E4%B8%89%EF%BC%9A%E5%85%B1%E8%BB%9B%E6%A2%AF%E5%BA%A6/)



#### 遇到的主要問題

> 遇到雙重for時應該挑外面的迴圈平行還是裡面的迴圈平行？  


這次的cg不像上次蒙地卡羅求圓面積這麼簡單，於是我只挑出for迴圈平行，但遇到雙重迴圈時，無法使用omp的`#pragma omp parallel for collapse(2)`，omp要使用者自己檢查沒有data race的問題，`collapse(2)`最好先確定內外迴圈不會有data race，所以我大多都只選其中一個迴圈平行。  

關於應該選裡面還是外面的迴圈，首先在這個例子中因為運算量很大，可以預期cpu會滿載，考慮到每次平行都會有個固定大小的overhead，應該要盡量選外面，除非(outer loop,inner loop)差到(100,10^10)這樣誇張。因為外面的迴圈所包的範圍較大，代表每次的平行化所涵蓋的內容最多，這樣可以使滿載情況下overhead所佔的比例下降；相反地如果選裡面，平行化範圍小造成每次平行化很快就完成，要多用幾次平行才可以讓程式在運行期間保持平行運算，造成overhead比例增高。  



