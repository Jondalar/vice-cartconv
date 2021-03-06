/** \file   cartconv.c
 * \brief   Cartridge Conversion utility
 *
 * \author  Marco van den heuvel <blackystardust68@yahoo.com>
 * \author  groepaz <groepaz@gmx.net>
 */

/*
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

//#include "vice.h"

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <strings.h>

#include <unistd.h>


//#include "version.h"


#define VERSION "3.5"

//#include "cartridge.h"

/* FIXME: cartconv: the sizes are used in a bitfield and also by their absolute values */
#define CARTRIDGE_SIZE_2KB     0x00000800
#define CARTRIDGE_SIZE_4KB     0x00001000
#define CARTRIDGE_SIZE_8KB     0x00002000
#define CARTRIDGE_SIZE_12KB    0x00003000
#define CARTRIDGE_SIZE_16KB    0x00004000
#define CARTRIDGE_SIZE_20KB    0x00005000
#define CARTRIDGE_SIZE_24KB    0x00006000
#define CARTRIDGE_SIZE_32KB    0x00008000
#define CARTRIDGE_SIZE_64KB    0x00010000
#define CARTRIDGE_SIZE_96KB    0x00018000
#define CARTRIDGE_SIZE_128KB   0x00020000
#define CARTRIDGE_SIZE_256KB   0x00040000
#define CARTRIDGE_SIZE_512KB   0x00080000
#define CARTRIDGE_SIZE_1024KB  0x00100000
#define CARTRIDGE_SIZE_2048KB  0x00200000
#define CARTRIDGE_SIZE_4096KB  0x00400000
#define CARTRIDGE_SIZE_8192KB  0x00800000
#define CARTRIDGE_SIZE_16384KB 0x01000000
#define CARTRIDGE_SIZE_MAX     CARTRIDGE_SIZE_16384KB


/* Carts that don't have a rom image */
#define CARTRIDGE_DIGIMAX            -100 /* digimax.c */
#define CARTRIDGE_DQBB               -101 /* dqbb.c */
#define CARTRIDGE_GEORAM             -102 /* georam.c */
#define CARTRIDGE_ISEPIC             -103 /* isepic.c */
#define CARTRIDGE_RAMCART            -104 /* ramcart.c */
#define CARTRIDGE_REU                -105 /* reu.c */
#define CARTRIDGE_SFX_SOUND_EXPANDER -106 /* sfx_soundexpander.c, fmopl.c */
#define CARTRIDGE_SFX_SOUND_SAMPLER  -107 /* sfx_soundsampler.c */
#define CARTRIDGE_MIDI_PASSPORT      -108 /* c64-midi.c */
#define CARTRIDGE_MIDI_DATEL         -109 /* c64-midi.c */
#define CARTRIDGE_MIDI_SEQUENTIAL    -110 /* c64-midi.c */
#define CARTRIDGE_MIDI_NAMESOFT      -111 /* c64-midi.c */
#define CARTRIDGE_MIDI_MAPLIN        -112 /* c64-midi.c */
#define CARTRIDGE_DS12C887RTC        -113 /* ds12c887rtc.c */
#define CARTRIDGE_TFE                -116 /* ethernetcart.c */
#define CARTRIDGE_TURBO232           -117 /* c64acia1.c */
#define CARTRIDGE_SWIFTLINK          -118 /* c64acia1.c */
#define CARTRIDGE_ACIA               -119 /* c64acia1.c */
#define CARTRIDGE_PLUS60K            -120 /* plus60k.c */
#define CARTRIDGE_PLUS256K           -121 /* plus256k.c */
#define CARTRIDGE_C64_256K           -122 /* c64_256k.c */
#define CARTRIDGE_CPM                -123 /* cpmcart.c */
#define CARTRIDGE_DEBUGCART          -124 /* debugcart.c */

/* Known cartridge types.  */
/* TODO: cartconv (4k and 12k binaries) */
#define CARTRIDGE_ULTIMAX              -6 /* generic.c */
#define CARTRIDGE_GENERIC_8KB          -3 /* generic.c */
#define CARTRIDGE_GENERIC_16KB         -2 /* generic.c */

#define CARTRIDGE_NONE                 -1
#define CARTRIDGE_CRT                   0

/* the following must match the CRT IDs */
#define CARTRIDGE_ACTION_REPLAY         1 /* actionreplay.c */
#define CARTRIDGE_KCS_POWER             2 /* kcs.c */
#define CARTRIDGE_FINAL_III             3 /* final3.c */
#define CARTRIDGE_SIMONS_BASIC          4 /* simonsbasic.c */
#define CARTRIDGE_OCEAN                 5 /* ocean.c */
#define CARTRIDGE_EXPERT                6 /* expert.c */
#define CARTRIDGE_FUNPLAY               7 /* funplay.c */
#define CARTRIDGE_SUPER_GAMES           8 /* supergames.c */
#define CARTRIDGE_ATOMIC_POWER          9 /* atomicpower.c */
#define CARTRIDGE_EPYX_FASTLOAD        10 /* epyxfastload.c */
#define CARTRIDGE_WESTERMANN           11 /* westermann.c */
#define CARTRIDGE_REX                  12 /* rexutility.c */
#define CARTRIDGE_FINAL_I              13 /* final.c */
#define CARTRIDGE_MAGIC_FORMEL         14 /* magicformel.c */
#define CARTRIDGE_GS                   15 /* gs.c */
#define CARTRIDGE_WARPSPEED            16 /* warpspeed.c */
#define CARTRIDGE_DINAMIC              17 /* dinamic.c */
#define CARTRIDGE_ZAXXON               18 /* zaxxon.c */
#define CARTRIDGE_MAGIC_DESK           19 /* magicdesk.c */
#define CARTRIDGE_SUPER_SNAPSHOT_V5    20 /* supersnapshot.c */
#define CARTRIDGE_COMAL80              21 /* comal80.c */
#define CARTRIDGE_STRUCTURED_BASIC     22 /* stb.c */
#define CARTRIDGE_ROSS                 23 /* ross.c */
#define CARTRIDGE_DELA_EP64            24 /* delaep64.c */
#define CARTRIDGE_DELA_EP7x8           25 /* delaep7x8.c */
#define CARTRIDGE_DELA_EP256           26 /* delaep256.c */

/* TODO: cartconv */
#define CARTRIDGE_REX_EP256            27 /* rexep256.c */

#define CARTRIDGE_MIKRO_ASSEMBLER      28 /* mikroass.c */

/* TODO: cartconv (24k binary) */
#define CARTRIDGE_FINAL_PLUS           29 /* finalplus.c */

#define CARTRIDGE_ACTION_REPLAY4       30 /* actionreplay4.c */
#define CARTRIDGE_STARDOS              31 /* stardos.c */
#define CARTRIDGE_EASYFLASH            32 /* easyflash.c */

/* TODO: cartconv (no cart exists?) */
#define CARTRIDGE_EASYFLASH_XBANK      33 /* easyflash.c */

#define CARTRIDGE_CAPTURE              34 /* capture.c */
#define CARTRIDGE_ACTION_REPLAY3       35 /* actionreplay3.c */
#define CARTRIDGE_RETRO_REPLAY         36 /* retroreplay.c */
#define CARTRIDGE_MMC64                37 /* mmc64.c, spi-sdcard.c */
#define CARTRIDGE_MMC_REPLAY           38 /* mmcreplay.c, ser-eeprom.c, spi-sdcard.c */
#define CARTRIDGE_IDE64                39 /* ide64.c */
#define CARTRIDGE_SUPER_SNAPSHOT       40 /* supersnapshot4.c */
#define CARTRIDGE_IEEE488              41 /* c64tpi.c */
#define CARTRIDGE_GAME_KILLER          42 /* gamekiller.c */
#define CARTRIDGE_P64                  43 /* prophet64.c */
#define CARTRIDGE_EXOS                 44 /* exos.c */
#define CARTRIDGE_FREEZE_FRAME         45 /* freezeframe.c */
#define CARTRIDGE_FREEZE_MACHINE       46 /* freezemachine.c */
#define CARTRIDGE_SNAPSHOT64           47 /* snapshot64.c */
#define CARTRIDGE_SUPER_EXPLODE_V5     48 /* superexplode5.c */
#define CARTRIDGE_MAGIC_VOICE          49 /* magicvoice.c, tpicore.c, t6721.c */
#define CARTRIDGE_ACTION_REPLAY2       50 /* actionreplay2.c */
#define CARTRIDGE_MACH5                51 /* mach5.c */
#define CARTRIDGE_DIASHOW_MAKER        52 /* diashowmaker.c */
#define CARTRIDGE_PAGEFOX              53 /* pagefox.c */
#define CARTRIDGE_KINGSOFT             54 /* kingsoft.c */
#define CARTRIDGE_SILVERROCK_128       55 /* silverrock128.c */
#define CARTRIDGE_FORMEL64             56 /* formel64.c */
#define CARTRIDGE_RGCD                 57 /* rgcd.c */
#define CARTRIDGE_RRNETMK3             58 /* rrnetmk3.c */
#define CARTRIDGE_EASYCALC             59 /* easycalc.c */
#define CARTRIDGE_GMOD2                60 /* gmod2.c */
#define CARTRIDGE_MAX_BASIC            61 /* maxbasic.c */
#define CARTRIDGE_GMOD3                62 /* gmod3.c */
#define CARTRIDGE_ZIPPCODE48           63 /* zippcode48.c */
#define CARTRIDGE_BLACKBOX8            64 /* blackbox8.c */
#define CARTRIDGE_BLACKBOX3            65 /* blackbox3.c */
#define CARTRIDGE_BLACKBOX4            66 /* blackbox4.c */
#define CARTRIDGE_REX_RAMFLOPPY        67 /* rexramfloppy.c */
#define CARTRIDGE_BISPLUS              68 /* bisplus.c */
#define CARTRIDGE_SDBOX                69 /* sdbox.c */
#define CARTRIDGE_MULTIMAX             70 /* multimax.c */
#define CARTRIDGE_BLACKBOX9            71 /* blackbox9.c */
#define CARTRIDGE_LT_KERNAL            72 /* ltkernal.c */
#define CARTRIDGE_RAMLINK              73 /* ramlink.c */
#define CARTRIDGE_HERO                 74 /* hero.c */

#define CARTRIDGE_LAST                 74 /* cartconv: last cartridge in list */




