# 准备 #

需要准备的工具有：
  * SGOS2最新的源代码
  * sgos2\_bin GCC 编译器集合（包含GDB、QEMU虚拟机）
  * SGOS2开发使用的硬盘镜像文件

# 步骤 #
  1. 首先在http://code.google.com/p/sgos/downloads/list下载sgos2\_bin和sgos2硬盘镜像文件。
  1. 把sgos2\_bin解压至任意一个目录中。
  1. 在本地建立一个开发目录，打开命令提示符切换至当前目录。使用“svn checkout https://sgos.googlecode.com/svn/trunk/ sgos --username 用户名”下载源代码（必须要安装svn工具）。
  1. 在命令提示符中使用“path sgos\_bin的路径”设置编译器目录。
  1. 转至sgos2目录，把sgos2硬盘镜像文件也复制到这个地方。输入make命令，即可编译。
  1. 编译完成后，输入make windbg来调试内核。