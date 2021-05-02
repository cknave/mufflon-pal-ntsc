#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <sys/time.h>

#include <inttypes.h>
#include <ctype.h>
#include <limits.h>
#include <libgen.h>



/* log levels */
//#define ENABLE_DEBUG
#define ENABLE_INFO
#define ENABLE_ERROR
#define ENABLE_FATAL

/* message macros */
#ifdef ENABLE_DEBUG
#define debug_message(format,args...);	{ if(!m->option_shutup) { fprintf(stdout,"DEBUG: %s: %s(): ",__FILE__,__FUNCTION__); fprintf(stdout,format,##args); } }
#define status_message(format,args...);
#else
#define debug_message(format,args...);
#define status_message(format,args...);	{ if(!m->option_shutup) { fprintf(stdout,format,##args); } }
#endif

#ifdef ENABLE_INFO
#define info_message(format,args...);	{ if(!m->option_shutup) { fprintf(stdout,format,##args); } }
#else
#define info_message(format,args...);
#endif

#ifdef ENABLE_ERROR
#define error_message(format,args...);	{ fprintf(stderr,"ERROR: %s: ",__FILE__); fprintf(stderr,format,##args); }
#else
#define error_message(format,args...);
#endif

#ifdef ENABLE_FATAL
#define fatal_message_and_exit(format,args...);	{ fprintf(stderr,"FATAL: %s: ",__FILE__); fprintf(stderr,format,##args); exit(2); }
#else
#define fatal_message_and_exit(format,args...); { exit(2); }
#endif

#define ARRAY_ELEMS(a) (sizeof(a) / sizeof((a)[0]))

#define DDC_PI atan2 (0.0, -1.0)

#define PREV_LINE	1
#define CURR_LINE	2

#define C64COLUMNS 40
#define C64ROWS 25
#define C64XRES 320
#define C64YRES 200
#define SPLITSTART	0x3e
#define SPLITEND	0x52
#define SPRITEWIDTH 48
#define SPRITEHEIGHT 2
#define BLOCKSIZE 8
#define FLI 0x18

#define COMB_I_S 0
#define COMB_I_I 1
#define COMB_I_P 2

#define COMB_P_I 8
#define COMB_P_P 9
#define COMB_P_S 10

#define COMB_S_I 16
#define COMB_S_P 17
#define COMB_S_S 18

#define BMP_SIGNATURE		0
#define BMP_FILE_SIZE		2
#define BMP_RESERVED1		6
#define BMP_RESERVED2		8
#define BMP_DATA_OFFSET		10
#define BMP_HEADER_SIZE		14
#define BMP_SIZEX		18
#define BMP_SIZEY		22
#define BMP_NUM_PLANES		26
#define BMP_BPP			28
#define BMP_COMPRESSION_TYPE	30
#define BMP_DATA_SIZE		34
#define BMP_DPI_X		38
#define BMP_DPI_Y		42
#define BMP_USED_COLORS		46
#define BMP_IMPORTANT_COLORS	50

#define INK		0
#define PAPER		1
#define SPRITE		2
#define SPRITEM1	3
#define SPRITEM2	4
#define SPRITEM3	5
#define FLISPRITE	6
#define FLISPRITEM1	7
#define FLISPRITEM2	8
#define FLISPRITEM3	9

#define TURQUISE	0
#define PURPLE		1
#define BLUE		2
#define RED		3
#define GREEN		4
#define GREY		5

#define OTYPE_BMP	0
#define OTYPE_NUFLI	1
#define OTYPE_MUIFLI	2
#define OTYPE_MUFFLON	3

#define ITYPE_BMP	0
#define ITYPE_IFLI	1
#define ITYPE_DRL	2

static const uint8_t switch_vals[]={0x70,0x50,0x60,0xe0};
static const uint8_t sprite_conf_alt[]={ 0x01, 0x0a, 0x00, 0x0b, 0x0c, 0x0b, 0x00, 0x00, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0x00 };
static const uint8_t sprite_conf[]={ 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0x00 };

static const uint8_t blanking_sprite[]={
	0xff,0x00,0x00,
	0xff,0xff,0xff,
	0xff,0xff,0xff,
	0xff,0xff,0xff,
	0xff,0x00,0x00,
	0xff,0xff,0xff,
	0xff,0xff,0xff,
	0xff,0xff,0xff,
	0xff,0x00,0x00,
	0xff,0xff,0xff,
	0xff,0xff,0xff,
	0xff,0xff,0xff,
	0xff,0x00,0x00,
	0xff,0xff,0xff,
	0xff,0xff,0xff,
	0xff,0xff,0xff,
	0xff,0x00,0x00,
	0xff,0xff,0xff,
	0xff,0xff,0xff,
	0xff,0xff,0xff,
	0xff,0x00,0x00
};

static const int combinations_muifli[] = {
	COMB_I_I, COMB_I_I,
	COMB_I_I, COMB_P_P,
	COMB_I_I, COMB_S_S,
	COMB_I_I, COMB_P_I,
	COMB_I_I, COMB_I_P,
	COMB_I_I, COMB_S_I,
	COMB_I_I, COMB_I_S,
	COMB_I_I, COMB_P_S,
	COMB_I_I, COMB_S_P,

	COMB_P_P, COMB_I_I,
	COMB_P_P, COMB_P_P,
	COMB_P_P, COMB_P_I,
	COMB_P_P, COMB_I_P,
	COMB_P_P, COMB_S_P,

	COMB_S_S, COMB_I_I,
	COMB_S_S, COMB_S_S,
	COMB_S_S, COMB_S_I,
	COMB_S_S, COMB_I_S,

	COMB_I_P, COMB_I_I,
	COMB_I_P, COMB_P_P,
	COMB_I_P, COMB_P_I,
	COMB_I_P, COMB_I_P,
	COMB_I_P, COMB_S_I,
	COMB_I_P, COMB_S_P,

	COMB_P_I, COMB_I_I,
	COMB_P_I, COMB_P_P,
	COMB_P_I, COMB_P_I,
	COMB_P_I, COMB_I_P,
	COMB_P_I, COMB_I_S,
	COMB_P_I, COMB_P_S,

	COMB_I_S, COMB_I_I,
	COMB_I_S, COMB_S_S,
	COMB_I_S, COMB_P_I,
	COMB_I_S, COMB_S_I,
	COMB_I_S, COMB_I_S,
	COMB_I_S, COMB_P_S,

	COMB_S_I, COMB_I_I,
	COMB_S_I, COMB_S_S,
	COMB_S_I, COMB_I_P,
	COMB_S_I, COMB_S_I,
	COMB_S_I, COMB_I_S,
	COMB_S_I, COMB_S_P,

	COMB_S_P, COMB_I_I,
	COMB_S_P, COMB_S_S,
	COMB_S_P, COMB_I_P,
	COMB_S_P, COMB_S_I,
	COMB_S_P, COMB_S_P,

	COMB_P_S, COMB_I_I,
	COMB_P_S, COMB_S_S,
	COMB_P_S, COMB_P_I,
	COMB_P_S, COMB_I_S,
	COMB_P_S, COMB_P_S,
};


static int combinations_normal[] = {
	INK    , INK   ,
	INK    , SPRITE,
	INK    , PAPER ,
	PAPER  , INK   ,
	PAPER  , PAPER ,
	SPRITE , SPRITE,
	SPRITE , INK   ,
};