/* list of canonical names for the c64 cartridges:
   note: often it is hard to determine "the" official name, let alone the way it
   should be capitalized. because of that we go by the following rules:
   - if even the actual spelling and/or naming is unclear, then the most "common"
     variant is choosen ("Expert Cartridge" vs "The Expert")
   - in many cases the name is printed all uppercase both on screen and in other
     sources (manual, adverts). we refrain from doing the same here and convert
     to Camel Case ("ACTION REPLAY V5" -> "Action Replay V5"), *except* if the
     cart name constitutes an actual name (as in noun) by itself ("ISEPIC", "EXOS").
     additionally common abrevations such as RAM or EPROM will get written uppercase
     if in doubt.
   - although generally these cartridge names should never get translated, some
     generic stuff is translated to english ("EPROM Karte" -> "EPROM Cart")
*/
#define CARTRIDGE_NAME_ACIA               "ACIA"
#define CARTRIDGE_NAME_ACTION_REPLAY      "Action Replay V5" /* http://rr.pokefinder.org/wiki/Action_Replay */
#define CARTRIDGE_NAME_ACTION_REPLAY2     "Action Replay MK2" /* http://rr.pokefinder.org/wiki/Action_Replay */
#define CARTRIDGE_NAME_ACTION_REPLAY3     "Action Replay MK3" /* http://rr.pokefinder.org/wiki/Action_Replay */
#define CARTRIDGE_NAME_ACTION_REPLAY4     "Action Replay MK4" /* http://rr.pokefinder.org/wiki/Action_Replay */
#define CARTRIDGE_NAME_ATOMIC_POWER       "Atomic Power" /* also: "Nordic Power" */ /* http://rr.pokefinder.org/wiki/Nordic_Power */
#define CARTRIDGE_NAME_BISPLUS            "BIS-Plus"
#define CARTRIDGE_NAME_BLACKBOX3          "Blackbox V3"
#define CARTRIDGE_NAME_BLACKBOX4          "Blackbox V4"
#define CARTRIDGE_NAME_BLACKBOX8          "Blackbox V8"
#define CARTRIDGE_NAME_BLACKBOX9          "Blackbox V9"
#define CARTRIDGE_NAME_GS                 "C64 Games System" /* http://retro.lonningdal.net/home.php?page=Computers&select=c64gs&image=c64gs4.jpg */
#define CARTRIDGE_NAME_CAPTURE            "Capture" /* see manual http://rr.pokefinder.org/wiki/Capture */
#define CARTRIDGE_NAME_COMAL80            "Comal 80" /* http://www.retroport.de/C64_C128_Hardware.html */
#define CARTRIDGE_NAME_CPM                "CP/M cartridge"
#define CARTRIDGE_NAME_MIDI_DATEL         "Datel MIDI"
#define CARTRIDGE_NAME_DEBUGCART          "Debug Cartridge"
#define CARTRIDGE_NAME_DELA_EP64          "Dela EP64"
#define CARTRIDGE_NAME_DELA_EP7x8         "Dela EP7x8"
#define CARTRIDGE_NAME_DELA_EP256         "Dela EP256"
#define CARTRIDGE_NAME_DIASHOW_MAKER      "Diashow-Maker" /* http://www.retroport.de/Rex.html */
#define CARTRIDGE_NAME_DIGIMAX            "DigiMAX" /* http://starbase.globalpc.net/~ezekowitz/vanessa/hobbies/projects.html */
#define CARTRIDGE_NAME_DINAMIC            "Dinamic"
#define CARTRIDGE_NAME_DQBB               "Double Quick Brown Box" /* on the cart itself its all uppercase ? */
#define CARTRIDGE_NAME_DS12C887RTC        "DS12C887 Real Time Clock" /* Title of the page at http://ytm.bossstation.dnsalias.org/html/rtcds12c887.html */
#define CARTRIDGE_NAME_EASYCALC           "Easy Calc Result" /* on the cart itself it's "Calc Result EASY", in the manual it's EASYCALC, but we'll go with what is defined ;) */
#define CARTRIDGE_NAME_EASYFLASH          "EasyFlash" /* see http://skoe.de/easyflash/ */
#define CARTRIDGE_NAME_EASYFLASH_XBANK    "EasyFlash Xbank" /* see http://skoe.de/easyflash/ */
#define CARTRIDGE_NAME_EPYX_FASTLOAD      "Epyx FastLoad" /* http://rr.pokefinder.org/wiki/Epyx_FastLoad */
#define CARTRIDGE_NAME_ETHERNETCART       "Ethernet cartridge"
#define CARTRIDGE_NAME_EXOS               "EXOS" /* http://rr.pokefinder.org/wiki/ExOS */
#define CARTRIDGE_NAME_EXPERT             "Expert Cartridge" /* http://rr.pokefinder.org/wiki/Expert_Cartridge */
#define CARTRIDGE_NAME_FINAL_I            "The Final Cartridge" /* http://rr.pokefinder.org/wiki/Final_Cartridge */
#define CARTRIDGE_NAME_FINAL_III          "The Final Cartridge III" /* http://rr.pokefinder.org/wiki/Final_Cartridge */
#define CARTRIDGE_NAME_FINAL_PLUS         "Final Cartridge Plus" /* http://rr.pokefinder.org/wiki/Final_Cartridge */
#define CARTRIDGE_NAME_TFE                "The Final Ethernet"
#define CARTRIDGE_NAME_FORMEL64           "Formel 64"
#define CARTRIDGE_NAME_FREEZE_FRAME       "Freeze Frame" /* http://rr.pokefinder.org/wiki/Freeze_Frame */
#define CARTRIDGE_NAME_FREEZE_MACHINE     "Freeze Machine" /* http://rr.pokefinder.org/wiki/Freeze_Frame */
#define CARTRIDGE_NAME_FUNPLAY            "Fun Play" /* also: "Power Play" */ /* http://home.nomansland.biz/~zerqent/commodore_salg/CIMG2132.JPG */
#define CARTRIDGE_NAME_GAME_KILLER        "Game Killer" /* http://rr.pokefinder.org/wiki/Game_Killer */
#define CARTRIDGE_NAME_GEORAM             "GEO-RAM" /* http://www.retroport.de/Rex.html */
#define CARTRIDGE_NAME_GMOD2              "GMod2" /* http://wiki.icomp.de/wiki/GMod2 */
#define CARTRIDGE_NAME_GMOD3              "GMod3" /* http://wiki.icomp.de/wiki/GMod3 */
#define CARTRIDGE_NAME_HERO               "H.E.R.O. (Drean)"
#define CARTRIDGE_NAME_IDE64              "IDE64" /* see http://www.ide64.org/ */
#define CARTRIDGE_NAME_IEEE488            "IEEE-488 Interface"
#define CARTRIDGE_NAME_ISEPIC             "ISEPIC" /* http://rr.pokefinder.org/wiki/Isepic */
#define CARTRIDGE_NAME_KCS_POWER          "KCS Power Cartridge" /* http://rr.pokefinder.org/wiki/Power_Cartridge */
#define CARTRIDGE_NAME_KINGSOFT           "Kingsoft"
#define CARTRIDGE_NAME_LT_KERNAL          "Lt. Kernal Host Adaptor"
#define CARTRIDGE_NAME_MACH5              "MACH 5" /* http://rr.pokefinder.org/wiki/MACH_5 */
#define CARTRIDGE_NAME_MAGIC_DESK         "Magic Desk" /* also: "Domark, Hes Australia" */
#define CARTRIDGE_NAME_MAGIC_FORMEL       "Magic Formel" /* http://rr.pokefinder.org/wiki/Magic_Formel */
#define CARTRIDGE_NAME_MAGIC_VOICE        "Magic Voice" /* all lowercase on cart ? */
#define CARTRIDGE_NAME_MAX_BASIC          "MAX Basic"
#define CARTRIDGE_NAME_MIDI_MAPLIN        "Maplin MIDI"
#define CARTRIDGE_NAME_MIKRO_ASSEMBLER    "Mikro Assembler"
#define CARTRIDGE_NAME_MMC64              "MMC64" /* see manual */
#define CARTRIDGE_NAME_MMC_REPLAY         "MMC Replay" /* see manual */
#define CARTRIDGE_NAME_MIDI_NAMESOFT      "Namesoft MIDI"
#define CARTRIDGE_NAME_MIDI_PASSPORT      "Passport MIDI"
#define CARTRIDGE_NAME_MIDI_SEQUENTIAL    "Sequential MIDI"
#define CARTRIDGE_NAME_MULTIMAX           "MultiMAX" /* http://www.multimax.co/ */
#define CARTRIDGE_NAME_NORDIC_REPLAY      "Nordic Replay" /* "Retro Replay v2" see manual */
#define CARTRIDGE_NAME_OCEAN              "Ocean"
#define CARTRIDGE_NAME_PAGEFOX            "Pagefox"
#define CARTRIDGE_NAME_P64                "Prophet64" /* see http://www.prophet64.com/ */
#define CARTRIDGE_NAME_RAMCART            "RamCart" /* see cc65 driver */
#define CARTRIDGE_NAME_RAMLINK            "RAMLink"
#define CARTRIDGE_NAME_REU                "RAM Expansion Module" /* http://www.retroport.de/C64_C128_Hardware.html */
#define CARTRIDGE_NAME_REX_EP256          "REX 256K EPROM Cart" /* http://www.retroport.de/Rex.html */
#define CARTRIDGE_NAME_REX                "REX Utility"
#define CARTRIDGE_NAME_REX_RAMFLOPPY      "REX RAM-Floppy"
#define CARTRIDGE_NAME_RGCD               "RGCD"
#define CARTRIDGE_NAME_RRNET              "RR-Net" /* see manual */
#define CARTRIDGE_NAME_RRNETMK3           "RR-Net MK3" /* see manual */
#define CARTRIDGE_NAME_RETRO_REPLAY       "Retro Replay" /* see manual */
#define CARTRIDGE_NAME_ROSS               "ROSS"
#define CARTRIDGE_NAME_SDBOX              "SD-BOX" /* http://c64.com.pl/index.php/sdbox106.html */
#define CARTRIDGE_NAME_SFX_SOUND_EXPANDER "SFX Sound Expander" /* http://www.floodgap.com/retrobits/ckb/secret/cbm-sfx-fmbport.jpg */
#define CARTRIDGE_NAME_SFX_SOUND_SAMPLER  "SFX Sound Sampler" /* http://www.floodgap.com/retrobits/ckb/secret/cbm-ssm-box.jpg */
#define CARTRIDGE_NAME_SILVERROCK_128     "Silverrock 128KiB Cartridge"
#define CARTRIDGE_NAME_SIMONS_BASIC       "Simons' BASIC" /* http://en.wikipedia.org/wiki/Simons'_BASIC */
#define CARTRIDGE_NAME_SNAPSHOT64         "Snapshot 64" /* http://rr.pokefinder.org/wiki/Super_Snapshot */
#define CARTRIDGE_NAME_STARDOS            "Stardos" /* see manual http://rr.pokefinder.org/wiki/StarDOS */
#define CARTRIDGE_NAME_STRUCTURED_BASIC   "Structured BASIC"
#define CARTRIDGE_NAME_SUPER_EXPLODE_V5   "Super Explode V5.0" /* http://rr.pokefinder.org/wiki/Super_Explode */
#define CARTRIDGE_NAME_SUPER_GAMES        "Super Games"
#define CARTRIDGE_NAME_SUPER_SNAPSHOT     "Super Snapshot V4" /* http://rr.pokefinder.org/wiki/Super_Snapshot */
#define CARTRIDGE_NAME_SUPER_SNAPSHOT_V5  "Super Snapshot V5" /* http://rr.pokefinder.org/wiki/Super_Snapshot */
#define CARTRIDGE_NAME_SWIFTLINK          "Swiftlink" /* http://mikenaberezny.com/hardware/peripherals/swiftlink-rs232-interface/ */
#define CARTRIDGE_NAME_TURBO232           "Turbo232" /* also: "ACIA/SWIFTLINK" */ /*http://www.retroport.de/C64_C128_Hardware2.html */
#define CARTRIDGE_NAME_WARPSPEED          "Warp Speed" /* see manual http://rr.pokefinder.org/wiki/WarpSpeed */
#define CARTRIDGE_NAME_WESTERMANN         "Westermann Learning"
#define CARTRIDGE_NAME_ZAXXON             "Zaxxon"
#define CARTRIDGE_NAME_ZIPPCODE48         "ZIPP-CODE 48"

#define CARTRIDGE_NAME_GENERIC_8KB        "generic 8KiB game"
#define CARTRIDGE_NAME_GENERIC_16KB       "generic 16KiB game"
#define CARTRIDGE_NAME_ULTIMAX            "generic Ultimax"




static FILE *infile, *outfile;
static int load_address = 0;
static int loadfile_offset = 0;
static unsigned int loadfile_size = 0;
static char *output_filename = NULL;
static char *input_filename[33];
static char *cart_name = NULL;
static signed char cart_type = -1;
static unsigned char cart_subtype = 0;
static char convert_to_bin = 0;
static char convert_to_prg = 0;
static char convert_to_ultimax = 0;
static unsigned char input_filenames = 0;
static char loadfile_is_crt = 0;
static char loadfile_is_ultimax = 0;
static int loadfile_cart_type = 0;
static unsigned char filebuffer[CARTRIDGE_SIZE_MAX + 2];
static unsigned char headerbuffer[0x40];
static unsigned char extra_buffer_32kb[0x8000];
static unsigned char chipbuffer[16];
static int repair_mode = 0;
static int input_padding = 0;
static int quiet_mode = 0;
static int omit_empty_banks = 1;

static int load_input_file(char *filename);

typedef struct cart_s {
    unsigned char exrom;
    unsigned char game;
    unsigned int sizes;
    unsigned int bank_size;
    unsigned int load_address;
    unsigned char banks;   /* 0 means the amount of banks need to be taken from the load-size and bank-size */
    unsigned int data_type;
    char *name;
    char *opt;
    void (*save)(unsigned int p1, unsigned int p2, unsigned int p3, unsigned int p4, unsigned char gameline, unsigned char exromline);
} cart_t;

typedef struct sorted_cart_s {
    char *opt;
    char *name;
    int crt_id;
    int insertion;
} sorted_cart_t;

