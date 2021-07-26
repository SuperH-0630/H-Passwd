# H-Password 密码管理软件

## 简介
 * H-Password可以帮助你更快的管理密码。通常，我们习惯将密码记录到本子或电脑上以防止忘记。但是名文记录的密码总是难免会泄露。
因此，我们可以使用H-Password管理密码。你只需要自定义并且记住一个密钥到脑子中即可。H-Passwd会根据你的密钥和账户信息生成
一个无规则的标签文本。你可以将该便签文本直接记录在任何地方，不用担心被人看见。当你需要寻找密码的时候，只需要启动H-Password
然后输入你的密钥和对应的标签即可获得相应的账号信息。  
 * 你的账号信息不会被H-Password存储在然和地方，包括你的电脑和云端。H-Password通过计算获取账号信息。  
 * 支持从文件中导入密码。
 * 支持将密码标签保存在`.hdp`文件中。

## 命令行参数
```
Usage: H_Passwd <[option]..> [key]
Option:
 -v --version       显示当前版本信息。
 -h --help          显示帮助文档。
 -s --set-pw        根据密钥和账户信息生成一个标签文本。
 -g --get-pw        根据密钥和标签文本获取账户信息。
 -t --tips          打印提示信息。
 -i --in-file       从文件中获取密码标签或保存密码标签到文件中。
 -p --print-label   打印文件中所有的密码标签。
 -d --delete-label  删除文件中指定的密码标签。
 -f --file          从.txt文件中导入账户信息。
 --set-tips         设置一个标签文本到文件中。
 --delete-old       删除同名的旧密码。
 -c                 检查key是否和可执行文件绑定的key相同。
```
若启用可执行文件绑定密钥，则`-i`以及依赖`-i`的选项将不能使用。  
若未启用可执行文件绑定密钥，则`-c`不能使用。  
若未启用可执行文件绑定密钥，则`-t`依赖于`-i`选项。  
`-p`，`-d`，`-f`，`--set-tips`，`--delete-old`选项依赖于`-i`选项。
## 文件模式
H-Passwd支持将长的不便记忆的密码标签存储到文件中，并且设置一个可自定义的名字与其对应。
可以使用`-p`参数查看文件中所有的密码标签及其对应的名字。具体使用可参见命令行参数介绍。  
`p`、`-d`等涉及文件操作的选项均依赖`-i`选项。使用`-i`选项后，密码标签存储在可执行文件
目录下的`passwd.hpd`文件中。  
H-Password使用md5校验码验证`hdp`的完整性，可以防止`hdp`被篡改。

## 可执行文件绑定密钥
在编译过程可以设置`cmake`参数：`WriteKey`，`UserName`以及`KeyTips`。设置后可以启用
该模式。该模式将key直接写入`exe`中，并且`UserName`和`KeyTips`作为`key`的提示信息。
启用该模式后，文件模式将无法使用。

## 文件导入
H-Password支持从`txt`文件中导入账号信息。  
账号信息以行为单位，按`账户 密码 "备注" "名字"`的格式。其中，账号和密码不需要引号，也不能
存在空格符。名字不是必须的，当不存在名字时，H-Password会使用备充当名字。

## 构建
本项目使用`cmake`管理生成，最低版本要求为`3.16`。
创建构建目录
```shell
$ mkdir build
$ cd build
```
使用`cmake`配置工程
```shell
$ cmake ..
```
若在`windows`下使用`Mingw gcc`编译，则执行
```shell
$ cmake .. -G "MinGW Makefiles"
```
使用`cmake`配置工程时，可以通过`-D`添加参数。
也可以使用`cmake gui`配置工程。
使用`MSVC`生成项目
```shell
$ MSBuild.exe .\ALL_BUILD.vcxproj
```
或使用`make`生成项目
```shell
$ make
```
项目生成后，即可正常运行可执行文件。
H-Password不需要安装即可运行。

## 声明
### 版权声明
版权所有 (c) 2021 [SuperHuan](https://github.com/SuperH-0630) 保留所有权利。  
H-Password由SuperHuan开发，技术归属SuperHuan。  
本授权协议适用于H-Password，SuperHuan拥有对本授权协议最终解释权和修改权。
### 免责声明
H-Password为免费开源程序。  
编译、运行该程序过程中造成的损失（包括但不限于系统损坏、文件丢失）均由您个人承担， 与开发者无关。
### LICENSE
该代码是在 MIT License 下发布的。