static int combinations_flibug[] = {
	PAPER      , PAPER      ,
	PAPER      , FLISPRITE  ,
	FLISPRITE  , PAPER      ,
	FLISPRITEM1, FLISPRITEM1,
	FLISPRITEM1, FLISPRITE  ,
	FLISPRITEM2, FLISPRITEM2,
	FLISPRITEM2, FLISPRITE  ,
	FLISPRITEM3, FLISPRITEM3,
	FLISPRITEM3, FLISPRITE  ,
	FLISPRITE  , FLISPRITE  ,
	FLISPRITE  , FLISPRITEM1,
	FLISPRITE  , FLISPRITEM2,
	FLISPRITE  , FLISPRITEM3,
	INK        , PAPER      ,
	PAPER      , INK        ,
	FLISPRITEM3, INK        ,
	FLISPRITEM2, INK        ,
	FLISPRITEM1, INK        ,
	FLISPRITE  , INK        ,
	INK        , FLISPRITEM3,
	INK        , FLISPRITEM2,
	INK        , FLISPRITEM1,
	INK        , FLISPRITE  ,
	INK        , INK        ,
};


typedef struct MUFblock {
	int x;
	int y;
	int* colorspace;
	int ink[2];
	int pap[2];
	int sprite1[2];
	int sprite2[2];
} MUFblock;

static const char* write_mode="wb";
static const char* read_mode="rb";

typedef struct ColorSet {
	int ink1;
	int ink2;
	int pap1;
	int pap2;
	int spr[2][2];
	int spr1[2];
	int spr2[2];
} ColorSet;

//XXX auf 2 frames aufgepumpt
typedef struct PixelSet {
	int pixeltype[2];
	int pixelsize[2];
	//calc error als func
	int pixelcol[2];
} PixelSet;

typedef struct FLIBugBlock {
	int allowed_colors[16];
	int sprite;
	int paper[3];
	int ink[3];
	int dimx;	//24
	int dimy;	//1
	PixelSet pixel[12];
	//find_best pixelset als func
	//calc error als func dranhängen
} FLIBugBlock;

typedef struct FLIBugSet {
	int mul1;
	int mul2;
	FLIBugBlock block[100];
	//calc error als func dranhängen
} FLIBugSet;

typedef struct MufflonContext {
	int combinations[2][C64XRES*C64YRES];
	int* inks[2];
	int* papers[2];
	int* sprites[2];
	uint8_t* hires_bitmap[2];
	uint8_t* sprite_bitmap[2];
	uint8_t* colormap[2];
	uint8_t* sprite_col_tab[2];

	int fb_sprite_col[2][4][C64YRES+1];

	int fb_hsprite_bitmap[2][C64YRES*3+1];
	int fb_msprite_bitmap[2][C64YRES*3+1];

	/* 0..5 = free/not free, col 6 = sum of 0..5 */
	int free_switches[2][(C64YRES/2+1)][7];
	int free_switches_sum[2][(C64YRES/2+1)];
	int free_switches_flibug[2][(C64YRES/2+1)];
	int free_switches_temp[2][(C64YRES/2+1)];
	/* available free switches form y 0x3e-0x52 */
	int free_switches_max;
	int final_sprite_tab[2][(C64YRES/2+1)*6];
        int final_sprite_init[2][6];
	uint32_t diff_lut[C64XRES*C64YRES+1][16];

	uint8_t* source;
	uint8_t* data;
	uint8_t* error_map;
	uint8_t* flick_map;
	uint8_t* result_map;

	int* mixes_r;
	int* mixes_g;
	int* mixes_b;
	int* luma;

	int allowed_mix_map[16][16];
	int used_colors[200][40][16];

	int sizex;
	int sizey;
	/* size of spritelayer (sizex or 320-8 if sizex>320-8) */
	int sl_sizex;

	int bpp;

	char* path;
	char* read_name;
	char* base_name;
	char* result_map_name;
	char* error_map_name;
	char* flick_map_name;
	char* write_name;
	char switch_ypos[2][8];
	char switch_xpos[2][8];

	FILE* fdw;
	int itype;
	int otype;
	int mix_count;
	int secondpaper; //2papermode
	int secondink; //2paperink
	int secondsprite; //2spritemode

	char* option_src_pal_name;
	char* option_dest_pal_name;
	int option_bruteforce;
	int option_prepare;
	int option_deflicker;
	int option_rastered;
	int option_solid_only;
	int option_multipass;
	int option_no_truncate;
	int option_flibug;
	int option_block_spcol;
	int option_sbugcol;
	int option_shutup;
	double option_darkest;
	double option_brightest;
	int src_palette[16][3];
	int dest_palette[16][3];


} MufflonContext;

static const char header_funpaint[16] = {
	0xf0, 0x3f, 0x46, 0x55, 0x4E, 0x50, 0x41, 0x49, 0x4E, 0x54, 0x20, 0x28, 0x4D, 0x54, 0x29, 0x20
};

static const char header_gunpaint[16] = {
	0x47, 0x55, 0x4E, 0x50, 0x41, 0x49, 0x4E, 0x54, 0x20, 0x28, 0x4A, 0x5A, 0x29, 0x20, 0x20, 0x20
};

static const unsigned int patterns[4][2][8]= {
{
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0}
},
{
	{1,0,0,0,1,0,0,0},
	{0,0,1,0,0,0,1,0}
},
{
	{1,0,1,0,1,0,1,0},
	{0,1,0,1,0,1,0,1}
},
{
	{1,1,1,0,1,1,1,0},
	{1,0,1,1,1,0,1,1}
}
};

static const int flibug_hires_spram[100]={
	0x59c0, 0x5980, 0x5940, 0x5900, 0x58c0, 0x5880, 0x5840, 0x5800,
	0x59c0, 0x5980, 0x5940, 0x5900, 0x58c0, 0x5880, 0x5840, 0x5800,
	0x59c0, 0x5980, 0x5940, 0x5900, 0x58c0, 0x5880, 0x5840, 0x5800,
	0x59c0, 0x5980, 0x5940, 0x5900, 0x58c0, 0x5880, 0x5840, 0x5800,
	0x59c0, 0x5980, 0x5940, 0x5900, 0x58c0, 0x5880, 0x5840, 0x5800,
	0x59c0, 0x5980, 0x5940, 0x5900, 0x58c0, 0x5880, 0x5840, 0x5800,
	0x59c0, 0x5980, 0x5940, 0x5900, 0x58c0, 0x5880, 0x5840, 0x5800,
	0x59c0, 0x5980, 0x5940, 0x5900, 0x58c0, 0x5880, 0x5840, 0x5800,
	0x1200, 0x1240, 0x1280, 0x12c0,
	0x1200, 0x1240, 0x1280, 0x12c0,
	0x1200, 0x1240, 0x1280, 0x12c0,
	0x1200, 0x1240, 0x1280, 0x12c0,
	0x1200, 0x1240, 0x1280, 0x12c0,
	0x1240, 0x1280, 0x12c0, 0x1200,
	0x1240, 0x1280, 0x12c0, 0x1200,
	0x1240, 0x1280, 0x12c0, 0x1200,
	0x1240, 0x1280, 0x12c0, 0x1200
};

static const int flibug_multi_spram[100]={
	0x57c0, 0x5780, 0x5740, 0x5700, 0x56c0, 0x5680, 0x5640, 0x5600,
	0x57c0, 0x5780, 0x5740, 0x5700, 0x56c0, 0x5680, 0x5640, 0x5600,
	0x57c0, 0x5780, 0x5740, 0x5700, 0x56c0, 0x5680, 0x5640, 0x5600,
	0x57c0, 0x5780, 0x5740, 0x5700, 0x56c0, 0x5680, 0x5640, 0x5600,
	0x57c0, 0x5780, 0x5740, 0x5700, 0x56c0, 0x5680, 0x5640, 0x5600,
	0x57c0, 0x5780, 0x5740, 0x5700, 0x56c0, 0x5680, 0x5640, 0x5600,
	0x57c0, 0x5780, 0x5740, 0x5700, 0x56c0, 0x5680, 0x5640, 0x5600,
	0x57c0, 0x5780, 0x5740, 0x5700, 0x56c0, 0x5680, 0x5640, 0x5600,
	0x0000, 0x0040, 0x0080, 0x00c0,
	0x0000, 0x0040, 0x0080, 0x00c0,
	0x0000, 0x0040, 0x0080, 0x00c0,
	0x0000, 0x0040, 0x0080, 0x00c0,
	0x0000, 0x0040, 0x0080, 0x00c0,
	0x0040, 0x0080, 0x00c0, 0x0000,
	0x0040, 0x0080, 0x00c0, 0x0000,
	0x0040, 0x0080, 0x00c0, 0x0000,
	0x0040, 0x0080, 0x00c0, 0x0000
};