/* some prototypes to save routines */
static void save_regular_crt(unsigned int p1, unsigned int p2, unsigned int p3, unsigned int p4, unsigned char game, unsigned char exrom);
static void save_fcplus_crt(unsigned int p1, unsigned int p2, unsigned int p3, unsigned int p4, unsigned char game, unsigned char exrom);
static void save_2_blocks_crt(unsigned int p1, unsigned int p2, unsigned int p3, unsigned int p4, unsigned char game, unsigned char exrom);
static void save_generic_crt(unsigned int p1, unsigned int p2, unsigned int p3, unsigned int p4, unsigned char p5, unsigned char p6);
static void save_easyflash_crt(unsigned int p1, unsigned int p2, unsigned int p3, unsigned int p4, unsigned char game, unsigned char exrom);
static void save_ocean_crt(unsigned int p1, unsigned int p2, unsigned int p3, unsigned int p4, unsigned char game, unsigned char exrom);
static void save_funplay_crt(unsigned int p1, unsigned int p2, unsigned int p3, unsigned int p4, unsigned char game, unsigned char exrom);
static void save_zaxxon_crt(unsigned int p1, unsigned int p2, unsigned int p3, unsigned int p4, unsigned char game, unsigned char exrom);
static void save_stardos_crt(unsigned int p1, unsigned int p2, unsigned int p3, unsigned int p4, unsigned char game, unsigned char exrom);
static void save_delaep64_crt(unsigned int p1, unsigned int p2, unsigned int p3, unsigned int p4, unsigned char game, unsigned char exrom);
static void save_delaep256_crt(unsigned int p1, unsigned int p2, unsigned int p3, unsigned int p4, unsigned char game, unsigned char exrom);
static void save_delaep7x8_crt(unsigned int p1, unsigned int p2, unsigned int p3, unsigned int p4, unsigned char game, unsigned char exrom);
static void save_rexep256_crt(unsigned int p1, unsigned int p2, unsigned int p3, unsigned int p4, unsigned char game, unsigned char exrom);
static void save_easycalc_crt(unsigned int p1, unsigned int p2, unsigned int p3, unsigned int p4, unsigned char game, unsigned char exrom);

/* this table must be in correct order so it can be indexed by CRT ID */
/*
    exrom, game, sizes, bank size, load addr, num banks, data type, name, option, saver

    num banks == 0 - take number of banks from input file size
*/
static const cart_t cart_info[] = {
/*  {0, 1, CARTRIDGE_SIZE_8KB, 0x2000, 0x8000, 1, 0, "Generic 8KiB", NULL, NULL}, */ /* 8k game config */
/*  {0, 0, CARTRIDGE_SIZE_12KB, 0x3000, 0x8000, 1, 0, "Generic 12KiB", NULL, NULL}, */ /* 16k game config */
/*  {0, 0, CARTRIDGE_SIZE_16KB, 0x4000, 0x8000, 1, 0, "Generic 16KiB", NULL, NULL}, */ /* 16k game config */
/*  {1, 0, CARTRIDGE_SIZE_4KB | CARTRIDGE_SIZE_16KB, 0, 0, 1, 0, "Ultimax", NULL, NULL}, */ /* ultimax config */

/* FIXME: initial exrom/game values are often wrong in this table
 *        don't forget to also update vice.texi accordingly */

    {0, 1, CARTRIDGE_SIZE_4KB | CARTRIDGE_SIZE_8KB | CARTRIDGE_SIZE_12KB | CARTRIDGE_SIZE_16KB, 0, 0, 0, 0, "Generic Cartridge", NULL, save_generic_crt},
    {0, 1, CARTRIDGE_SIZE_32KB, 0x2000, 0x8000, 4, 0, CARTRIDGE_NAME_ACTION_REPLAY, "ar5", save_regular_crt}, /* this is NOT AR1, but 4.2,5,6 etc */
    {0, 0, CARTRIDGE_SIZE_16KB, 0x2000, 0, 2, 0, CARTRIDGE_NAME_KCS_POWER, "kcs", save_2_blocks_crt},
    {0, 0, CARTRIDGE_SIZE_64KB | CARTRIDGE_SIZE_256KB, 0x4000, 0x8000, 0, 0, CARTRIDGE_NAME_FINAL_III, "fc3", save_regular_crt},
    {0, 0, CARTRIDGE_SIZE_16KB, 0x2000, 0, 2, 0, CARTRIDGE_NAME_SIMONS_BASIC, "simon", save_2_blocks_crt},
    {0, 0, CARTRIDGE_SIZE_32KB | CARTRIDGE_SIZE_128KB | CARTRIDGE_SIZE_256KB | CARTRIDGE_SIZE_512KB, 0x2000, 0, 0, 0, CARTRIDGE_NAME_OCEAN, "ocean", save_ocean_crt},
    {1, 0, CARTRIDGE_SIZE_8KB, 0x2000, 0x8000, 1, 2, CARTRIDGE_NAME_EXPERT, "expert", NULL},
    {0, 1, CARTRIDGE_SIZE_128KB, 0x2000, 0x8000, 16, 0, CARTRIDGE_NAME_FUNPLAY, "fp", save_funplay_crt},
    {0, 0, CARTRIDGE_SIZE_64KB, 0x4000, 0x8000, 4, 0, CARTRIDGE_NAME_SUPER_GAMES, "sg", save_regular_crt},
    {0, 1, CARTRIDGE_SIZE_32KB, 0x2000, 0x8000, 4, 0, CARTRIDGE_NAME_ATOMIC_POWER, "ap", save_regular_crt},
    {0, 1, CARTRIDGE_SIZE_8KB, 0x2000, 0x8000, 1, 0, CARTRIDGE_NAME_EPYX_FASTLOAD, "epyx", save_regular_crt},
    {0, 0, CARTRIDGE_SIZE_16KB, 0x4000, 0x8000, 1, 0, CARTRIDGE_NAME_WESTERMANN, "wl", save_regular_crt},
    {0, 1, CARTRIDGE_SIZE_8KB, 0x2000, 0x8000, 1, 0, CARTRIDGE_NAME_REX, "ru", save_regular_crt},
    {0, 0, CARTRIDGE_SIZE_16KB, 0x4000, 0x8000, 1, 0, CARTRIDGE_NAME_FINAL_I, "fc1", save_regular_crt},
    {1, 0, CARTRIDGE_SIZE_64KB | CARTRIDGE_SIZE_96KB | CARTRIDGE_SIZE_128KB, 0x2000, 0xe000, 0, 0, CARTRIDGE_NAME_MAGIC_FORMEL, "mf", save_regular_crt}, /* FIXME: 64k (v1), 96k (v2) and 128k (full) bins exist */
    {0, 1, CARTRIDGE_SIZE_512KB, 0x2000, 0x8000, 64, 0, CARTRIDGE_NAME_GS, "gs", save_regular_crt},
    {0, 0, CARTRIDGE_SIZE_16KB, 0x4000, 0x8000, 1, 0, CARTRIDGE_NAME_WARPSPEED, "ws", save_regular_crt},
    {0, 1, CARTRIDGE_SIZE_128KB, 0x2000, 0x8000, 16, 0, CARTRIDGE_NAME_DINAMIC, "din", save_regular_crt},
    {0, 0, CARTRIDGE_SIZE_20KB, 0, 0, 3, 0, CARTRIDGE_NAME_ZAXXON, "zaxxon", save_zaxxon_crt},
    {0, 1, CARTRIDGE_SIZE_32KB | CARTRIDGE_SIZE_64KB | CARTRIDGE_SIZE_128KB | CARTRIDGE_SIZE_256KB | CARTRIDGE_SIZE_512KB | CARTRIDGE_SIZE_1024KB, 0x2000, 0x8000, 0, 0, CARTRIDGE_NAME_MAGIC_DESK, "md", save_regular_crt},
    {0, 0, CARTRIDGE_SIZE_64KB, 0x4000, 0x8000, 4, 0, CARTRIDGE_NAME_SUPER_SNAPSHOT_V5, "ss5", save_regular_crt},
    {0, 0, CARTRIDGE_SIZE_64KB | CARTRIDGE_SIZE_128KB, 0x4000, 0x8000, 0, 0, CARTRIDGE_NAME_COMAL80, "comal", save_regular_crt},
    {1, 0, CARTRIDGE_SIZE_16KB, 0x2000, 0x8000, 2, 0, CARTRIDGE_NAME_STRUCTURED_BASIC, "sb", save_regular_crt},
    {0, 0, CARTRIDGE_SIZE_16KB | CARTRIDGE_SIZE_32KB, 0x4000, 0x8000, 0, 0, CARTRIDGE_NAME_ROSS, "ross", save_regular_crt},
    {0, 1, CARTRIDGE_SIZE_8KB, 0, 0x8000, 0, 0, CARTRIDGE_NAME_DELA_EP64, "dep64", save_delaep64_crt},
    {0, 1, CARTRIDGE_SIZE_8KB, 0x2000, 0x8000, 0, 0, CARTRIDGE_NAME_DELA_EP7x8, "dep7x8", save_delaep7x8_crt},
    {0, 1, CARTRIDGE_SIZE_8KB, 0x2000, 0x8000, 0, 0, CARTRIDGE_NAME_DELA_EP256, "dep256", save_delaep256_crt},
    {0, 1, CARTRIDGE_SIZE_8KB, 0, 0x8000, 0, 0, CARTRIDGE_NAME_REX_EP256, "rep256", save_rexep256_crt},
    {0, 1, CARTRIDGE_SIZE_8KB, 0x2000, 0x8000, 1, 0, CARTRIDGE_NAME_MIKRO_ASSEMBLER, "mikro", save_regular_crt},
    {1, 0, CARTRIDGE_SIZE_24KB | CARTRIDGE_SIZE_32KB, 0x8000, 0x0000, 1, 0, CARTRIDGE_NAME_FINAL_PLUS, "fcp", save_fcplus_crt},
    {0, 1, CARTRIDGE_SIZE_32KB, 0x2000, 0x8000, 4, 0, CARTRIDGE_NAME_ACTION_REPLAY4, "ar4", save_regular_crt},
    {1, 0, CARTRIDGE_SIZE_16KB, 0x2000, 0, 4, 0, CARTRIDGE_NAME_STARDOS, "star", save_stardos_crt},
    {1, 0, CARTRIDGE_SIZE_1024KB, 0x2000, 0, 128, 0, CARTRIDGE_NAME_EASYFLASH, "easy", save_easyflash_crt},
    {0, 0, 0, 0, 0, 0, 0, CARTRIDGE_NAME_EASYFLASH_XBANK, NULL, NULL}, /* TODO ?? */
    {1, 1, CARTRIDGE_SIZE_8KB, 0x2000, 0x8000, 1, 0, CARTRIDGE_NAME_CAPTURE, "cap", save_regular_crt},
    {0, 1, CARTRIDGE_SIZE_16KB, 0x2000, 0x8000, 2, 0, CARTRIDGE_NAME_ACTION_REPLAY3, "ar3", save_regular_crt},
    {0, 1, CARTRIDGE_SIZE_32KB | CARTRIDGE_SIZE_64KB | CARTRIDGE_SIZE_128KB, 0x2000, 0x8000, 0, 0, CARTRIDGE_NAME_RETRO_REPLAY, "rr", save_regular_crt},
    {0, 1, CARTRIDGE_SIZE_8KB, 0x2000, 0x8000, 1, 0, CARTRIDGE_NAME_MMC64, "mmc64", save_regular_crt},
    {0, 0, CARTRIDGE_SIZE_64KB | CARTRIDGE_SIZE_512KB, 0x2000, 0x8000, 0, 0, CARTRIDGE_NAME_MMC_REPLAY, "mmcr", save_regular_crt},
    {0, 1, CARTRIDGE_SIZE_64KB | CARTRIDGE_SIZE_128KB | CARTRIDGE_SIZE_512KB, 0x4000, 0x8000, 0, 2, CARTRIDGE_NAME_IDE64, "ide64", save_regular_crt},
    {0, 0, CARTRIDGE_SIZE_32KB, 0x4000, 0x8000, 2, 0, CARTRIDGE_NAME_SUPER_SNAPSHOT, "ss4", save_regular_crt},
    {0, 1, CARTRIDGE_SIZE_4KB, 0x1000, 0x8000, 1, 0, CARTRIDGE_NAME_IEEE488, "ieee", save_regular_crt},
    {1, 0, CARTRIDGE_SIZE_8KB, 0x2000, 0xe000, 1, 0, CARTRIDGE_NAME_GAME_KILLER, "gk", save_regular_crt},
    {0, 1, CARTRIDGE_SIZE_256KB, 0x2000, 0x8000, 32, 0, CARTRIDGE_NAME_P64, "p64", save_regular_crt},
    {1, 0, CARTRIDGE_SIZE_8KB, 0x2000, 0xe000, 1, 0, CARTRIDGE_NAME_EXOS, "exos", save_regular_crt},
    {0, 1, CARTRIDGE_SIZE_8KB, 0x2000, 0x8000, 1, 0, CARTRIDGE_NAME_FREEZE_FRAME, "ff", save_regular_crt},
    {0, 1, CARTRIDGE_SIZE_16KB | CARTRIDGE_SIZE_32KB, 0x4000, 0x8000, 0, 0, CARTRIDGE_NAME_FREEZE_MACHINE, "fm", save_regular_crt},
    {0, 0, CARTRIDGE_SIZE_4KB, 0x1000, 0xe000, 1, 0, CARTRIDGE_NAME_SNAPSHOT64, "s64", save_regular_crt},
    {0, 1, CARTRIDGE_SIZE_16KB, 0x2000, 0x8000, 2, 0, CARTRIDGE_NAME_SUPER_EXPLODE_V5, "se5", save_regular_crt},
    {1, 0, CARTRIDGE_SIZE_16KB, 0x4000, 0x8000, 1, 0, CARTRIDGE_NAME_MAGIC_VOICE, "mv", save_regular_crt},
    {0, 1, CARTRIDGE_SIZE_16KB, 0x2000, 0x8000, 2, 0, CARTRIDGE_NAME_ACTION_REPLAY2, "ar2", save_regular_crt},
    {0, 1, CARTRIDGE_SIZE_4KB | CARTRIDGE_SIZE_8KB, 0x2000, 0x8000, 0, 0, CARTRIDGE_NAME_MACH5, "mach5", save_regular_crt},
    {0, 1, CARTRIDGE_SIZE_8KB, 0x2000, 0x8000, 1, 0, CARTRIDGE_NAME_DIASHOW_MAKER, "dsm", save_regular_crt},
    {0, 0, CARTRIDGE_SIZE_64KB, 0x4000, 0x8000, 4, 0, CARTRIDGE_NAME_PAGEFOX, "pf", save_regular_crt},
    {0, 0, CARTRIDGE_SIZE_24KB, 0x2000, 0x8000, 3, 0, CARTRIDGE_NAME_KINGSOFT, "ks", save_regular_crt},
    {0, 1, CARTRIDGE_SIZE_128KB, 0x2000, 0x8000, 16, 0, CARTRIDGE_NAME_SILVERROCK_128, "silver", save_regular_crt},
    {1, 0, CARTRIDGE_SIZE_32KB, 0x2000, 0xe000, 4, 0, CARTRIDGE_NAME_FORMEL64, "f64", save_regular_crt},
    {0, 1, CARTRIDGE_SIZE_64KB, 0x2000, 0x8000, 8, 0, CARTRIDGE_NAME_RGCD, "rgcd", save_regular_crt},
    {0, 1, CARTRIDGE_SIZE_8KB, 0x2000, 0x8000, 1, 0, CARTRIDGE_NAME_RRNETMK3, "rrnet", save_regular_crt},
    {0, 0, CARTRIDGE_SIZE_24KB, 0, 0, 3, 0, CARTRIDGE_NAME_EASYCALC, "ecr", save_easycalc_crt},
    {0, 1, CARTRIDGE_SIZE_512KB, 0x2000, 0x8000, 64, 0, CARTRIDGE_NAME_GMOD2, "gmod2", save_regular_crt},
    {1, 0, CARTRIDGE_SIZE_16KB, 0x2000, 0, 0, 0, CARTRIDGE_NAME_MAX_BASIC, "max", save_generic_crt},
    {0, 1, CARTRIDGE_SIZE_2048KB | CARTRIDGE_SIZE_4096KB | CARTRIDGE_SIZE_8192KB | CARTRIDGE_SIZE_16384KB, 0x2000, 0x8000, 0, 0, CARTRIDGE_NAME_GMOD3, "gmod3", save_regular_crt},
    {0, 1, CARTRIDGE_SIZE_8KB, 0x2000, 0x8000, 1, 0, CARTRIDGE_NAME_ZIPPCODE48, "zipp", save_regular_crt},
    {0, 0, CARTRIDGE_SIZE_32KB | CARTRIDGE_SIZE_64KB, 0x4000, 0x8000, 0, 0, CARTRIDGE_NAME_BLACKBOX8, "bb8", save_regular_crt},
    {0, 1, CARTRIDGE_SIZE_8KB, 0x2000, 0x8000, 1, 0, CARTRIDGE_NAME_BLACKBOX3, "bb3", save_regular_crt},
    {0, 0, CARTRIDGE_SIZE_16KB, 0x4000, 0x8000, 1, 0, CARTRIDGE_NAME_BLACKBOX4, "bb4", save_regular_crt},
    {0, 1, CARTRIDGE_SIZE_8KB, 0x2000, 0x8000, 1, 0, CARTRIDGE_NAME_REX_RAMFLOPPY, "rrf", save_regular_crt},
    {0, 1, CARTRIDGE_SIZE_2KB | CARTRIDGE_SIZE_4KB | CARTRIDGE_SIZE_8KB, 0x2000, 0x8000, 0, 0, CARTRIDGE_NAME_BISPLUS, "bis", save_regular_crt},
    {0, 0, CARTRIDGE_SIZE_128KB, 0x4000, 0x8000, 8, 0, CARTRIDGE_NAME_SDBOX, "sdbox", save_regular_crt},
    {1, 0, CARTRIDGE_SIZE_1024KB, 0x4000, 0x8000, 64, 0, CARTRIDGE_NAME_MULTIMAX, "mm", save_regular_crt},
    {0, 0, CARTRIDGE_SIZE_32KB, 0x4000, 0x8000, 0, 0, CARTRIDGE_NAME_BLACKBOX9, "bb9", save_regular_crt},
    {0, 1, CARTRIDGE_SIZE_8KB, 0x2000, 0x8000, 1, 0, CARTRIDGE_NAME_LT_KERNAL, "ltk", save_regular_crt},
    {0, 1, CARTRIDGE_SIZE_64KB, 0x2000, 0x8000, 8, 0, CARTRIDGE_NAME_RAMLINK, "rl", save_regular_crt},
    {0, 1, CARTRIDGE_SIZE_32KB, 0x2000, 0x8000, 4, 0, CARTRIDGE_NAME_HERO, "hero", save_regular_crt},
    {0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL}
};

