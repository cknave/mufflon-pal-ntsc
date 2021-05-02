// SVN revision: r193
//
// MUFFLON v1.0
// High Quality Commodore 64 Imageconverter
//
// (c) 2008-2010 by Crest & Metalvotze
//
// Made by: Bitbreaker/Metalvotze (Tobias Bindhammer)
//          Crossbow/Crest (Roland T�gel)
//          DeeKay/Crest (Daniel Kottmair)
// 
// Blabla Legal BS:           
// 
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product or c64-release, an acknowledgment in the product 
//    documentation or credits would be appreciated. Or beer!
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
// 4. If you alter this software and release it, you must also provide the
//    source. You may not turn this into yet-another-windows-only-c64-
//    software. The software's platform independance and portability must
//    be maintained, no x86-only assembly code (unless there's also a plain
//    C version of it), no Windows-only API shit. If you violate this,
//    pestilence shall come upon you, and DeeKay shall come to your house 
//    and format your harddrive to install Linux. Your C64 will be
//    confiscated and you will be forced to use a Sinclair Spectrum ZX81 
//    to do all your coding - with its shitty membrane keyboard!

#include "mufflon.h"

#define NOSPEEDUPINK	1
#define NOSPEEDUPSPRITE	1
#define NOSPEEDUPFLIBUG	0
#define EVEN		0

struct timespec start;
struct timespec end;


void find_hires_colors(MufflonContext* m);
void find_sprite_colors(MufflonContext* m);
void find_sprite_colors_bruteforce(MufflonContext* m);
void interlace_render(MufflonContext* m);
void make_flick_result_map(MufflonContext* m);

void create_structures(MufflonContext *);
void begin_measure();
void end_measure(MufflonContext*);

void convert_nufli(MufflonContext *);
void convert_muifli(MufflonContext *);
void convert_bmp(MufflonContext *);

void load_bitmap(MufflonContext *, char*);
void render(MufflonContext *);

void find_free_register_switches(MufflonContext *);
void set_8_switches(MufflonContext *);
void prepare(MufflonContext *);
uint64_t compare_cols(MufflonContext *m, int x, int y, int r2, int g2, int b2);

void load_ifli_drl(MufflonContext *, char*);
uint64_t find_best_colors_normal_colums(MufflonContext *);
uint64_t find_best_colors_new(MufflonContext *);
uint64_t find_best_colors_flibug(MufflonContext *m);

void rewrite_palette(MufflonContext *);
void write_data(MufflonContext*, char*, uint8_t*);
void convert(MufflonContext* m);

char* yes_no(int value) {
	if(value>0) return "Yes";
	else return "No";
}

char* imode_name(int value) {
	switch(value) {
		case 1:
			return "rastered";
		break;
		case 0:
			return "blended";
		break;
	}
	return "";
}

char* itype_name(int value) {
	switch(value) {
		case ITYPE_BMP:
			return "bmp";
		break;
		case ITYPE_IFLI:
			return "ifli";
		break;
		case ITYPE_DRL:
			return "drl";
		break;
	}
	return "";
}

char* otype_name(int value) {
	switch(value) {
		case OTYPE_BMP:
			return "bmp";
		break;
		case OTYPE_NUFLI:
			return "nufli";
		break;
		case OTYPE_MUIFLI:
			return "muifli";
		break;
	}
	return "";
}

int main(int argc, char **argv) {
	int a;
	int c,v;
	MufflonContext *m = (MufflonContext *)calloc(1, sizeof(MufflonContext));

	m->option_sbugcol=0;
	m->mix_count=0;
	m->itype=ITYPE_BMP;
	m->otype=OTYPE_NUFLI;
	m->option_prepare=0;
	m->option_flibug=0;
	m->option_darkest=0.0;
	m->option_brightest=1.0;
	m->option_src_pal_name="Pepto";
	m->option_dest_pal_name="Pepto";
	m->option_rastered=1;

        m->secondpaper=-1; //2papermode
        m->secondink=-1; //2paperink
        m->secondsprite=-1; //2spritemode
        m->option_bruteforce=0;
	m->option_multipass=0;
        m->option_deflicker=0;
	m->write_name=NULL;
	m->option_solid_only=-1;
	m->option_no_truncate=0;
	m->option_block_spcol=-3;
	m->option_shutup=0;

	for(c=0;c<16;c++) {
		for(v=0;v<3;v++) {
			m->src_palette[c][v]=palette_pepto[c][v];
		}
	}
	for(c=0;c<16;c++) {
		for(v=0;v<3;v++) {
			m->dest_palette[c][v]=palette_pepto[c][v];
		}
	}

	if(argc==1) {
		info_message("Usage: mufflon <filename> [options]\n");
		info_message("Converts a bmp-file to NUFLI.\n");
		info_message("  --otype              outputtype: nufli, muifli,  bmp - default = %s\n",otype_name(m->otype));
		info_message("  --flibug             renders also the flibug (nufli).\n");
		info_message("  --multipass          renders teh flibug in multiple passes. Very slow!.\n");
		info_message("  --itype              type input file can be ifli, drl or bmp - default = %s\n",itype_name(m->itype));
		info_message("  --imode              how to render imported file: rastered, blended - default = %s\n",imode_name(m->option_rastered));
		info_message("  -o                   output filename. Default \"output      .nuf\"\n");
		info_message("  --src-palette        palette used in source pic (bmp). use one of the following palettes: pepto, deekay - default = %s\n",m->option_src_pal_name);
		info_message("  --dest-palette       transcode pic to given palette before conversion (bmp) or use this palette for conversion (muifli, nufli). one of the following palettes: pepto, deekay - default = %s\n",m->option_dest_pal_name);
		info_message("  --solid              only use solid colors, no mixed colors.\n");
		info_message("  -p                   prepare pic, so it fits better to c64-like colour gradients using dest-palette.\n");
		info_message("  --no-truncate        don't truncate output filename length to 12 characters.\n");
		info_message("  --block-sprite-col   don't use given color for sprites (NUFLI).\n");
		info_message("  --shutup             how's about a sweet cup of shut the fuck up.\n");

		info_message("\nAdditional options for --otype muifli:\n");
		info_message("  --bugcol             color of sprites covering the fli bug.\n");
		info_message("  --2ink               allows separate ink colour for each frame.\n");
		info_message("  --2paper             allows separate paper colour for each frame.\n");
		info_message("  --2sprite            allows separate sprite colour for each frame.\n");
		info_message("  --anything-goes      enables --2ink --2paper and --2sprites.\n");
		info_message("  -b                   enables bruteforce detection of sprite colours, slow but better result.\n");
		info_message("  -d                   deflicker pic.\n");

		info_message("\nAdditional options prepare-mode -p:\n");
		info_message("  --min                luminance of darkest color (0..2) - default = %.1f\n",m->option_darkest);
		info_message("  --max                luminance of brightest color (0..2) - default = %.1f\n",m->option_brightest);
//		info_message("  --pname              if given, the prepared pic will be saved under that name.\n");

		exit(2);
	}

	for(a=1;a<argc;a++) {
		if (strcmp(argv[a],"-o")==0) {
			a++;
			if(argc>a) {
				m->write_name=strdup(argv[a]);
			}
			else {
				fatal_message_and_exit("No output file given.\n");
			}
		}
		else if (strcmp(argv[a],"--min")==0) {
			a++;
			if(argc>a) {
				m->option_darkest=atof(argv[a]);
			}
			else {
				fatal_message_and_exit("No value given for --min.\n");
			}
		}
		else if (strcmp(argv[a],"--max")==0) {
			a++;
			if(argc>a) {
				m->option_brightest=atof(argv[a]);
			}
			else {
				fatal_message_and_exit("No value given for --max.\n");
			}
		}
		else if (strcmp(argv[a],"--otype")==0) {
			a++;
			if(argc>a) {
				if(strcmp(argv[a],"muifli")==0) {
					m->otype=OTYPE_MUIFLI;
				}
				else if(strcmp(argv[a],"nufli")==0) {
					m->otype=OTYPE_NUFLI;
				}
				else if(strcmp(argv[a],"bmp")==0) {
					m->otype=OTYPE_BMP;
				}
				else {
					fatal_message_and_exit("Unknown output image type given \"%s\".\n",argv[a]);
				}
			}
			else {
				m->otype=OTYPE_NUFLI;
			}
		}
		else if (strcmp(argv[a],"--imode")==0) {
			a++;
			if(argc>a) {
				if(strcmp(argv[a],"rastered")==0) {
					m->option_rastered=1;
				}
				else if(strcmp(argv[a],"blended")==0) {
					m->option_rastered=0;
				}
				else {
					fatal_message_and_exit("Unknown input image type given \"%s\".\n",argv[a]);
				}
			}
		}
		else if (strcmp(argv[a],"--itype")==0) {
			a++;
			if(argc>a) {
				if(strcmp(argv[a],"ifli")==0) {
					m->itype=ITYPE_IFLI;
				}
				else if(strcmp(argv[a],"drl")==0) {
					m->itype=ITYPE_DRL;
				}
				else if(strcmp(argv[a],"bmp")==0) {
					m->itype=ITYPE_BMP;
				}
				else {
					fatal_message_and_exit("Unknown input image type given \"%s\".\n",argv[a]);
				}
			}
		}
		else if (strcmp(argv[a],"--bugcol")==0) {
			a++;
			if(argc>a) {
				m->option_sbugcol=atoi(argv[a]);
			}
			else {
				fatal_message_and_exit("No value given for --bugcol.\n");
			}
			if(m->option_sbugcol<0 || m->option_sbugcol>15) {
				fatal_message_and_exit("Illegal quantity for --bugcol.\n");
			}
		}
		else if (strcmp(argv[a],"--block-sprite-col")==0) {
			a++;
			if(argc>a) {
				m->option_block_spcol=atoi(argv[a]);
			}
			else {
				fatal_message_and_exit("No value given for --block-sprite-col.\n");
			}
			if(m->option_block_spcol<0 || m->option_block_spcol>15) {
				fatal_message_and_exit("Illegal quantity for --block-sprite-col.\n");
			}
		}
		else if (strcmp(argv[a],"--solid")==0) {
			m->option_solid_only=1;
		}
		else if (strcmp(argv[a],"--flibug")==0) {
			m->option_flibug=1;
		}
		else if (strcmp(argv[a],"--2ink")==0) {
			m->secondink=15;
		}
		else if (strcmp(argv[a],"--2paper")==0) {
			m->secondpaper=15;
		}
		else if (strcmp(argv[a],"--2sprite")==0) {
			m->secondsprite=15;
		}
		else if (strcmp(argv[a],"--anything-goes")==0) {
			m->secondsprite=15;
			m->secondpaper=15;
			m->secondink=15;
		}
		else if (strcmp(argv[a],"-d")==0) {
			m->option_deflicker=1;
		}
		else if (strcmp(argv[a],"-b")==0) {
			m->option_bruteforce=1;
		}
		else if (strcmp(argv[a],"-p")==0) {
			m->option_prepare=1;
		}
		else if (strcmp(argv[a],"--no-truncate")==0) {
			m->option_no_truncate=1;
		}
		else if (strcmp(argv[a],"--multipass")==0) {
			m->option_multipass=1;
		}
		else if (strcmp(argv[a],"--shutup")==0) {
			m->option_shutup=1;
		}
		else if (strcmp(argv[a],"--dest-palette")==0) {
			a++;
			if(argc>a) {
				if(strcmp(argv[a],"pepto")==0) {
					m->option_dest_pal_name="Pepto";
					for(c=0;c<16;c++) {
						for(v=0;v<3;v++) {
							m->dest_palette[c][v]=palette_pepto[c][v];
						}
					}
				}
				else if(strcmp(argv[a],"deekay")==0) {
					m->option_dest_pal_name="Deekay";
					for(c=0;c<16;c++) {
						for(v=0;v<3;v++) {
							m->dest_palette[c][v]=palette_deekay[c][v];
						}
					}
				}
				else {
					fatal_message_and_exit("Unknown destintation palette \"%s\".\n",argv[a]);
				}
			}
			else {
				fatal_message_and_exit("No destination palette name given.\n");
			}
		}
		else if (strcmp(argv[a],"--src-palette")==0) {
			a++;
			if(argc>a) {
				if(strcmp(argv[a],"pepto")==0) {
					m->option_src_pal_name="Pepto";
					for(c=0;c<16;c++) {
						for(v=0;v<3;v++) {
							m->src_palette[c][v]=palette_pepto[c][v];
						}
					}
				}
				else if(strcmp(argv[a],"deekay")==0) {
					m->option_src_pal_name="Deekay";
					for(c=0;c<16;c++) {
						for(v=0;v<3;v++) {
							m->src_palette[c][v]=palette_deekay[c][v];
						}
					}
				}
				else {
					fatal_message_and_exit("Unknown palette \"%s\".\n",argv[a]);
				}
			}
			else {
				fatal_message_and_exit("No palette name given.\n");
			}
		}
		else if(argc>a) {
			if(m->read_name==NULL) {
				m->read_name=strdup(argv[a]);
			}
			else {
				fatal_message_and_exit("Input name already given. (\"%s\" vs. \"%s\")\n",m->read_name,argv[a]);
			}
		}
		else {
			fatal_message_and_exit("No input file given.\n");
		}
	}
	if(m->read_name==NULL) {
		fatal_message_and_exit("No input file given.\n");
	}
	convert(m);
	exit(0);
}

void make_error_map(MufflonContext* m) {
	int x,y;
	double error=0;
	double max_error=C64XRES*C64YRES*256;
	int pixels=0;
	double diff;
	int r,g,b;
	for(y=0;y<C64YRES;y++) {
		for(x=0;x<C64XRES;x++) {
			r=(m->result_map[(x+y*C64XRES)*3+0]);
			g=(m->result_map[(x+y*C64XRES)*3+1]);
			b=(m->result_map[(x+y*C64XRES)*3+2]);
			diff=compare_cols(m,x,y,r,g,b);
			m->error_map[(x+y*C64XRES)*3+0]=diff;
			m->error_map[(x+y*C64XRES)*3+1]=diff;
			m->error_map[(x+y*C64XRES)*3+2]=diff;
			error+=diff;
			if(diff>10) pixels++;
		}
	}
	info_message("Overall error: %.2f%%\n",(double)error/(double)max_error*100);
	info_message("Erroneous pixels (error>10) %d\n",pixels);
}

void convert(MufflonContext* m) {
	begin_measure();

	create_structures(m);

	switch(m->itype) {
		case ITYPE_IFLI:
		case ITYPE_DRL:
			load_ifli_drl(m, m->read_name);
		break;
		case ITYPE_BMP:
			load_bitmap(m,m->read_name);
		break;
	}
	/* copy data to keep source untouched */
	memcpy(m->data,m->source,C64XRES*C64YRES*3);

	info_message("Conversion-settings:\n");
	info_message("--------------------\n");
	info_message("number of allowed color mixes: %d\n",m->mix_count);
	info_message("c64-palette (source): %s\n",m->option_src_pal_name);
	info_message("c64-palette (destination): %s\n",m->option_dest_pal_name);
	info_message("preparing colors: %s\n",yes_no(m->option_prepare));
	if(m->otype==OTYPE_MUIFLI) {
		info_message("2ink-mode: %s\n",yes_no(m->secondink));
		info_message("2paper-mode: %s\n",yes_no(m->secondpaper));
		info_message("2sprite-mode: %s\n",yes_no(m->secondsprite));
		info_message("deflickering: %s\n",yes_no(m->option_deflicker));
		info_message("bruteforcing: %s\n",yes_no(m->option_bruteforce));
	}
	else {
		if(m->option_block_spcol>=0) {
			info_message("blocking sprite color: %d\n",m->option_block_spcol);
		}
		info_message("render flibug: %s\n",yes_no(m->option_flibug));
		info_message("multipass: %s\n",yes_no(m->option_multipass));
	}
	info_message("only use solid colors: %s\n",yes_no(m->option_solid_only));
	info_message("input name (%s - %s): %s\n",itype_name(m->itype),imode_name(m->option_rastered),m->read_name);
	info_message("base name: %s\n",m->base_name);
	info_message("output file (%s): %s\n",otype_name(m->otype),m->write_name);
	info_message("result name: %s\n",m->result_map_name);
	info_message("errormap name: %s\n",m->error_map_name);
	if(m->otype==OTYPE_MUIFLI) {
		info_message("flickmap name: %s\n",m->flick_map_name);
	}
	info_message("\n");
	info_message("Conversion-progress:\n");
	info_message("--------------------\n\n");

	if(m->option_solid_only==-1) {
		switch(m->otype) {
			case OTYPE_BMP:
			case OTYPE_MUIFLI:
				m->option_solid_only=0;
			break;
			case OTYPE_NUFLI:
				m->option_solid_only=1;
			break;
			default:
				m->option_solid_only=0;
			break;
		}
	}
	if(m->option_prepare!=0) {
		prepare(m);
	}
	rewrite_palette(m);

	switch(m->otype) {
		case OTYPE_MUIFLI:
		case OTYPE_NUFLI:
			if(m->otype==OTYPE_MUIFLI) {
				convert_muifli(m);
				end_measure(m);
				write_data(m, m->flick_map_name, m->flick_map);
			}
			else {
				convert_nufli(m);
				end_measure(m);
			}
			make_error_map(m);
			write_data(m, m->error_map_name, m->error_map);
			write_data(m, m->result_map_name, m->result_map);
		break;
		case OTYPE_BMP:
			convert_bmp(m);
			end_measure(m);
		break;
	}
}

void rgb_to_hsl(int rs, int gs, int bs, double* h, double* s, double* l) {
	double r,g,b;
	double max=0;
	double min=0;

	r=(double)rs/255.0;
	g=(double)gs/255.0;
	b=(double)bs/255.0;

	max=0;
	min=1;

	if(r>=max) max=r;
	if(g>=max) max=g;
	if(b>=max) max=b;

	if(r<=min) min=r;
	if(g<=min) min=g;
	if(b<=min) min=b;

	if(max==min) *h=0;
	else {
		if(max==r) *h=fmod((60*((g-b)/(max-min))+360),360);
		if(max==g) *h=60*((b-r)/(max-min))+180;
		if(max==b) *h=60*((r-g)/(max-min))+240;
	}

	if(max==0) *s=0;
	else *s=(max-min)/max;

	*l=(max+min)/2;
	if(max==min) *s=0;
	else if(*l<=0.5) *s=(max-min)/(max+min);
	else *s=(max-min)/(2-(max+min));
}

void find_used_colors(MufflonContext *m) {
	int64_t diff;
	int64_t best;
	int x, y;
	int a;
	int best_col=0;
	for(y=0;y<C64YRES;y++) {
                for(x=0;x<C64XRES;x++) {
			best=-1;
			for(a=0;a<16;a++) {
				diff=compare_cols(m,x,y,m->dest_palette[a][0],m->dest_palette[a][1],m->dest_palette[a][2]);
				m->diff_lut[y*C64XRES+x][a]=diff;
				if(diff<best || best<0) {
					best=diff;
					best_col=a;
				}
			}
			m->used_colors[y][x/8][best_col]++;
		}
	}
}