static const int nuifli_spram[100]={
	0x3e80, 0x3a80, 0x3680, 0x3280, 0x2e80, 0x2a80, 0x2680, 0x2280,
	0x3e80, 0x3a80, 0x3680, 0x3280, 0x2e80, 0x2a80, 0x2680, 0x2280,
	0x3e80, 0x3a80, 0x3680, 0x3280, 0x2e80, 0x2a80, 0x2680, 0x2280,
	0x3e80, 0x3a80, 0x3680, 0x3280, 0x2e80, 0x2a80, 0x2680, 0x2280,
	0x3e80, 0x3a80, 0x3680, 0x3280, 0x2e80, 0x2a80, 0x2680, 0x2280,
	0x3e80, 0x3a80, 0x3680, 0x3280, 0x2e80, 0x2a80, 0x2680, 0x2280,
	0x3e80, 0x3a80, 0x3680, 0x3280, 0x2e80, 0x2a80, 0x2680, 0x2280,
	0x3e80, 0x3a80, 0x3680, 0x3280, 0x2e80, 0x2a80, 0x2680, 0x2280,
	0x0100, 0x0500, 0x0900, 0x0d00,
	0x0100, 0x0500, 0x0900, 0x0d00,
	0x0100, 0x0500, 0x0900, 0x0d00,
	0x0100, 0x0500, 0x0900, 0x0d00,
	0x0100, 0x0500, 0x0900, 0x0d00,
	0x0500, 0x0900, 0x0d00, 0x0100,
	0x0500, 0x0900, 0x0d00, 0x0100,
	0x0500, 0x0900, 0x0d00, 0x0100,
	0x0500, 0x0900, 0x0d00, 0x0100
};

static const int nuifli_scram[100]={
	0x3c00, 0x3800, 0x3400, 0x3000, 0x2c28, 0x2828, 0x2428, 0x2028,
	0x3c50, 0x3850, 0x3450, 0x3050, 0x2c78, 0x2878, 0x2478, 0x2078,
	0x3ca0, 0x38a0, 0x34a0, 0x30a0, 0x2cc8, 0x28c8, 0x24c8, 0x20c8,
	0x3cf0, 0x38f0, 0x34f0, 0x30f0, 0x2d18, 0x2918, 0x2518, 0x2118,
	0x3d40, 0x3940, 0x3540, 0x3140, 0x2d68, 0x2968, 0x2568, 0x2168,
	0x3d90, 0x3990, 0x3590, 0x3190, 0x2db8, 0x29b8, 0x25b8, 0x21b8,
	0x3de0, 0x39e0, 0x35e0, 0x31e0, 0x2e08, 0x2a08, 0x2608, 0x2208,
	0x3e30, 0x3a30, 0x3630, 0x3230, 0x2e58, 0x2a58, 0x2658, 0x2258,
	0x0280, 0x0680, 0x0a80, 0x0e80,
	0x02a8, 0x06a8, 0x0aa8, 0x0ea8,
	0x02d0, 0x06d0, 0x0ad0, 0x0ed0,
	0x02f8, 0x06f8, 0x0af8, 0x0ef8,
	0x0320, 0x0720, 0x0b20, 0x0f20,
	0x0748, 0x0b48, 0x0f48, 0x0348,
	0x0770, 0x0b70, 0x0f70, 0x0370,
	0x0798, 0x0b98, 0x0f98, 0x0398,
	0x07c0, 0x0bc0, 0x0fc0, 0x03c0
};

static const int muifli_spram[100]={
	0x3d80, 0x3980, 0x3580, 0x3180, 0x2d80, 0x2980, 0x2580, 0x2180,
	0x3d80, 0x3980, 0x3580, 0x3180, 0x2d80, 0x2980, 0x2580, 0x2180,
	0x3d80, 0x3980, 0x3580, 0x3180, 0x2d80, 0x2980, 0x2580, 0x2180,
	0x3d80, 0x3980, 0x3580, 0x3180, 0x2d80, 0x2980, 0x2580, 0x2180,
	0x3d80, 0x3980, 0x3580, 0x3180, 0x2d80, 0x2980, 0x2580, 0x2180,
	0x3d80, 0x3980, 0x3580, 0x3180, 0x2d80, 0x2980, 0x2580, 0x2180,
	0x3d80, 0x3980, 0x3580, 0x3180, 0x2d80, 0x2980, 0x2580, 0x2180,
	0x3d80, 0x3980, 0x3580, 0x3180, 0x2d80, 0x2980, 0x2580, 0x2180,
	0x0000, 0x0400, 0x0800, 0x0c00,
	0x0000, 0x0400, 0x0800, 0x0c00,
	0x0000, 0x0400, 0x0800, 0x0c00,
	0x0000, 0x0400, 0x0800, 0x0c00,
	0x0000, 0x0400, 0x0800, 0x0c00,
	0x0400, 0x0800, 0x0c00, 0x0000,
	0x0400, 0x0800, 0x0c00, 0x0000,
	0x0400, 0x0800, 0x0c00, 0x0000,
	0x0400, 0x0800, 0x0c00, 0x0000
};

static const int muifli_scram[100]={
	0x3b00, 0x3700, 0x3300, 0x2f00, 0x2b28, 0x2728, 0x2328, 0x1f28,
	0x3b50, 0x3750, 0x3350, 0x2f50, 0x2b78, 0x2778, 0x2378, 0x1f78,
	0x3ba0, 0x37a0, 0x33a0, 0x2fa0, 0x2bc8, 0x27c8, 0x23c8, 0x1fc8,
	0x3bf0, 0x37f0, 0x33f0, 0x2ff0, 0x2c18, 0x2818, 0x2418, 0x2018,
	0x3c40, 0x3840, 0x3440, 0x3040, 0x2c68, 0x2868, 0x2468, 0x2068,
	0x3c90, 0x3890, 0x3490, 0x3090, 0x2cb8, 0x28b8, 0x24b8, 0x20b8,
	0x3ce0, 0x38e0, 0x34e0, 0x30e0, 0x2d08, 0x2908, 0x2508, 0x2108,
	0x3d30, 0x3930, 0x3530, 0x3130, 0x2d58, 0x2958, 0x2558, 0x2158,
	0x0180, 0x0580, 0x0980, 0x0d80,
	0x01a8, 0x05a8, 0x09a8, 0x0da8,
	0x01d0, 0x05d0, 0x09d0, 0x0dd0,
	0x01f8, 0x05f8, 0x09f8, 0x0df8,
	0x0220, 0x0620, 0x0a20, 0x0e20,
	0x0648, 0x0a48, 0x0e48, 0x0248,
	0x0670, 0x0a70, 0x0e70, 0x0270,
	0x0698, 0x0a98, 0x0e98, 0x0298,
	0x06c0, 0x0ac0, 0x0ec0, 0x02c0
};

static const uint8_t nuifli_sprite_pointers[12][8]={
	{ 0xc8,0x84,0x85,0x86,0x87,0x88,0x89,0x80 },
	{ 0xc9,0x94,0x95,0x96,0x97,0x98,0x99,0x81 },
	{ 0xca,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0x82 },
	{ 0xcb,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0x83 },
	{ 0xe0,0x0a,0x0b,0x0c,0x0d,0x0e,0xd0,0xd8 },
	{ 0xe1,0x1a,0x1b,0x1c,0x1d,0x1e,0xd1,0xd9 },
	{ 0xe2,0x2a,0x2b,0x2c,0x2d,0x2e,0xd2,0xda },
	{ 0xe3,0x3a,0x3b,0x3c,0x3d,0x3e,0xd3,0xdb },
	{ 0xe4,0x4a,0x4b,0x4c,0x4d,0x4e,0xd4,0xdc },
	{ 0xe5,0x5a,0x5b,0x5c,0x5d,0x5e,0xd5,0xdd },
	{ 0xe6,0x6a,0x6b,0x6c,0x6d,0x6e,0xd6,0xde },
	{ 0xe7,0x7a,0x7b,0x7c,0x7d,0x7e,0xd7,0xdf }
};