//#ifndef HAVE_MEMMOVE
#define memmove(x, y, z) bcopy(y, x, z)
//#endif

#ifndef HAVE_STRDUP
char *strdup(const char *string)
{
    char *new;

    new = malloc(strlen(string) + 1);
    if (new != NULL) {
        strcpy(new, string);
    }
    return new;
}
#endif

#if !defined(HAVE_STRNCASECMP)
static const unsigned char charmap[] = {
    '\000', '\001', '\002', '\003', '\004', '\005', '\006', '\007',
    '\010', '\011', '\012', '\013', '\014', '\015', '\016', '\017',
    '\020', '\021', '\022', '\023', '\024', '\025', '\026', '\027',
    '\030', '\031', '\032', '\033', '\034', '\035', '\036', '\037',
    '\040', '\041', '\042', '\043', '\044', '\045', '\046', '\047',
    '\050', '\051', '\052', '\053', '\054', '\055', '\056', '\057',
    '\060', '\061', '\062', '\063', '\064', '\065', '\066', '\067',
    '\070', '\071', '\072', '\073', '\074', '\075', '\076', '\077',
    '\100', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
    '\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
    '\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
    '\170', '\171', '\172', '\133', '\134', '\135', '\136', '\137',
    '\140', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
    '\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
    '\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
    '\170', '\171', '\172', '\173', '\174', '\175', '\176', '\177',
    '\200', '\201', '\202', '\203', '\204', '\205', '\206', '\207',
    '\210', '\211', '\212', '\213', '\214', '\215', '\216', '\217',
    '\220', '\221', '\222', '\223', '\224', '\225', '\226', '\227',
    '\230', '\231', '\232', '\233', '\234', '\235', '\236', '\237',
    '\240', '\241', '\242', '\243', '\244', '\245', '\246', '\247',
    '\250', '\251', '\252', '\253', '\254', '\255', '\256', '\257',
    '\260', '\261', '\262', '\263', '\264', '\265', '\266', '\267',
    '\270', '\271', '\272', '\273', '\274', '\275', '\276', '\277',
    '\300', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
    '\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
    '\360', '\361', '\362', '\363', '\364', '\365', '\366', '\367',
    '\370', '\371', '\372', '\333', '\334', '\335', '\336', '\337',
    '\340', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
    '\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
    '\360', '\361', '\362', '\363', '\364', '\365', '\366', '\367',
    '\370', '\371', '\372', '\373', '\374', '\375', '\376', '\377',
};

int strncasecmp(const char *s1, const char *s2, size_t n)
{
    unsigned char u1, u2;

    for (; n != 0; --n) {
        u1 = (unsigned char)*s1++;
        u2 = (unsigned char)*s2++;
        if (charmap[u1] != charmap[u2]) {
            return charmap[u1] - charmap[u2];
        }

        if (u1 == '\0') {
            return 0;
        }
    }
    return 0;
}
#endif

static void cleanup(void)
{
    int i;

    if (output_filename != NULL) {
        free(output_filename);
    }
    if (cart_name != NULL) {
        free(cart_name);
    }
    for (i = 0; i < 33; i++) {
        if (input_filename[i] != NULL) {
            free(input_filename[i]);
        }
    }
}

static unsigned int count_valid_option_elements(void)
{
    unsigned int i = 1;
    unsigned int amount = 0;

    while (cart_info[i].name) {
        if (cart_info[i].opt) {
            amount++;
        }
        i++;
    }
    return amount;
}

static int compare_elements(const void *op1, const void *op2)
{
    const sorted_cart_t *p1 = (const sorted_cart_t *)op1;
    const sorted_cart_t *p2 = (const sorted_cart_t *)op2;

    return strcmp(p1->opt, p2->opt);
}

static void usage_types(void)
{
    unsigned int i = 1;
    int n = 0;
    unsigned int amount;
    sorted_cart_t *sorted_option_elements;

    cleanup();
    printf("supported cart types:\n\n");

    printf("bin      Binary .bin file (Default crt->bin)\n");
    printf("prg      Binary C64 .prg file with load-address\n\n");
    printf("normal   Generic 8KiB/12KiB/16KiB .crt file (Default bin->crt)\n");
    printf("ulti     Ultimax mode 4KiB/8KiB/16KiB .crt file\n\n");

    /* get the amount of valid options, excluding crt id 0 */
    amount = count_valid_option_elements();

    sorted_option_elements = malloc(amount * sizeof(sorted_cart_t));

    /* fill in the array with the information needed */
    while (cart_info[i].name) {
        if (cart_info[i].opt) {
            sorted_option_elements[n].opt = cart_info[i].opt;
            sorted_option_elements[n].name = cart_info[i].name;
            sorted_option_elements[n].crt_id = (int)i;
            switch (i) {
                case CARTRIDGE_DELA_EP7x8:
                case CARTRIDGE_DELA_EP64:
                case CARTRIDGE_REX_EP256:
                case CARTRIDGE_DELA_EP256:
                    sorted_option_elements[n].insertion = 1;
                    break;
                default:
                    sorted_option_elements[n].insertion = 0;
                    break;
            }
            n++;
        }
        i++;
    }

    qsort(sorted_option_elements, amount, sizeof(sorted_cart_t), compare_elements);

    /* output the sorted list */
    for (i = 0; i < amount; i++) {
        n = sorted_option_elements[i].insertion;
        printf("%-8s %2d %s .crt file%s\n",
               sorted_option_elements[i].opt,
               sorted_option_elements[i].crt_id,
               sorted_option_elements[i].name, n ? ", extra files can be inserted" : "");
    }
    free(sorted_option_elements);
    exit(1);
}

static void usage(void)
{
    cleanup();
    printf("convert:    cartconv [-r] [-q] [-t cart type] [-s cart revision] -i \"input name\" -o \"output name\" [-n \"cart name\"] [-l load address]\n");
    printf("print info: cartconv [-r] -f \"input name\"\n\n");
    printf("-f <name>    print info on file\n");
    printf("-r           repair mode (accept broken input files)\n");
    printf("-p           accept non padded binaries as input\n");
    printf("-b           output all banks (do not optimize the .crt file)\n");
    printf("-t <type>    output cart type\n");
    printf("-s <rev>     output cart revision/subtype\n");
    printf("-i <name>    input filename\n");
    printf("-o <name>    output filename\n");
    printf("-n <name>    crt cart name\n");
    printf("-l <addr>    load address\n");
    printf("-q           quiet\n");
    printf("--types      show the supported cart types\n");
    printf("--version    print cartconv version\n");
    exit(1);
}


/** \brief  Dump cartconv version string on stdout
 *
 * Dumps the SVN revision as well, if compiled from SVN
 */