void rewrite_palette(MufflonContext *m) {
	int64_t diff;
	int64_t best;
	int x, y;
	int a;
	int best_col;
	int r,g,b;
	for(y=0;y<C64YRES;y++) {
                for(x=0;x<C64XRES;x++) {
			best=-1;
			best_col=0;
			for(a=0;a<m->mix_count;a++) {
				if(m->option_solid_only==0 || color_mixes[a][0]==color_mixes[a][1]) {
					r=(m->src_palette[color_mixes[a][0]][0]+m->src_palette[color_mixes[a][1]][0])/2;
					g=(m->src_palette[color_mixes[a][0]][1]+m->src_palette[color_mixes[a][1]][1])/2;
					b=(m->src_palette[color_mixes[a][0]][2]+m->src_palette[color_mixes[a][1]][2])/2;

					diff=compare_cols(m,x,y,r,g,b);
					if(diff<best || best<0) {
						best=diff;
						best_col=a;
					}
				}
			}
			r=(m->dest_palette[color_mixes[best_col][0]][0]+m->dest_palette[color_mixes[best_col][1]][0])/2;
			g=(m->dest_palette[color_mixes[best_col][0]][1]+m->dest_palette[color_mixes[best_col][1]][1])/2;
			b=(m->dest_palette[color_mixes[best_col][0]][2]+m->dest_palette[color_mixes[best_col][1]][2])/2;

			m->data[(y*C64XRES+x)*3+0]=r;
			m->data[(y*C64XRES+x)*3+1]=g;
			m->data[(y*C64XRES+x)*3+2]=b;

		}
	}
}

void load_ifli_drl(MufflonContext *m, char* name) {
	int x,y,shift;
	int multi1=0;
	int multi2=0;
	int colram=0;
	int bgcol=0;

	int frame=0;

	FILE* fd;
	uint8_t pic[65536];
	uint8_t buf[65536];
	uint8_t byte=0;

	int src=0;
	int dest=0;
	int pkb=0;
	int cnt=0;
	uint8_t val=0;
	int pos;

	int color;

	m->option_prepare=0;
	m->sizex=C64XRES;
	m->sizey=C64YRES;

	if((fd=fopen(name,read_mode))==NULL) {
		fatal_message_and_exit("can't open input file \"%s\"\n",name);
	}

	if(!fread(&buf,1,64000,fd)) {
		fatal_message_and_exit("error while reading input file \"%s\"\n",name);
	}

	if(m->itype==ITYPE_IFLI) {
		/* check for funpaint/gunpaint header */
		if(memcmp(&buf[0x03ea],&header_gunpaint,14)==0) {
			bgcol=buf[0x3f42];
			/* reorganize data to funpaint format */
			/* frame 1 screen + bitmap */
			memcpy(&pic[0x0000],&buf[0x0002],0x4000);
			/* colorram */
			memcpy(&pic[0x43e8],&buf[0x4402],0x4000);
			/* frame 1 screen + bitmap */
			memcpy(&pic[0x4000],&buf[0x4002],0x0400);
		}
		else if(memcmp(&buf[0x0000],&header_funpaint,14)!=0) {
			fatal_message_and_exit("unknown file-format\n");
		}
		else {
			/* rle depack first? */
			if(buf[0x10]!=0) {
				pkb=buf[0x11];
				src=0x12;
				while(dest<0x8400) {
					if(buf[src]==pkb) {
						src++;
						cnt=buf[src];
						src++;
						val=buf[src];
						src++;
						while(cnt>0) {
							pic[dest]=val; dest++; cnt--;
						}
					}
					else {
						pic[dest]=buf[src];
						dest++;
						src++;
					}
				}
			}
			else {
				/* remove header */
				memcpy(&pic[0x0000],&buf[0x0012],0x8400);
			}
			bgcol=pic[0x3f40];
		}
		bgcol=0;
	}
	else {
		memcpy(&pic[0x0000],&buf[0x0002],0x8000);
		bgcol=pic[0x2740];
	}

	for(frame=0;frame<2;frame++) {
		for(y=0;y<C64YRES;y++) {
			for(x=0;x<C64XRES;x+=8) {
				if(m->itype==ITYPE_IFLI) {
					multi1=pic[(y&7)*0x400+frame*0x43e8+x/8+(y>>3)*40]>>4;
					multi2=pic[(y&7)*0x400+frame*0x43e8+x/8+(y>>3)*40]&0xf;
					colram=pic[x/8+(y>>3)*40+0x4000]&0xf;
					byte=pic[(y&7)+(y>>3)*0x140+x+0x2000+frame*0x43e8];
				}
				else {
					colram=pic[x/8+(y>>3)*40 + 0x0000]&0xf;
					multi1=pic[x/8+(y>>3)*40 + 0x0400]>>4;
					multi2=pic[x/8+(y>>3)*40 + 0x0400]&0xf;
					byte=pic[(y&7)+(y>>3)*0x140 + x + frame * 0x2000 + 0x0800];
				}
				for(shift=0;shift<8;shift+=2) {
					pos=(y*C64XRES+x+shift)*3;
					switch((byte&0xc0)) {
						case 0x00:
							color=bgcol;
						break;
						case 0x40:
							color=multi1;
						break;
						case 0x80:
							color=multi2;
						break;
						case 0xc0:
							color=colram;
						break;
						default:
							color=0;
						break;
					}
					if(!m->option_rastered) {
						/* write first frame to result */
						if(frame==0) {
							m->source[pos+0]=m->dest_palette[color][0];
							m->source[pos+1]=m->dest_palette[color][1];
							m->source[pos+2]=m->dest_palette[color][2];
							m->source[pos+3]=m->dest_palette[color][0];
							m->source[pos+4]=m->dest_palette[color][1];
							m->source[pos+5]=m->dest_palette[color][2];
						}
						/* blend second frame with result */
						else {
							m->source[pos+0]=(m->dest_palette[color][0]+m->source[pos+3])/2;
							m->source[pos+1]=(m->dest_palette[color][1]+m->source[pos+4])/2;
							m->source[pos+2]=(m->dest_palette[color][2]+m->source[pos+5])/2;
							if(x+shift<C64XRES-2) {
								m->source[pos+3]=(m->dest_palette[color][0]+m->source[pos+6])/2;
								m->source[pos+4]=(m->dest_palette[color][1]+m->source[pos+7])/2;
								m->source[pos+5]=(m->dest_palette[color][2]+m->source[pos+8])/2;
							}
						}
					}
					else {
						pos+=frame*3;
						m->source[pos+0]=m->dest_palette[color][0];
						m->source[pos+1]=m->dest_palette[color][1];
						m->source[pos+2]=m->dest_palette[color][2];
					}
					byte<<=2;
				}
			}
		}
	}
}

void get_colors_from_line(uint8_t* color_line, int max, int* dither, int* col1, int* col2, int* col3, int* col4, int dithersteps, double lum) {
	int brightness=0;
	int index;
	int maxindex=(max/2)-1;
	int maxsteps=maxindex*dithersteps+1;

	int pos=brightness+(maxsteps*lum);
	if(pos>maxsteps) pos=maxsteps;
	if(pos<0) pos=0;

	index=pos/dithersteps*2;
	*dither = pos % dithersteps;

	*col1=color_line[index+0];
	*col2=color_line[index+1];
	if(pos==maxsteps) {
		*col3=*col1;
		*col4=*col2;
	}
	else {
		*col3=color_line[index+2+0];
		*col4=color_line[index+2+1];
	}
	return;
}

void prepare(MufflonContext *m) {
	int x,y;
	double lum;
	int dither=0;
	double max=0;
	double min=1;
	double fact;
	int red=0;
	int green=0;
	int blue=0;
	int grey=0;
	int purple=0;
	int yellow=0;
	int cyan=0;
	int col1=0;
	int col2=0;
	int col3=0;
	int col4=0;
	int dithersteps=4;

	static double h[C64XRES*C64YRES];
	static double s[C64XRES*C64YRES];
	static double l[C64XRES*C64YRES];

	memset(h, 0, sizeof(h));
	memset(s, 0, sizeof(s));
	memset(l, 0, sizeof(l));

	for(y=0;y<C64YRES;y++) {
		uint8_t r,g,b;
		for(x=0;x<C64XRES;x++) {
			r=m->data[(y*C64XRES+x)*3+0];
			g=m->data[(y*C64XRES+x)*3+1];
			b=m->data[(y*C64XRES+x)*3+2];
			rgb_to_hsl(r,g,b,&h[y*C64XRES+x],&s[y*C64XRES+x],&l[y*C64XRES+x]);
		}
	}

	min=m->option_darkest;
	max=m->option_brightest;

	if(max>2 || max<0) {
		fatal_message_and_exit("illegal quantity for --max.\n");
	}
	if(min>2 || min<0) {
		fatal_message_and_exit("illegal quantity for --min.\n");
	}
	if(max<=min) {
		fatal_message_and_exit("--max must be greater than --min.\n");
	}
	fact=(1/(max-min));

	for(y=0;y<C64YRES;y++) {
		status_message("Preparing pic: Pr0ncessing line #%d   \n\033[A",y);
		for(x=0;x<C64XRES;x++) {
			uint8_t r,g,b;
			lum=(l[y*C64XRES+x]-min)*fact;
			if(lum<0) lum=0;
			if(lum>1) lum=1;
			// FIXME: pick one or the other, to make the static analyser happy :)
			//double off=(1/(lum+1));
			//off=0.05;
			double off=0.05;

			if(s[y*C64XRES+x]>off) {
				//red
				if(h[y*C64XRES+x]>=330 || h[y*C64XRES+x]<30) {
					if(m->option_solid_only==0) get_colors_from_line(color_line_red, ARRAY_ELEMS(color_line_red), &dither, &col1, &col2, &col3, &col4, dithersteps, lum);
					else get_colors_from_line(cl_solid_red, ARRAY_ELEMS(cl_solid_red), &dither, &col1, &col2, &col3, &col4, dithersteps, lum);
					red++;
				}
				//yellow
				if(h[y*C64XRES+x]>=30 && h[y*C64XRES+x]<60) {
					if(m->option_solid_only==0) get_colors_from_line(color_line_yellow, ARRAY_ELEMS(color_line_red), &dither, &col1, &col2, &col3, &col4, dithersteps, lum);
					else get_colors_from_line(cl_solid_yellow, ARRAY_ELEMS(cl_solid_red), &dither, &col1, &col2, &col3, &col4, dithersteps, lum);
					yellow++;
				}
				//green
				if(h[y*C64XRES+x]>=60 && h[y*C64XRES+x]<160) {
					if(m->option_solid_only==0) get_colors_from_line(color_line_green, ARRAY_ELEMS(color_line_green), &dither, &col1, &col2, &col3, &col4, dithersteps, lum);
					else get_colors_from_line(cl_solid_green, ARRAY_ELEMS(cl_solid_green), &dither, &col1, &col2, &col3, &col4, dithersteps, lum);
					green++;
				}
				//cyan
				if(h[y*C64XRES+x]>=160 && h[y*C64XRES+x]<190) {
					if(m->option_solid_only==0) get_colors_from_line(color_line_cyan, ARRAY_ELEMS(color_line_cyan), &dither, &col1, &col2, &col3, &col4, dithersteps, lum);
					else get_colors_from_line(cl_solid_cyan, ARRAY_ELEMS(cl_solid_cyan), &dither, &col1, &col2, &col3, &col4, dithersteps, lum);
					cyan++;
				}
				//blue
				if(h[y*C64XRES+x]>=190 && h[y*C64XRES+x]<270) {
					if(m->option_solid_only==0) get_colors_from_line(color_line_blue, ARRAY_ELEMS(color_line_blue), &dither, &col1, &col2, &col3, &col4, dithersteps, lum);
					else get_colors_from_line(cl_solid_blue, ARRAY_ELEMS(cl_solid_blue), &dither, &col1, &col2, &col3, &col4, dithersteps, lum);
					blue++;
				}
				//purple
				if(h[y*C64XRES+x]>=270 && h[y*C64XRES+x]<330) {
					if(m->option_solid_only==0) get_colors_from_line(color_line_purple, ARRAY_ELEMS(color_line_purple), &dither, &col1, &col2, &col3, &col4, dithersteps, lum);
					else get_colors_from_line(cl_solid_purple, ARRAY_ELEMS(cl_solid_purple), &dither, &col1, &col2, &col3, &col4, dithersteps, lum);
					purple++;
				}
			}
			//grey
			else {
				if(m->option_solid_only==0) get_colors_from_line(color_line_grey, ARRAY_ELEMS(color_line_grey), &dither, &col1, &col2, &col3, &col4, dithersteps, lum);
				else get_colors_from_line(cl_solid_grey, ARRAY_ELEMS(cl_solid_grey), &dither, &col1, &col2, &col3, &col4, dithersteps, lum);
				grey++;
			}
			//XXX also dither solid colors for nufli mode. find best mixed color, check in 5 steps if it is closer to col1 or col2 -> ditherstep
			if(patterns[dither][y&1][x&7]==1) {
				r=(m->dest_palette[col3][0]+m->dest_palette[col4][0])/2;
				g=(m->dest_palette[col3][1]+m->dest_palette[col4][1])/2;
				b=(m->dest_palette[col3][2]+m->dest_palette[col4][2])/2;
			}
			else {
				r=(m->dest_palette[col1][0]+m->dest_palette[col2][0])/2;
				g=(m->dest_palette[col1][1]+m->dest_palette[col2][1])/2;
				b=(m->dest_palette[col1][2]+m->dest_palette[col2][2])/2;
			}
			m->data[(y*C64XRES+x)*3+0]=r;
			m->data[(y*C64XRES+x)*3+1]=g;
			m->data[(y*C64XRES+x)*3+2]=b;
		}
	}
	status_message("\n");
}

void create_structures(MufflonContext *m) {
	int a;
	int b;
	int c;
	int frame;

	m->mix_count=ARRAY_ELEMS(color_mixes);
	m->mixes_r=(int*)calloc(m->mix_count,sizeof(int));
	m->mixes_g=(int*)calloc(m->mix_count,sizeof(int));
	m->mixes_b=(int*)calloc(m->mix_count,sizeof(int));
	m->luma=(int*)calloc(16,sizeof(int));

	/* build 2dim arrays */
	for(frame=0;frame<2;frame++) {
		m->inks[frame]=              (int*)calloc(C64XRES/8*C64YRES,sizeof(int));
		m->papers[frame]=            (int*)calloc(C64XRES/8*C64YRES,sizeof(int));
		// allocate +2 here to deal with overflow in sprite code!
		m->sprites[frame]=           (int*)calloc((C64YRES+2)*6,sizeof(int));
		m->sprite_col_tab[frame]=(uint8_t*)calloc(C64YRES*6,sizeof(uint8_t));
		m->colormap[frame]=      (uint8_t*)calloc(C64YRES/2*C64COLUMNS,sizeof(uint8_t));
		m->hires_bitmap[frame]=  (uint8_t*)calloc(0x2000,sizeof(uint8_t));
		m->sprite_bitmap[frame]= (uint8_t*)calloc(C64YRES*3*6,sizeof(uint8_t));
	}

	for(frame=0;frame<2;frame++) {
		for(a=0;a<C64YRES*6;a++) m->sprites[frame][a]=-1;
	}

	m->error_map= calloc(C64XRES*C64YRES*3,sizeof(uint8_t));
	m->flick_map= calloc(C64XRES*C64YRES*3,sizeof(uint8_t));
	m->result_map=calloc(C64XRES*C64YRES*3,sizeof(uint8_t));
	m->data=      calloc(C64XRES*C64YRES*3,sizeof(uint8_t));
	m->source=    calloc(C64XRES*C64YRES*3,sizeof(uint8_t));

	for(a=0;a<16;a++) {
		m->luma[a]=((double)m->dest_palette[a][0]*0.3+(double)m->dest_palette[a][1]*0.59+(double)m->dest_palette[a][2]*0.11);
	}
	for(a=0;a<200;a++) {
		for(b=0;b<40;b++) {
			for(c=0;c<16;c++) {
				m->used_colors[a][b][c]=0;
			}
		}
	}
	for(a=0;a<16;a++) {
		for(b=0;b<16;b++) {
			m->allowed_mix_map[a][b]=-1;
		}
	}
	for(a=0;a<m->mix_count;a++) {
		m->mixes_r[a]=(m->dest_palette[color_mixes[a][0]][0]+m->dest_palette[color_mixes[a][1]][0])/2;
		m->mixes_g[a]=(m->dest_palette[color_mixes[a][0]][1]+m->dest_palette[color_mixes[a][1]][1])/2;
		m->mixes_b[a]=(m->dest_palette[color_mixes[a][0]][2]+m->dest_palette[color_mixes[a][1]][2])/2;

		m->allowed_mix_map[color_mixes[a][0]][color_mixes[a][1]]=a;
		m->allowed_mix_map[color_mixes[a][1]][color_mixes[a][0]]=a;
	}
	for(frame=0;frame<2;frame++) {
		for(a=0;a<C64XRES/8*C64YRES;a++) {
			m->inks[frame][a]=-1;
			m->papers[frame][a]=-1;
		}
		for(a=0;a<6*C64YRES;a++) {
			m->sprites[frame][a]=-1;
		}
		for(a=0;a<C64YRES;a++) {
			m->fb_sprite_col[frame][0][a]=-1;
			m->fb_sprite_col[frame][1][a]=-1;
			m->fb_sprite_col[frame][2][a]=-1;
			m->fb_sprite_col[frame][3][a]=-1;
		}
	}

	if(!m->write_name) { m->base_name=strdup(m->read_name); m->path=strdup(m->read_name); }
	else { m->base_name=strdup(m->write_name); m->path=strdup(m->write_name); }

	m->base_name=basename(m->base_name);
	m->path=dirname(m->path);

	if (m->option_no_truncate) {
		// Just strip off extension
		for(a=strlen(m->base_name)-1;a>=0;a--){
			if(m->base_name[a]=='.') break;
		}
	}
	else {
		/* truncate base_name on first . or at 12th char */
		for(a=0;a<(int)strlen(m->base_name) && a < 12; a++) {
			if(m->base_name[a]=='.') break;
		}
	}
	m->base_name[a]=0;

	m->error_map_name=malloc(strlen(m->path)+strlen(m->base_name)+25);
	sprintf(m->error_map_name,"%s/%s_errormap.bmp",m->path,m->base_name);
	m->flick_map_name=malloc(strlen(m->path)+strlen(m->base_name)+27);
	sprintf(m->flick_map_name,"%s/%s_flickermap.bmp",m->path,m->base_name);
	m->result_map_name=malloc(strlen(m->path)+strlen(m->base_name)+23);
	sprintf(m->result_map_name,"%s/%s_result.bmp",m->path,m->base_name);

	switch(m->otype) {
		case OTYPE_MUIFLI:
		case OTYPE_NUFLI:
			if(!m->write_name) {
				m->write_name=malloc(strlen(m->path)+18);
				if(m->otype==OTYPE_MUIFLI) {
					sprintf(m->write_name,"%s/%-12s.mui",m->path,m->base_name);
				}
				else {
					sprintf(m->write_name,"%s/%-12s.nuf",m->path,m->base_name);
				}
			}
		break;
		case OTYPE_BMP:
			if(!m->write_name) {
				m->write_name=malloc(strlen(m->path)+strlen(m->base_name)+6);
				sprintf(m->write_name,"%s/%s.bmp",m->path,m->base_name);
			}
		break;
	}
	return;
}

void write_result(MufflonContext *m, uint8_t* result, int size) {
	uint8_t c;
	info_message("Writing '%s'\n",m->write_name);
	m->fdw = fopen (m->write_name, write_mode);
	if(m->fdw==NULL) {
		fatal_message_and_exit("can't open output file \"%s\"\n",m->write_name);
	}
	/* write load adress if c64-format is used */
	if(m->otype!=OTYPE_BMP) {
		c=0x00;
		fwrite(&c,1,1,m->fdw);
		if(m->otype==OTYPE_MUIFLI) c=0x21;
		else c=0x20;
		fwrite(&c,1,1,m->fdw);
	}
	fwrite(&result[0],1,size,m->fdw);
	fclose(m->fdw);
}