static const uint8_t muifli_sprite_pointers[12][8]={
	{ 0xfd,0x84,0x85,0x86,0x87,0x88,0x89,0x00 },
	{ 0xfd,0x94,0x95,0x96,0x97,0x98,0x99,0x00 },
	{ 0xfd,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0x00 },
	{ 0xfd,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0x00 },
	{ 0xd8,0x0a,0x0b,0x0c,0x0d,0x0e,0xd0,0x00 },
	{ 0xd8,0x1a,0x1b,0x1c,0x1d,0x1e,0xd1,0x00 },
	{ 0xd8,0x2a,0x2b,0x2c,0x2d,0x2e,0xd2,0x00 },
	{ 0xd8,0x3a,0x3b,0x3c,0x3d,0x3e,0xd3,0x00 },
	{ 0xd8,0x4a,0x4b,0x4c,0x4d,0x4e,0xd4,0x00 },
	{ 0xd8,0x5a,0x5b,0x5c,0x5d,0x5e,0xd5,0x00 },
	{ 0xd8,0x6a,0x6b,0x6c,0x6d,0x6e,0xd6,0x00 },
	{ 0xd8,0x7a,0x7b,0x7c,0x7d,0x7e,0xd7,0x00 }
};

/* 6502 muifli displayer code */

/* $3000 */
static const uint8_t nuifli_displayer_code1[] = {
	0x78, 0xA9, 0x01, 0x8D, 0x0D, 0xDC, 0xA9, 0x10, 0x8D, 0x14, 0x03, 0xA9, 0x31, 0x8D, 0x15, 0x03, 0xA2, 0x2F, 0xBD, 0xC0, 0x3F, 0x9D, 0x00, 0xD0, 0xCA, 0x10, 0xF7, 0xA9, 0x00, 0x85, 0xFB, 0xA9,
	0x10, 0x85, 0xFC, 0xA2, 0x00, 0x8A, 0x29, 0x03, 0xA8, 0xB9, 0x48, 0x33, 0x8D, 0x50, 0x30, 0xB9,	0x44, 0x33, 0x8D, 0x3D, 0x30, 0xB9, 0x40, 0x33, 0x8D, 0xA4, 0x30, 0xA8, 0xB9, 0x00, 0x33, 0x99,
	0x00, 0x33, 0xC8, 0xC0, 0x07, 0xD0, 0xF5, 0xBD, 0x01, 0x24, 0xA0, 0x28, 0x20, 0xB0, 0x33, 0x8D,	0x00, 0x33, 0x8C, 0x08, 0x33, 0xBD, 0x81, 0x24, 0xA0, 0x29, 0x20, 0xB0, 0x33, 0x8D, 0x0B, 0x33,
	0x8C, 0x0D, 0x33, 0xBD, 0x01, 0x28, 0xA0, 0x2A, 0x20, 0xB0, 0x33, 0x8D, 0x10, 0x33, 0x8C, 0x12,	0x33, 0xBD, 0x81, 0x28, 0xA0, 0x2B, 0x20, 0xB0, 0x33, 0x8D, 0x15, 0x33, 0x8C, 0x17, 0x33, 0xBD,
	0x01, 0x2C, 0xA0, 0x2C, 0x20, 0xB0, 0x33, 0x8D, 0x1A, 0x33, 0x8C, 0x1C, 0x33, 0xBD, 0x81, 0x2C,	0xA0, 0x2D, 0x20, 0xB0, 0x33, 0x8D, 0x1F, 0x33, 0x8C, 0x21, 0x33, 0xBD, 0x4C, 0x33, 0x8D, 0x24,
	0x33, 0x86, 0x02, 0xA2, 0x00, 0xA0, 0x00, 0xBD, 0x00, 0x33, 0x91, 0xFB, 0xC8, 0xE8, 0xE0, 0x28,	0xD0, 0xF5, 0x98, 0x18, 0x65, 0xFB, 0x90, 0x02, 0xE6, 0xFC, 0x85, 0xFB, 0xA6, 0x02, 0xE8, 0xE0,
	0x64, 0xF0, 0x03, 0x4C, 0x25, 0x30, 0xA0, 0x05, 0xB9, 0xD0, 0x33, 0x91, 0xFB, 0x88, 0x10, 0xF8,	0xA9, 0x00, 0x8D, 0xBE, 0x19, 0xA9, 0xDD, 0x8D, 0xBF, 0x19, 0xA9, 0x3A, 0x85, 0x3A, 0xA9, 0xA0,
	0x8D, 0x26, 0x10, 0xA2, 0x27, 0xBD, 0x80, 0x22, 0x9D, 0x80, 0x02, 0xBD, 0xD8, 0x73, 0x9D, 0xD8,	0x33, 0xCA, 0x10, 0xF1, 0xA2, 0x07, 0xBD, 0xF8, 0x23, 0x9D, 0xF8, 0x03, 0xA9, 0x00, 0x9D, 0xF8,
	0x7F, 0xA9, 0xFE, 0x9D, 0xF8, 0x07, 0xCA, 0x10, 0xED, 0xAD, 0x0D, 0xDC, 0x58, 0x4C, 0x0D, 0x31,	0xA9, 0x2A, 0x8D, 0x12, 0xD0, 0xA9, 0x30, 0x8D, 0x14, 0x03, 0xEE, 0x19, 0xD0, 0x58, 0xEA, 0xEA,
	0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0x4C, 0x1E, 0x31,	0xBA, 0x8A, 0x18, 0x69, 0x06, 0xAA, 0x9A, 0xA0, 0xFF, 0xB9, 0x01, 0xDC, 0x29, 0xFC, 0xAE, 0x12,
	0xD0, 0xE0, 0x2A, 0xF0, 0x00, 0x8C, 0x18, 0xD0, 0x09, 0x02, 0x8D, 0x00, 0xDD, 0xA2, 0x2B, 0x8E,	0x01, 0xD0, 0x8E, 0x03, 0xD0, 0x8E, 0x05, 0xD0, 0x8E, 0x07, 0xD0, 0x8E, 0x09, 0xD0, 0x8E, 0x0B,
	0xD0, 0x8E, 0x0D, 0xD0, 0x8E, 0x0F, 0xD0, 0xAE, 0x00, 0x24, 0x8E, 0x28, 0xD0, 0xAE, 0x80, 0x24,	0x8E, 0x29, 0xD0, 0xAE, 0x00, 0x28, 0x8E, 0x2A, 0xD0, 0xAE, 0x80, 0x28, 0x8E, 0x2B, 0xD0, 0xAE,
	0x00, 0x2C, 0x8E, 0x2C, 0xD0, 0xAE, 0x80, 0x2C, 0x8E, 0x2D, 0xD0, 0xA2, 0xAA, 0x8E, 0x01, 0xD0,	0x8E, 0x03, 0xD0, 0x8E, 0x05, 0xD0, 0x8E, 0x07, 0xD0, 0x8E, 0x09, 0xD0, 0x8E, 0x0B, 0xD0, 0x8E,
	0x0D, 0xD0, 0x8E, 0x0F, 0xD0, 0x09, 0x01, 0x8D, 0xBC, 0x19, 0xA9, 0x00, 0x8C, 0x17, 0xD0, 0x99,	0x18, 0xD0, 0x8C, 0x17, 0xD0, 0xA0, 0x05, 0x88, 0xD0, 0xFD, 0xAD, 0xF0, 0x3F, 0x8D, 0x26, 0xD0,
	0xAD, 0xF1, 0x3F, 0x8D, 0x25, 0xD0, 0xAD, 0xF7, 0x3F, 0x8D, 0x27, 0xD0, 0xAD, 0xF6, 0x3F, 0x8D,	0x2E, 0xD0, 0x24, 0x00, 0xA9, 0x38, 0xA0, 0x78, 0x8C, 0x18, 0xD0, 0xA2, 0x3E, 0x20, 0x00, 0x10,
	0xA9, 0x00, 0x8D, 0x17, 0xD0, 0xA9, 0x29, 0x8D, 0x12, 0xD0, 0xA9, 0x10, 0x8D, 0x14, 0x03, 0xEE,	0x19, 0xD0, 0x4C, 0x81, 0xEA
};