static void dump_version(void)
{

    printf("cartconv dkl modded (VICE %s)\n", VERSION);
}


static void printbanks(char *name)
{
    FILE *f;
    FILE *bout;
    FILE *hout;
    unsigned char b[0x10];
    unsigned char headerbuffer[0x40];
    unsigned char bankbuffer[0x4010];
    long len, filelen;
    long pos;
    unsigned int type, bank, start, size;
    char *typestr[4] = { "ROM", "RAM", "FLASH", "UNK" };
    unsigned int numbanks;
    unsigned long tsize;

    f = fopen(name, "rb");
    fseek(f, 0, SEEK_END);
    filelen = ftell(f);

    tsize = 0; numbanks = 0;
    if (f) {
        
        fseek(f, 0, SEEK_SET);
        fread(headerbuffer, 1, 0x40, f);
        hout = fopen("000_0000_0040_CRT_header", "wb");
        fwrite(headerbuffer, 0x40, 1, hout);
        fclose(hout);
        
        fseek(f, 0x40, SEEK_SET); /* skip crt header */
        pos = 0x40; //offset header !
        printf("\noffset  sig  type  bank start size  chunklen\n");
        while (!feof(f)) {
            fseek(f, pos, SEEK_SET);
            /* get chip header */
            if (fread(b, 1, 0x10, f) < 0x10) {
                break;
            }
            len = (b[7] + (b[6] * 0x100) + (b[5] * 0x10000) + (b[4] * 0x1000000));
            type = (unsigned int)((b[8] * 0x100) + b[9]);
            bank = (unsigned int)((b[10] * 0x100) + b[11]);
            start = (unsigned int)((b[12] * 0x100) + b[13]);
            size = (unsigned int)((b[14] * 0x100) + b[15]);
            if (type > 2) {
                type = 3; /* invalid */
            }
            printf("$%06lx %-1c%-1c%-1c%-1c %-5s #%03u $%04x $%04x $%04lx\n",
                    (unsigned long)pos, b[0], b[1], b[2], b[3],
                    typestr[type], bank, start, size, (unsigned long)len);
            if ((size + 0x10) > len) {
                printf("  Error: data size exceeds chunk length\n");
            }
            if (len > (filelen - pos)) {
                printf("  Error: data size exceeds end of file\n");
                break;
            }
            
            // FSEEK -0x10 zeichen zur??ck
            
            fseek(f, -16, SEEK_CUR);
            
            fread(bankbuffer, 1, len + 0x10, f);
            char bankname[25];

            sprintf(bankname, "%03x_%04x_%04x", bank, start, (start+size-1));
            bout = fopen(bankname, "wb");
            fwrite(bankbuffer, len, 1, bout);
            fclose(bout);
            
            // FSEEK +0x10 zeichen vor
            fseek(f, 16, SEEK_CUR);
            
           pos += len;
            numbanks++;
            tsize += size;
        }
        fclose(f);
        printf("\ntotal banks: %u size: $%06lx\n", numbanks, tsize);
    }
}

static void printinfo(char *name)
{
    int crtid;
    char *idname, *modename;
    char cartname[0x20 + 1];
    char *exrom_warning = NULL;
    char *game_warning = NULL;

    if (load_input_file(name) < 0) {
        printf("Error: this file seems broken.\n\n");
    }
    crtid = headerbuffer[0x17] + (headerbuffer[0x16] << 8);
    if (headerbuffer[0x17] & 0x80) {
        /* handle our negative test IDs */
        crtid -= 0x10000;
    }
    if ((crtid >= 0) && (crtid <= CARTRIDGE_LAST)) {
        idname = cart_info[crtid].name;
    } else {
        idname = "unknown";
    }
    if ((headerbuffer[0x18] == 1) && (headerbuffer[0x19] == 0)) {
        modename = "ultimax";
    } else if ((headerbuffer[0x18] == 0) && (headerbuffer[0x19] == 0)) {
        modename = "16k Game";
    } else if ((headerbuffer[0x18] == 0) && (headerbuffer[0x19] == 1)) {
        modename = "8k Game";
    } else {
        modename = "?";
    }
    if (crtid && headerbuffer[0x18] != cart_info[crtid].exrom) {
        exrom_warning = "Warning: exrom in crt image set incorrectly.\n";
    }
    if (crtid && headerbuffer[0x19] != cart_info[crtid].game) {
        game_warning = "Warning: game in crt image set incorrectly.\n";
    }
    memcpy(cartname, &headerbuffer[0x20], 0x20); cartname[0x20] = 0;
    printf("CRT Version: %d.%d\n", headerbuffer[0x14], headerbuffer[0x15]);
    printf("Name: %s\n", cartname);
    printf("Hardware ID: %d (%s)\n", crtid, idname);
    printf("Hardware Revision: %d\n", headerbuffer[0x1a]);
    printf("Mode: exrom: %d game: %d (%s)\n", headerbuffer[0x18], headerbuffer[0x19], modename);
    if (exrom_warning) {
        printf("%s", exrom_warning);
    }
    if (game_warning) {
        printf("%s", game_warning);
    }
    printbanks(name);
    exit (0);
}

static void checkarg(char *arg)
{
    if (arg == NULL) {
        usage();
    }
}

static int checkflag(char *flg, char *arg)
{
    int i;

    switch (tolower((int)(flg[1]))) {
        case 'f':
            printinfo(arg);
            return 2;
        case 'r':
            repair_mode = 1;
            return 1;
        case 'b':
            omit_empty_banks = 0;
            return 1;
        case 'q':
            quiet_mode = 1;
            return 1;
        case 'p':
            input_padding = 1;
            return 1;
        case 'o':
            checkarg(arg);
            if (output_filename == NULL) {
                output_filename = strdup(arg);
            } else {
                usage();
            }
            return 2;
        case 'n':
            checkarg(arg);
            if (cart_name == NULL) {
                cart_name = strdup(arg);
            } else {
                usage();
            }
            return 2;
        case 'l':
            checkarg(arg);
            if (load_address == 0) {
                load_address = atoi(arg);
            } else {
                usage();
            }
            return 2;
        case 's':
            checkarg(arg);
            if (cart_subtype == 0) {
                cart_subtype = atoi(arg);
            } else {
                usage();
            }
            return 2;
        case 't':
            checkarg(arg);
            if (cart_type != -1 || convert_to_bin != 0 || convert_to_prg != 0 || convert_to_ultimax != 0) {
                usage();
            } else {
                for (i = 0; cart_info[i].name != NULL; i++) {
                    if (cart_info[i].opt != NULL) {
                        if (!strcasecmp(cart_info[i].opt, arg)) {
                            cart_type = (signed char)i;
                            break;
                        }
                    }
                }
                if (cart_type == -1) {
                    if (!strcmp(arg, "bin")) {
                        convert_to_bin = 1;
                    } else if (!strcmp(arg, "normal")) {
                        cart_type = CARTRIDGE_CRT;
                    } else if (!strcmp(arg, "prg")) {
                        convert_to_prg = 1;
                    } else if (!strcmp(arg, "ulti")) {
                        cart_type = CARTRIDGE_CRT;
                        convert_to_ultimax = 1;
                    } else {
                        usage();
                    }
                } else if (cart_type == 61) { /* MAX Basic */
                    convert_to_ultimax = 1;
                }
            }
            return 2;
        case 'i':
            checkarg(arg);
            if (input_filenames == 33) {
                usage();
            }
            input_filename[input_filenames] = strdup(arg);
            input_filenames++;
            return 2;
        default:
            usage();
            break;
    }
    return 1;
}

static void too_many_inputs(void)
{
    fprintf(stderr, "Error: too many input files\n");
    cleanup();
    exit(1);
}


/* this loads the easyflash cart and puts it as the interleaved way into
   the buffer for easy binary saving */
static int load_easyflash_crt(void)
{
    unsigned int load_position;

    memset(filebuffer, 0xff, 0x100000);
    while (1) {
        if (fread(chipbuffer, 1, 16, infile) != 16) {
            if (loadfile_size == 0) {
                return -1;
            } else {
                return 0;
            }
        }
        loadfile_size = 0x100000;
        if (chipbuffer[0] != 'C' || chipbuffer[1] != 'H' || chipbuffer[2] != 'I' || chipbuffer[3] != 'P') {
            return -1;
        }
        if (load_address == 0) {
            load_address = (chipbuffer[0xc] << 8) + chipbuffer[0xd];
        }
        load_position = (unsigned int)((chipbuffer[0xb] * 0x4000) + ((chipbuffer[0xc] == 0x80) ? 0 : 0x2000));
        if (fread(filebuffer + load_position, 1, 0x2000, infile) != 0x2000) {
            return -1;
        }
    }
}

static int load_all_banks(void)
{
    unsigned int length, datasize, loadsize, pad;

    if (loadfile_cart_type == CARTRIDGE_EASYFLASH) {
        return load_easyflash_crt();
    }

    while (1) {
        /* get CHIP header */
        if (fread(chipbuffer, 1, 16, infile) != 16) {
            if (loadfile_size == 0) {
                fprintf(stderr, "Error: could not read data from file.\n");
                return -1;
            } else {
                return 0;
            }
        }
        if (chipbuffer[0] != 'C' || chipbuffer[1] != 'H' || chipbuffer[2] != 'I' || chipbuffer[3] != 'P') {
            fprintf(stderr, "Error: CHIP tag not found.\n");
            return -1;
        }
        /* set load address to the load address of first CHIP in the file. this is not quite
           correct, but works ok for the few cases when it matters */
        if (load_address == 0) {
            load_address = (chipbuffer[0xc] << 8) + chipbuffer[0xd];
        }
        length = (unsigned int)((chipbuffer[4] << 24) + (chipbuffer[5] << 16) + (chipbuffer[6] << 8) + chipbuffer[7]);
        datasize = (unsigned int)((chipbuffer[14] * 0x100) + chipbuffer[15]);
        loadsize = datasize;
        if ((datasize + 0x10) > length) {
            if (repair_mode) {
                fprintf(stderr, "Warning: data size exceeds chunk length. (data:%04x chunk:%04x)\n", datasize, length);
                loadsize = length - 0x10;
            } else {
                fprintf(stderr, "Error: data size exceeds chunk length. (data:%04x chunk:%04x) (use -r to force)\n", datasize, length);
                return -1;
            }
        }
        /* load data */
        if (fread(filebuffer + loadfile_size, 1, loadsize, infile) != loadsize) {
            if (repair_mode) {
                fprintf(stderr, "Warning: unexpected end of file.\n");
                loadfile_size += datasize;
                break;
            }
            fprintf(stderr, "Error: could not read data from file. (use -r to force)\n");
            return -1;
        }
        /* if the chunk is larger than the contained data+chip header, skip the rest */
        pad = length - (datasize + 0x10);
        if (pad > 0) {
            fprintf(stderr, "Warning: chunk length exceeds data size (data:%04x chunk:%04x), skipping %04x bytes.\n", datasize, length, pad);
            fseek(infile, pad, SEEK_CUR);
        }
        loadfile_size += datasize;
    }
    return 0;
}


/*
 static int save_banks_to_file(void) {
    
    return -1;
}
 */

static int save_binary_output_file(void)
{
    unsigned char address_buffer[2];

    outfile = fopen(output_filename, "wb");
    if (outfile == NULL) {
        fprintf(stderr, "Error: Can't open output file %s\n", output_filename);
        return -1;
    }
    if (convert_to_prg == 1) {
        address_buffer[0] = (unsigned char)(load_address & 0xff);
        address_buffer[1] = (unsigned char)(load_address >> 8);
        if (fwrite(address_buffer, 1, 2, outfile) != 2) {
            fprintf(stderr, "Error: Can't write to file %s\n", output_filename);
            fclose(outfile);
            return -1;
        }
    }
    if (fwrite(filebuffer, 1, loadfile_size, outfile) != loadfile_size) {
        fprintf(stderr, "Error: Can't write to file %s\n", output_filename);
        fclose(outfile);
        return -1;
    }
    fclose(outfile);
    if (!quiet_mode) {
        printf("Input file : %s\n", input_filename[0]);
        printf("Output file : %s\n", output_filename);
        printf("Conversion from %s .crt to binary format successful.\n", cart_info[loadfile_cart_type].name);
    }
    return 0;
}