void write_bitmap(char* name, uint8_t* data) {
	FILE* fdw;
	int filesize=54+C64XRES*C64YRES*3;
	int datasize=filesize-54;
	uint8_t bmp_header[54];
	memset(bmp_header,0,54);
	bmp_header[BMP_SIGNATURE+0]='B';
	bmp_header[BMP_SIGNATURE+1]='M';
	bmp_header[BMP_FILE_SIZE+0]=(filesize % 256);
	bmp_header[BMP_FILE_SIZE+1]=(filesize/256 % 256);
	bmp_header[BMP_FILE_SIZE+2]=(filesize/65536 % 256);
	bmp_header[BMP_FILE_SIZE+3]=(filesize/16777216 % 256);
	bmp_header[BMP_DATA_OFFSET+0]=54;
	bmp_header[BMP_DATA_OFFSET+1]=0;
	bmp_header[BMP_DATA_OFFSET+2]=0;
	bmp_header[BMP_DATA_OFFSET+3]=0;
	bmp_header[BMP_HEADER_SIZE+0]=40;
	bmp_header[BMP_HEADER_SIZE+1]=0;
	bmp_header[BMP_HEADER_SIZE+2]=0;
	bmp_header[BMP_HEADER_SIZE+3]=0;
	bmp_header[BMP_SIZEX+0]=(C64XRES % 256);
	bmp_header[BMP_SIZEX+1]=(C64XRES/256 % 256);
	bmp_header[BMP_SIZEX+2]=(C64XRES/65536 % 256);
	bmp_header[BMP_SIZEX+3]=(C64XRES/16777216 % 256);
	bmp_header[BMP_SIZEY+0]=(C64YRES % 256);
	bmp_header[BMP_SIZEY+1]=(C64YRES/256 % 256);
	bmp_header[BMP_SIZEY+2]=(C64YRES/65536 % 256);
	bmp_header[BMP_SIZEY+3]=(C64YRES/16777216 % 256);
	bmp_header[BMP_NUM_PLANES+0]=1;
	bmp_header[BMP_NUM_PLANES+1]=0;
	bmp_header[BMP_BPP+0]=24;
	bmp_header[BMP_BPP+1]=0;
	bmp_header[BMP_DATA_SIZE+0]=(datasize % 256);
	bmp_header[BMP_DATA_SIZE+1]=(datasize/256 % 256);
	bmp_header[BMP_DATA_SIZE+2]=(datasize/65536 % 256);
	bmp_header[BMP_DATA_SIZE+3]=(datasize/16777216 % 256);
	fdw = fopen (name, write_mode);
	if(fdw==NULL) {
		fatal_message_and_exit("can't open output file \"%s\"\n",name);
	}
	fwrite(&bmp_header[0],1,54,fdw);
	fwrite(&data[0],1,datasize,fdw);
	fclose(fdw);
}

void write_data(MufflonContext* m, char* name, uint8_t* dat) {
        static uint8_t dest[C64XRES*C64YRES*3];
	memset(dest, 0, sizeof(dest));

        int x,y;
        for(y=0;y<C64YRES;y++) {
                for(x=0;x<C64XRES*3;x+=3) {
                        dest[(C64YRES-1-y)*C64XRES*3+x+0]=dat[x+y*C64XRES*3+2];
                        dest[(C64YRES-1-y)*C64XRES*3+x+1]=dat[x+y*C64XRES*3+1];
                        dest[(C64YRES-1-y)*C64XRES*3+x+2]=dat[x+y*C64XRES*3+0];
                }
        }
	info_message("Writing '%s'\n",name);
        write_bitmap(name,dest);
}

void load_bitmap(MufflonContext *m, char* name) {
	FILE* fdr;
	uint8_t b,g,r;
	int x, y;
	int data_offset;
	uint8_t bmp_header[54];
	int padded;
	int compression;

	if((fdr=fopen(name,read_mode))==NULL) {
		fatal_message_and_exit("can't open input file \"%s\"\n",name);
	}
	if(!fread(&bmp_header,1,sizeof(bmp_header),fdr)) {
		fatal_message_and_exit("error while reading input file \"%s\"\n",name);
	}


	if(bmp_header[BMP_SIGNATURE+0]!='B' || bmp_header[BMP_SIGNATURE+1]!='M') {
		fatal_message_and_exit("input file is not a BMP.\n"); exit(2);
	}
	compression=((bmp_header[BMP_COMPRESSION_TYPE+0]<<0)+(bmp_header[BMP_COMPRESSION_TYPE+1]<<8));
	m->bpp=((bmp_header[BMP_BPP+0]<<0)+(bmp_header[BMP_BPP+1]<<8))/8;
	m->sizex=((bmp_header[BMP_SIZEX+0]<<0)+(bmp_header[BMP_SIZEX+1]<<8)+(bmp_header[BMP_SIZEX+2]<<16)+(bmp_header[BMP_SIZEX+3]<<24));
	m->sizey=((bmp_header[BMP_SIZEY+0]<<0)+(bmp_header[BMP_SIZEY+1]<<8)+(bmp_header[BMP_SIZEY+2]<<16)+(bmp_header[BMP_SIZEY+3]<<24));
	padded=m->sizex*m->bpp;
	while((padded % 4) !=0) padded++;

    // if image height is negative, bmp file is stored in y-flip mode, lines are from bottom to top
	int flip_vertical = 0;
	if (m->sizey < 0)
	{
		flip_vertical = 1;
		m->sizey = -m->sizey;
	}

	info_message("BMP-information:\n");
	info_message("----------------\n");
	info_message("sizex: %d\n",m->sizex);
	info_message("sizey: %d\n",m->sizey);
	info_message("bytes per pixel: %d\n",m->bpp);
	info_message("\n");

	if(compression!=0) {
		fatal_message_and_exit("Wrong compression type (no RLE please)\n");
	}
	if(m->bpp!=3) {
		fatal_message_and_exit("Colourdepth != 24\n");
	}
	if(m->sizex>320) {
		fatal_message_and_exit("width > 320 pixels\n");
	}
	if(m->sizey>200) {
		fatal_message_and_exit("heigth < 320 pixels\n");
	}

	data_offset=((bmp_header[BMP_DATA_OFFSET+0]<<0)+(bmp_header[BMP_DATA_OFFSET+1]<<8)+(bmp_header[BMP_DATA_OFFSET+2]<<16)+(bmp_header[BMP_DATA_OFFSET+3]<<24));

	fseek(fdr,data_offset,SEEK_SET);

	for(y=m->sizey-1;y>=0;y--) {
		for(x=0;x<m->sizex*m->bpp;x+=3) {
			if(!fread(&b,1,1,fdr)) {
				error_message("Preliminary end of file\n");
			}
			if(!fread(&g,1,1,fdr)) {
				error_message("Preliminary end of file\n");
			}
			if(!fread(&r,1,1,fdr)) {
				error_message("Preliminary end of file\n");
			}
			/* crop if necessary */
			if(y<C64YRES && x<C64XRES*3) {
    			int real_y = flip_vertical ? m->sizey - 1 - y : y;
				m->source[0+x+real_y*C64XRES*3]=r;
				m->source[1+x+real_y*C64XRES*3]=g;
				m->source[2+x+real_y*C64XRES*3]=b;
			}
		}
		/* skip padding */
		while(x<padded) {
			if(!fread(&b,1,1,fdr)) {
				error_message("Preliminary end of file\n");
			}
			x++;
		}
	}

	fclose(fdr);
	return;
}

void begin_measure() {                                                                  //set measure start point
        clock_gettime(CLOCK_MONOTONIC, &start);
        return;
}

void end_measure(MufflonContext* m) {                                                                    //end of measure, calculate time needed
        double time;
        double timediff;
        clock_gettime(CLOCK_MONOTONIC, &end);
        timediff=(end.tv_sec-start.tv_sec)*1000000000.0+(double)end.tv_nsec-(double)start.tv_nsec;

        time=timediff/1000000000.0;
        info_message("calc.time: %.4fs\n", time);
        return;
}

void convert_nufli(MufflonContext *m) {
	int x=0;
	int y;
	static uint8_t result[64000]={0};
	int frame;
	int i,p,b;
	int address1;
	int srow;
	int row;
	memset(result, 0, sizeof(result));

	find_used_colors(m);
	if(EVEN) find_best_colors_new(m);
	find_best_colors_normal_colums(m);
	find_free_register_switches(m);
	if(m->option_flibug) find_best_colors_flibug(m);
	set_8_switches(m);
	render(m);

	for(frame=0;frame<1;frame++) {
		/* wipe out 0xff/-1 in sprite color table */
		for(y=0;y<C64YRES/2+1;y++) {
			for(b=0;b<6;b++) {
				if(m->final_sprite_tab[frame][y*6+b]==0xff) m->final_sprite_tab[frame][y*6+b]=0;
			}
		}
		for(y=0;y<(C64YRES/2+1);y++) {
			result[0x0400+y+frame*0x8000]=m->final_sprite_tab[frame][y*6+0];
			result[0x0480+y+frame*0x8000]=m->final_sprite_tab[frame][y*6+1];
			result[0x0800+y+frame*0x8000]=m->final_sprite_tab[frame][y*6+2];
			result[0x0880+y+frame*0x8000]=m->final_sprite_tab[frame][y*6+3];
			result[0x0c00+y+frame*0x8000]=m->final_sprite_tab[frame][y*6+4];
			result[0x0c80+y+frame*0x8000]=m->final_sprite_tab[frame][y*6+5];
		}

		/* copy first half of hires bitmaps */
		memcpy(&result[0x4000+frame*0x8000],&m->hires_bitmap[frame][0],0x1400);

		/* copy second half of hires bitmaps */
		memcpy(&result[0x1400+frame*0x8000],&m->hires_bitmap[frame][0x1400],0x0b40);

		/* copy screen lines */
		for(y=0;y<C64YRES;y+=2) {
			for(x=0;x<C64XRES/8;x++) {
				i=m->inks[frame][y*C64XRES/8+x];
				p=m->papers[frame][y*C64XRES/8+x];
				if(i<0) i=0;
				if(p<0) p=0;
				result[nuifli_scram[y/2]+frame*0x8000+x]=(i<<4)|p;
			}
		}

		/* copy sprite bitmap */
		row=5;
		for(y=0;y<200;y++) {
			for(x=0;x<(C64XRES-8-FLI)/8/2;x++) {
				srow=row+(x % 3);
				srow=srow&0x3f;
				/* Extraworschd für Sprite 6 bis Zeile 128 */
				if(x>=5*3 && y<128) {
					address1=0x5400+( (((y/2)&0x7)^0x7)*0x40 )+srow+frame*0x8000;
				}
				else {
					address1=nuifli_spram[y/2]+srow+x/3*0x40+frame*0x8000;
				}
				result[address1]=m->sprite_bitmap[frame][y*3*6+x];
			}
			if((y&1)==0) row+=3;
			if(row>0x3f) row=row&0x3f;
			else if(row==0x3f) row=0;
		}

		/* copy flibug sprite bitmaps */
		row=5;
		for(y=0;y<200;y++) {
			for(x=0;x<3;x++) {
				srow=row+(x % 3);
				srow=srow&0x3f;

				address1=flibug_hires_spram[y/2]+srow+frame*0x8000;
				result[address1]=m->fb_hsprite_bitmap[frame][y*3+x];

				address1=flibug_multi_spram[y/2]+srow+frame*0x8000;
				result[address1]=m->fb_msprite_bitmap[frame][y*3+x];
			}
			if((y&1)==0) row+=3;
			if(row>0x3f) row=row&0x3f;
			else if(row==0x3f) row=0;
		}

		/* copy sprite pointers */
		memcpy(&result[0x03f8+frame*0x8000],&nuifli_sprite_pointers[0][0],8);
		memcpy(&result[0x07f8+frame*0x8000],&nuifli_sprite_pointers[1][0],8);
		memcpy(&result[0x0bf8+frame*0x8000],&nuifli_sprite_pointers[2][0],8);
		memcpy(&result[0x0ff8+frame*0x8000],&nuifli_sprite_pointers[3][0],8);
		memcpy(&result[0x23f8+frame*0x8000],&nuifli_sprite_pointers[4][0],8);
		memcpy(&result[0x27f8+frame*0x8000],&nuifli_sprite_pointers[5][0],8);
		memcpy(&result[0x2bf8+frame*0x8000],&nuifli_sprite_pointers[6][0],8);
		memcpy(&result[0x2ff8+frame*0x8000],&nuifli_sprite_pointers[7][0],8);
		memcpy(&result[0x33f8+frame*0x8000],&nuifli_sprite_pointers[8][0],8);
		memcpy(&result[0x37f8+frame*0x8000],&nuifli_sprite_pointers[9][0],8);
		memcpy(&result[0x3bf8+frame*0x8000],&nuifli_sprite_pointers[10][0],8);
		memcpy(&result[0x3ff8+frame*0x8000],&nuifli_sprite_pointers[11][0],8);
	}

	/* first row of sprite_tab are set as initial sprite colors */
	result[0x1ff7]=m->fb_sprite_col[0][0][0]&0xf;	//hires sprite
	result[0x1ff1]=m->fb_sprite_col[0][1][0]&0xf;	//01 mul1
	result[0x1ff0]=m->fb_sprite_col[0][2][0]&0xf;	//10 mul2
	result[0x1ff6]=m->fb_sprite_col[0][3][0]&0xf;	//11 mul3

	/* copy the displayer code */
	memcpy(&result[0x1000],nuifli_displayer_code1,0x01f5);
	memcpy(&result[0x1300],nuifli_displayer_code2,0x00d6);
	memcpy(&result[0x1fc0],nuifli_displayer_code3,0x0030);

	write_result(m,result,0x5a00);
}

/* functions for setting/swapping/getting and comparing colors. yet still unused */
int is_used(MufflonContext *m, int x, int y, int xdim, int pixel_type) {
	int xx;
	for(xx=x;xx<x+xdim;xx++) {
		if(m->combinations[0][y*C64YRES+xx]==pixel_type) return 1;
	}
	return 0;
}

int is_aligned(MufflonContext *m, int x, int y, int xdim, int ydim, int pixel_type) {
	int xx, yy;
	if(pixel_type==SPRITE || pixel_type==FLISPRITEM1 || pixel_type==FLISPRITEM2 || pixel_type==FLISPRITEM3) return 1;
	for(yy=y;yy<y+ydim;yy++) {
		for(xx=x;xx<x+xdim;xx+=2) {
			if(m->combinations[0][yy*C64YRES+xx+0]==pixel_type && (m->combinations[0][yy*C64YRES+xx+1]!=pixel_type)) return 0;
			if(m->combinations[0][yy*C64YRES+xx+1]==pixel_type && (m->combinations[0][yy*C64YRES+xx+0]!=pixel_type)) return 0;
		}
	}
	return 1;
}

int is_replaceable(MufflonContext *m, int x, int y, int xdim, int pixel_type, int color) {
	int xx;
	switch(pixel_type) {
		case SPRITE:
			for(xx=x;xx<x+xdim;xx+=8) {
				if(m->inks[0][y*C64YRES/8+xx/8]==color || m->papers[0][y*C64YRES/8+xx/8]==color) {
					if(y<C64YRES-2 && m->inks[0][(y+2)*C64YRES/8+xx/8]!=color && m->papers[0][(y+2)*C64YRES/8+xx/8]!=color) return 0;
				}
				else return 0;
			}
		break;
	}
	return 1;
}

void set_color(MufflonContext *m, int x, int y, int xdim, int pixel_type, int color) {
	int xx;
	switch(pixel_type) {
		case INK:
			for(xx=x;xx<x+xdim;xx+=8) {
				m->inks[0][y*C64YRES/8+xx/8]=color;
			}
		break;

		case PAPER:
			for(xx=x;xx<x+xdim;xx+=8) {
				m->papers[0][y*C64YRES/8+xx/8]=color;
			}
		break;

		case SPRITE:
			if(x<FLI) { xdim=xdim-(FLI-x); x=FLI; }
			for(xx=x;xx<x+xdim;xx+=SPRITEWIDTH) {
				m->sprites[0][y*6+xx/SPRITEWIDTH]=color;
				if((y&1)==0) { if(y>0) m->sprites[0][(y-1)*6+xx/SPRITEWIDTH]=color; }
				else m->sprites[0][(y+1)*6+xx/SPRITEWIDTH]=color;
			}
		break;

		case FLISPRITE:
		break;

		case FLISPRITEM1:
		break;

		case FLISPRITEM2:
		break;

		case FLISPRITEM3:
		break;
	}
}

void free_color(MufflonContext *m, int x, int y, int xdim, int pixel_type) {
	set_color(m,x,y,xdim,pixel_type,-1);
}

void set_free_register(MufflonContext *m, int y, uint8_t reg) {
	int a;
	int frame=0;
	/* no registers to be set in line 0, will be doen with initial colors */
	if(y==0) return;
	for(a=5;a>=0;a--) {
		if(m->free_switches[frame][y][a]) {
			m->final_sprite_tab[frame][y*6+a]=reg;
			m->free_switches[frame][y][a]=0;
			m->free_switches_sum[frame][y]--;
			m->free_switches_flibug[frame][y]=m->free_switches_sum[frame][y]-m->free_switches[frame][y][0];
			return;
		}
	}
}

void find_free_register_switches(MufflonContext *m) {
	int frame=0;
	int y,b;
	int sum;
	int src,dest;

	/* copy sprites* to final_sprite_tab, so we don't spoil srpite* that is needed for rendering later on */
	src=0;
	dest=0;
	while(src<C64YRES) {
		for(b=0;b<6;b++) {
			if(m->sprites[frame][src*6+b]<0) m->final_sprite_tab[frame][dest*6+b] = 0xff; // = 0xff;
			else m->final_sprite_tab[frame][dest*6+b]=m->sprites[frame][src*6+b];
		}
		dest++;
		src+=2;
		if(src==2) src=1;
	}

	/* do a statistic of switching opportunities */
	for(y=0;y<C64YRES/2+1;y++) {
		sum=0;
		for(b=0;b<6;b++) {
			/* first row always works out, as we will write values into init locations */
			if(y==0) {
				m->free_switches[frame][y][b]=1; sum++;
			}
			else {
				/* if sprite == -1 (unused) or same as 2 lines before, location is usable for a color code */
				if(m->final_sprite_tab[frame][y*6+b]==m->final_sprite_tab[frame][(y-1)*6+b] || m->final_sprite_tab[frame][y*6+b]==0xff) {
					m->free_switches[frame][y][b]=1; sum++;
				}
				else m->free_switches[frame][y][b]=0;
			}
		}
		m->free_switches_sum[frame][y]=sum;
		m->free_switches_flibug[frame][y]=sum-m->free_switches[frame][y][0];
	}
	m->free_switches_max=0;
	for(y=SPLITEND;y>=SPLITSTART;y--) {
		m->free_switches_max+=m->free_switches_sum[frame][y];
	}
	if(m->free_switches_max<8) {
		error_message("not enough switches for change of spritepointers. This is a serious problem and will make your pic look b0rken.\n");
	}
}

void set_8_switches(MufflonContext *m) {
	uint8_t reg;
	int b,y;
	int frame=0;
	reg = 0x11;
	for(b=6;b>0;b--) {
		for(y=SPLITEND;y>=SPLITSTART;y--) {
			if(reg<0x20) {
				if(b==m->free_switches_sum[frame][y]) {
					/* find first free switch and replace color code */
					set_free_register(m, y, reg);
					/* increment register */
					reg+=2;
				}
			}
		}
	}
}

