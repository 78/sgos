# Introduction #

本页介绍如何在sgos2上写一个hello world程序


# Details #

下面介绍的是Windows上的编写方法。

首先要下载sgos2\_bin编译环境，然后建立一个hello.cpp

输入以下内容：

#include <stdio.h>

int main()
{

> printf("Hello world.\n");

> return 0;

}

把libc.dll(/apps/libc)和crt.a(/crt)复制到当前目录下。

在命令行里编译

g++ hello.cpp -c -nostdlib -o hello.o

ld hello.o crt.a libc.dll -o hello.exe

ld2 hello.exe hello.run

接着把hello.run导入到镜像文件中去

wf hd.img -src hello.run -dest sgos/hello.run

然后编辑scripts/menu.lst，添加加载模块
module sgos2/hello.run

最后，用虚拟机运行hd.img即可看到Hello world.的输出了。