static int write_crt_header(unsigned char gameline, unsigned char exromline)
{
    unsigned char crt_header[0x40] = "C64 CARTRIDGE   ";
    int endofname = 0;
    int i;

    /* header length */
    crt_header[0x10] = 0;
    crt_header[0x11] = 0;
    crt_header[0x12] = 0;
    crt_header[0x13] = 0x40;

    crt_header[0x14] = 1;   /* crt version high */
    /* crt version low */
    if (cart_subtype > 0) {
        crt_header[0x15] = 1;
    } else {
        crt_header[0x15] = 0;
    }

    crt_header[0x16] = 0;   /* cart type high */
    crt_header[0x17] = (unsigned char)cart_type;

    crt_header[0x18] = exromline;
    crt_header[0x19] = gameline;

    crt_header[0x1a] = cart_subtype;
    
    /* unused/reserved */
    crt_header[0x1b] = 0;
    crt_header[0x1c] = 0;
    crt_header[0x1d] = 0;
    crt_header[0x1e] = 0;
    crt_header[0x1f] = 0;

    if (cart_name == NULL) {
        cart_name = strdup("VICE CART");
    }

    for (i = 0; i < 32; i++) {
        if (endofname == 1) {
            crt_header[0x20 + i] = 0;
        } else {
            if (cart_name[i] == 0) {
                endofname = 1;
            } else {
                crt_header[0x20 + i] = (unsigned char)toupper((int)cart_name[i]);
            }
        }
    }

    outfile = fopen(output_filename, "wb");
    if (outfile == NULL) {
        fprintf(stderr, "Error: Can't open output file %s\n", output_filename);
        return -1;
    }
    if (fwrite(crt_header, 1, 0x40, outfile) != 0x40) {
        fprintf(stderr, "Error: Can't write crt header to file %s\n", output_filename);
        fclose(outfile);
        unlink(output_filename);
        return -1;
    }
    return 0;
}

static int write_chip_package(unsigned int length, unsigned int bank, unsigned int address, unsigned char type)
{
    unsigned char chip_header[0x10] = "CHIP";

    chip_header[4] = 0;
    chip_header[5] = 0;
    chip_header[6] = (unsigned char)((length + 0x10) >> 8);
    chip_header[7] = (unsigned char)((length + 0x10) & 0xff);

    chip_header[8] = 0;
    chip_header[9] = type;

    chip_header[0xa] = (unsigned char)(bank >> 8);
    chip_header[0xb] = (unsigned char)(bank & 0xff);

    chip_header[0xc] = (unsigned char)(address >> 8);
    chip_header[0xd] = (unsigned char)(address & 0xff);

    chip_header[0xe] = (unsigned char)(length >> 8);
    chip_header[0xf] = (unsigned char)(length & 0xff);
    if (fwrite(chip_header, 1, 0x10, outfile) != 0x10) {
        fprintf(stderr, "Error: Can't write chip header to file %s\n", output_filename);
        fclose(outfile);
        unlink(output_filename);
        return -1;
    }
    if (fwrite(filebuffer + loadfile_offset, 1, length, outfile) != length) {
        fprintf(stderr, "Error: Can't write data to file %s\n", output_filename);
        fclose(outfile);
        unlink(output_filename);
        return -1;
    }
    loadfile_offset += (int)length;
    return 0;
}

static void bin2crt_ok(void)
{
    if (!quiet_mode) {
        printf("Input file : %s\n", input_filename[0]);
        printf("Output file : %s\n", output_filename);
        printf("Conversion from binary format to %s .crt successful.\n",
               cart_info[(unsigned char)cart_type].name);
    }
}

static void save_regular_crt(unsigned int length, unsigned int banks, unsigned int address, unsigned int type, unsigned char game, unsigned char exrom)
{
    unsigned int i;
    unsigned int real_banks = banks;

    if (write_crt_header(game, exrom) < 0) {
        cleanup();
        exit(1);
    }

    if (real_banks == 0) {
        /* handle the case when a chip of half/4th the regular size
           is used on an otherwise identical hardware (eg 2k/4k
           chip on a 8k cart)
        */
        if (loadfile_size == (length / 2)) {
            length /= 2;
        } else if (loadfile_size == (length / 4)) {
            length /= 4;
        }
        real_banks = loadfile_size / length;
    }

    for (i = 0; i < real_banks; i++) {
        if (write_chip_package(length, i, address, (unsigned char)type) < 0) {
            cleanup();
            exit(1);
        }
    }
    fclose(outfile);
    bin2crt_ok();
    cleanup();
    exit(0);
}

static void save_fcplus_crt(unsigned int length, unsigned int banks, unsigned int address, unsigned int type, unsigned char game, unsigned char exrom)
{
    unsigned int i;
    unsigned int real_banks = banks;

    /* printf("save_fcplus_crt length: %d banks:%d address: %d\n", length, banks, address); */

    if (write_crt_header(game, exrom) < 0) {
        cleanup();
        exit(1);
    }

    if (real_banks == 0) {
        real_banks = loadfile_size / length;
    }

    if (loadfile_size != 0x8000) {
        memmove(filebuffer + 0x2000, filebuffer, 0x6000);
        memset(filebuffer, 0xff, 0x2000);
    }

    for (i = 0; i < real_banks; i++) {
        if (write_chip_package(length, i, address, (unsigned char)type) < 0) {
            cleanup();
            exit(1);
        }
    }

    fclose(outfile);
    bin2crt_ok();
    cleanup();
    exit(0);
}

static void save_2_blocks_crt(unsigned int l1, unsigned int l2, unsigned int a1, unsigned int a2, unsigned char game, unsigned char exrom)
{
    if (write_crt_header(game, exrom) < 0) {
        cleanup();
        exit(1);
    }

    if (write_chip_package(0x2000, 0, 0x8000, 0) < 0) {
        cleanup();
        exit(1);
    }

    if (write_chip_package(0x2000, 0, (a2 == 0xe000) ? 0xe000 : 0xa000, 0) < 0) {
        cleanup();
        exit(1);
    }

    fclose(outfile);
    bin2crt_ok();
    cleanup();
    exit(0);
}

static int check_empty_easyflash(void)
{
    int i;

    for (i = 0; i < 0x2000; i++) {
        if (filebuffer[loadfile_offset + i] != 0xff) {
            return 0;
        }
    }
    return 1;
}

static void save_easyflash_crt(unsigned int p1, unsigned int p2, unsigned int p3, unsigned int p4, unsigned char p5, unsigned char p6)
{
    unsigned int i, j;

    if (write_crt_header(0, 0) < 0) {
        cleanup();
        exit(1);
    }

    for (i = 0; i < 64; i++) {
        for (j = 0; j < 2; j++) {
            if ((omit_empty_banks == 1) && (check_empty_easyflash() == 1)) {
                loadfile_offset += 0x2000;
            } else {
                if (write_chip_package(0x2000, i, (j == 0) ? 0x8000 : 0xa000, 2) < 0) {
                    cleanup();
                    exit(1);
                }
            }
        }
    }

    fclose(outfile);
    bin2crt_ok();
    cleanup();
    exit(0);
}

static void save_ocean_crt(unsigned int p1, unsigned int p2, unsigned int p3, unsigned int p4, unsigned char p5, unsigned char p6)
{
    unsigned int i;

    if (loadfile_size != CARTRIDGE_SIZE_256KB) {
        save_regular_crt(0x2000, 0, 0x8000, 0, 0, 0);
    } else {
        if (write_crt_header(1, 0) < 0) {
            cleanup();
            exit(1);
        }

        for (i = 0; i < 16; i++) {
            if (write_chip_package(0x2000, i, 0x8000, 0) < 0) {
                cleanup();
                exit(1);
            }
        }

        for (i = 0; i < 16; i++) {
            if (write_chip_package(0x2000, i + 16, 0xa000, 0) < 0) {
                cleanup();
                exit(1);
            }
        }

        fclose(outfile);
        bin2crt_ok();
        cleanup();
        exit(0);
    }
}

static void save_funplay_crt(unsigned int p1, unsigned int p2, unsigned int p3, unsigned int p4, unsigned char p5, unsigned char p6)
{
    unsigned int i = 0;

    if (write_crt_header(1, 0) < 0) {
        cleanup();
        exit(1);
    }

    while (i != 0x41) {
        if (write_chip_package(0x2000, i, 0x8000, 0) < 0) {
            cleanup();
            exit(1);
        }
        i += 8;
        if (i == 0x40) {
            i = 1;
        }
    }

    fclose(outfile);
    bin2crt_ok();
    cleanup();
    exit(0);
}

static void save_easycalc_crt(unsigned int p1, unsigned int p2, unsigned int p3, unsigned int p4, unsigned char p5, unsigned char p6)
{
    if (write_crt_header(1, 1) < 0) {
        cleanup();
        exit(1);
    }

    if (write_chip_package(0x2000, 0, 0x8000, 0) < 0) {
        cleanup();
        exit(1);
    }

    if (write_chip_package(0x2000, 0, 0xa000, 0) < 0) {
        cleanup();
        exit(1);
    }

    if (write_chip_package(0x2000, 1, 0xa000, 0) < 0) {
        cleanup();
        exit(1);
    }

    fclose(outfile);
    bin2crt_ok();
    cleanup();
    exit(0);
}

static void save_zaxxon_crt(unsigned int p1, unsigned int p2, unsigned int p3, unsigned int p4, unsigned char p5, unsigned char p6)
{
    if (write_crt_header(0, 0) < 0) {
        cleanup();
        exit(1);
    }

    if (write_chip_package(0x1000, 0, 0x8000, 0) < 0) {
        cleanup();
        exit(1);
    }

    if (write_chip_package(0x2000, 0, 0xa000, 0) < 0) {
        cleanup();
        exit(1);
    }

    if (write_chip_package(0x2000, 1, 0xa000, 0) < 0) {
        cleanup();
        exit(1);
    }

    fclose(outfile);
    bin2crt_ok();
    cleanup();
    exit(0);
}

static void save_stardos_crt(unsigned int p1, unsigned int p2, unsigned int p3, unsigned int p4, unsigned char p5, unsigned char p6)
{
    if (write_crt_header(1, 0) < 0) {
        cleanup();
        exit(1);
    }

    if (write_chip_package(0x2000, 0, 0x8000, 0) < 0) {
        cleanup();
        exit(1);
    }

    if (write_chip_package(0x2000, 0, 0xe000, 0) < 0) {
        cleanup();
        exit(1);
    }

    fclose(outfile);
    bin2crt_ok();
    cleanup();
    exit(0);
}