uint64_t compare_cols(MufflonContext *m, int x, int y, int r2, int g2, int b2) {
	int r1,g1,b1;
	int result;

	r1=m->data[(y*C64XRES+x)*3+0];
	g1=m->data[(y*C64XRES+x)*3+1];
	b1=m->data[(y*C64XRES+x)*3+2];

	//result = abs(r1-r2)*0.3+abs(g1-g2)*0.59+abs(b1-b2)*0.11;
	result = abs(r1-r2)*19661+abs(g1-g2)*38666+abs(b1-b2)*7209;
	return (uint64_t)(result/65536);
}


//XXX aufbohren damit es priorities und dimensions berücksichtigt?
//gern auch mit variabler breite, 8 sind jetzt halt mal statisch, muss aber nicht
inline uint64_t find_best_combinations(MufflonContext *m, int block, int y, int *cols) {
	int comb;
	uint64_t delta;
	uint64_t blk_best=0;
	uint64_t best=256*256*256;
	int res_0, res_1;
	int x;
	int *combos;
	int num;

	if(block<FLI) {
		combos=combinations_flibug;
		num=ARRAY_ELEMS(combinations_flibug);
	}
	else {
		combos=combinations_normal;
		num=ARRAY_ELEMS(combinations_normal);
	}

	//XXX stepping dann hier auch variabel! (4 bei multicol)
	for(x=block;x<block+8;x+=2) {

		best=256*256*256;
		res_0=-1;
		res_1=-1;

		register int pixel_0 = y*C64XRES+x+0;
		register int pixel_1 = y*C64XRES+x+1;

		for(comb=0;comb<num;comb+=2) {

			register int col_0 = cols[combos[comb+0]];
			register int col_1 = cols[combos[comb+1]];

			/* only check if both colors for combo are set */
			if(col_0>=0 && col_1>=0) {
				/* same value for both pixel but different colorsource?! that's silly, so forbid */
				if(col_0==col_1 && combos[comb+0]!=combos[comb+1]) {}
				else {
					//XXX hier check ob farbe jeder combo auch in used_cols ist!
					delta =m->diff_lut[pixel_0][col_0];
					delta+=m->diff_lut[pixel_1][col_1];
					if(delta<best) {
						best=delta;
						res_0=combos[comb+0];
						res_1=combos[comb+1];
					}
				}
			}
			/* perfect match, no need to bother */
			if(best==0) break;
		}
		/* optimize a bit */
		if(res_0>=0 && cols[res_0]==cols[INK]) res_0=INK;
		if(res_1>=0 && cols[res_1]==cols[INK]) res_1=INK;
		if(x<FLI) {
			if(res_0>=0 && cols[res_0]==cols[FLISPRITE]) res_0=FLISPRITE;
			if(res_1>=0 && cols[res_1]==cols[FLISPRITE]) res_1=FLISPRITE;
		}

		m->combinations[0][pixel_0]=res_0;
		m->combinations[0][pixel_1]=res_1;
		blk_best += best;
	}

	return blk_best;
}

uint64_t find_best_colors_line(MufflonContext *m, int block, int y, int width, int* cols1, int* cols2) {
	uint64_t delta;
	uint64_t blk_delta;
	uint64_t blk_best=256*256*256;
	int x;
	int best_paper=-1;
	int best_ink=-1;
	int inkstart=-1;
	int paperend=15;
	int ink,paper;
	int row1[10];
	int row2[10];
	int a;

	delta=0;
	for(x=block;x<block+width;x+=8) {
		blk_best=256*256*256;
		/* only do full afli in first two lines in flibug */
		inkstart=-1;
		paperend=15;
		if(x<FLI) {
			if((y&7)>=2) { inkstart=15; paperend=-1; }
		}

		for(ink=15;ink>=inkstart;ink--) {
			for(paper=paperend;paper>=-1;paper--) {
				if(NOSPEEDUPINK || (
					(ink<0 || m->used_colors[y][x/8][ink])
				 &&
					(paper<0 || m->used_colors[y][x/8][paper])
				)) {
					memcpy(row1, cols1, sizeof(row1));
					row1[INK]=ink;
					row1[PAPER]=paper;

					memcpy(row2, cols2, sizeof(row2));
					row2[INK]=ink;
					row2[PAPER]=paper;

					blk_delta = find_best_combinations(m,x,y+0,row1);
					blk_delta += find_best_combinations(m,x,y+1,row2);
					if(blk_delta<blk_best) {
						for(a=0;a<10;a++) {
							if(row2[a]>=0) {
								blk_best=blk_delta;
								/* remember result, take combinations result, as colors might have changed to -1 if unused */
								best_paper=row1[PAPER];
								best_ink=row1[INK];
								break;
							}
						}
					}
				}
			}
			/* perfect match, no need to bother */
			if(blk_best==0) break;
		}
		delta+=blk_best;

		/* optimize a bit */
		if(best_paper==row1[INK] && best_paper==row2[INK]) {
			best_paper=-1;
		}
		if(x>=FLI && best_ink==row1[SPRITE] && best_ink==row2[SPRITE]) {
			best_ink=best_paper;
			best_paper=-1;
		}
		if(x>=FLI && best_paper==row1[SPRITE] && best_paper==row2[SPRITE]) {
			best_paper=-1;
		}

		if(x<FLI && best_ink==row1[FLISPRITE] && best_ink==row2[FLISPRITE]) {
			best_ink=best_paper;
			best_paper=-1;
		}
		if(x<FLI && best_paper==row1[FLISPRITE] && best_paper==row2[FLISPRITE]) {
			best_paper=-1;
		}
		if(y>=0) {
			m->papers[0][(y+0)*C64XRES/8+x/8]=best_paper;
			m->inks  [0][(y+0)*C64XRES/8+x/8]=best_ink;
		}
		if(y<C64YRES-1) {
			m->papers[0][(y+1)*C64XRES/8+x/8]=best_paper;
			m->inks  [0][(y+1)*C64XRES/8+x/8]=best_ink;
		}
	}
	return delta;
}

uint64_t find_best_colors_new(MufflonContext *m) {
	int y, colum;
	uint64_t best_line_delta=256*256*256;
	uint64_t line_delta;
	uint64_t delta;

	int row1[10];

	int width=48;
	int offset=FLI;
	int a;
	int best_spr1=-1;
	int sprite1;

	delta=0;

	/* do normal 6 colums with expanded sprite overlay */
	for(colum=0;colum<5;colum++) {
		for(y=0;y<C64YRES;y+=2) {
			status_message("Processing normal colums: Pr0ncessing line #%d on colum %d    \n\033[A",y,colum);

			best_line_delta=256*256*256;
			for(sprite1=-1;sprite1<=15;sprite1++) {
				if(NOSPEEDUPSPRITE || (
				   (sprite1<0
					      || m->used_colors[y+0][colum*6+offset/8+0][sprite1]
					      || m->used_colors[y+0][colum*6+offset/8+1][sprite1]
					      || m->used_colors[y+0][colum*6+offset/8+2][sprite1]
					      || m->used_colors[y+0][colum*6+offset/8+3][sprite1]
					      || m->used_colors[y+0][colum*6+offset/8+4][sprite1]
					      || m->used_colors[y+0][colum*6+offset/8+5][sprite1])
					)) {
					for(a=0;a<10;a++) { row1[a]=-1; }
					row1[SPRITE]=sprite1;
					line_delta=find_best_colors_line(m,colum*width+offset,y,width,row1,row1);
					if(line_delta<best_line_delta) {
						best_line_delta=line_delta;
						best_spr1=row1[SPRITE];
					}
				}
				/* perfekt match, no need to try further */
				if(best_line_delta==0) break;
			}
			row1[SPRITE]=best_spr1;
			/* now finally recalculate with best sprite colors */
			find_best_colors_line(m,colum*width+offset,y,width,row1,row1);

			m->sprites[0][(y+0)*6+colum]=best_spr1;
			m->sprites[0][(y+1)*6+colum]=best_spr1;

			delta+=best_line_delta;
		}
	}
	info_message("\n");
	return delta;
}

void swap(int *a, int *b) {
	int t;
	t=*a;
	*a=*b;
	*b=t;
}

uint64_t find_best_colors_normal_colums(MufflonContext *m) {
	int y, colum;
	uint64_t best_line_delta=256*256*256;
	uint64_t line_delta;
	uint64_t delta;

	int row1[10];
	int row2[10];

	int width=48;
	int offset=FLI;
	int a;
	int best_spr1=-1;
	int best_spr2=-1;
	int spr1s, spr1e;
	int spr2s, spr2e;
	int sprite1, sprite2;

	delta=0;

	/* do normal 6 colums with expanded sprite overlay */
	for(colum=EVEN*5;colum<6;colum++) {
		for(y=0;y<C64YRES;y+=2) {
			status_message("Processing normal colums: Pr0ncessing line #%d on colum %d    \n\033[A",y,colum);

			spr2s=-1; spr2e=15;
			spr1s=-1; spr1e=15;

			/* get current amount of free switches */
			find_free_register_switches(m);

			/* 8 switches left, set sprite color to the last used color, so we do not waste any further switch */
			if(y/2>=SPLITSTART && y/2<=SPLITEND && m->free_switches_max<=8) {
				spr1s=spr1e=spr2s=spr2e=m->sprites[0][y*6+colum];
			}
			/* all okay, no need to save switches */
			else {
				/* if spritecolor for even line is not in use, also permutate spritecolor for even line, else set to static value */
				if(y!=0 && m->sprites[0][y*6+colum]>=0) {
					/* is in use, set sprite1 to a static value */
					spr1s=spr1e=m->sprites[0][y*6+colum];
				}
			}
			best_line_delta=256*256*256;

			for(sprite1=spr1e;sprite1>=spr1s;sprite1--) {
				for(sprite2=spr2e;sprite2>=spr2s;sprite2--) {
					if(NOSPEEDUPSPRITE || (
					   (sprite1<0 || spr1e == spr1s
						      || m->used_colors[y+0][colum*6+offset/8+0][sprite1]
						      || m->used_colors[y+0][colum*6+offset/8+1][sprite1]
						      || m->used_colors[y+0][colum*6+offset/8+2][sprite1]
						      || m->used_colors[y+0][colum*6+offset/8+3][sprite1]
						      || m->used_colors[y+0][colum*6+offset/8+4][sprite1]
						      || m->used_colors[y+0][colum*6+offset/8+5][sprite1]) &&
					   (sprite2<0 || spr2e == spr2s
						      || m->used_colors[y+1][colum*6+offset/8+0][sprite2]
						      || m->used_colors[y+1][colum*6+offset/8+1][sprite2]
						      || m->used_colors[y+1][colum*6+offset/8+2][sprite2]
						      || m->used_colors[y+1][colum*6+offset/8+3][sprite2]
						      || m->used_colors[y+1][colum*6+offset/8+4][sprite2]
						      || m->used_colors[y+1][colum*6+offset/8+5][sprite2])
					)) {
						if(sprite1!=m->option_block_spcol && sprite2!=m->option_block_spcol) {
							for(a=0;a<10;a++) { row1[a]=-1; row2[a]=-1; }
							row1[SPRITE]=sprite1;
							row2[SPRITE]=sprite2;
							line_delta=find_best_colors_line(m,colum*width+offset,y,width,row1,row2);
							if(line_delta<best_line_delta) {
								best_line_delta=line_delta;
								best_spr1=row1[SPRITE];
								best_spr2=row2[SPRITE];
							}
						}
					}
				}
				/* perfekt match, no need to try further */
				if(best_line_delta==0) break;
			}
			row1[SPRITE]=best_spr1;
			row2[SPRITE]=best_spr2;
			/* now finally recalculate with best sprite colors */
			find_best_colors_line(m,colum*width+offset,y,width,row1,row2);
			if(spr1s!=spr1e) {
				if(y!=0) m->sprites[0][(y-1)*6+colum]=best_spr1;
				m->sprites[0][(y+0)*6+colum]=best_spr1;
			}

			m->sprites[0][(y+1)*6+colum]=best_spr2;
			if(y<C64YRES-2) m->sprites[0][(y+2)*6+colum]=best_spr2;

			delta+=best_line_delta;
		}
	}
	status_message("\n");

	/* column 39 */
	for(y=0;y<C64YRES;y+=2) {
		status_message("Processing colum 39: Pr0ncessing line #%d   \n\033[A",y);
		for(a=0;a<10;a++) { row1[a]=-1; row2[a]=-1; }
		best_line_delta=find_best_colors_line(m,C64XRES-8,y,8,row1,row2);
		delta+=best_line_delta;
	}
	status_message("\n");
	return delta;
}

int flibug_col_is_used(MufflonContext *m, int y, int col) {
	if(col<0) return 1;
	if(m->used_colors[y+0][0][col]) return 1;
	if(m->used_colors[y+0][1][col]) return 1;
	if(m->used_colors[y+0][2][col]) return 1;
	if(m->used_colors[y+1][0][col]) return 1;
	if(m->used_colors[y+1][1][col]) return 1;
	if(m->used_colors[y+1][2][col]) return 1;
	return 0;
}

void flibug_optimize_colors(MufflonContext* m, int y, int* row, int* test_col, uint64_t best) {
	int i,j;
	int temp;
	int swaps=0;
	uint64_t new_delta;

	/* now delete/preset each color once and see if it makes a difference, if not, kill color */
	for(i=0;i<4;i++) {
		/* save color */
		temp=row[FLISPRITE+i];
		/* clear color */
		row[FLISPRITE+i]=-1;
		if(find_best_colors_line(m,0,y,24,row,row)>best) row[FLISPRITE+i]=temp; /*restore color */
		else { test_col[i]=0; }
		/* set color to preset color */
		row[FLISPRITE+i]=m->fb_sprite_col[0][i][y];
		if(find_best_colors_line(m,0,y,24,row,row)>best) row[FLISPRITE+i]=temp; /*restore color */
		else { test_col[i]=0; }
	}
	debug_message("needed cols:    %02x %02x %02x %02x\n",row[FLISPRITE+0],row[FLISPRITE+1],row[FLISPRITE+2],row[FLISPRITE+3]);
	/* voila, now we have the colors we _really_ need */
	//XXX TODO, also swap with ink + paper at pos 0, 8, 16 and see if result differs, if not, sprite can be freed, or 0xf be pushed to ink/paper
	/* now swap where necessary to save more switches do x times (2 times should be enough however -> n-1) */
	swaps=0;
	for(i=0;i<4;i++) {
		for(j=0;j<4;j++) {
			if(i!=j && (row[FLISPRITE+i]==m->fb_sprite_col[0][j][y])) {
				//swap: cols should be swapped, except for preste cols, as we would need a set to swap
				if(test_col[i]==0 || test_col[j]==0) {}
				else {
					/* swap in row */
					swap(&row[FLISPRITE+i],&row[FLISPRITE+j]);
					new_delta=find_best_colors_line(m,0,y,24,row,row);
					if(new_delta>best) {
						/* swap back */
						swap(&row[FLISPRITE+i],&row[FLISPRITE+j]);
					}
					else {
						/* swap successful, so also change order of flags too so nothing goes wrong later on */
						if(test_col[i]!=0 && test_col[j]!=0) swap(&test_col[i],&test_col[j]);
						debug_message("new order:      %02x %02x %02x %02x\n",row[FLISPRITE+0],row[FLISPRITE+1],row[FLISPRITE+2],row[FLISPRITE+3]);
						swaps++;
					}
				}
			}
		}
	}
	/* now AGAIN delete/preset each color once and see if it makes a difference, if not, kill color, now we can really save switches by that */
	for(i=0;i<4;i++) {
		/* save color */
		temp=row[FLISPRITE+i];
		/* clear color */
		row[FLISPRITE+i]=-1;
		if(find_best_colors_line(m,0,y,24,row,row)>best) row[FLISPRITE+i]=temp; /*restore color */
		else { test_col[i]=0; }
			/* set color to preset color */
		row[FLISPRITE+i]=m->fb_sprite_col[0][i][y];
		if(find_best_colors_line(m,0,y,24,row,row)>best) row[FLISPRITE+i]=temp; /*restore color */
		else { test_col[i]=0; }
	}
}

int find_last_available_switch(MufflonContext* m, int y, int column, int* newy) {
	int ypos;
	*newy=y+2;
	for(ypos=y;ypos>=0;ypos-=2) {
	//	printf("y: %d  ypos: %d  switches: %d  fb_col: %d\n",ypos,*newy,m->free_switches_temp[0][ypos/2],m->fb_sprite_col[0][column][ypos]);
		/* end if any color is set, applies only for preceeding lines */
		if(ypos!=y && m->fb_sprite_col[0][column][ypos]>=0) break;
		/* as long as there are free switches in current line, update newy */
		if(m->free_switches_temp[0][ypos/2]>0) *newy=ypos;
	}
	/* have we found anything? */
	if(*newy>y) return 0;
	return 1;
}