/* $3300 */
static const uint8_t nuifli_displayer_code2[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8C, 0x28, 0xD0, 0xA0, 0x00, 0x8C, 0x29, 0xD0, 0xA0,	0x00, 0x8C, 0x2A, 0xD0, 0xA0, 0x00, 0x8C, 0x2B, 0xD0, 0xA0, 0x00, 0x8C, 0x2C, 0xD0, 0xA0, 0x00,
	0x8C, 0x2D, 0xD0, 0xA0, 0x00, 0x8C, 0x18, 0xD0, 0xA4, 0x3A, 0x8C, 0x11, 0xD0, 0xA0, 0x00, 0xA0,	0x3C, 0x8C, 0x11, 0xD0, 0xA0, 0x00, 0xA0, 0x00, 0x8E, 0x11, 0xD0, 0x8D, 0x11, 0xD0, 0xA0, 0x00,
	0x02, 0x00, 0x00, 0x02, 0x39, 0x28, 0x2F, 0x34, 0x06, 0x06, 0x06, 0x03, 0x68, 0x58, 0x48, 0x38,	0x28, 0x18, 0x08, 0x78, 0x68, 0x58, 0x48, 0x38, 0x28, 0x18, 0x08, 0x78, 0x68, 0x58, 0x48, 0x38,
	0x28, 0x18, 0x08, 0x78, 0x68, 0x58, 0x48, 0x38, 0x28, 0x18, 0x08, 0x78, 0x68, 0x58, 0x48, 0x38,	0x28, 0x18, 0x08, 0x78, 0x68, 0x58, 0x48, 0x38, 0x28, 0x18, 0x08, 0x78, 0x68, 0x58, 0x48, 0x38,
	0x28, 0x18, 0x08, 0x78, 0x68, 0x58, 0x48, 0x38, 0x28, 0x18, 0x08, 0x03, 0x98, 0xA8, 0xB8, 0x88,	0x98, 0xA8, 0xB8, 0x88, 0x98, 0xA8, 0xB8, 0x88, 0x98, 0xA8, 0xB8, 0x88, 0x98, 0xA8, 0xB8, 0x98,
	0xA8, 0xB8, 0x88, 0x98, 0xA8, 0xB8, 0x88, 0x98, 0xA8, 0xB8, 0x88, 0x98, 0xA8, 0xB8, 0x88, 0x18,	0x85, 0x03, 0x4A, 0x4A, 0x4A, 0x4A, 0xF0, 0x15, 0xC9, 0x01, 0xD0, 0x08, 0xA5, 0x03, 0x29, 0x0F,
	0xA8, 0xA9, 0xD4, 0x60, 0xC9, 0x02, 0xD0, 0x02, 0xA9, 0x00, 0x09, 0x20, 0xA8, 0xA5, 0x03, 0x60,	0xA0, 0x10, 0x8C, 0x11, 0xD0, 0x60
};

