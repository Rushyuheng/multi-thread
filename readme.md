# Multithread Report

> Author INFO：
> F74076019
> 黃宇衡
> 資工111

## Develope Environment (Virtual Machine)
* OS : Ubuntu 18.04.2 LTS
* CPU : Intel(R) Core(TM) i7-9700 CPU @ 3.00GHz * 4
* System Memory : 8G
* HardDisk capacity：80G
* Programming Landuage : C++ 
* Compiler : g++ (Ubuntu 7.5.0-3ubuntu1~18.04) 7.5.0
* VMware : Oracle Virtual Box  6.1.14 r140239 (Qt5.6.2)

## How to run my code
```shell=
# compile
g++ -std=c++11 -pthread csv2json.cpp -o csv2json

# execution
./csv2json [number of threads]

```
ex:
```shell =
g++ -std=c++11 -pthread csv2json.cpp -o csv2json
./csv2json 8
```

## Program Developement & Runtime result
### 開發邏輯：
本次使用C++提供的thread class來實做multithread的部分，為了平行處理將程式規劃成三階段：CSV讀取、資料處理、JSON格式輸出，其中提供使用者可變數量的threads來做資料處理的階段。
1. CSV讀入：將CSV的每一行當作一個長字串儲存在1D vector裡，index為它CSV檔中的行號減一。
2. 資料處理：c++ 提供的thread class非常方便，只要寫一個function當作thread的功能，在初始化thread時將它作為參數傳入即可，而我使用vector來管理multithread，就可以依照使用者的需求來生成指定數目的threads。
    在thread的功能上我寫了一個function叫StringSeparate()，主要就是將大字串按照分隔符號拆解成個別的小字串並將結果全部儲存在一個2D vector result中，其主要使用c++ string class 的find()來找到分隔符號，並用substr()來切割token，最後用erase()來清除已經被存取過的substring。
    因為是多個threads同時輸入同一個1D vector並寫入同一個2D vector，必須保證每個thread互相獨立，於是加入了兩個參數用來標記該thread要處理的input vector index是從哪裡開始並且到哪裡結束(該數字同時也是result vector寫入的row number)，以確保thread之間沒有重疊而達到正確的平行化。