uint64_t find_best_colors_flibug(MufflonContext *m) {
	/* start positions for spritecolors during bruteforce */
	int spr_start[4];
	/* end positions for spritecolors during bruteforce */
	int spr_end[4];

	/* used sprite colors previous line 0=hires, 1=multi1, 2=multi2, 3=multi4 */
	int col_preset[4];

	/* flags for colors to be tested */
	int test_col[4];

	/* some vars for loops */
	int i,j,k;
	/* ypos */
	int y;

	/* counters for bruteforcing */
	int sprite=-1;
	int multi1=-1;
	int multi2=-1;
	int multi3=-1;

	/* delta stuff */
	uint64_t best_line_delta=256*256*256;
	uint64_t best_test_line_delta=256*256*256;
	uint64_t line_delta;
	uint64_t delta=0;

	/* colors in a row used for each bruteforce attempt */
	int row1[10];
	int best_row1[10];
	int best_free_ypos[4];
	int best_free_cols;

	/* results */
	int best[4]={0};

	/* temp vars */
	int free_cols;
	int free_ypos[4];

	int test;
	int test_pattern[24][4] = {
//			{1,0,2,3},

			{0,1,2,3},
			{0,1,3,2},
			{0,2,1,3},
			{0,2,3,1},
			{0,3,1,2},
			{0,3,2,1},

			{1,2,3,0},
			{1,2,0,3},
			{1,3,2,0},
			{1,3,0,2},
			{1,0,2,3},
			{1,0,3,2},

			{2,3,0,1},
			{2,3,1,0},
			{2,0,3,1},
			{2,0,1,3},
			{2,1,3,0},
			{2,1,0,3},

			{3,0,1,2},
			{3,0,2,1},
			{3,1,0,2},
			{3,1,2,0},
			{3,2,0,1},
			{3,2,1,0}
	};

	for(y=0;y<C64YRES;y+=2) {
		debug_message("line $%02x\n",y);

		best_free_cols = 0;
		best_line_delta=256*256*256;
		for(test=0;test<24;test++) {
			free_cols=0;
			/* make a copy of free_switches_flibug-table */
			for(j=0;j<C64YRES/2;j++) m->free_switches_temp[0][j]=m->free_switches_flibug[0][j];
			for(k=0;k<4;k++) {
				i=test_pattern[test][k];
				test_col[i]=find_last_available_switch(m,y,i,&free_ypos[i]);
//				debug_message("result: y: %d  test_col[%d]: %d  free_ypos[%d]: %d  switches: %d\n",y,i,test_col[i],i,free_ypos[i],m->free_switches_temp[0][free_ypos[i]/2]);
				if(test_col[i]) {
					/* always keep 8 switches for screensplit free, until splitarea ends */
					if(m->free_switches_max-free_cols>8 || (y/2<SPLITSTART && y/2>SPLITEND)) {
						/* occupy switch in temp-table so it can't be selected again */
						m->free_switches_temp[0][free_ypos[i]/2]--;
						free_cols++;
					}
				}

				/* load previous colors as preset */
				if(y==0) col_preset[i]=-1;
				else col_preset[i]=m->fb_sprite_col[0][i][y];

				/* now set start/stop positions for our colors */
				/* color is flagged for test, so set full range */
				if(test_col[i]>0) {
					/* check $0f as color in AFLI lines */
					if((y&7)==0) spr_start[i]=15;
					/* else don't as afli can display $0f only anyway */
					else spr_start[i]=14;
					spr_end[i]=-1;
				}
				/* else test preset color only */
				else {
					/* if preset is $0f and we are in non afli line then don't test at all */
					if((y&7)>=2 && col_preset[i]==15) spr_start[i]=spr_end[i]=-1;
					else spr_start[i]=spr_end[i]=col_preset[i];

				}
			}
//			//XXX for future expansion
//			free_next=5;
//			/* get free switches for the upcoming line, so that we can accumulate lines in case of no available switches */
//			if(y<C64YRES-2) free_next=m->free_switches_flibug[0][(y+2)/2];

			debug_message("max free switches: y:$%02x, max:%d, %d%d%d%d %d  $%02x $%02x $%02x $%02x\n",y,m->free_switches_max,test_col[0],test_col[1],test_col[2],test_col[3],free_cols,free_ypos[0],free_ypos[1],free_ypos[2],free_ypos[3]);
			debug_message("presets:        %02x %02x %02x %02x\n",col_preset[0],col_preset[1],col_preset[2],col_preset[3]);

			/* now bruteforce and all this shit */
			best_test_line_delta=256*256*256;
			for(sprite=spr_start[0];sprite>=spr_end[0];sprite--) {
				status_message("Processing fli bug: Pr0ncessing line #%d   %d %d %d %d    \n\033[A",y,sprite,multi1,multi2,multi3);
				for(multi1=spr_start[1];multi1>=spr_end[1];multi1--) {
						for(multi2=spr_start[2];multi2>=spr_end[2];multi2--) {
						for(multi3=spr_start[3];multi3>=spr_end[3];multi3--) {
							/* only proceed if choosen color is -1 or used */
						   	if(NOSPEEDUPFLIBUG || spr_start[0]==spr_end[0] || spr_start[1]==spr_end[1] || spr_start[2]==spr_end[2] || spr_start[3]==spr_end[3] ||
								(flibug_col_is_used(m,y,sprite) && flibug_col_is_used(m,y,multi1) && flibug_col_is_used(m,y,multi2) && flibug_col_is_used(m,y,multi3))) {
								row1[FLISPRITE]=sprite;
								row1[FLISPRITEM1]=multi1;
								row1[FLISPRITEM2]=multi2;
								row1[FLISPRITEM3]=multi3;

								line_delta=find_best_colors_line(m,0,y,24,row1,row1);
								/* count in following line if no switches available */
								//if(!free_next) {
								//	line_delta+=(find_best_colors_line(m,0,y+2,24,row1,row1)/(free_next+1));
								//}
								//XXX calc num switches?
								//if needed_switches < best?
								//would save swapping and optimization later on!
								if(line_delta<=best_test_line_delta) {
									/* prefer a solution that does not use 0xf as spritecolor */
									if(line_delta<best_test_line_delta || (line_delta==best_test_line_delta && sprite!=15 && multi1!=15 && multi2!=15 && multi3!=15 && (y&7)>=2)) {
										best_test_line_delta=line_delta;
										best[0]=row1[FLISPRITE];
										best[1]=row1[FLISPRITEM1];
										best[2]=row1[FLISPRITEM2];
										best[3]=row1[FLISPRITEM3];
									}
								}
								/* no need to test further dude */
								if(best_test_line_delta==0) { spr_start[0]=-2; spr_start[1]=-2; spr_start[2]=-2; spr_start[3]=-2; }
							}
						}
					}
				}
			}

			if(best_test_line_delta<best_line_delta) {
				best_line_delta=best_test_line_delta;

				/* now we have the best colors in best[] */
				/* time to optimize! */

				/* due to optimizing we can have colors with a value set, but being unused, try to sort them out */

				/* for that, first prepare the colors to calculate on */
				best_row1[FLISPRITE]=best[0];
				best_row1[FLISPRITEM1]=best[1];
				best_row1[FLISPRITEM2]=best[2];
				best_row1[FLISPRITEM3]=best[3];

				best_free_ypos[0]=free_ypos[0];
				best_free_ypos[1]=free_ypos[1];
				best_free_ypos[2]=free_ypos[2];
				best_free_ypos[3]=free_ypos[3];

				best_free_cols = free_cols;

				debug_message("new cols:       %02x %02x %02x %02x\n",best_row1[FLISPRITE+0],best_row1[FLISPRITE+1],best_row1[FLISPRITE+2],best_row1[FLISPRITE+3]);

				flibug_optimize_colors(m,y,best_row1,test_col,best_test_line_delta);

				debug_message("final cols:     %02x %02x %02x %02x\n",best_row1[FLISPRITE+0],best_row1[FLISPRITE+1],best_row1[FLISPRITE+2],best_row1[FLISPRITE+3]);
			}

			/* no need to bother in those cases */
			if(!m->option_multipass || best_free_cols==4 || best_free_cols==0 || best_test_line_delta == 0) break;
		}

		delta+=find_best_colors_line(m,0,y,24,best_row1,best_row1);

		/* now set the switches either in previous or current line */
		for(i=0;i<4;i++) {
			/* but no need to set if col is same as before (the reason why we sorted colors in the previous step) */
			debug_message("best_row1: %02x fb_sprite: %02x \n",best_row1[FLISPRITE+i],m->fb_sprite_col[0][i][y]);
			if(best_row1[FLISPRITE+i]>=0 && best_row1[FLISPRITE+i]!=m->fb_sprite_col[0][i][y] && y>0) {
				if(best_free_cols>0) {
					if(m->free_switches_flibug[0][best_free_ypos[i]/2]>0) {
						if(best_free_ypos[i]/2>=SPLITSTART && best_free_ypos[i]/2<=SPLITEND) m->free_switches_max--;
						if(best_free_ypos[i]/2!=0) set_free_register(m, best_free_ypos[i]/2, (best_row1[FLISPRITE+i]&0xf)|switch_vals[i]);
						best_free_cols--;
						/* set new color, even -1 */
						for(j=best_free_ypos[i];j<C64YRES;j++) m->fb_sprite_col[0][i][j]=best_row1[FLISPRITE+i];
					}
					else {
						error_message("Not enough switches available for this action, should not happen2\n");
					}
				}
				else {
					error_message("Not enough switches available for this action, should not happen\n");
				}
			}
		}
	}
	status_message("\n");
	return delta;
}

void render(MufflonContext *m) {
	uint8_t bmp_mask=0;
	uint8_t spr_mask=0;
	uint8_t hspr_mask=0;
	uint8_t mspr_mask=0;

	int x,y;
	int cols[10]={-1};
	int a;

	int col;
	int start=0;

	if(!m->option_flibug) start=FLI;

	for(y=0;y<C64YRES;y++) {
		for(x=0;x<C64XRES;x++) {
			/* evaluate combinations with final colours */
			//XXX move away from here, to a completely evaluate beforehand and resort/deflicker if needed
			//but then we would need a get_colors(x,y,cols*) function to make result_map work again;

			if((x&7)==0) {
				for(a=0;a<10;a++) cols[a]=-1;
				cols[INK]   =m->inks[0][y*C64XRES/8+x/8];
				cols[PAPER] =m->papers[0][y*C64XRES/8+x/8];
				if(x<FLI) {
					cols[FLISPRITE]=m->fb_sprite_col[0][0][y];
					cols[FLISPRITEM1]=m->fb_sprite_col[0][1][y];
					cols[FLISPRITEM2]=m->fb_sprite_col[0][2][y];
					cols[FLISPRITEM3]=m->fb_sprite_col[0][3][y];
				}
				else if((x-FLI)<(6*48)) {
					cols[SPRITE]=m->sprites[0][y*6+(x-FLI)/48];
				}
				find_best_combinations(m, x, y, cols);
			}

			/* build all kind of maps */
			col=cols[m->combinations[0][y*C64XRES+x]];
			if(col<0) col=0;
			if(col>15) col=0;
			m->result_map[(y*C64XRES+x)*3+0]=m->dest_palette[col][0];
			m->result_map[(y*C64XRES+x)*3+1]=m->dest_palette[col][1];
			m->result_map[(y*C64XRES+x)*3+2]=m->dest_palette[col][2];
		}
		for(x=start;x<C64XRES;x++) {
			/* bitmap is hires, shift 1 */
			bmp_mask<<=1;
			/* flibug sprite 1 is also hires, so shift by 1 */
			hspr_mask<<=1;

			/* shift normal sprites + flibug multicol sprite each 2 pixels */
			if((x&1)==0) {
				spr_mask<<=1;	//shift by one (expanded)
				mspr_mask<<=2;	//shift by two (multicol)
			}
			switch(m->combinations[0][y*C64XRES+x]) {
				case INK:
					bmp_mask|=1;
				break;
				case PAPER:
					/* doing nothing is enough to make paper appear */
				break;
				case SPRITE:
					spr_mask|=1;
				break;
				case FLISPRITEM1:
					mspr_mask|=1;
				break;
				case FLISPRITEM2:
					mspr_mask|=3;
				break;
				case FLISPRITEM3:
					mspr_mask|=2;
				break;
				case FLISPRITE:
					hspr_mask|=1;
				break;
			}
			if((x&7)==7) {
				m->hires_bitmap[0][y/8*C64XRES+(x-7)+(y&7)]=bmp_mask;
				bmp_mask=0;
			}
			if(((x-FLI)&15)==15 && x>=FLI && x<C64XRES-8) {
				m->sprite_bitmap[0][y*3*6+(x-FLI-15)/16]=spr_mask;
				spr_mask=0;
			}
			if((x&7)==7 && x<FLI) {
				m->fb_hsprite_bitmap[0][y*3+(x-7)/8]=hspr_mask;
				m->fb_msprite_bitmap[0][y*3+(x-7)/8]=mspr_mask;
				hspr_mask=0;
				mspr_mask=0;
			}
		}
	}
}

void interlace_find_free_register_switches(MufflonContext *m) {
	int frame;
	int y,b;
	int last;
	int temp;
	int pos;
	int offsets[5]={0x06,0x0b,0x10,0x15,0x1a};
	for(frame=0;frame<2;frame++) {
		last=-1;
		pos=0;
		for(b=0;b<5;b++) {
			for(y=0x3d;y<=0x52;y++) {
				temp=m->sprite_col_tab[frame][y*6+b];
				if(last==temp && y>0x3d && pos<7) {
					m->switch_ypos[frame][pos]=y;
					m->switch_xpos[frame][pos]=offsets[b];
					pos++;
				}
				last=temp;
			}
		}
	}
/*
	for(frame=0;frame<2;frame++) {
		reg=0x41;
		last=-1;
		for(b=0;b<6;b++) {
			for(y=0x3d;y<=0x52;y++) {
				if(reg<0x50) {
					temp=m->sprite_col_tab[frame][y*6+b];
					if(y>0x3d) {
						if(last==temp) {
							m->sprite_col_tab[frame][y*6+b]=reg;
							reg+=2;
						}
					}
					last=temp;
				}
			}
		}
		for(y=0x3d;y<=0x52;y++) {
			for(b=0;b<6;b++) {
				if(m->sprite_col_tab[frame][y*6+b]>=0x40) {
//					info_message("%02x* ",m->sprite_col_tab[frame][y*6+b]);
				}
				else {
//					info_message("%02x  ",m->sprite_col_tab[frame][y*6+b]);
				}
			}
//			info_message("\n");
		}
	}
*/
}

int compare(MufflonContext* m, int x, int y, int col1, int col2) {
	int rdiff;
	int gdiff;
	int bdiff;

	int a;
	double lum=0;

	if(col1<0 || col2<0 || (a=m->allowed_mix_map[col1][col2])==-1) return 65535*256;
	rdiff=m->data[(y*C64XRES+x)*3+0]-m->mixes_r[a];
	gdiff=m->data[(y*C64XRES+x)*3+1]-m->mixes_g[a];
	bdiff=m->data[(y*C64XRES+x)*3+2]-m->mixes_b[a];

	if(rdiff<0) rdiff=0-rdiff;
	if(gdiff<0) gdiff=0-gdiff;
	if(bdiff<0) bdiff=0-bdiff;

	return rdiff*rdiff+gdiff*gdiff+bdiff*bdiff+lum*lum;
}

//XXX add diff parameter to compare and return 0 if compare failure instead of 65535*256 
int compare_with_luma(MufflonContext* m, int x, int y, int col1, int col2) {
	int rdiff;
	int gdiff;
	int bdiff;

	int a;
	double lum=0;
	int result;
	int r1,g1,b1;
	int r2,g2,b2;

	if(col1!=col2 && m->option_solid_only==1) return 65535*256;
	if(x>=FLI) {
		if(col1<0 || col2<0 || (a=m->allowed_mix_map[col1][col2])==-1) return 65535*256;
		r1=m->mixes_r[a];
		g1=m->mixes_g[a];
		b1=m->mixes_b[a];
	}
	else {
		if(col1<0 || col2<0) return 65535*256;
		r1=(m->dest_palette[col1][0]+m->dest_palette[col2][0])/2;
		g1=(m->dest_palette[col1][1]+m->dest_palette[col2][1])/2;
		b1=(m->dest_palette[col1][2]+m->dest_palette[col2][2])/2;
	}

	r2=m->data[(y*C64XRES+x)*3+0];
	g2=m->data[(y*C64XRES+x)*3+1];
	b2=m->data[(y*C64XRES+x)*3+2];

	rdiff=r1-r2;
	gdiff=g1-g2;
	bdiff=b1-b2;

	if(rdiff<0) rdiff=0-rdiff;
	if(gdiff<0) gdiff=0-gdiff;
	if(bdiff<0) bdiff=0-bdiff;

	result=rdiff*rdiff+gdiff*gdiff+bdiff*bdiff;

	//XXX ggf. auch noch helligkeitsdiff zu 4 benachbarten pixeln aufrechnen mit gewissem %satz (20%?) aber erchenaufwand, junge, echt jetz
	if(result>10) {
		lum=(double)rdiff*0.30+(double)gdiff*0.59+(double)bdiff*0.11;
		result=lum*lum*3;
//		if(lum*lum*3<result) result=lum*lum*3;
	}

//	/* weighting linear */
//	if(weighting==1) {
//		return rdiff+gdiff+bdiff;
//	}
//	/* weighting color distance by square */
//	else {
//		return rdiff*rdiff+gdiff*gdiff+bdiff*bdiff;
//	}
	return result;
}

/* put all values for one evaluation into a extra type/struct (ink1/2, paper and so on) */
int eval_flick(MufflonContext* m, int frame_1, int frame_2, int ink1, int ink2, int paper1, int paper2, int sprite1, int sprite2) {
	int lum11=0;
	int lum12=0;
	int lum21=0;
	int lum22=0;
	int lum=65536*256;
	int lumx=0,lumy=0;
	switch(frame_1) {
		case COMB_I_I:
			if(ink1>=0) lum11=m->luma[ink1];
			if(ink2>=0) lum12=m->luma[ink2];
		break;
		case COMB_I_P:
			if(ink1>=0) lum11=m->luma[ink1];
			if(paper2>=0) lum12=m->luma[paper2];
		break;
		case COMB_P_I:
			if(paper1>=0) lum11=m->luma[paper1];
			if(ink2>=0) lum12=m->luma[ink2];
		break;
		case COMB_P_P:
			if(paper1>=0) lum11=m->luma[paper1];
			if(paper2>=0) lum12=m->luma[paper2];
		break;
		case COMB_P_S:
			if(paper1>=0) lum11=m->luma[paper1];
			if(sprite2>=0) lum12=m->luma[sprite2];
		break;
		case COMB_S_P:
			if(sprite1>=0) lum11=m->luma[sprite1];
			if(paper2>=0) lum12=m->luma[paper2];
		break;
		case COMB_S_S:
			if(sprite1>=0) lum11=m->luma[sprite1];
			if(sprite2>=0) lum12=m->luma[sprite2];
		break;
		case COMB_I_S:
			if(ink1>=0) lum11=m->luma[ink1];
			if(sprite2>=0) lum12=m->luma[sprite2];
		break;
		case COMB_S_I:
			if(sprite1>=0) lum11=m->luma[sprite1];
			if(ink2>=0) lum12=m->luma[ink2];
		break;
		default:
		break;
	}
	switch(frame_2) {
		case COMB_I_I:
			if(ink1>=0) lum21=m->luma[ink1];
			if(ink2>=0) lum22=m->luma[ink2];
		break;
		case COMB_I_P:
			if(ink1>=0) lum21=m->luma[ink1];
			if(paper2>=0) lum22=m->luma[paper2];
		break;
		case COMB_P_I:
			if(paper1>=0) lum21=m->luma[paper1];
			if(ink2>=0) lum22=m->luma[ink2];
		break;
		case COMB_P_P:
			if(paper1>=0) lum21=m->luma[paper1];
			if(paper2>=0) lum22=m->luma[paper2];
		break;
		case COMB_P_S:
			if(paper1>=0) lum21=m->luma[paper1];
			if(sprite2>=0) lum22=m->luma[sprite2];
		break;
		case COMB_S_P:
			if(sprite1>=0) lum21=m->luma[sprite1];
			if(paper2>=0) lum22=m->luma[paper2];
		break;
		case COMB_S_S:
			if(sprite1>=0) lum21=m->luma[sprite1];
			if(sprite2>=0) lum22=m->luma[sprite2];
		break;
		case COMB_I_S:
			if(ink1>=0) lum21=m->luma[ink1];
			if(sprite2>=0) lum22=m->luma[sprite2];
		break;
		case COMB_S_I:
			if(sprite1>=0) lum21=m->luma[sprite1];
			if(ink2>=0) lum22=m->luma[ink2];
		break;
		default:
		break;
	}

	lumx=(lum11-lum22);
	lumy=(lum12-lum21);

	if(lumx<0) lumx=0-lumx;
	if(lumy<0) lumy=0-lumy;

	lum=lumx+lumy;
	return lum;
}

double calc_flick(MufflonContext* m, int col1, int col2, int col3, int col4) {
	int lum1,lum2;

	lum1=(m->luma[col1]+m->luma[col3])/2;
	lum2=(m->luma[col2]+m->luma[col4])/2;

	return abs(lum1-lum2);

}