static const uint8_t nuifli_displayer_code3[] = {
	0x18, 0x2B, 0x30, 0x2B, 0x60, 0x2B, 0x90, 0x2B, 0xC0, 0x2B, 0xF0, 0x2B, 0x20, 0x2B, 0x18, 0x2B,	0x40, 0x78, 0x29, 0x00, 0x00, 0xFF, 0x08, 0x00, 0x14, 0x00, 0x01, 0xFF, 0x80, 0x7E, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const uint8_t muifli_displayer_code1[] = {
/*3000*/	0x78, 0xA9, 0x34, 0x85, 0x01, 0x20, 0x00, 0xB0, 0x2C, 0x00, 0x00, 0xA9, 0x32, 0x8D, 0x15, 0x03, 0xA2, 0x2F, 0xBD, 0xD0, 0x31, 0x9D, 0x00, 0xD0, 0xCA, 0x10, 0xF7, 0xA9, 0xDA, 0x85, 0xFB, 0xA9,
/*3020*/	0x0F, 0x85, 0xFC, 0xA2, 0x00, 0x8A, 0x29, 0x03, 0xA8, 0xB9, 0x3C, 0x31, 0x8D, 0x32, 0x30, 0xA0, 0x26, 0xB9, 0x0A, 0x31, 0x91, 0xFB, 0xC8, 0xC9, 0xD0, 0xD0, 0xF6, 0x98, 0x18, 0x65, 0xFB, 0x90,
/*3040*/	0x02, 0xE6, 0xFC, 0x85, 0xFB, 0xBD, 0x01, 0x33, 0x8D, 0x01, 0x31, 0xBD, 0x01, 0x24, 0x8D, 0x06, 0x31, 0xBD, 0x81, 0x24, 0x8D, 0x0B, 0x31, 0xBD, 0x01, 0x28, 0x8D, 0x10, 0x31, 0xBD, 0x81, 0x28,
/*3060*/	0x8D, 0x15, 0x31, 0xBD, 0x01, 0x2C, 0x8D, 0x1A, 0x31, 0xA0, 0x8D, 0xBD, 0x81, 0x2C, 0xC9, 0x08, 0xB0, 0x02, 0xA0, 0x8F, 0x1D, 0x41, 0x31, 0x8D, 0x1F, 0x31, 0x8C, 0x20, 0x31, 0xA0, 0x00, 0xB9,
/*3080*/	0x00, 0x31, 0x91, 0xFB, 0xC8, 0xC0, 0x26, 0xD0, 0xF6, 0x86, 0x02, 0xA2, 0x06, 0xA5, 0x02, 0xDD, 0xE0, 0x4F, 0xD0, 0x0E, 0xBC, 0xE8, 0x4F, 0xA9, 0xD4, 0x91, 0xFB, 0xC8, 0xC8, 0x8A, 0x38, 0x2A,
/*30a0*/	0x91, 0xFB, 0xCA, 0x10, 0xE8, 0xA6, 0x02, 0xE8, 0xE0, 0x64, 0xF0, 0x03, 0x4C, 0x25, 0x30, 0xB9, 0x0F, 0x31, 0x91, 0xFB, 0xC8, 0xC0, 0x2C, 0xD0, 0xF6, 0xA9, 0xEE, 0x8D, 0x9D, 0x1A, 0xA9, 0x00,
/*30c0*/	0x8D, 0x9E, 0x1A, 0xA9, 0xDD, 0x8D, 0x9F, 0x1A, 0xA9, 0xF8, 0x85, 0x02, 0xA2, 0x27, 0xBD, 0x80, 0x22, 0x9D, 0x80, 0x02, 0xCA, 0x10, 0xF7, 0xA2, 0x07, 0xBD, 0xF8, 0x23, 0x9D, 0xF8, 0x03, 0xA9,
/*30e0*/	0x00, 0x9D, 0xF8, 0x7F, 0xCA, 0x10, 0xF2, 0xA9, 0x00, 0x8D, 0xFF, 0x7F, 0xEE, 0x19, 0xD0, 0xAD, 0x0D, 0xDC, 0x58, 0x4C, 0xF3, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*3100*/	0xA9, 0x00, 0x8D, 0x1C, 0xD0, 0xA9, 0x00, 0x8D, 0x28, 0xD0, 0xA9, 0x00, 0x8D, 0x29, 0xD0, 0xA9, 0x00, 0x8D, 0x2A, 0xD0, 0xA9, 0x00, 0x8D, 0x2B, 0xD0, 0xA9, 0x00, 0x8D, 0x2C, 0xD0, 0xA9, 0x00,
/*3120*/	0x8D, 0x2D, 0xD0, 0x8D, 0x18, 0xD0, 0xA9, 0x3A, 0x8D, 0x11, 0xD0, 0xA9, 0x3C, 0x8D, 0x11, 0xD0, 0xA9, 0x3E, 0x8D, 0x11, 0xD0, 0xA0, 0x10, 0x8C, 0x11, 0xD0, 0x60, 0x00, 0x11, 0x00, 0x05, 0x0A,
/*3140*/	0x78, 0x68, 0x58, 0x48, 0x38, 0x28, 0x18, 0x08, 0x78, 0x68, 0x58, 0x48, 0x38, 0x28, 0x18, 0x08, 0x78, 0x68, 0x58, 0x48, 0x38, 0x28, 0x18, 0x08, 0x78, 0x68, 0x58, 0x48, 0x38, 0x28, 0x18, 0x08,
/*3160*/	0x78, 0x68, 0x58, 0x48, 0x38, 0x28, 0x18, 0x08, 0x78, 0x68, 0x58, 0x48, 0x38, 0x28, 0x18, 0x08, 0x78, 0x68, 0x58, 0x48, 0x38, 0x28, 0x18, 0x08, 0x78, 0x68, 0x58, 0x48, 0x38, 0x28, 0x18, 0x08,
/*3180*/	0x88, 0x98, 0xA8, 0xB8, 0x88, 0x98, 0xA8, 0xB8, 0x88, 0x98, 0xA8, 0xB8, 0x88, 0x98, 0xA8, 0xB8, 0x88, 0x98, 0xA8, 0xB8, 0x98, 0xA8, 0xB8, 0x88, 0x98, 0xA8, 0xB8, 0x88, 0x98, 0xA8, 0xB8, 0x88,
/*31a0*/	0x98, 0xA8, 0xB8, 0x88, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x3E, 0x3E, 0x3D, 0x3D, 0x3D, 0x3D, 0x00, 0x1A, 0x10, 0x06, 0x1A, 0x15, 0x0B, 0x06, 0x00,
/*31c0*/	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x2B, 0x31, 0x2B, 0x61, 0x2B, 0x91, 0x2B, 0xC1, 0x2B, 0xF1, 0x2B, 0x21, 0x2B, 0x01, 0x2B,
/*31e0*/	0x40, 0x78, 0x29, 0x00, 0x00, 0x7F, 0x09, 0x00, 0x14, 0x00, 0x01, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*3200*/	0xA9, 0x2A, 0x8D, 0x12, 0xD0, 0xA9, 0x20, 0x8D, 0x14, 0x03, 0xEE, 0x19, 0xD0, 0x58, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0x4C, 0x0E, 0x32,
/*3220*/	0xBA, 0x8A, 0x18, 0x69, 0x06, 0xAA, 0x9A, 0xA6, 0x02, 0xA9, 0x02, 0x8D, 0x00, 0xDD, 0xAD, 0x12, 0xD0, 0xC9, 0x2A, 0xF0, 0x00, 0x8E, 0x18, 0xD0, 0xA9, 0x2B, 0x8D, 0x01, 0xD0, 0x8D, 0x03, 0xD0,
/*3240*/	0x8D, 0x05, 0xD0, 0x8D, 0x07, 0xD0, 0x8D, 0x09, 0xD0, 0x8D, 0x0B, 0xD0, 0x8D, 0x0D, 0xD0, 0xAD, 0x00, 0x24, 0x8D, 0x28, 0xD0, 0xAD, 0x80, 0x24, 0x8D, 0x29, 0xD0, 0xAD, 0x00, 0x28, 0x8D, 0x2A,
/*3260*/	0xD0, 0xAD, 0x80, 0x28, 0x8D, 0x2B, 0xD0, 0xAD, 0x00, 0x2C, 0x8D, 0x2C, 0xD0, 0xAD, 0x80, 0x2C, 0x8D, 0x2D, 0xD0, 0xA9, 0xAA, 0x8D, 0x01, 0xD0, 0x8D, 0x03, 0xD0, 0x8D, 0x05, 0xD0, 0x8D, 0x07,
/*3280*/	0xD0, 0x8D, 0x09, 0xD0, 0x8D, 0x0B, 0xD0, 0x8D, 0x0D, 0xD0, 0xAD, 0x00, 0x33, 0x8D, 0x1C, 0xD0, 0xA2, 0xF7, 0xA9, 0x29, 0x8D, 0x12, 0xD0, 0xA9, 0x00, 0x8D, 0x14, 0x03, 0xA0, 0xFF, 0x8C, 0x17,
/*32a0*/	0xD0, 0x24, 0x00, 0xA9, 0x00, 0x8D, 0x17, 0xD0, 0x8C, 0x17, 0xD0, 0xA0, 0x09, 0x88, 0xD0, 0xFD, 0xAD, 0xF0, 0x3F, 0x8D, 0x26, 0xD0, 0xAD, 0xF1, 0x3F, 0x8D, 0x25, 0xD0, 0xEA, 0xEA, 0xA9, 0x78,
/*32c0*/	0x8D, 0x18, 0xD0, 0xA0, 0x38, 0x20, 0x00, 0x10, 0xA9, 0x00, 0x8D, 0x17, 0xD0, 0xA9, 0xB2, 0x8D, 0x15, 0x03, 0xEE, 0x19, 0xD0, 0x4C, 0x81, 0xEA, 0x20, 0xE0, 0x79, 0x4C, 0x00, 0x90, 0x00, 0x00,
/*32e0*/	0x78, 0xA9, 0x34, 0x85, 0x01, 0xA9, 0x21, 0xA0, 0xA1, 0xA2, 0x0F, 0x20, 0xE0, 0x0E, 0xA9, 0x33, 0xA0, 0xB3, 0xA2, 0x43, 0x20, 0xE0, 0x0E, 0xA9, 0x36, 0x85, 0x01, 0x58, 0x60, 0x00, 0x00, 0x00,
};

static const uint8_t muifli_displayer_code2[] = {
/*b000*/	0xA2, 0x2A, 0xA0, 0x00, 0xB9, 0x00, 0x77, 0x99, 0x00, 0xCD, 0xC8, 0xD0, 0xF7, 0xEE, 0x06, 0xB0, 0xEE, 0x09, 0xB0, 0xCA, 0xD0, 0xEE, 0xA9, 0x00, 0x8D, 0x14, 0x03, 0xA9, 0xDA, 0x85, 0xFB, 0xA9,
/*b020*/	0x8F, 0x85, 0xFC, 0xA2, 0x00, 0x8A, 0x29, 0x03, 0xA8, 0xB9, 0x3C, 0x31, 0x8D, 0x32, 0xB0, 0xA0, 0x26, 0xB9, 0x0A, 0x31, 0x91, 0xFB, 0xC8, 0xC9, 0xD0, 0xD0, 0xF6, 0x98, 0x18, 0x65, 0xFB, 0x90,
/*b040*/	0x02, 0xE6, 0xFC, 0x85, 0xFB, 0xBD, 0x01, 0xB3, 0x8D, 0x01, 0x31, 0xBD, 0x01, 0xA4, 0x8D, 0x06, 0x31, 0xBD, 0x81, 0xA4, 0x8D, 0x0B, 0x31, 0xBD, 0x01, 0xA8, 0x8D, 0x10, 0x31, 0xBD, 0x81, 0xA8,
/*b060*/	0x8D, 0x15, 0x31, 0xBD, 0x01, 0xAC, 0x8D, 0x1A, 0x31, 0xA0, 0x8D, 0xBD, 0x81, 0xAC, 0xC9, 0x08, 0xB0, 0x02, 0xA0, 0x8F, 0x1D, 0x41, 0x31, 0x8D, 0x1F, 0x31, 0x8C, 0x20, 0x31, 0xA0, 0x00, 0xB9,
/*b080*/	0x00, 0x31, 0x91, 0xFB, 0xC8, 0xC0, 0x26, 0xD0, 0xF6, 0x86, 0x02, 0xA2, 0x06, 0xA5, 0x02, 0xDD, 0xE0, 0xCF, 0xD0, 0x0E, 0xBC, 0xE8, 0xCF, 0xA9, 0xD4, 0x91, 0xFB, 0xC8, 0xC8, 0x8A, 0x38, 0x2A,
/*b0a0*/	0x91, 0xFB, 0xCA, 0x10, 0xE8, 0xA6, 0x02, 0xE8, 0xE0, 0x64, 0xF0, 0x03, 0x4C, 0x25, 0xB0, 0xB9, 0x0F, 0x31, 0x91, 0xFB, 0xC8, 0xC0, 0x2C, 0xD0, 0xF6, 0xA9, 0xEE, 0x8D, 0x9D, 0x9A, 0xA9, 0x00,
/*b0c0*/	0x8D, 0x9E, 0x9A, 0xA9, 0xDD, 0x8D, 0x9F, 0x9A, 0xA9, 0xF8, 0x85, 0x02, 0xA2, 0x27, 0xBD, 0x80, 0xA2, 0x9D, 0x80, 0x82, 0xCA, 0x10, 0xF7, 0xA2, 0x07, 0xBD, 0xF8, 0xA3, 0x9D, 0xF8, 0x83, 0xA9,
/*b0e0*/	0x00, 0x9D, 0xF8, 0xFF, 0xCA, 0x10, 0xF2, 0xA9, 0x00, 0x8D, 0xFF, 0xFF, 0xA9, 0x36, 0x85, 0x01, 0xA9, 0x01, 0x8D, 0x0D, 0xDC, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*b100*/	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*b120*/	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*b140*/	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*b160*/	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*b180*/	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*b1a0*/	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*b1c0*/	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*b1e0*/	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*b200*/	0xA9, 0x2A, 0x8D, 0x12, 0xD0, 0xA9, 0x20, 0x8D, 0x14, 0x03, 0xEE, 0x19, 0xD0, 0x58, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0x4C, 0x0E, 0xB2,
/*b220*/	0xBA, 0x8A, 0x18, 0x69, 0x06, 0xAA, 0x9A, 0xA6, 0x02, 0xA9, 0x00, 0x8D, 0x00, 0xDD, 0xAD, 0x12, 0xD0, 0xC9, 0x2A, 0xF0, 0x00, 0x8E, 0x18, 0xD0, 0xA9, 0x2B, 0x8D, 0x01, 0xD0, 0x8D, 0x03, 0xD0,
/*b240*/	0x8D, 0x05, 0xD0, 0x8D, 0x07, 0xD0, 0x8D, 0x09, 0xD0, 0x8D, 0x0B, 0xD0, 0x8D, 0x0D, 0xD0, 0xAD, 0x00, 0xA4, 0x8D, 0x28, 0xD0, 0xAD, 0x80, 0xA4, 0x8D, 0x29, 0xD0, 0xAD, 0x00, 0xA8, 0x8D, 0x2A,
/*b260*/	0xD0, 0xAD, 0x80, 0xA8, 0x8D, 0x2B, 0xD0, 0xAD, 0x00, 0xAC, 0x8D, 0x2C, 0xD0, 0xAD, 0x80, 0xAC, 0x8D, 0x2D, 0xD0, 0xA9, 0xAA, 0x8D, 0x01, 0xD0, 0x8D, 0x03, 0xD0, 0x8D, 0x05, 0xD0, 0x8D, 0x07,
/*b280*/	0xD0, 0x8D, 0x09, 0xD0, 0x8D, 0x0B, 0xD0, 0x8D, 0x0D, 0xD0, 0xAD, 0x00, 0xB3, 0x8D, 0x1C, 0xD0, 0xA2, 0xF7, 0xA9, 0x29, 0x8D, 0x12, 0xD0, 0xA9, 0x00, 0x8D, 0x14, 0x03, 0xA0, 0xFF, 0x8C, 0x17,
/*b2a0*/	0xD0, 0x24, 0x00, 0xA9, 0x00, 0x8D, 0x17, 0xD0, 0x8C, 0x17, 0xD0, 0xA0, 0x09, 0x88, 0xD0, 0xFD, 0xAD, 0xF0, 0xBF, 0x8D, 0x26, 0xD0, 0xAD, 0xF1, 0xBF, 0x8D, 0x25, 0xD0, 0xEA, 0xEA, 0xA9, 0x78,
/*b2c0*/	0x8D, 0x18, 0xD0, 0xA0, 0x38, 0x20, 0x00, 0x90, 0xA9, 0x00, 0x8D, 0x17, 0xD0, 0xA9, 0x32, 0x8D, 0x15, 0x03, 0xEE, 0x19, 0xD0, 0x4C, 0x81, 0xEA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*b2e0*/	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static uint8_t cl_solid_cyan[]={
	0x0,0x0,
	0x6,0x6,
	0xe,0xe,
	0x5,0x5,
	0x3,0x3,
	0xd,0xd,
	0x1,0x1
};

static uint8_t cl_solid_purple[]={
	0x0,0x0,
	0x6,0x6,
	0x4,0x4,
	0xe,0xe,
	0x3,0x3,
	0xd,0xd,
	0x1,0x1
};

static uint8_t cl_solid_blue[]={
	0x0,0x0,
	0x6,0x6,
	0x4,0x4,
	0xe,0xe,
	0xf,0xf,
	0xd,0xd,
	0x1,0x1
};

static uint8_t cl_solid_yellow[]={
	0x0,0x0,
	0x9,0x9,
	0x8,0x8,
	0xa,0xa,
	0xf,0xf,
	0x7,0x7,
	0x1,0x1
};

static uint8_t cl_solid_red[]={
	0x0,0x0,
	0x2,0x2,
	0x8,0x8,
	0xa,0xa,
	0xf,0xf,
	0x7,0x7,
	0x1,0x1
};

static uint8_t cl_solid_green[]={
	0x0,0x0,
	0xb,0xb,
	0xc,0xc,
	0x5,0x5,
	0xf,0xf,
	0xd,0xd,
	0x1,0x1
};

static uint8_t cl_solid_grey[]={
	0x0,0x0,
	0xb,0xb,
	0xc,0xc,
	0xf,0xf,
	0x1,0x1
};

static uint8_t color_line_cyan[]={
	0x0,0x0,
	0x0,0x6,
	0x6,0x6,
	0x6,0xb,
	0xb,0xb,
	0xb,0xc,
	0xe,0xe,
	0x5,0xe,
	0xe,0x3,
	0x5,0x3,
	0x3,0x3,
	0x3,0xd,
	0xd,0xd,
	0xd,0x1,
	0x1,0x1
};

static uint8_t color_line_purple[]={
	0x0,0x0,
	0x0,0x6,
	0x6,0x6,
	0x6,0x2,
	0xb,0xb,
	0xb,0x4,
	0x4,0x4,
	0x4,0xe,
	0xe,0xe,
	0xe,0x3,
	0x3,0x3,
	0x3,0xd,
	0xd,0xd,
	0xd,0x1,
	0x1,0x1
};

static uint8_t color_line_blue[]={
	0x0,0x0,
	0x0,0x6,
	0x6,0x6,
	0x6,0xb,
	0xb,0xb,
	0xb,0x4,
	0x4,0x4,
	0x4,0xe,
	0xe,0xe,
	0xe,0x3,
	0x3,0x3,
	0x3,0xd,
	0xd,0xd,
	0xd,0x1,
	0x1,0x1
};

static uint8_t color_line_yellow[]={
	0x0,0x0,
	0x0,0x9,
	0x9,0x9,
	0x9,0xb,
	0xb,0xb,
	0xb,0x8,
	0x8,0x8,
	0x8,0xa,
	0xa,0xa,
	0xa,0xf,
	0xf,0xf,
	0xf,0x7,
	0x7,0x7,
	0x7,0x1,
	0x1,0x1
};

static uint8_t color_line_red[]={
	0x0,0x0,
	0x0,0x9,
	0x9,0x9,
	0x9,0x2,
	0x2,0x2,
	0x2,0x8,
	0x8,0x8,
	0x8,0xa,
	0xa,0xa,
	0xa,0xf,
	0xf,0xf,
	0xf,0x7,
	0x7,0x7,
	0x7,0x1,
	0x1,0x1
};

static uint8_t color_line_green[]={
	0x0,0x0,
	0x0,0x9,
	0x9,0x9,
	0x9,0xb,
	0xb,0xb,
	0xb,0xc,
	0xc,0xc,
	0xc,0x5,
	0x5,0x5,
	0x5,0xf,
	0xf,0xf,
	0xf,0xd,
	0xd,0xd,
	0xd,0x1,
	0x1,0x1
};

static uint8_t color_line_grey[]={
	0x0,0x0,
	0x0,0xb,
	0xb,0xb,
	0xb,0xc,
	0xc,0xc,
	0xc,0xf,
	0xf,0xf,
	0xf,0x1,
	0x1,0x1
};
static const int color_mixes[][3] = {
	//vier zusätzliche Mischfarben, z.B. damit man auch nen Grünverlauf ohne Orange und Lila machen kann
	{0x2, 0xa, 3},
	{0x2, 0x5, 3},
	{0xb, 0xa, 3},
	{0xb, 0x5, 3},
	//Level 2: Nicht wahrnehmbarer Flicker
	{0x9, 0x6, 0},
	{0x2, 0xb, 0},
	{0x8, 0x4, 0},
	{0xc, 0xe, 0},
	{0xa, 0x5, 0},
	{0xf, 0x3, 0},
	{0x7, 0xd, 0},
	//Sonderfall ace5:
	{0xc, 0xa, 0},
	{0xc, 0x5, 0},
	{0xe, 0x5, 0},
	{0xe, 0xa, 0},
	//Level 3: Leichter Flicker (1 Helligkeitsstufe)
	{0x0, 0x9, 1},
	{0x0, 0xb, 1},
	{0x9, 0x2, 1},
	{0x9, 0xb, 1},
	{0x6, 0xb, 1},
	{0x6, 0x2, 1},
	{0x2, 0x8, 1},
	{0x2, 0x4, 1},
	{0xb, 0x8, 1},
	{0xb, 0x4, 1},
	{0x8, 0xc, 1},
	{0x8, 0xe, 1},
	{0x4, 0xc, 1},
	{0x4, 0xe, 1},
	//sonderfall ace5:
	{0x8, 0xa, 1},
	{0x8, 0x5, 1},
	{0x4, 0xa, 1},
	{0x4, 0x5, 1},
	{0xc, 0xf, 1},
	{0xc, 0x3, 1},
	{0xe, 0xf, 1},
	{0xe, 0x3, 1},
	{0xa, 0xf, 1},
	{0xa, 0x3, 1},
	{0x5, 0xf, 1},
	{0x5, 0x3, 1},
	//weiter normal:
	{0xf, 0x7, 1},
	{0xf, 0xd, 1},
	{0x3, 0x7, 1},
	{0x3, 0xd, 1},
	{0x7, 0x1, 1},
	{0xd, 0x1, 1},

	//2 Helligkeitsstufen aber immer noch super:
	//(Achtung! Hier kommt zum Tragen, dass der Helligkeitsverlauf am c64 nicht nur bei ace5 nicht-linear ist! So kommt erstmal ein großer Sprung von schwarz auf braun/blau, die nächsten drei Farben (96/2b/8/4) sind ziemlich nahe beieinander in der Helligkeit, dann kommt ein größerer Sprung auf ace5, die ja fast gleichhell sind, und dann nochmal ein größerer Sprung und die darüber sind wieder recht nahe beieinander!)
	{0x9, 0x8, 2},
	{0x9, 0x4, 2},
	{0x6, 0x8, 2},
	{0x6, 0x4, 2},
	{0xf, 0x1, 2},
	{0x3, 0x1, 2},
	//	Level 4: Stärkerer Flicker (2 Helligkeitsstufen), aber noch tolerabel:
	{0x0, 0x6, 3},
	{0x0, 0x2, 3},
	{0x0, 0xb, 3},
	{0x2, 0xc, 3},
	{0x9, 0xc, 3},
	{0x2, 0xe, 3},
	{0xb, 0xc, 3},
	{0xb, 0xe, 3},
	{0xa, 0x7, 3},
	{0xa, 0xd, 3},
	{0x5, 0x7, 3},
	{0x5, 0xd, 3},

	/* plain colors */
	{0x0,0x0, 0},
	{0x1,0x1, 0},
	{0x2,0x2, 0},
	{0x3,0x3, 0},
	{0x4,0x4, 0},
	{0x5,0x5, 0},
	{0x6,0x6, 0},
	{0x7,0x7, 0},
	{0x8,0x8, 0},
	{0x9,0x9, 0},
	{0xa,0xa, 0},
	{0xb,0xb, 0},
	{0xc,0xc, 0},
	{0xd,0xd, 0},
	{0xe,0xe, 0},
	{0xf,0xf, 0},

//	{0x1,0x0, 7},
//	{0xb,0xf, 5},
	//{0x2,0xa, 4 },
	//
};

//c64 palette in RGB
static const int palette_deekay[16][3]={
	{0x00, 0x00, 0x00},
	{0xff, 0xff, 0xff},
	{0x85, 0x1f, 0x02},
	{0x65, 0xcd, 0xa8},
	{0xa7, 0x3b, 0x9f},
	{0x4d, 0xab, 0x19},
	{0x1a, 0x0c, 0x92},
	{0xeb, 0xe3, 0x53},
	{0xa9, 0x4b, 0x02},
	{0x44, 0x1e, 0x00},
	{0xd2, 0x80, 0x74},
	{0x46, 0x46, 0x46},
	{0x8b, 0x8b, 0x8b},
	{0x8e, 0xf6, 0x8e},
	{0x4d, 0x91, 0xd1},
	{0xba, 0xba, 0xba}
};


static const int palette_pepto[16][3]={
	{0x00, 0x00, 0x00},
	{0xFF, 0xFF, 0xFF},
	{0x68, 0x37, 0x2B},
	{0x70, 0xA4, 0xB2},
	{0x6F, 0x3D, 0x86},
	{0x58, 0x8D, 0x43},
	{0x35, 0x28, 0x79},
	{0xB8, 0xC7, 0x6F},
	{0x6F, 0x4F, 0x25},
	{0x43, 0x39, 0x00},
	{0x9A, 0x67, 0x59},
	{0x44, 0x44, 0x44},
	{0x6C, 0x6C, 0x6C},
	{0x9A, 0xD2, 0x84},
	{0x6C, 0x5E, 0xB5},
	{0x95, 0x95, 0x95},
};

static const int palette_faker[16][3]={
	{0x00,0x00,0x00},
	{0xff,0xff,0xff},
	{0xb5,0x21,0x21},
	{0x73,0xff,0xff},
	{0xb5,0x21,0xb5},
	{0x21,0xb5,0x21},
	{0x21,0x21,0xb5},
	{0xff,0xff,0x21},
	{0xb5,0x73,0x21},
	{0x94,0x42,0x21},
	{0xff,0x73,0x73},
	{0x73,0x73,0x73},
	{0x94,0x94,0x94},
	{0x73,0xff,0x73},
	{0x73,0x73,0xff},
	{0xb5,0xb5,0xb5}
};
