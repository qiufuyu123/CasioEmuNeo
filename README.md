# CasioEmuNeo

[中文版本](./README.md) | [English](./docs/README_en.md)

卡西欧classwizard系列模拟器，汇编器，调试器，rop自动注入器

## 教程
- [基础界面操作](./docs/intro_ui.md)
- [汇编器使用](./docs/intro_asm.md)
- [rop注入/调试](./docs/intro_rop.md)

## 构建
0. 安装 xmake & Mingw
   1. `curl -fsSL https://xmake.io/shget.text | bash`   

   2. 安装并配置 Mingw64
1. 构建模拟器  
   ```
   cd emulator
   xmake f -p mingw
   xmake
   xmake run CasioEmuX ../models/fx991cnx
   ```  

2. *可选* `反编译器`  
   ```
   cd disas
   make
   ```
3. *可选* 构建机型  
	下载对应机型的 `rom`,命名为 `rom.bin` 放在models 目录下对应名称的目录内  
    **注意:**  
    **由于casio版权问题，源码不包含任何rom,rom文件请自行找资源下载**  
    **如果想要fx991cnx的rom，请到release页面下载编译好的exe版本**
   
   例子, fx991cnx:
   ```
	cd disas
   ```
   ```
   ./bin/u8-disas ../models/fx991cnx/rom.bin  0 0x40000 ./_disas.txt
   ```
   将_disas.txt复制到 models/fxcnx991目录
   ```
   cp ./_disas.txt ../models/fx991cnx/
   ```
   修改 `model.lua`  
   设置`rom_path` 为`"rom.bin"`  