int suggest_second(MufflonContext* m, int x, int y, int ink1, int ink2, int paper1, int paper2, int sprite1, int sprite2, int* result, int last, int* bflick) {
	int best=-1;
	int diff;
	int bestflick=-1;
	int flick;

	x++;

	*result=-1;

	if(ink1>=0 && ink2>=0) {
		if(last==COMB_I_I || last==COMB_I_P || last==COMB_P_I || last==COMB_I_S || last==COMB_S_I  || last==COMB_P_P || last==COMB_S_S || last==COMB_P_S || last==COMB_S_P) {
			diff=compare_with_luma(m,x,y,ink1,ink2);
			if(diff<best || best<0) { best=diff; *result=COMB_I_I; }
			if(diff==best) {
				flick=eval_flick(m,last,COMB_I_I,ink1,ink2,paper1,paper2,sprite1,sprite2);
				if(flick<bestflick || bestflick<0) { *result=COMB_I_I; bestflick=flick; }
			}
		}
	}
	if(paper1>=0 && paper2>=0) {
		if(last==COMB_I_I || last==COMB_I_P || last==COMB_P_I || last==COMB_P_P) {
			diff=compare_with_luma(m,x,y,paper1,paper2);
			if(diff<best || best<0) { best=diff; *result=COMB_P_P; }
			if(diff==best) {
				flick=eval_flick(m,last,COMB_P_P,ink1,ink2,paper1,paper2,sprite1,sprite2);
				if(flick<bestflick || bestflick<0) { *result=COMB_P_P; bestflick=flick; }
			}
		}
	}
	if(ink1>=0 && paper2>=0) {
		if(last==COMB_I_I || last==COMB_I_P || last==COMB_P_I || last==COMB_P_P || last==COMB_S_I || last==COMB_S_P) {
			diff=compare_with_luma(m,x,y,ink1,paper2);
			if(diff<best || best<0) { best=diff; *result=COMB_I_P; }
			if(diff==best) {
				flick=eval_flick(m,last,COMB_I_P,ink1,ink2,paper1,paper2,sprite1,sprite2);
				if(flick<bestflick || bestflick<0) { *result=COMB_I_P; bestflick=flick; }
			}
		}
	}
	if(ink2>=0 && paper1>=0) {
		if(last==COMB_I_I || last==COMB_P_I || last==COMB_I_P || last==COMB_P_P || last==COMB_I_S || last==COMB_P_S) {
			diff=compare_with_luma(m,x,y,paper1,ink2);
			if(diff<best || best<0) { best=diff; *result=COMB_P_I; }
			if(diff==best) {
				flick=eval_flick(m,last,COMB_P_I,ink1,ink2,paper1,paper2,sprite1,sprite2);
				if(flick<bestflick || bestflick<0) { *result=COMB_P_I; bestflick=flick; }
			}
		}
	}
	if(ink1>=0 && sprite2>=0) {
		if(last==COMB_I_I || last==COMB_I_S || last==COMB_S_I || last==COMB_S_S || last==COMB_P_S || last==COMB_P_I) {
			diff=compare_with_luma(m,x,y,ink1,sprite2);
			if(diff<best || best<0) { best=diff; *result=COMB_I_S; }
			if(diff==best) {
				flick=eval_flick(m,last,COMB_I_S,ink1,ink2,paper1,paper2,sprite1,sprite2);
				if(flick<bestflick || bestflick<0) { *result=COMB_I_S; bestflick=flick; }
			}
		}
	}
	if(ink2>=0 && sprite1>=0) {
		if(last==COMB_I_I || last==COMB_S_I || last==COMB_I_S || last==COMB_S_S || last==COMB_S_P || last==COMB_I_P) {
			diff=compare_with_luma(m,x,y,sprite1,ink2);
			if(diff<best || best<0) { best=diff; *result=COMB_S_I; }
			if(diff==best) {
				flick=eval_flick(m,last,COMB_S_I,ink1,ink2,paper1,paper2,sprite1,sprite2);
				if(flick<bestflick || bestflick<0) { *result=COMB_S_I; bestflick=flick; }
			}
		}
	}
	if(sprite1>=0 && sprite2>=0) {
		if(last==COMB_I_I || last==COMB_I_S || last==COMB_S_I || last==COMB_S_S) {
			diff=compare_with_luma(m,x,y,sprite1,sprite2);
			if(diff<best || best<0) { best=diff; *result=COMB_S_S; }
			if(diff==best) {
				flick=eval_flick(m,last,COMB_S_S,ink1,ink2,paper1,paper2,sprite1,sprite2);
				if(flick<bestflick || bestflick<0) { *result=COMB_S_S; bestflick=flick; }
			}
		}
	}
	if(paper1>=0 && sprite2>=0) {
		if(last==COMB_I_I || last==COMB_P_S || last==COMB_I_S || last==COMB_P_I) {
			diff=compare_with_luma(m,x,y,paper1,sprite2);
			if(diff<best || best<0) { best=diff; *result=COMB_P_S; }
			if(diff==best) {
				flick=eval_flick(m,last,COMB_P_S,ink1,ink2,paper1,paper2,sprite1,sprite2);
				if(flick<bestflick || bestflick<0) { *result=COMB_P_S; bestflick=flick; }
			}
		}
	}
	if(paper2>=0 && sprite1>=0) {
		if(last==COMB_I_I || last==COMB_S_P || last==COMB_S_I || last==COMB_I_P) {
			diff=compare_with_luma(m,x,y,sprite1,paper2);
			if(diff<best || best<0) { best=diff; *result=COMB_S_P; }
			if(diff==best) {
				flick=eval_flick(m,last,COMB_S_P,ink1,ink2,paper1,paper2,sprite1,sprite2);
				if(flick<bestflick || bestflick<0) { *result=COMB_S_P; bestflick=flick; }
			}
		}
	}
	*bflick=bestflick;
	return best;
}

int interlace_find_best_combination(MufflonContext* m, int x, int y, int ink1, int  ink2, int paper1, int paper2, int sprite1, int sprite2, int* frame_1, int* frame_2, int* lum) {
	int comb;
	int64_t bestflick=-1;
	int64_t best=-1;
	int64_t flick;
	int64_t diff;
	int num;
	int xx;

	*lum=0;

	num=ARRAY_ELEMS(combinations_muifli);

	best=-1;
	for(comb=0;comb<num;comb+=2) {
		diff=0;
		flick=0;
		for(xx=0;xx<2;xx++) {
			switch(combinations_muifli[comb+xx]) {
				case COMB_I_I:
					diff+=compare_with_luma(m,x+xx,y,ink1,ink2);
				break;
				case COMB_P_P:
					diff+=compare_with_luma(m,x+xx,y,paper1,paper2);
				break;
				case COMB_S_S:
					diff+=compare_with_luma(m,x+xx,y,sprite1,sprite2);
				break;
				case COMB_I_P:
					diff+=compare_with_luma(m,x+xx,y,ink1,paper2);
				break;
				case COMB_P_I:
					diff+=compare_with_luma(m,x+xx,y,paper1,ink2);
				break;
				case COMB_I_S:
					diff+=compare_with_luma(m,x+xx,y,ink1,sprite2);
				break;
				case COMB_S_I:
					diff+=compare_with_luma(m,x+xx,y,sprite1,ink2);
				break;
				case COMB_P_S:
					diff+=compare_with_luma(m,x+xx,y,paper1,sprite2);
				break;
				case COMB_S_P:
					diff+=compare_with_luma(m,x+xx,y,sprite1,paper2);
				break;
			}
		}
		if(diff<best || best<0) {
			best=diff;
			*frame_1=combinations_muifli[comb+0];
			*frame_2=combinations_muifli[comb+1];
		}
		//XXX else if?
		if(diff==best) {
			flick=eval_flick(m,combinations_muifli[comb+0],combinations_muifli[comb+1],ink1,ink2,paper1,paper2,sprite1,sprite2);
			if(flick<bestflick || bestflick<0) {
				bestflick=flick;
				*frame_1=combinations_muifli[comb+0];
				*frame_2=combinations_muifli[comb+1];
			}
		}
	}
	return best;
}

void interlace_find_used_colors(MufflonContext* m) {
	int diff;
	int best;
	int x, y;
	int a;
	int mix=0;
	for(y=0;y<C64YRES;y++) {
		status_message("Searching for used colors per block: Pr0ncessing line #%d\n\033[A",y);
                for(x=0;x<C64XRES;x++) {
			best=-1;
			for(a=0;a<m->mix_count;a++) {
				diff=compare(m,x,y,color_mixes[a][0],color_mixes[a][1]);
				if(diff<best || best<0) {
					best=diff;
					mix=a;
				}
			}
			m->used_colors[y/2][x/8][color_mixes[mix][0]]++;
			m->used_colors[y/2][x/8][color_mixes[mix][1]]++;
		}
	}
	status_message("\n");
}

void convert_muifli(MufflonContext* m) {
	int x=0;
	int y;
	static uint8_t result[64000]={0};
	int frame;

	m->sl_sizex=C64XRES-8;

	interlace_find_used_colors(m);

	if(m->option_bruteforce==0) {
		find_sprite_colors(m);
		find_hires_colors(m);
	}
	else {
		find_sprite_colors_bruteforce(m);
		find_hires_colors(m);
	}
	interlace_render(m);
	make_flick_result_map(m);

	interlace_find_free_register_switches(m);

	for(frame=0;frame<2;frame++) {
		/* copy sprite pointers */
		memcpy(&result[0x02f8+frame*0x8000],&muifli_sprite_pointers[0][0],8);
		memcpy(&result[0x06f8+frame*0x8000],&muifli_sprite_pointers[1][0],8);
		memcpy(&result[0x0af8+frame*0x8000],&muifli_sprite_pointers[2][0],8);
		memcpy(&result[0x0ef8+frame*0x8000],&muifli_sprite_pointers[3][0],8);
		memcpy(&result[0x22f8+frame*0x8000],&muifli_sprite_pointers[4][0],8);
		memcpy(&result[0x26f8+frame*0x8000],&muifli_sprite_pointers[5][0],8);
		memcpy(&result[0x2af8+frame*0x8000],&muifli_sprite_pointers[6][0],8);
		memcpy(&result[0x2ef8+frame*0x8000],&muifli_sprite_pointers[7][0],8);
		memcpy(&result[0x32f8+frame*0x8000],&muifli_sprite_pointers[8][0],8);
		memcpy(&result[0x36f8+frame*0x8000],&muifli_sprite_pointers[9][0],8);
		memcpy(&result[0x3af8+frame*0x8000],&muifli_sprite_pointers[10][0],8);
		memcpy(&result[0x3ef8+frame*0x8000],&muifli_sprite_pointers[11][0],8);

		/* fill blanking sprites with 0xff */
		memset(&result[0x5500+frame*0x8000],0xff,0x40);
		memset(&result[0x1e40+frame*0x8000],0xff,0x40);

		for(y=0;y<=C64YRES/2;y++) {
			result[0x0300+y+frame*0x8000]=m->sprite_col_tab[frame][y*6+0];
			result[0x0380+y+frame*0x8000]=m->sprite_col_tab[frame][y*6+1];
			result[0x0700+y+frame*0x8000]=m->sprite_col_tab[frame][y*6+2];
			result[0x0780+y+frame*0x8000]=m->sprite_col_tab[frame][y*6+3];
			result[0x0b00+y+frame*0x8000]=m->sprite_col_tab[frame][y*6+4];
			result[0x0b80+y+frame*0x8000]=m->sprite_col_tab[frame][y*6+5];
		}

		/* copy first half of hires bitmaps */
		memcpy(&result[0x3f00+frame*0x8000],&m->hires_bitmap[frame][0],0x1400);

		/* copy second half of hires bitmaps */
		memcpy(&result[0x1300+frame*0x8000],&m->hires_bitmap[frame][0x1400],0x0b40);

		/* copy screen lines */
		for(y=0;y<C64YRES/2 && y<C64YRES/2;y++) {
			memcpy(&result[muifli_scram[y]]+frame*0x8000,&m->colormap[frame][y*C64COLUMNS],0x28);
		}

		/* copy sprite bitmap */
		int address1;
		int srow;
		int row=5;
		for(y=0;y<C64YRES;y++) {
			for(x=0;x<(m->sl_sizex-FLI)/8/2;x++) {
				srow=row+(x % 3);
				srow=srow&0x3f;
				/* Extraworschd für Sprite 6 bis Zeile 128 */
				if(x>=5*3 && y<128) {
					address1=0x5300+( (((y/2)&0x7)^0x7)*0x40 )+srow+frame*0x8000;
				}
				else {
					address1=muifli_spram[y/2]+srow+x/3*0x40+frame*0x8000;
				}
				result[address1]=m->sprite_bitmap[frame][y*3*6+x];
			}
			if((y&1)==0) row+=3;
			if(row>0x3f) row=row&0x3f;
			else if(row==0x3f) row=0;
		}

		/* unsure what that data means, but we better copy it as well */
		memcpy(&result[0x1ef8+frame*0x8000],&sprite_conf[0],0x8);
	}
	result[0x7fff]=0xf;

	/* copy displayer code */
	memcpy(&result[0x0f00],&muifli_displayer_code1[0x000],0x300);
	memcpy(&result[0x8f00],&muifli_displayer_code2[0x000],0x300);

	/* copy sprite switches */
	memcpy(&result[0x4fe0-0x2100],&m->switch_ypos[0][0],0x8);
	memcpy(&result[0x4fe8-0x2100],&m->switch_xpos[0][0],0x8);
	memcpy(&result[0xcfe0-0x2100],&m->switch_ypos[1][0],0x8);
	memcpy(&result[0xcfe8-0x2100],&m->switch_xpos[1][0],0x8);

	/* now copy things so we get a saveable file */
	memcpy(&result[0x5600],&result[0xac00],0x2a00);

	result[0x10f1]=m->option_sbugcol;
	result[0x10f7]=m->option_sbugcol;
	result[0x10f8]=m->option_sbugcol;
	result[0x10f9]=m->option_sbugcol;
	result[0x10fa]=m->option_sbugcol;
	result[0x10fb]=m->option_sbugcol;
	result[0x10fc]=m->option_sbugcol;
	result[0x10fd]=m->option_sbugcol;
	result[0x10fe]=m->option_sbugcol;
	result[0x10ff]=m->option_sbugcol;

	write_result(m,result,0xac00);
}

void swap_line(MufflonContext* m, int y, int x) {
	uint8_t byte1, byte2;
	uint8_t frame_1, frame_2;
	uint8_t sbyte1, sbyte2;
	uint8_t sframe_1, sframe_2;
	byte1=m->hires_bitmap[0][y/8*0x140+(x*8)+(y&7)];
	byte2=m->hires_bitmap[1][y/8*0x140+(x*8)+(y&7)];
	frame_1=byte2;
	frame_2=byte1;
	m->hires_bitmap[0][y/8*0x140+(x*8)+(y&7)]=frame_1;
	m->hires_bitmap[1][y/8*0x140+(x*8)+(y&7)]=frame_2;

	if(x>=FLI/8 && x<m->sl_sizex/8) {
		sbyte1=m->sprite_bitmap[0][y*3*6+(x-3)/2];
		sbyte2=m->sprite_bitmap[1][y*3*6+(x-3)/2];
		if((x&1)==1) {
			sframe_1=(sbyte1&0x0f)|(sbyte2&0xf0);
			sframe_2=(sbyte2&0x0f)|(sbyte1&0xf0);
		}
		else {
			sframe_1=(sbyte1&0xf0)|(sbyte2&0x0f);
			sframe_2=(sbyte2&0xf0)|(sbyte1&0x0f);
		}
		m->sprite_bitmap[0][y*3*6+(x-3)/2]=sframe_1;
		m->sprite_bitmap[1][y*3*6+(x-3)/2]=sframe_2;
	}
}

void make_flick_result_map(MufflonContext* m) {
	int x,y;
	int ink1,ink2,paper1,paper2,sprite1,sprite2;
	int shift;
	int col1=0;
	int col2=0;
	int col3=0;
	int col4=0;
	double flick=0;
	uint8_t byte1;
	uint8_t byte2;
	uint8_t sbyte1=0;
	uint8_t sbyte2=0;

	for(y=0;y<C64YRES;y++) {
		for(x=0/8;x<C64XRES/8;x++) {
			ink1=m->colormap[0][y/2*C64XRES/8+x]>>4;
			ink2=m->colormap[1][y/2*C64XRES/8+x]>>4;
			paper1=m->colormap[0][y/2*C64XRES/8+x]&0xf;
			paper2=m->colormap[1][y/2*C64XRES/8+x]&0xf;
			if(x>=FLI/8 && x<m->sl_sizex/8) {
				sprite1=m->sprite_col_tab[0][((y+1)/2)*6+(x-3)/6];
				sprite2=m->sprite_col_tab[1][((y+1)/2)*6+(x-3)/6];
			}
			else {
				sprite1=-1;
				sprite2=-1;
			}
			byte1=m->hires_bitmap[0][y/8*0x140+(x*8)+(y&7)];
			byte2=m->hires_bitmap[1][y/8*0x140+(x*8)+(y&7)];
			if(x>=FLI/8 && x<m->sl_sizex/8) {
				sbyte1=m->sprite_bitmap[0][y*3*6+(x-3)/2];
				sbyte2=m->sprite_bitmap[1][y*3*6+(x-3)/2];
				/* CAUTION!!! mus equal to 1 coz of FLI bug, so we start at x/8!=0) */
				if((x&1)==1) {
					sbyte1>>=4;
					sbyte2>>=4;
				}
				else {
					sbyte1&=0xf;
					sbyte2&=0xf;
				}
			}
			for(shift=0;shift<8;shift++) {
				if((shift&1)==0) {
					flick=0;
					if((byte1&1)==1) col1=ink1;
					else if((sbyte1&1)==1) col1=sprite1;
					else col1=paper1;

					if((byte2&1)==1) col2=ink2;
					else if((sbyte2&1)==1) col2=sprite2;
					else col2=paper2;

					m->result_map[(y*C64XRES+x*8+7-shift)*3+0]=(m->dest_palette[col1][0]+m->dest_palette[col2][0])/2;
					m->result_map[(y*C64XRES+x*8+7-shift)*3+1]=(m->dest_palette[col1][1]+m->dest_palette[col2][1])/2;
					m->result_map[(y*C64XRES+x*8+7-shift)*3+2]=(m->dest_palette[col1][2]+m->dest_palette[col2][2])/2;
				}
				else {
					if((byte1&1)==1) col3=ink1;
					else if((sbyte1&1)==1) col3=sprite1;
					else col3=paper1;

					if((byte2&1)==1) col4=ink2;
					else if((sbyte2&1)==1) col4=sprite2;
					else col4=paper2;

					m->result_map[(y*C64XRES+x*8+7-shift)*3+0]=(m->dest_palette[col3][0]+m->dest_palette[col4][0])/2;
					m->result_map[(y*C64XRES+x*8+7-shift)*3+1]=(m->dest_palette[col3][1]+m->dest_palette[col4][1])/2;
					m->result_map[(y*C64XRES+x*8+7-shift)*3+2]=(m->dest_palette[col3][2]+m->dest_palette[col4][2])/2;

					flick=calc_flick(m,col1,col2,col3,col4);

					m->flick_map[(y*C64XRES+x*8+7-shift-1)*3+0]=flick;
					m->flick_map[(y*C64XRES+x*8+7-shift)*3+0]=flick;
					m->flick_map[(y*C64XRES+x*8+7-shift-1)*3+1]=flick;
					m->flick_map[(y*C64XRES+x*8+7-shift)*3+1]=flick;
					m->flick_map[(y*C64XRES+x*8+7-shift-1)*3+2]=flick;
					m->flick_map[(y*C64XRES+x*8+7-shift)*3+2]=flick;

					sbyte1>>=1;
					sbyte2>>=1;
				}
				byte1>>=1;
				byte2>>=1;
			}
		}
	}
}