4. JSON格式輸出：用兩層的巢狀迴圈將result的結果一一讀出，並按照``.json``的格式寫檔。因為最後一個元素後不能有逗號，實作時將最後一個元素的輸出獨立於迴圈之外。
:::warning
一開始參考教授上課的簡報時有考慮連輸入輸出都做multithreads，但這樣要考慮使用者給定的數字要如何分配給三階段以及操作file pointer的問題，於是轉將輸入輸出都限制在單執行緒，可變量的多執行緒都拿來做資料處理
:::
### 執行時間結果：
:::warning
在重複資料讀取時，linux會將資料讀入cache，為了避免影響實驗結果，測定資料讀取階段的資源使用狀況前會先執行指令清空cache
```shell=
sync; 
echo 3 | sudo tee /proc/sys/vm/drop_caches
```
:::
為了檢視各階段的時間，我加入了新的時間輸出方便觀察，該時間是以程式開始執行後累加，主要觀察結束資料讀入到資料分割完(tokenizing)的時間
![](https://i.imgur.com/v8VIuRk.png)
![](https://i.imgur.com/u4GVWqc.png)
![](https://i.imgur.com/6Igcod6.png)
![](https://i.imgur.com/stWsypI.png)

由於虛擬機的處理器只有4顆，根據[wikichip](https://en.wikichip.org/wiki/intel/core_i7/i7-9700)的資料，i7-9700一顆核心最多只能負荷一個thread，因此當thread數超過4時，切割字串的時間就無法快很多了

## Analysis Report 
### Program optimization
* 在分配thread個別要處理的行數時，原本是用比較簡單的做法：將CSV的行數除以thread數，其餘數全部加在最後一個thread上。
    比如：CSV有500行，thread有3個，500/3 = 166餘2，那麼thread1會負責處理前166行，thread2會處理中間166行，thread3則是分到最後166+2=168行。
    乍看之下差異不大，但若今天thread數增加時，可能的餘數也會變大，個別要thread要處理的行數會越差越多，為了均衡負荷而改用了新的分配法：
    ```c++=
    for(int i = 0;i < numberOfThread;++i){
		//load balacing
		threadCap = averageThreadCap;
		if(remainer > 0){
			++threadCap;
			--remainer;
		}
    ```
    先算出平均一個thread要處理多少行，再看餘數的個數，如果有還有剩，就分配一個給接下來要呼叫的thread，如此一來上面的例子就會變成thread1會負責處理前167行，thread2會處理中間167行，thread3則是處理最後166行。

### OS Performance Analysis
#### Resource usuage in each stage

* Disk I/O
    為了記錄disk I/O使用iotop -ao紀錄「隨時間累積」的I/O狀態
    
    * 資料讀取：
    ![](https://i.imgur.com/PllM8GB.png)
    可以看到程式開始執行讀取資料時command``./csv2json``有disk read
    * 資料處理：
    ![](https://i.imgur.com/VqUnZfO.png)
    第二次執行程式在進入tokenize階段後，iotop完全沒記錄到``./csv2json``任何「累積活動」，表示此階段前沒有disk I/O(會沒有disk read的是因為資料上次被讀進到cache裡了)
    * 資料寫入：
    ![](https://i.imgur.com/9G9UZBg.png)
    第三次執行程式在進入寫入階段後，iotop最底下記錄到``./csv2json``開始有累積disk write，表示此階段才開始disk write(會沒有disk read的是因為資料第一次被讀進到cache裡了)
    
    :::warning
    在multithread作用的資料處理階段沒有disk I/O因此這裡不討論開多個thread後的狀況
    :::
    
* memory usuage & CPU usage plot

    本階段使用工具[psrecord](https://github.com/astrofrog/psrecord)繪製曲線圖
    指令如下
    ```shell=
    precord [command to run] --plot [plot file name] --include-children --interval [time interval to sample]

    psrecord "./csv2json 4" --plot plot1.png --include-children --interval 0.1
    ```

    搭配各狀態的完成時間對照圖表X軸
    * one thread
       ![](https://i.imgur.com/62xVEhD.png)
       ![](https://i.imgur.com/WPxE4Yn.png)

    * two threads
        ![](https://i.imgur.com/4Wn7Tbh.png)
        ![](https://i.imgur.com/iD9Vqy6.png)

    * four threads
        ![](https://i.imgur.com/uQfhVgX.png)
        ![](https://i.imgur.com/fzD1o06.png)
    
    * eight threads
        ![](https://i.imgur.com/e5Qvr1A.png)
        ![](https://i.imgur.com/Okp1rPe.png)

* Memory usuage
    記憶體的使用狀況不管幾個thread均是在資料讀入時開始逐漸使用，直到結束前均處於將整個檔案讀入加上程式變數的大小。

* CPU usuage
    CPU的使用在資料讀入階段約是50-80%。
    在資料處理階段會因為開的thread數不同而不一樣，一個thread時大約就是100%，兩個thread則是200%，四個thread是400%，四以上的thead數因為硬體限制而還是最多只能400%。由這邊的結果可以得知c++的thread class不會new一個出來就真的佔用一個硬體thread。
    在資料輸出階段則是0-100%跳動，推測是檔案寫入時有部分時間會去等disk I/O。

* Concludsion
    1.從c++可以new超過硬體限制的thread數得知，作業系統需要提供軟體上的thread數和硬體上thread數的對應，以及分配該thread要給哪個core處理。
    3.不同CPU核心的thread硬體限制不同，如i7-9700是8core/8thread，i7-9900則是8core/16thread，也存在hyper-theading的技術實現在一個實體CPU中，提供兩個logical thread。
    4.資料讀取時會將資料暫放在cache上，導致每次實驗前需清空cache，得知作業系統也需要主動管理cache的使用。
    5.這支程式的bottleneck是資料輸出階段，要改善效能應優先處理該部分，看eight threads的cpu使用圖會發現在20-25秒間cpu使用率並非100%，但重複實驗偶爾也會出現輸出階段全部100%的狀況，僅推測是某些情況下會觸發cpu要去等disk I/O，或是採樣區間(interval)造成的差異。

### Reference
* [psrecord](https://github.com/astrofrog/psrecord)
* [wikichip](https://en.wikichip.org/wiki/intel/core_i7/i7-9700)
* [Hyper-threading](https://en.wikipedia.org/wiki/Hyper-threading)