static int load_input_file(char *filename)
{
    loadfile_offset = 0;
    infile = fopen(filename, "rb");
    if (infile == NULL) {
        fprintf(stderr, "Error: Can't open %s\n", filename);
        return -1;
    }
    /* fill buffer with 0xff, like empty eproms */
    memset(filebuffer, 0xff, CARTRIDGE_SIZE_MAX);
    /* read first 16 bytes */
    if (fread(filebuffer, 1, 16, infile) != 16) {
        fprintf(stderr, "Error: Can't read %s\n", filename);
        fclose(infile);
        return -1;
    }
    if (!strncmp("C64 CARTRIDGE   ", (char *)filebuffer, 16)) {
        loadfile_is_crt = 1;
        if (fread(headerbuffer + 0x10, 1, 0x30, infile) != 0x30) {
            fprintf(stderr, "Error: Can't read the full header of %s\n", filename);
            fclose(infile);
            return -1;
        }
        if (headerbuffer[0x10] != 0 || headerbuffer[0x11] != 0 || headerbuffer[0x12] != 0 || headerbuffer[0x13] != 0x40) {
            fprintf(stderr, "Error: Illegal header size in %s\n", filename);
            if (!repair_mode) {
                fclose(infile);
                return -1;
            }
        }
        if (headerbuffer[0x18] == 1 && headerbuffer[0x19] == 0) {
            loadfile_is_ultimax = 1;
        } else {
            loadfile_is_ultimax = 0;
        }

        loadfile_cart_type = headerbuffer[0x17] + (headerbuffer[0x16] << 8);
        if (headerbuffer[0x17] & 0x80) {
            /* handle our negative test IDs */
            loadfile_cart_type -= 0x10000;
        }
        if (!((loadfile_cart_type >= 0) && (loadfile_cart_type <= CARTRIDGE_LAST))) {
            fprintf(stderr, "Error: Unknown CRT ID: %d\n", loadfile_cart_type);
            fclose(infile);
            return -1;
        }

        loadfile_size = 0;
        if (load_all_banks() < 0) {
            if (repair_mode) {
                fprintf(stderr, "Warning: Can't load all banks of %s\n", filename);
                fclose(infile);
                return 0;
            } else {
                fprintf(stderr, "Error: Can't load all banks of %s (use -r to force)\n", filename);
                fclose(infile);
                return -1;
            }
        } else {
            fclose(infile);
            return 0;
        }
    } else {
        loadfile_is_crt = 0;
        /* read the rest of the file */
        loadfile_size = (unsigned int)fread(filebuffer + 0x10, 1, CARTRIDGE_SIZE_MAX - 14, infile) + 0x10;

        switch (loadfile_size) {
            case CARTRIDGE_SIZE_2KB:
            case CARTRIDGE_SIZE_4KB:
            case CARTRIDGE_SIZE_8KB:
            case CARTRIDGE_SIZE_12KB:
            case CARTRIDGE_SIZE_16KB:
            case CARTRIDGE_SIZE_20KB:
            case CARTRIDGE_SIZE_24KB:
            case CARTRIDGE_SIZE_32KB:
            case CARTRIDGE_SIZE_64KB:
            case CARTRIDGE_SIZE_96KB:
            case CARTRIDGE_SIZE_128KB:
            case CARTRIDGE_SIZE_256KB:
            case CARTRIDGE_SIZE_512KB:
            case CARTRIDGE_SIZE_1024KB:
            case CARTRIDGE_SIZE_2048KB:
            case CARTRIDGE_SIZE_4096KB:
            case CARTRIDGE_SIZE_8192KB:
            case CARTRIDGE_SIZE_16384KB:
                loadfile_offset = 0;
                fclose(infile);
                return 0;
                break;
            case CARTRIDGE_SIZE_2KB + 2:
            case CARTRIDGE_SIZE_4KB + 2:
            case CARTRIDGE_SIZE_8KB + 2:
            case CARTRIDGE_SIZE_12KB + 2:
            case CARTRIDGE_SIZE_16KB + 2:
            case CARTRIDGE_SIZE_20KB + 2:
            case CARTRIDGE_SIZE_24KB + 2:
            case CARTRIDGE_SIZE_32KB + 2:
            case CARTRIDGE_SIZE_64KB + 2:
            case CARTRIDGE_SIZE_96KB + 2:
            case CARTRIDGE_SIZE_128KB + 2:
            case CARTRIDGE_SIZE_256KB + 2:
            case CARTRIDGE_SIZE_512KB + 2:
            case CARTRIDGE_SIZE_1024KB + 2:
            case CARTRIDGE_SIZE_2048KB + 2:
            case CARTRIDGE_SIZE_4096KB + 2:
            case CARTRIDGE_SIZE_8192KB + 2:
            case CARTRIDGE_SIZE_16384KB + 2:
                loadfile_size -= 2;
                loadfile_offset = 2;
                fclose(infile);
                return 0;
                break;
            case CARTRIDGE_SIZE_32KB + 4:
                loadfile_size -= 4;
                loadfile_offset = 4;
                fclose(infile);
                return 0;
                break;
            default:
                fclose(infile);
                if (input_padding) {
                    return 0;
                }
                fprintf(stderr, "Error: Illegal file size of %s\n", filename);
                return -1;
                break;
        }
    }
}

static void close_output_cleanup(void)
{
    fclose(outfile);
    unlink(output_filename);
    cleanup();
    exit(1);
}

static void save_delaep64_crt(unsigned int p1, unsigned int p2, unsigned int p3, unsigned int p4, unsigned char p5, unsigned char p6)
{
    unsigned int i;

    if (loadfile_size != CARTRIDGE_SIZE_8KB) {
        fprintf(stderr, "Error: wrong size of Dela EP64 base file %s (%u)\n",
                input_filename[0], loadfile_size);
        cleanup();
        exit(1);
    }

    if (write_crt_header(1, 0) < 0) {
        cleanup();
        exit(1);
    }

    /* write base file */
    if (write_chip_package(0x2000, 0, 0x8000, 0) < 0) {
        cleanup();
        exit(1);
    }

    if (input_filenames > 1) {
        /* write user eproms */
        for (i = 0; i < input_filenames; i++) {
            if (load_input_file(input_filename[i]) < 0) {
                close_output_cleanup();
            }
            if (loadfile_is_crt == 1) {
                fprintf(stderr, "Error: to be inserted file can only be a binary for Dela EP64\n");
                close_output_cleanup();
            }
            if (loadfile_size != CARTRIDGE_SIZE_32KB) {
                fprintf(stderr, "Error: to be inserted file can only be 32KiB in size for Dela EP64\n");
                close_output_cleanup();
            }
            if (write_chip_package(0x8000, i + 1, 0x8000, 0) < 0) {
                close_output_cleanup();
            }
        }
    }

    fclose(outfile);
    bin2crt_ok();
    cleanup();
    exit(0);
}

static void save_delaep256_crt(unsigned int p1, unsigned int p2, unsigned int p3, unsigned int p4, unsigned char p5, unsigned char p6)
{
    unsigned int i, j;
    unsigned int insert_size = 0;

    if (loadfile_size != CARTRIDGE_SIZE_8KB) {
        fprintf(stderr, "Error: wrong size of Dela EP256 base file %s (%u)\n",
                input_filename[0], loadfile_size);
        cleanup();
        exit(1);
    }

    if (input_filenames == 1) {
        fprintf(stderr, "Error: no files to insert into Dela EP256 .crt\n");
        cleanup();
        exit(1);
    }

    if (write_crt_header(1, 0) < 0) {
        cleanup();
        exit(1);
    }

    if (write_chip_package(0x2000, 0, 0x8000, 0) < 0) {
        cleanup();
        exit(1);
    }

    for (i = 0; i < (unsigned int)input_filenames - 1; i++) {
        if (load_input_file(input_filename[i + 1]) < 0) {
            close_output_cleanup();
        }

        if (loadfile_size != CARTRIDGE_SIZE_32KB && loadfile_size != CARTRIDGE_SIZE_8KB) {
            fprintf(stderr, "Error: only 32KiB binary files or 8KiB bin/crt files can be inserted in Dela EP256\n");
            close_output_cleanup();
        }

        if (insert_size == 0) {
            insert_size = loadfile_size;
        }

        if (insert_size == CARTRIDGE_SIZE_32KB && input_filenames > 8) {
            fprintf(stderr, "Error: a maximum of 8 32KiB images can be inserted\n");
            close_output_cleanup();
        }

        if (insert_size != loadfile_size) {
            fprintf(stderr, "Error: only one type of insertion is allowed at this time for Dela EP256\n");
            close_output_cleanup();
        }

        if (loadfile_is_crt == 1 && (loadfile_size != CARTRIDGE_SIZE_8KB || load_address != 0x8000 || loadfile_is_ultimax == 1)) {
            fprintf(stderr, "Error: you can only insert generic 8KiB .crt files for Dela EP256\n");
            close_output_cleanup();
        }

        if (insert_size == CARTRIDGE_SIZE_32KB) {
            for (j = 0; j < 4; j++) {
                if (write_chip_package(0x2000, (i * 4) + j + 1, 0x8000, 0) < 0) {
                    close_output_cleanup();
                }
            }
            if (!quiet_mode) {
                printf("inserted %s in banks %u-%u of the Dela EP256 .crt\n",
                        input_filename[i + 1], (i * 4) + 1, (i * 4) + 4);
            }
        } else {
            if (write_chip_package(0x2000, i + 1, 0x8000, 0) < 0) {
                close_output_cleanup();
            }
            if (!quiet_mode) {
                printf("inserted %s in bank %u of the Dela EP256 .crt\n",
                        input_filename[i + 1], i + 1);
            }
        }
    }

    fclose(outfile);
    bin2crt_ok();
    cleanup();
    exit(0);
}

static void save_delaep7x8_crt(unsigned int p1, unsigned int p2, unsigned int p3, unsigned int p4, unsigned char p5, unsigned char p6)
{
    int inserted_size = 0;
    int name_counter = 1;
    unsigned int chip_counter = 1;

    if (loadfile_size != CARTRIDGE_SIZE_8KB) {
        fprintf(stderr, "Error: wrong size of Dela EP7x8 base file %s (%u)\n",
                input_filename[0], loadfile_size);
        cleanup();
        exit(1);
    }

    if (input_filenames == 1) {
        fprintf(stderr, "Error: no files to insert into Dela EP7x8 .crt\n");
        cleanup();
        exit(1);
    }

    if (write_crt_header(1, 0) < 0) {
        cleanup();
        exit(1);
    }

    if (write_chip_package(0x2000, 0, 0x8000, 0) < 0) {
        cleanup();
        exit(1);
    }

    while (name_counter != input_filenames) {
        if (load_input_file(input_filename[name_counter]) < 0) {
            close_output_cleanup();
        }

        if (loadfile_size == CARTRIDGE_SIZE_32KB) {
            if (loadfile_is_crt == 1) {
                fprintf(stderr, "Error: (%s) only binary 32KiB images can be inserted into a Dela EP7x8 .crt\n",
                        input_filename[name_counter]);
                close_output_cleanup();
            } else {
                if (inserted_size != 0) {
                    fprintf(stderr, "Error: (%s) only the first inserted image can be a 32KiB image for Dela EP7x8\n",
                            input_filename[name_counter]);
                    close_output_cleanup();
                } else {
                    if (write_chip_package(0x2000, chip_counter, 0x8000, 0) < 0) {
                        close_output_cleanup();
                    }
                    if (write_chip_package(0x2000, chip_counter + 1, 0x8000, 0) < 0) {
                        close_output_cleanup();
                    }
                    if (write_chip_package(0x2000, chip_counter + 2, 0x8000, 0) < 0) {
                        close_output_cleanup();
                    }
                    if (write_chip_package(0x2000, chip_counter + 3, 0x8000, 0) < 0) {
                        close_output_cleanup();
                    }
                    if (!quiet_mode) {
                        printf("inserted %s in banks %u-%u of the Dela EP7x8 .crt\n",
                               input_filename[name_counter], chip_counter,
                               chip_counter + 3);
                    }
                    chip_counter += 4;
                    inserted_size += 0x8000;
                }
            }
        }

        if (loadfile_size == CARTRIDGE_SIZE_16KB) {
            if (loadfile_is_crt == 1 && (loadfile_cart_type != 0 || loadfile_is_ultimax == 1)) {
                fprintf(stderr, "Error: (%s) only generic 16KiB .crt images can be inserted into a Dela EP7x8 .crt\n",
                        input_filename[name_counter]);
                close_output_cleanup();
            } else {
                if (inserted_size >= 0xc000) {
                    fprintf(stderr, "Error: (%s) no room to insert a 16KiB binary file into the Dela EP7x8 .crt\n",
                            input_filename[name_counter]);
                    close_output_cleanup();
                } else {
                    if (write_chip_package(0x2000, chip_counter, 0x8000, 0) < 0) {
                        close_output_cleanup();
                    }
                    if (write_chip_package(0x2000, chip_counter + 1, 0x8000, 0) < 0) {
                        close_output_cleanup();
                    }
                    if (!quiet_mode) {
                        printf("inserted %s in banks %u and %u of the Dela EP7x8 .crt\n",
                               input_filename[name_counter], chip_counter, chip_counter + 1);
                    }
                    chip_counter += 2;
                    inserted_size += 0x4000;
                }
            }
        }

        if (loadfile_size == CARTRIDGE_SIZE_8KB) {
            if (loadfile_is_crt == 1 && (loadfile_cart_type != 0 || loadfile_is_ultimax == 1)) {
                fprintf(stderr, "Error: (%s) only generic 8KiB .crt images can be inserted into a Dela EP7x8 .crt\n",
                        input_filename[name_counter]);
                close_output_cleanup();
            } else {
                if (inserted_size >= 0xe000) {
                    fprintf(stderr, "Error: (%s) no room to insert a 8KiB binary file into the Dela EP7x8 .crt\n",
                            input_filename[name_counter]);
                    close_output_cleanup();
                } else {
                    if (write_chip_package(0x2000, chip_counter, 0x8000, 0) < 0) {
                        close_output_cleanup();
                    }
                    if (!quiet_mode) {
                        printf("inserted %s in bank %u of the Dela EP7x8 .crt\n",
                                input_filename[name_counter], chip_counter);
                    }
                    chip_counter++;
                    inserted_size += 0x2000;
                }
            }
        }

        name_counter++;
    }

    fclose(outfile);
    bin2crt_ok();
    cleanup();
    exit(0);
}