void resort_combinations(MufflonContext* m, int x, int y, int* frame_1, int* frame_2, int ink1, int ink2, int paper1, int paper2, int sprite1, int sprite2) {
	int dith;
	int defl=0;

	/* align remaiing pixels if 50/50 and possible */
	//first SI pixel
	if(ink1==ink2 && sprite1==sprite2) {
		dith=y&1;
		if(m->luma[sprite1]>m->luma[ink1]) dith^=1;
		/* for first pixel take second frame into account coz sprite might collide with paper on second pixel */
		if( (*frame_1==COMB_S_I || *frame_1==COMB_I_S) ) {
			if(dith==0) {
				if(*frame_2==COMB_I_S || *frame_2==COMB_S_I || *frame_2==COMB_I_I || *frame_2==COMB_S_S || *frame_2==COMB_I_P) *frame_1=COMB_S_I;
			}
			else {
				if(*frame_2==COMB_I_S || *frame_2==COMB_S_I || *frame_2==COMB_I_I || *frame_2==COMB_S_S || *frame_2==COMB_P_I) *frame_1=COMB_I_S;
			}
		}
	}

	//first PI pixel
	if(ink1==ink2 && paper1==paper2) {
		dith=y&1;
		if(m->luma[paper1]>m->luma[ink1]) dith^=1;
		if( (*frame_1==COMB_P_I || *frame_1==COMB_I_P) ) {
			if(dith==0) {
				if(*frame_2==COMB_I_P || *frame_2==COMB_P_I || *frame_2==COMB_I_I || *frame_2==COMB_P_P || *frame_2==COMB_I_S) *frame_1=COMB_P_I;
			}
			else {
				if(*frame_2==COMB_I_P || *frame_2==COMB_P_I || *frame_2==COMB_I_I || *frame_2==COMB_P_P || *frame_2==COMB_S_I) *frame_1=COMB_I_P;
			}
		}
	}

	//second SI pixel
	if(ink1==ink2 && sprite1==sprite2) {
		dith=y&1;
		if(m->luma[sprite1]>m->luma[ink1]) dith^=1;
		if(*frame_2==COMB_S_I || *frame_2==COMB_I_S) {
			if(dith==0) {
				if(*frame_1==COMB_I_S || *frame_1==COMB_S_I || *frame_1==COMB_I_I || *frame_1==COMB_S_S || *frame_1==COMB_P_I) *frame_2=COMB_I_S;
			}
			else {
				if(*frame_1==COMB_I_S || *frame_1==COMB_S_I || *frame_1==COMB_I_I || *frame_1==COMB_S_S || *frame_1==COMB_I_P) *frame_2=COMB_S_I;
			}
		}
	}

	//second PI pixel
	if(ink1==ink2 && paper1==paper2) {
		dith=y&1;
		if(m->luma[paper1]>m->luma[ink1]) dith^=1;
		if(*frame_2==COMB_P_I || *frame_2==COMB_I_P) {
			if(dith==0) {
				if(*frame_1==COMB_I_P || *frame_1==COMB_P_I || *frame_1==COMB_I_I || *frame_1==COMB_P_P || *frame_1==COMB_S_I) *frame_2=COMB_I_P;
			}
			else {
				if(*frame_1==COMB_I_P || *frame_1==COMB_P_I || *frame_1==COMB_I_I || *frame_1==COMB_P_P || *frame_1==COMB_I_S) *frame_2=COMB_P_I;
			}
		}
	}


	if(ink1==ink2 && paper1==paper2) {
		dith=y&1;
		if( (*frame_1==COMB_P_I && *frame_2==COMB_I_I) ) {
			if(m->luma[paper1]>m->luma[ink1]) { *frame_1=COMB_I_P; *frame_2=COMB_I_I; return; }
			else return;
		}
		if( (*frame_1==COMB_I_P && *frame_2==COMB_I_I) ) {
			if(m->luma[ink1]>m->luma[paper1]) { *frame_1=COMB_P_I; *frame_2=COMB_I_I; return; }
			else return;
		}
		if( (*frame_1==COMB_I_I && *frame_2==COMB_I_P) ) {
			if(m->luma[paper1]>m->luma[ink1]) { *frame_1=COMB_I_I; *frame_2=COMB_P_I; return; }
			else return;
		}
		if( (*frame_1==COMB_I_I && *frame_2==COMB_P_I) ) {
			if(m->luma[ink1]>m->luma[paper1]) { *frame_1=COMB_I_I; *frame_2=COMB_I_P; return; }
			else return;
		}
	}

	if(ink1==ink2 && paper1==paper2) {
		dith=y&1;
		if( (*frame_1==COMB_P_I && *frame_2==COMB_P_P) ) {
			if(m->luma[paper1]>m->luma[ink1]) { *frame_1=COMB_I_P; *frame_2=COMB_P_P; return; }
			else return;
		}
		if( (*frame_1==COMB_I_P && *frame_2==COMB_P_P) ) {
			if(m->luma[ink1]>m->luma[paper1]) { *frame_1=COMB_P_I; *frame_2=COMB_P_P; return; }
			else return;
		}
		if( (*frame_1==COMB_P_P && *frame_2==COMB_I_P) ) {
			if(m->luma[paper1]>m->luma[ink1]) { *frame_1=COMB_P_P; *frame_2=COMB_P_I; return; }
			else return;
		}
		if( (*frame_1==COMB_P_P && *frame_2==COMB_P_I) ) {
			if(m->luma[ink1]>m->luma[paper1]) { *frame_1=COMB_P_P; *frame_2=COMB_I_P; return; }
			else return;
		}
	}

	if(ink1==ink2 && sprite1==sprite2) {
		dith=y&1;
		if( (*frame_1==COMB_S_I && *frame_2==COMB_I_I) ) {
			if(m->luma[sprite1]>m->luma[ink1]) { *frame_1=COMB_I_S; *frame_2=COMB_I_I; return; }
			else return;
		}
		if( (*frame_1==COMB_I_S && *frame_2==COMB_I_I) ) {
			if(m->luma[ink1]>m->luma[sprite1]) { *frame_1=COMB_S_I; *frame_2=COMB_I_I; return; }
			else return;
		}
		if( (*frame_1==COMB_I_I && *frame_2==COMB_I_S) ) {
			if(m->luma[sprite1]>m->luma[ink1]) { *frame_1=COMB_I_I; *frame_2=COMB_S_I; return; }
			else return;
		}
		if( (*frame_1==COMB_I_I && *frame_2==COMB_S_I) ) {
			if(m->luma[ink1]>m->luma[sprite1]) { *frame_1=COMB_I_I; *frame_2=COMB_I_S; return; }
			else return;
		}
	}

	if(ink1==ink2 && sprite1==sprite2) {
		dith=y&1;
		if( (*frame_1==COMB_S_I && *frame_2==COMB_S_S) ) {
			if(m->luma[sprite1]>m->luma[ink1]) { *frame_1=COMB_I_S; *frame_2=COMB_S_S; return; }
			else return;
		}
		if( (*frame_1==COMB_I_S && *frame_2==COMB_S_S) ) {
			if(m->luma[ink1]>m->luma[sprite1]) { *frame_1=COMB_S_I; *frame_2=COMB_S_S; return; }
			else return;
		}
		if( (*frame_1==COMB_S_S && *frame_2==COMB_I_S) ) {
			if(m->luma[sprite1]>m->luma[ink1]) { *frame_1=COMB_S_S; *frame_2=COMB_S_I; return; }
			else return;
		}
		if( (*frame_1==COMB_S_S && *frame_2==COMB_S_I) ) {
			if(m->luma[ink1]>m->luma[sprite1]) { *frame_1=COMB_S_S; *frame_2=COMB_I_S; return; }
			else return;
		}
	}

	/* the 25/75 mixes */
/*	if( (*frame_1==COMB_P_I && *frame_2==COMB_I_I) || (*frame_1==COMB_I_P && *frame_2==COMB_I_I) || (*frame_1==COMB_I_I && *frame_2==COMB_P_I) || (*frame_1==COMB_I_I && *frame_2==COMB_I_P)) {
		if(ink1==ink2 && paper1==paper2) {
			dith=y&1;
			if(dith==0) {
				if(m->luma[paper1]>m->luma[ink1]) { *frame_1=COMB_I_P; *frame_2=COMB_I_I; return; }
				else { *frame_1=COMB_P_I; *frame_2=COMB_I_I; return; }
			}
			else {
				if(m->luma[paper1]>m->luma[ink1]) { *frame_1=COMB_I_I; *frame_2=COMB_P_I; return; }
				*frame_1=COMB_I_I; *frame_2=COMB_I_P; return;
			}
		}
	}

	if( (*frame_1==COMB_I_P && *frame_2==COMB_P_P) || (*frame_1==COMB_P_I && *frame_2==COMB_P_P) || (*frame_1==COMB_P_P && *frame_2==COMB_I_P) || (*frame_1==COMB_P_P && *frame_2==COMB_P_I)) {
		if(ink1==ink2 && paper1==paper2) {
			dith=y&1;
			if(dith==0) {
				if(m->luma[paper1]>m->luma[ink1]) { *frame_1=COMB_I_P; *frame_2=COMB_P_P; return; }
				else { *frame_1=COMB_P_I; *frame_2=COMB_P_P; return;}
			}
			else {
				if(m->luma[paper1]>m->luma[ink1]) { *frame_1=COMB_P_P; *frame_2=COMB_P_I; return; }
				else { *frame_1=COMB_P_P; *frame_2=COMB_I_P; return; }
			}
		}
	}

	if( (*frame_1==COMB_S_I && *frame_2==COMB_I_I) || (*frame_1==COMB_I_S && *frame_2==COMB_I_I) || (*frame_1==COMB_I_I && *frame_2==COMB_S_I) || (*frame_1==COMB_I_I && *frame_2==COMB_I_S)) {
		if(ink1==ink2 && sprite1==sprite2) {
			dith=y&1;
			if(dith==0) {
				if(m->luma[sprite1]>m->luma[ink1]) { *frame_1=COMB_I_S; *frame_2=COMB_I_I; return; }
				else { *frame_1=COMB_S_I; *frame_2=COMB_I_I; return; }
			}
			else {
				if(m->luma[sprite1]>m->luma[ink1]) { *frame_1=COMB_I_I; *frame_2=COMB_S_I; return; }
				*frame_1=COMB_I_I; *frame_2=COMB_I_S; return;
			}
		}
	}

	if( (*frame_1==COMB_I_S && *frame_2==COMB_S_S) || (*frame_1==COMB_S_I && *frame_2==COMB_S_S) || (*frame_1==COMB_S_S && *frame_2==COMB_I_S) || (*frame_1==COMB_S_S && *frame_2==COMB_S_I)) {
		if(ink1==ink2 && sprite1==sprite2) {
			dith=y&1;
			if(dith==0) {
				if(m->luma[sprite1]>m->luma[ink1]) { *frame_1=COMB_I_S; *frame_2=COMB_S_S; return; }
				else { *frame_1=COMB_S_I; *frame_2=COMB_S_S; return;}
			}
			else {
				if(m->luma[sprite1]>m->luma[ink1]) { *frame_1=COMB_S_S; *frame_2=COMB_S_I; return; }
				else { *frame_1=COMB_S_S; *frame_2=COMB_I_S; return; }
			}
		}
	}
*/
	/* also compare two solids, as paper/ink/sprite color may differe between frames */
	if( (*frame_1==COMB_P_P && *frame_2==COMB_P_P) || (*frame_1==COMB_I_I && *frame_2==COMB_I_I) || (*frame_1==COMB_P_P && *frame_2==COMB_I_I) || (*frame_1==COMB_I_I && *frame_2==COMB_P_P) ) {
		/* are ink/paper crossed in other frame? */
		if(ink1==paper2 && paper1==ink2) {
			dith=y&1;
			if(m->luma[paper1]>m->luma[ink1]) dith^=1;
			if(dith==0) { *frame_1=COMB_P_P; *frame_2=COMB_I_I; return; }
			else { *frame_1=COMB_I_I; *frame_2=COMB_P_P; return; }
		}
	}

	if( (*frame_1==COMB_S_S && *frame_2==COMB_S_S) || (*frame_1==COMB_I_I && *frame_2==COMB_I_I) || (*frame_1==COMB_S_S && *frame_2==COMB_I_I) || (*frame_1==COMB_I_I && *frame_2==COMB_S_S) ) {
		if(ink1==sprite2 && sprite1==ink2) {
			/* are ink/sprite crossed in other frame? */
			dith=y&1;
			if(m->luma[sprite1]>m->luma[ink1]) dith^=1;
			if(dith==0) { *frame_1=COMB_S_S; *frame_2=COMB_I_I; return; }
			else { *frame_1=COMB_I_I; *frame_2=COMB_S_S; return; }
		}
	}

	// cases that need two pixels //
	// P S crossed
	if( (*frame_1==COMB_P_P && *frame_2==COMB_P_P) || (*frame_1==COMB_P_P && *frame_2==COMB_S_S) || (*frame_1==COMB_S_S && *frame_2==COMB_P_P) || (*frame_1==COMB_S_S && *frame_2==COMB_S_S)) {
		/* are paper/sprite crossed in other frame? */
		if(paper1==sprite2 && paper2==sprite1) {
			dith=y&1;
			if(m->luma[paper1]>m->luma[sprite1]) dith^=1;
			dith^=((x>>1)&1);
			if(dith==0) { *frame_1=COMB_P_P; *frame_2=COMB_P_P; return; }
			else { *frame_1=COMB_S_S; *frame_2=COMB_S_S; return; }
		}
	}

	/* this case should not happen?! unsure */
	if( (*frame_1==COMB_S_P && *frame_2==COMB_P_S) || (*frame_1==COMB_P_S && *frame_2==COMB_S_P) || (*frame_1==COMB_P_S && *frame_2==COMB_P_S) || (*frame_1==COMB_S_P && *frame_2==COMB_S_P)) {
		/* are paper/sprite crossed in other frame? */
		if(paper1==paper2 && sprite1==sprite2) {
			dith=y&1;
			if(m->luma[paper1]>m->luma[sprite1]) dith^=1;
			dith^=((x>>1)&1);
			if(dith==0) { *frame_1=COMB_P_S; *frame_2=COMB_P_S; return; }
			else { *frame_1=COMB_S_P; *frame_2=COMB_S_P; return; }
		}
	}

	//the following tests only can succeed if 2ink and 2paper is used, or 2ink and 2 sprite
	if(ink1==paper2 && paper1==sprite2) {
		if((*frame_1==COMB_I_S && *frame_2==COMB_P_P) || (*frame_1==COMB_P_P && *frame_2==COMB_I_S) || (*frame_1==COMB_I_S && *frame_2==COMB_I_S) || (*frame_1==COMB_P_P && *frame_2==COMB_P_P)) {
			dith=(y<<defl)&1;
			dith^=(x>>1)&1;
			if(m->luma[ink1]>m->luma[paper1]) dith^=1;
			if(dith==0) { *frame_1=COMB_I_S; *frame_2=COMB_P_P; return; }
			else { *frame_1=COMB_P_P; *frame_2=COMB_I_S; return; }
		}
	}
	if(ink1==paper2 && sprite1==ink2) {
		if((*frame_1==COMB_I_I && *frame_2==COMB_S_P) || (*frame_1==COMB_S_P && *frame_2==COMB_I_I) || (*frame_1==COMB_S_P && *frame_2==COMB_S_P) || (*frame_1==COMB_I_I && *frame_2==COMB_I_I)) {
			dith=(y<<defl)&1;
			dith^=(x>>1)&1;
			if(m->luma[ink1]>m->luma[sprite1]) dith^=1;
			if(dith==0) { *frame_1=COMB_I_I; *frame_2=COMB_S_P; return; }
			else { *frame_1=COMB_S_P; *frame_2=COMB_I_I; return; }
		}
	}
	if(ink1==paper2 && sprite1==sprite2) {
		if((*frame_1==COMB_I_S && *frame_2==COMB_S_P) || (*frame_1==COMB_S_P && *frame_2==COMB_I_S) || (*frame_1==COMB_S_P && *frame_2==COMB_S_P) || (*frame_1==COMB_I_S && *frame_2==COMB_I_S)) {
			dith=(y<<defl)&1;
			dith^=(x>>1)&1;
			if(m->luma[ink1]>m->luma[sprite1]) dith^=1;
			if(dith==0) { *frame_1=COMB_I_S; *frame_2=COMB_S_P; return; }
			else { *frame_1=COMB_S_P; *frame_2=COMB_I_S; return; }
		}
	}
	if(ink1==sprite2 && paper1==paper2) {
		if((*frame_1==COMB_I_P && *frame_2==COMB_P_S) || (*frame_1==COMB_P_S && *frame_2==COMB_I_P) || (*frame_1==COMB_P_S && *frame_2==COMB_P_S) || (*frame_1==COMB_I_P && *frame_2==COMB_I_P)) {
			dith=(y<<defl)&1;
			dith^=(x>>1)&1;
			if(m->luma[ink1]>m->luma[paper1]) dith^=1;
			if(dith==0) { *frame_1=COMB_I_P; *frame_2=COMB_P_S; return; }
			else { *frame_1=COMB_P_S; *frame_2=COMB_I_P; return; }
		}
	}
	if(ink1==sprite2 && paper1==ink2) {
		if((*frame_1==COMB_I_I && *frame_2==COMB_P_S) || (*frame_1==COMB_P_S && *frame_2==COMB_I_I) || (*frame_1==COMB_P_S && *frame_2==COMB_P_S) || (*frame_1==COMB_I_I && *frame_2==COMB_I_I)) {
			dith=(y<<defl)&1;
			dith^=(x>>1)&1;
//			if(m->luma[ink1]>m->luma[paper1]) dith^=1;
			if(dith==0) { *frame_1=COMB_I_I; *frame_2=COMB_P_S; return; }
			else { *frame_1=COMB_P_S; *frame_2=COMB_I_I; return; }
		}
	}
	if(ink1==sprite2 && sprite1==paper2) {
		if((*frame_1==COMB_I_P && *frame_2==COMB_S_S) || (*frame_1==COMB_S_S && *frame_2==COMB_I_P) || (*frame_1==COMB_S_S && *frame_2==COMB_S_S) || (*frame_1==COMB_I_P && *frame_2==COMB_I_P)) {
			dith=(y<<defl)&1;
			dith^=(x>>1)&1;
			if(m->luma[ink1]>m->luma[sprite1]) dith^=1;
			if(dith==0) { *frame_1=COMB_I_P; *frame_2=COMB_S_S; return; }
			else { *frame_1=COMB_S_S; *frame_2=COMB_I_P; return; }
		}
	}
	if(ink1==ink2 && paper1==sprite2) {
		if((*frame_1==COMB_I_S && *frame_2==COMB_P_I) || (*frame_1==COMB_P_I && *frame_2==COMB_I_S) || (*frame_1==COMB_P_I && *frame_2==COMB_P_I) || (*frame_1==COMB_I_S && *frame_2==COMB_I_S)) {
			dith=(y<<defl)&1;
			dith^=(x>>1)&1;
			if(m->luma[ink1]>m->luma[paper1]) dith^=1;
			if(dith==0) { *frame_1=COMB_I_S; *frame_2=COMB_P_I; return; }
			else { *frame_1=COMB_P_I; *frame_2=COMB_I_S; return; }
		}
	}
	if(ink1==ink2 && sprite1==paper2) {
		if((*frame_1==COMB_I_P && *frame_2==COMB_S_I) || (*frame_1==COMB_S_I && *frame_2==COMB_I_P) || (*frame_1==COMB_S_I && *frame_2==COMB_S_I) || (*frame_1==COMB_I_P && *frame_2==COMB_I_P)) {
			dith=(y<<defl)&1;
			dith^=(x>>1)&1;
			if(m->luma[ink1]>m->luma[sprite1]) dith^=1;
			if(dith==0) { *frame_1=COMB_I_P; *frame_2=COMB_S_I; return; }
			else { *frame_1=COMB_S_I; *frame_2=COMB_I_P; return; }
		}
	}

	/* align 50/50 mixes in each pixel if black is involved */
	if(ink1==ink2 && sprite1==sprite2 && (ink1==0 || sprite1==0) ) {
		dith=y&1;
		dith^=(x>>1)&1;
		if(m->luma[sprite1]>m->luma[ink1]) dith^=1;
		if( (*frame_1==COMB_S_I && *frame_2==COMB_I_S) || (*frame_1==COMB_S_I && *frame_2==COMB_S_I) || (*frame_1==COMB_I_S && *frame_2==COMB_I_S) || (*frame_1==COMB_I_S && *frame_2==COMB_S_I) ) {
			if(dith==0) { *frame_1=COMB_S_I; *frame_2=COMB_S_I; return; }
			else { *frame_1=COMB_I_S; *frame_2=COMB_I_S; return; }
		}
	}
	if(ink1==ink2 && paper1==paper2 && (ink1==0 || paper1==0) ) {
		dith=y&1;
		dith^=(x>>1)&1;
		if(m->luma[paper1]>m->luma[ink1]) dith^=1;
		if( (*frame_1==COMB_P_I && *frame_2==COMB_I_P) || (*frame_1==COMB_P_I && *frame_2==COMB_P_I) || (*frame_1==COMB_I_P && *frame_2==COMB_I_P) || (*frame_1==COMB_I_P && *frame_2==COMB_P_I) ) {
			if(dith==0) { *frame_1=COMB_P_I; *frame_2=COMB_P_I; return; }
			else { *frame_1=COMB_I_P; *frame_2=COMB_I_P; return; }
		}
	}

	//XXX more combinations possible?!?!?
}

