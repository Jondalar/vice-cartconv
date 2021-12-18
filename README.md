# vice-cartconv

A simple hack to make the tool cartconv in vice extracte the banks of a .crt file. Those can be edited and glued back together with e.g. "cat" in unix style OS's.

Original can be found here: https://sourceforge.net/p/vice-emu/code/HEAD/tree/branches/cpx-gtk3ui/vice/src/tools/cartconv/. Just build it with standard gcc/c compiler. Copy it to a location that is in your path (/usr/local/bin in my case).

Use 'cartconv -f FILENAME.crt' and the tool will create the chunks in the same directory where you called the command. 

The resulting chunks will consists of the header (0x10 bytes) plus the payload of the bank, usually 0x2000 bytes or 0x400 bytes. The modification has been tested with Easyföash and GMOD/2 .crt files so far, they might *not* work for other cartridge files.

Example:
```
cartconv -f ../anewgame.crt
CRT Version: 1.0
Name: EasyFlash
Hardware ID: 32 (EasyFlash)
Hardware Revision: 0
Mode: exrom: 1 game: 0 (ultimax)

offset  sig  type  bank start size  chunklen
$000040 CHIP FLASH #000 $8000 $2000 $2010
$002050 CHIP FLASH #000 $a000 $2000 $2010
...
$0a4560 CHIP FLASH #060 $8000 $2000 $2010
```
will lead to:

```
test % ls -ll

-rw-r--r--  1 alex  staff    64 18 Dez 13:04 000_0000_0040_CRT_header
-rw-r--r--  1 alex  staff  8208 18 Dez 13:04 000_8000_9fff
-rw-r--r--  1 alex  staff  8208 18 Dez 13:04 000_a000_bfff
...
-rw-r--r--  1 alex  staff  8208 18 Dez 13:04 03c_8000_9fff
```
cat all the files in ascending order and you will get a working CRT file again.

Example:
```
cat 000_0000_0030_CRT_header 000_8000_9fff ... 03c_8000_9fff > thecrtfile.crt
```
This way you can edit/modify the CRT, and if you follow the CRTs logic (like easyflash3) you can modify the chunks.

I use Kickassembler to create/add banks into an Easyflash cartridge based on this. 