static void save_rexep256_crt(unsigned int p1, unsigned int p2, unsigned int p3, unsigned int p4, unsigned char p5, unsigned char p6)
{
    int eprom_size_for_8kb = 0;
    int images_of_8kb_started = 0;
    int name_counter = 1;
    unsigned int chip_counter = 1;
    int subchip_counter = 1;

    if (loadfile_size != CARTRIDGE_SIZE_8KB) {
        fprintf(stderr, "Error: wrong size of Rex EP256 base file %s (%u)\n",
                input_filename[0], loadfile_size);
        cleanup();
        exit(1);
    }

    if (input_filenames == 1) {
        fprintf(stderr, "Error: no files to insert into Rex EP256 .crt\n");
        cleanup();
        exit(1);
    }

    if (write_crt_header(1, 0) < 0) {
        cleanup();
        exit(1);
    }

    if (write_chip_package(0x2000, 0, 0x8000, 0) < 0) {
        cleanup();
        exit(1);
    }

    while (name_counter != input_filenames) {
        if (load_input_file(input_filename[name_counter]) < 0) {
            close_output_cleanup();
        }

        if (chip_counter > 8) {
            fprintf(stderr, "Error: no more room for %s in the Rex EP256 .crt\n", input_filename[name_counter]);
        }

        if (loadfile_size == CARTRIDGE_SIZE_32KB) {
            if (loadfile_is_crt == 1) {
                fprintf(stderr, "Error: (%s) only binary 32KiB images can be inserted into a Rex EP256 .crt\n",
                        input_filename[name_counter]);
                close_output_cleanup();
            } else {
                if (images_of_8kb_started != 0) {
                    fprintf(stderr, "Error: (%s) only the first inserted images can be a 32KiB image for Rex EP256\n",
                            input_filename[name_counter]);
                    close_output_cleanup();
                } else {
                    if (write_chip_package(0x8000, chip_counter, 0x8000, 0) < 0) {
                        close_output_cleanup();
                    }
                    if (!quiet_mode) {
                        printf("inserted %s in bank %u as a 32KiB eprom of the Rex EP256 .crt\n",
                               input_filename[name_counter], chip_counter);
                    }
                    chip_counter++;
                }
            }
        }

        if (loadfile_size == CARTRIDGE_SIZE_8KB) {
            if (loadfile_is_crt == 1 && (loadfile_cart_type != 0 || loadfile_is_ultimax == 1)) {
                fprintf(stderr, "Error: (%s) only generic 8KiB .crt images can be inserted into a Rex EP256 .crt\n",
                        input_filename[name_counter]);
                close_output_cleanup();
            } else {
                if (images_of_8kb_started == 0) {
                    images_of_8kb_started = 1;
                    if ((9 - chip_counter) * 4 < (unsigned int)(input_filenames - name_counter)) {
                        fprintf(stderr, "Error: no room for the amount of input files given\n");
                        close_output_cleanup();
                    }
                    eprom_size_for_8kb = 1;
                    if ((9 - chip_counter) * 2 < (unsigned int)(input_filenames - name_counter)) {
                        eprom_size_for_8kb = 4;
                    }
                    if (9 - chip_counter < (unsigned int)(input_filenames - name_counter)) {
                        eprom_size_for_8kb = 2;
                    }
                }

                if (eprom_size_for_8kb == 1) {
                    if (write_chip_package(0x2000, chip_counter, 0x8000, 0) < 0) {
                        close_output_cleanup();
                        if (!quiet_mode) {
                            printf("inserted %s as an 8KiB eprom in bank %u of the Rex EP256 .crt\n",
                                   input_filename[name_counter], chip_counter);
                        }
                        chip_counter++;
                    }

                    if (eprom_size_for_8kb == 4 && (subchip_counter == 4 || name_counter == input_filenames - 1)) {
                        memcpy(extra_buffer_32kb + ((subchip_counter - 1) * 0x2000), filebuffer + loadfile_offset, 0x2000);
                        memcpy(filebuffer, extra_buffer_32kb, 0x8000);
                        loadfile_offset = 0;
                        if (write_chip_package(0x8000, chip_counter, 0x8000, 0) < 0) {
                            close_output_cleanup();
                        }
                        if (!quiet_mode) {
                            if (subchip_counter == 1) {
                                printf("inserted %s as a 32KiB eprom in bank %u of the Rex EP256 .crt\n",
                                       input_filename[name_counter], chip_counter);
                            } else {
                                printf(" and %s as a 32KiB eprom in bank %u of the Rex EP256 .crt\n",
                                       input_filename[name_counter], chip_counter);
                            }
                        }
                        chip_counter++;
                        subchip_counter = 1;
                    }

                    if (eprom_size_for_8kb == 4 && (subchip_counter == 3 || subchip_counter == 2) &&
                        name_counter != input_filenames) {
                        memcpy(extra_buffer_32kb + ((subchip_counter - 1) * 0x2000), filebuffer + loadfile_offset, 0x2000);
                        if (!quiet_mode) {
                            printf(", %s", input_filename[name_counter]);
                        }
                        subchip_counter++;
                    }

                    if (eprom_size_for_8kb == 2) {
                        if (subchip_counter == 2 || name_counter == input_filenames - 1) {
                            memcpy(extra_buffer_32kb + ((subchip_counter - 1) * 0x2000),
                                   filebuffer + loadfile_offset, 0x2000);
                            memcpy(filebuffer, extra_buffer_32kb, 0x4000);
                            loadfile_offset = 0;
                            if (write_chip_package(0x4000, chip_counter, 0x8000, 0) < 0) {
                                close_output_cleanup();
                            }
                            if (!quiet_mode) {
                                if (subchip_counter == 1) {
                                    printf("inserted %s as a 16KiB eprom in bank %u of the Rex EP256 .crt\n",
                                           input_filename[name_counter], chip_counter);
                                } else {
                                    printf(" and %s as a 16KiB eprom in bank %u of the Rex EP256 .crt\n",
                                           input_filename[name_counter], chip_counter);
                                }
                            }
                            chip_counter++;
                            subchip_counter = 1;
                        } else {
                            memcpy(extra_buffer_32kb, filebuffer + loadfile_offset, 0x2000);
                            if (!quiet_mode) {
                                printf("inserted %s", input_filename[name_counter]);
                            }
                            subchip_counter++;
                        }
                    }

                    if (eprom_size_for_8kb == 4 && subchip_counter == 1 && name_counter != input_filenames) {
                        memcpy(extra_buffer_32kb, filebuffer + loadfile_offset, 0x2000);
                        if (!quiet_mode) {
                            printf("inserted %s", input_filename[name_counter]);
                        }
                        subchip_counter++;
                    }
                }
            }
        }
        name_counter++;
    }

    fclose(outfile);
    bin2crt_ok();
    cleanup();
    exit(0);
}

static void save_generic_crt(unsigned int p1, unsigned int p2, unsigned int p3, unsigned int p4, unsigned char p5, unsigned char p6)
{
    /* printf("save_generic_crt ultimax: %d size: %08x\n", convert_to_ultimax, loadfile_size); */
    if (convert_to_ultimax == 1) {
        switch (loadfile_size) {
            case CARTRIDGE_SIZE_2KB:
                save_regular_crt(0x0800, 1, 0xf800, 0, 0, 1);
                break;
            case CARTRIDGE_SIZE_4KB:
                save_regular_crt(0x1000, 1, 0xf000, 0, 0, 1);
                break;
            case CARTRIDGE_SIZE_8KB:
                save_regular_crt(0x2000, 1, 0xe000, 0, 0, 1);
                break;
            case CARTRIDGE_SIZE_16KB:
                save_2_blocks_crt(0x2000, 0x2000, 0x8000, 0xe000, 0, 1);
                break;
            default:
                fprintf(stderr, "Error: invalid size for generic ultimax cartridge\n");
                cleanup();
                exit(1);
                break;
        }
    } else {
        switch (loadfile_size) {
            case CARTRIDGE_SIZE_2KB:
                save_regular_crt(0x0800, 0, 0x8000, 0, 1, 0);
                break;
            case CARTRIDGE_SIZE_4KB:
                save_regular_crt(0x1000, 0, 0x8000, 0, 1, 0);
                break;
            case CARTRIDGE_SIZE_8KB:
                save_regular_crt(0x2000, 0, 0x8000, 0, 1, 0);
                break;
            case CARTRIDGE_SIZE_12KB:
                save_regular_crt(0x3000, 1, 0x8000, 0, 0, 0);
                break;
            case CARTRIDGE_SIZE_16KB:
                save_regular_crt(0x4000, 1, 0x8000, 0, 0, 0);
                break;
            default:
                fprintf(stderr, "Error: invalid size for generic cartridge\n");
                cleanup();
                exit(1);
                break;
        }
    }
}

int main(int argc, char *argv[])
{
    int i;
    int arg_counter = 1;
    char *flag, *argument;

    if (argc > 1) {
        if(strcmp(argv[1], "--types") == 0) {
            usage_types();
            return EXIT_SUCCESS;
        } else if (strcmp(argv[1], "--version") == 0) {
            dump_version();
            return EXIT_SUCCESS;
        }
    }

    if (argc < 3) {
        usage();
        return EXIT_FAILURE;
    }

    for (i = 0; i < 33; i++) {
        input_filename[i] = NULL;
    }

    while (arg_counter < argc) {
        flag = argv[arg_counter];
        argument = (arg_counter + 1 < argc) ? argv[arg_counter + 1] : NULL;
        if (flag[0] != '-') {
            usage();
        } else {
            arg_counter += checkflag(flag, argument);
        }
    }

    if (output_filename == NULL) {
        fprintf(stderr, "Error: no output filename\n");
        cleanup();
        exit(1);
    }
    if (input_filenames == 0) {
        fprintf(stderr, "Error: no input filename\n");
        cleanup();
        exit(1);
    }
    if (!strcmp(output_filename, input_filename[0])) {
        fprintf(stderr, "Error: output filename = input filename\n");
        cleanup();
        exit(1);
    }
    if (load_input_file(input_filename[0]) < 0) {
        cleanup();
        exit(1);
    }
    if (input_filenames > 1 && cart_type != CARTRIDGE_DELA_EP64 && cart_type != CARTRIDGE_DELA_EP256 &&
        cart_type != CARTRIDGE_DELA_EP7x8 && cart_type != CARTRIDGE_REX_EP256 && loadfile_cart_type != CARTRIDGE_DELA_EP64 &&
        loadfile_cart_type != CARTRIDGE_DELA_EP256 && loadfile_cart_type != CARTRIDGE_DELA_EP7x8 &&
        loadfile_cart_type != CARTRIDGE_REX_EP256) {
        too_many_inputs();
    }
    if ((cart_type == CARTRIDGE_DELA_EP64 || loadfile_cart_type == CARTRIDGE_DELA_EP64) && input_filenames > 3) {
        too_many_inputs();
    }
    if ((cart_type == CARTRIDGE_DELA_EP7x8 || loadfile_cart_type == CARTRIDGE_DELA_EP7x8) && input_filenames > 8) {
        too_many_inputs();
    }
    if (loadfile_is_crt == 1) {
        if (cart_type == CARTRIDGE_DELA_EP64 || cart_type == CARTRIDGE_DELA_EP256 || cart_type == CARTRIDGE_DELA_EP7x8 ||
            cart_type == CARTRIDGE_REX_EP256) {
            cart_info[(unsigned char)cart_type].save(0, 0, 0, 0, 0, 0);
        } else {
            if (cart_type == -1) {
                if (save_binary_output_file() < 0) {
                    cleanup();
                    exit(1);
                }
            } else {
                fprintf(stderr, "Error: File is already .crt format\n");
                cleanup();
                exit(1);
            }
        }
    } else {
        if (cart_type == -1) {
            fprintf(stderr, "Error: File is already in binary format\n");
            cleanup();
            exit(1);
        }
        /* FIXME: the sizes are used in a bitfield, and also by their absolute values. this
                  check is doomed to fail because of that :)
        */
        if (input_padding) {
            while ((loadfile_size & cart_info[(unsigned char)cart_type].sizes) != loadfile_size) {
                loadfile_size++;
            }
        } else {
            if ((loadfile_size & cart_info[(unsigned char)cart_type].sizes) != loadfile_size) {
                fprintf(stderr, "Error: Input file size (%u) doesn't match %s requirements\n",
                        loadfile_size, cart_info[(unsigned char)cart_type].name);
                cleanup();
                exit(1);
            }
        }
        if (cart_info[(unsigned char)cart_type].save != NULL) {
            cart_info[(unsigned char)cart_type].save(cart_info[(unsigned char)cart_type].bank_size,
                                                     cart_info[(unsigned char)cart_type].banks,
                                                     cart_info[(unsigned char)cart_type].load_address,
                                                     cart_info[(unsigned char)cart_type].data_type,
                                                     cart_info[(unsigned char)cart_type].game,
                                                     cart_info[(unsigned char)cart_type].exrom);
        }
    }
    return 0;
}