void interlace_render1x1(uint8_t* hmask1, uint8_t* hmask2, uint8_t* smask1, uint8_t* smask2, int frame) {
	switch(frame) {
		case COMB_I_P:
			*hmask1=*hmask1|1;
		break;
		case COMB_P_I:
			*hmask2=*hmask2|1;
		break;
		case COMB_I_I:
			*hmask1=*hmask1|1;
			*hmask2=*hmask2|1;
		break;
		case COMB_I_S:
			*hmask1=*hmask1|1;
			*smask2=*smask2|1;
		break;
		case COMB_S_I:
			*hmask2=*hmask2|1;
			*smask1=*smask1|1;
		break;
		case COMB_P_P:
		break;
		case COMB_P_S:
			*smask2=*smask2|1;
		break;
		case COMB_S_P:
			*smask1=*smask1|1;
		break;
		case COMB_S_S:
			*smask1=*smask1|1;
			*smask2=*smask2|1;
		break;
		default:
		break;
	}
}

void interlace_render(MufflonContext* m) {
	int x;
	int y;
	int blockx;
	uint8_t smask1;
	uint8_t smask2;
	uint8_t hmask1;
	uint8_t hmask2;
	int ink1,ink2;
	int paper1,paper2;
	int sprite1,sprite2;
	int frame_1;
	int frame_2;

	int flicker;

	for(y=0;y<C64YRES;y++) {
		status_message("Rendering bitmaps: Pr0ncessing line #%d   \n\033[A",y);
		smask1=0;
		smask2=0;
		for(blockx=0;blockx<C64XRES;blockx+=8) {
			ink1=m->inks[0][y/2*C64XRES/8+blockx/8];
			ink2=m->inks[1][y/2*C64XRES/8+blockx/8];
			paper1=m->papers[0][y/2*C64XRES/8+blockx/8];
			paper2=m->papers[1][y/2*C64XRES/8+blockx/8];

			if(blockx>=FLI && blockx<m->sl_sizex) {
				sprite1=m->sprites[0][((y+1)/2)*6+(blockx-FLI)/48];
				sprite2=m->sprites[1][((y+1)/2)*6+(blockx-FLI)/48];
			}
			else {
				sprite1=-1; sprite2=-1;
			}

			hmask1=0;
			hmask2=0;
			if(((blockx-FLI)&0xf)==0) {
				smask1=0;
				smask2=0;
			}

			for(x=blockx;x<blockx+8;x+=2) {
				interlace_find_best_combination(m, x, y, ink1, ink2, paper1, paper2, sprite1, sprite2, &frame_1, &frame_2,&flicker);
				if(m->option_deflicker!=0) resort_combinations(m,x, y, &frame_1, &frame_2, ink1, ink2, paper1, paper2, sprite1, sprite2);
				smask1<<=1;
				smask2<<=1;
				hmask1<<=1;
				hmask2<<=1;
				interlace_render1x1(&hmask1, &hmask2, &smask1, &smask2, frame_1);
				hmask1<<=1;
				hmask2<<=1;
				interlace_render1x1(&hmask1, &hmask2, &smask1, &smask2, frame_2);
			}
			if(blockx>=FLI && blockx<m->sl_sizex) {
				if(((blockx-FLI)&0x8)!=0) {
					m->sprite_bitmap[0][y*3*6+(blockx-FLI)/16]=smask1;
					m->sprite_bitmap[1][y*3*6+(blockx-FLI)/16]=smask2;
				}
			}
			m->hires_bitmap[0][y/8*0x140+(blockx)+(y&7)]=hmask1;
			m->hires_bitmap[1][y/8*0x140+(blockx)+(y&7)]=hmask2;
		}
	}
	status_message("\n");
}

int block_diff(MufflonContext* m, int blockx, int blocky, int ink1, int ink2, int paper1, int paper2, int* sprite1, int* sprite2, int best, int* flick) {
	int x;
	int blockdiff=0;
	int frame_1,frame_2;
	int flicker;

	*flick=0;

	for(x=blockx;x<blockx+8;x+=2) {
		blockdiff+=interlace_find_best_combination(m, x, blocky, ink1, ink2, paper1, paper2, sprite1[0], sprite2[0], &frame_1,&frame_2,&flicker);
		if(*flick>=0) *flick+=flicker;
		if(best>=0 && blockdiff>best) return blockdiff;
		blockdiff+=interlace_find_best_combination(m, x, blocky+1, ink1, ink2, paper1, paper2, sprite1[1], sprite2[1], &frame_1,&frame_2,&flicker);
		if(*flick>=0) *flick+=flicker;
		if(best>=0 && blockdiff>best) return blockdiff;
	}
	return blockdiff;
}

void find_hires_colors(MufflonContext* m) {
	int y;
	int blockx;
	int sprite1[2];
	int sprite2[2];
	int ink1, ink2;
	int paper1, paper2;
	int ic1,ic2;
	int pc1,pc2;
	int blockdiff2ink;
	int best;
	int limit;
	int oneinkonepaper=0;
	int twoinkonepaper=0;
	int oneinktwopaper=0;
	int twoinktwopaper=0;
	int flicker;
	int flickbest;
	int icc1,icc2,pcc1,pcc2;

	for(y=0; y<C64YRES; y+=2) {
		status_message("Searching for best paper/ink colours and combinations: Pr0ncessing line #%d   1ink1paper: %d    1ink2paper: %d    2ink1paper: %d    2ink2paper: %d\n\033[A",y,oneinkonepaper, oneinktwopaper, twoinkonepaper, twoinktwopaper);
		for(blockx=0;blockx<C64XRES;blockx+=8) {
			paper1=-1;
			paper2=-1;
			ink1=-1;
			ink2=-1;
			best=-1;
			flickbest=-1;
			if(blockx>=FLI && blockx<m->sl_sizex) {
				sprite1[0]=m->sprites[0][((1+y)/2)*6+(blockx-FLI)/48];
				sprite1[1]=m->sprites[0][((2+y)/2)*6+(blockx-FLI)/48];
				sprite2[0]=m->sprites[1][((1+y)/2)*6+(blockx-FLI)/48];
				sprite2[1]=m->sprites[1][((2+y)/2)*6+(blockx-FLI)/48];
			}
			else {
				sprite1[0]=-1;
				sprite1[1]=-1;
				sprite2[0]=-1;
				sprite2[1]=-1;
			}
//			limit=15;
			if(blockx<FLI) limit=1;
			else limit=15;
			/* if we count backwards we also check the case of a color being unused (-1) */
			for(icc1=limit;icc1>=-1;icc1--) {
				ic1=icc1;
				for(icc2=m->secondink;icc2>=-1;icc2--) {
					if(m->secondink==-1) ic2=ic1;
					else ic2=icc2;
					for(pcc1=limit;pcc1>=-1;pcc1--) {
						pc1=pcc1;
						for(pcc2=m->secondpaper;pcc2>=-1;pcc2--) {
							if(m->secondpaper==-1) pc2=pcc1;
							else pc2=pcc2;

							if(blockx<FLI) {
								sprite1[0]=-1;
								sprite1[1]=-1;
								sprite2[0]=-1;
								sprite2[1]=-1;
								pc2=m->option_sbugcol;
								pc1=m->option_sbugcol;
								ic1=15;
								ic2=15;
							}

							if( (  (ic1==-1 || m->used_colors[y/2][blockx/8][ic1]>0) && (ic2==-1 || m->used_colors[y/2][blockx/8][ic2]>0) && (pc1==-1 || m->used_colors[y/2][blockx/8][pc1]>0) && (pc2==-1 || m->used_colors[y/2][blockx/8][pc2]>0) ) || blockx<FLI) {
								blockdiff2ink=block_diff(m,blockx,y,ic1,ic2,pc1,pc2, sprite1, sprite2,best,&flicker);
								if(blockdiff2ink<best || best<0) {
									ink1=ic1;
									ink2=ic2;
									paper1=pc1;
									paper2=pc2;
									best=blockdiff2ink;
								}
								else if(blockdiff2ink==best && flicker>=0 && (flicker<flickbest || flickbest<0)) {
									flickbest=flicker;
									ink1=ic1;
									ink2=ic2;
									paper1=pc1;
									paper2=pc2;
								}
							}
						}
					}
				}
			}
			if(blockx>=FLI) {
				if(ink1==paper1) { paper1=paper2; }
				if(ink2==paper2) { paper2=paper1; }

				if(ink1==paper1) { ink1=ink2; }
				if(ink2==paper2) { ink2=ink2; }

				if(sprite1[0]==sprite1[1] && sprite1[0]==sprite2[0] && sprite2[0]==sprite2[1]) {
					if(paper1==sprite1[0]) paper1=paper2;
					if(paper2==sprite1[0]) paper2=paper1;
//					if(ink1==sprite1[0]) ink1=ink2;
//					if(ink2==sprite1[0]) ink2=ink1;
					if(ink1==paper2 && ink2==paper1) {
						ink2=ink1;
						paper2=paper1;
					}
				}

				/* ink is still free? then set ink1 to ink2 and vice verse */
				if(ink2<0) ink2=ink1;
				if(ink1<0) ink1=ink2;

				if(paper2<0) paper2=paper1;
				if(paper1<0) paper1=paper2;
			}

			m->inks[0][y/2*C64XRES/8+blockx/8]=ink1;
			m->inks[1][y/2*C64XRES/8+blockx/8]=ink2;
			m->papers[0][y/2*C64XRES/8+blockx/8]=paper1;
			m->papers[1][y/2*C64XRES/8+blockx/8]=paper2;

			if(paper1<0) paper1=0;
			if(paper2<0) paper2=0;
			if(ink1<0) ink1=0;
			if(ink2<0) ink2=0;

			m->colormap[0][(y/2)*C64COLUMNS+(blockx/8)]=(ink1<<4)|paper1;
			m->colormap[1][(y/2)*C64COLUMNS+(blockx/8)]=(ink2<<4)|paper2;

			if(ink1!=ink2 && paper1!=paper2) twoinktwopaper++;
			if(ink1!=ink2 && paper1==paper2) twoinkonepaper++;
			if(ink1==ink2 && paper1!=paper2) oneinktwopaper++;
			if(ink1==ink2 && paper1==paper2) oneinkonepaper++;
		}
	}
	status_message("\n");
}

void find_sprite_colors_bruteforce(MufflonContext* m) {
	int x;
	int blockx;
	int blocky;
	int blockdiff;
	int subblockdiff2ink;
	int subbest;
	int best;
	int sprite1[2], sprite2[2];
	int bestcol1;
	int bestcol2;
	int spr1;
	int spr2;
	int ic1,ic2,pc1,pc2;
	int icc2,pcc2,sprc2;
	int flicker;
	int flickbest;
	int subflicker;
	int subflickbest;
	int unused;

	for(blocky=0;blocky<C64YRES;blocky+=2) {
		status_message("Searching for best sprite colours: Pr0ncessing line #%d   \n\033[A",blocky);
		for(blockx=FLI;blockx<m->sl_sizex;blockx+=48) {
			sprite2[1]=-1;
			sprite1[1]=-1;
			bestcol1=-1;
			bestcol2=-1;
			unused=0;
			//XXX if sprite1[0] && sprite2[0]==-1 -> also evaluata those colors!
			if(blocky>0) {
				/* fetch already found spritecol for first line if possible */
				sprite1[0]=m->sprites[0][((blocky)/2)*6+(blockx-FLI)/48];
				sprite2[0]=m->sprites[1][((blocky)/2)*6+(blockx-FLI)/48];
				if(sprite1[0]==-1 && sprite2[0]==-1) {
					unused=1;
				}
			}
			else {
				sprite1[0]=0;
				sprite2[0]=0;
			}
			best=-1;
			flickbest=-1;
			/* try all colors for spriteline 2 */
			for(spr1=15;spr1>=-1;spr1--) {
				for(sprc2=m->secondsprite;sprc2>=-1;sprc2--) {
					if(m->secondsprite==-1) spr2=spr1;
					else spr2=sprc2;
					if(
					   (spr1==-1 ||
					   m->used_colors[blocky/2][blockx/8+0][spr1]>0 ||
					   m->used_colors[blocky/2][blockx/8+1][spr1]>0 ||
					   m->used_colors[blocky/2][blockx/8+2][spr1]>0 ||
					   m->used_colors[blocky/2][blockx/8+3][spr1]>0 ||
					   m->used_colors[blocky/2][blockx/8+4][spr1]>0 ||
					   m->used_colors[blocky/2][blockx/8+5][spr1]>0) &&

					   (spr2==-1 ||
					   m->used_colors[blocky/2][blockx/8+0][spr2]>0 ||
					   m->used_colors[blocky/2][blockx/8+1][spr2]>0 ||
					   m->used_colors[blocky/2][blockx/8+2][spr2]>0 ||
					   m->used_colors[blocky/2][blockx/8+3][spr2]>0 ||
					   m->used_colors[blocky/2][blockx/8+4][spr2]>0 ||
					   m->used_colors[blocky/2][blockx/8+5][spr2]>0)
					   ) {

						sprite1[1]=spr1;
						sprite2[1]=spr2;
						if(unused) {
							sprite1[0]=spr1;
							sprite2[0]=spr2;
						}
						/* now build blockdiff */
						blockdiff=0;
						flicker=0;
						for(x=blockx;x<blockx+48 && x<m->sl_sizex;x+=8) {
							subbest=-1;
							subflickbest=-1;
							/* combine with all permutations for paper and ink */
							for(ic1=15;ic1>=-1;ic1--) {
								for(icc2=m->secondink;icc2>=-1;icc2--) {
									if(m->secondink==-1) ic2=ic1;
									else ic2=icc2;
									for(pc1=15;pc1>=-1;pc1--) {
										for(pcc2=m->secondpaper;pcc2>=-1;pcc2--) {
											if(m->secondpaper==-1) pc2=pc1;
											else pc2=pcc2;
											if(   (ic1==-1 || m->used_colors[blocky/2][x/8][ic1]>0) && (ic2==-1 || m->used_colors[blocky/2][x/8][ic2]>0) && (pc1==-1 || m->used_colors[blocky/2][x/8][pc1]>0) && (pc2==-1 || m->used_colors[blocky/2][x/8][pc2]>0) ) {
												subblockdiff2ink=block_diff(m,x,blocky,ic1,ic2,pc1,pc2,sprite1,sprite2,subbest,&subflicker);
												if(subblockdiff2ink<subbest || subbest<0) {
													subbest=subblockdiff2ink;
												}
												else if(subblockdiff2ink==subbest && subflicker>=0 && (subflicker<subflickbest || subflickbest<0)) {
													subflickbest=subflicker;
												}
											}
										}
									}
								}
							}
							blockdiff+=subbest;
							flicker+=subflickbest;
						}
						/* did this combi work out well? */
						if(blockdiff<best || best<0) {
							best=blockdiff;
							bestcol1=spr1;
							bestcol2=spr2;
						}
						if(blockdiff==best && flicker>=0 && (flicker<flickbest || flickbest<0)) {
							best=blockdiff;
							bestcol1=spr1;
							bestcol2=spr2;
							flickbest=flicker;
						}
					}
				}
			}
			if(bestcol1==-1) bestcol1=bestcol2;
			if(bestcol2==-1) bestcol2=bestcol1;
			m->sprites[0][(blocky/2+1)*6+(blockx-FLI)/48]=bestcol1;
			m->sprites[1][(blocky/2+1)*6+(blockx-FLI)/48]=bestcol2;
			if(bestcol1<0) bestcol1=0;
			if(bestcol2<0) bestcol2=0;
			m->sprite_col_tab[0][(blocky/2+1)*6+(blockx-FLI)/48]=bestcol1;
			m->sprite_col_tab[1][(blocky/2+1)*6+(blockx-FLI)/48]=bestcol2;
		}
	}
	status_message("\n");
	return;
}

//XXX falls in Zeile 2 Spritefarbe nicht genutzt wird, dann für Zeile 1 Block 2 Spritefarbe neu berechnen
void find_sprite_colors(MufflonContext* m) {
	int x,y;
	int blockx;
	int blocky;
	int col1,col2;
	int c,d;
	int blockdiff;
	int best;

	for(blocky=-1;blocky<C64YRES;blocky+=2) {
		status_message("Searching for best sprite colours: Pr0ncessing line #%d   \n\033[A",blocky);
		for(blockx=FLI;blockx<m->sl_sizex;blockx+=48) {
			best=-1;
			for(c=15;c>=-1;c--) {
				for(d=15;d>=-1;d--) {
					col1=c;
					if(m->secondsprite==-1) col2=c;
					else col2=d;
					blockdiff=0;
					//XXX besser: Farbe in der Mitte bevorzugen wenn irgendwie möglich.
					//alle farben die sich inenrhalb eines 8er blocks ändern zählen, farben die über längere strecken gehen, nicht zählen
					//Farben die über lange strecken gehen nicht beachten (8 Pixel) bis zu max. 2 solcher farben, die dritte farbe -> mögliche spritefarbe
					//alle Kandidaten merken und dann den rauspicken der den geringsten fehler erzeugt (blockdiff+compare...)
					for(y=blocky;y<blocky+2;y++) {
						if(y>=0) {
							for(x=blockx;x<blockx+48 && x<m->sl_sizex;x++) {
								if(best!=-1 && blockdiff>best) { break; break; }
								blockdiff+=compare_with_luma(m,x,y,col1,col2);
							}
						}
					}
					if(blockdiff<best || best<0) {
						best=blockdiff;
						m->sprites[0][(blocky+1)/2*6+(blockx-FLI)/48]=col1;
						m->sprites[1][(blocky+1)/2*6+(blockx-FLI)/48]=col2;
						m->sprite_col_tab[0][((blocky+1)/2)*6+(blockx-FLI)/48]=col1;
						m->sprite_col_tab[1][((blocky+1)/2)*6+(blockx-FLI)/48]=col2;
					}
				}
			}
		}
	}
	status_message("\n");
}

void convert_bmp(MufflonContext* m) {
	write_data(m, m->write_name, m->data);
	return;
}
