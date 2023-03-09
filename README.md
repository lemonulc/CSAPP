# CSAPP
CSAPP Lab
2022-09-01至2022-12-01


**Data Lab**，在题目限制下实现相关二进制操作和浮点数操作。
- 主要修改 Data Lab/datalab-handout/ 文件夹下的 bits.c 文件

**Bomb Lab**，个人认为最有趣的Lab，通过对汇编代码debug找出方法运行所需要的参数。
- 答案为 Bomb Lab/bomb/ 文件夹下的 ans.txt 文件，每一行是一个方法的参数。

**Attack Lab**，就是让我们破坏程序的运行顺序，让程序执行我们指定的代码。

- 答案为 Attack Lab/target1/ans/ 文件夹下的 touch1 - touch4，主要是通过缓冲区溢出，修改函数的返回位置，或者是传入特殊的二进制串，让程序运行二进制串对应的指令。

**Cache Lab**，分为两部分：
- Part A：缓存模拟器，个人使用LRU策略进行缓存模拟
- Part B：对矩阵转置函数进行优化，利用局部性原理，对矩阵进行分块处理

主要修改 Cache Lab/cachelab-handout/ 文件夹下的 csim.c，trans.c 文件，笔记地址：[Cache Lab](https://2273835878.notion.site/Cache-Lab-9dcc32219a5849249951dcf7f624bce7)  

**Malloc Lab**，模拟内存分配：见笔记 [Malloc Lab](https://2273835878.notion.site/Malloc-Lab-01b40eee597e4584b18d251164972a94)

**Shell Lab**，模拟Shell终端，主要考察线程的和信号的一些操作，修改的文件为 Shell Lab/shlab-handout/tsh.c 文件
  




