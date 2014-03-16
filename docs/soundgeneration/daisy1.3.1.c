//Daisy mmc mp3
//Firmware V1.2 CC 2001-2006 Raphael Abrams, www.teuthis.com;
//This code is under a Creative Commons license. You may use it for whatever purpose you 
//wish in whatever way you wish, but attribution is required. 
//Please include this paragraph in your source or somewhere easily found by the end user. 
//If there is a printed manual, please make a note somewhere in there too. Like: "mp3 and SD handling routines by teuthis.com" or something like that. 

//The preceding is to be included in any derivative work excepting works falling under "fair use". 
//Consult www.eff.org for more information on Creative Commons.
//Also, while not required, a link to "teuthis.com" would be just the nicest thing! I love free promotion!


#include <18f45j10.H>
//#device *=16 ADC=10
#fuses H4_SW,NOWDT//,DEBUG
#use delay(clock=40,000,000)
//was 24mhz

#use fast_io(a)
#use fast_io(b)
#use fast_io(c)
#use fast_io(d) 
#use fast_io(e)
//#use rs232(baud=57600, xmit=PIN_C6,rcv=PIN_C7)

#use rs232(baud=9600, xmit=PIN_C6,rcv=PIN_C7)


//////////.////////////////.////////////////.///////////////./////////////////




#byte TOSU 	 = 0xFFF
#byte TOSH 	 = 0xFFE
#byte TOSL 	 = 0xFFD
#byte STKPTR 	 = 0xFFC
#byte PCLATU 	 = 0xFFB
#byte PCLATH 	 = 0xFFA
  #byte PCL 	 = 0xFF9
#byte TBLPTRU 	 = 0xFF8
#byte TBLPTRH 	 = 0xFF7
#byte TBLPTRL 	 = 0xFF6
#byte TABLAT 	 = 0xFF5
#byte PRODH 	 = 0xFF4
#byte PRODL 	 = 0xFF3

#byte INTCON 	 = 0xFF2
#bit TMR0IF = INTCON.2
#bit INT0IF = INTCON.1

#byte INTCON2 	 = 0xFF1
#bit RBPU=INTCON2.7
#bit	   INTEDG0  = INTCON2.6
#byte INTCON3 	 = 0xFF0

#byte INDF0 	 = 0xFEF
#byte POSTINC0 	 = 0xFEE
#byte POSTDEC0 	 = 0xFED
#byte PREINC0 	 = 0xFEC
#byte PLUSW0 	 = 0xFEB
#byte FSR0H 	 = 0xFEA
#byte FSR0L 	 = 0xFE9
#byte WREG 	 = 0xFE8
#byte INDF1 	 = 0xFE7
#byte POSTINC1 	 = 0xFE6
#byte POSTDEC1 	 = 0xFE5
#byte PREINC1 	 = 0xFE4
#byte PLUSW1 	 = 0xFE3
#byte FSR1H 	 = 0xFE2
#byte FSR1L 	 = 0xFE1
#byte BSR 	 = 0xFE0
#byte INDF2	 = 0xFDF
#byte POSTINC2	 = 0xFDE
#byte POSTDEC2 	 = 0xFDD
#byte PREINC2 	 = 0xFDC
#byte PLUSW2 	 = 0xFDB
#byte FSR2H 	 = 0xFDA
#byte FSR2L 	 = 0xFD9
#byte STATUS 	 = 0xFD8

#byte TMR0H	 = 0xFD7
#byte TMR0L 	 = 0xFD6
#byte T0CON 	 = 0xFD5
#byte OSCCON 	 = 0xFD3
#byte HLVDCON 	 = 0xFD2
#byte WDTCON 	 = 0xFD1
#byte RCON	 = 0xFD0
#byte TMR1H 	 = 0xFCF
#byte TMR1L 	 = 0xFCE
#byte T1CON 	 = 0xFCD
#byte TMR2 	 = 0xFCC
#byte PR2 	 = 0xFCB
#byte T2CON 	 = 0xFCA

#byte SSPBUF 	 = 0xFC9
#byte SSPADD 	 = 0xFC8
#byte SSPSTAT	 = 0xFC7
#byte SSPCON1	 = 0xFC6
#byte SSPCON2	 = 0xFC5

#byte ADRESH 	 = 0xFC4
#byte ADRESL 	 = 0xFC3
#byte ADCON0 	 = 0xFC2
#bit	   GODONE= ADCON0.1
#byte ADCON1 	 = 0xFC1
#byte ADCON2 	 = 0xFC0

#byte CCPR1H 	 = 0xFBF
#byte CCPR1L 	 = 0xFBE
#byte CCP1CON	 = 0xFBD
#byte CCPR2H 	 = 0xFBC
#byte CCPR2L 	 = 0xFBB
#byte CCP2CON	 = 0xFBA
#byte CVRCON	 = 0xFB5
#byte CMCON 	 = 0xFB4

#byte TMR3H 	 = 0xFB3
#byte TMR3L 	 = 0xFB2
#byte T3CON 	 = 0xFB1

#byte SPBRG1 	 = 0xFAF
#byte RCREG1 	 = 0xFAE
#byte TXREG1 	 = 0xFAD
#byte TXSTA1 	 = 0xFAC
#byte RCSTA1 	 = 0xFAB
  	   	  
  	   	   	  
#byte IPR3 	 = 0xFA5
#byte PIR3 	 = 0xFA4
#byte PIE3 	 = 0xFA3
#byte IPR2 	 = 0xFA2
#byte PIR2 	 = 0xFA1
#byte PIE2 	 = 0xFA0
#byte IPR1	 = 0xF9F
#byte PIR1	 = 0xF9E
#byte PIE1	 = 0xF9D
#byte MEMCON	 = 0xF9C
#byte OSCTUNE	 = 0xF9B

#byte TRIS_E	 = 0xF96
#byte TRIS_D	 = 0xF95
#byte TRIS_C	 = 0xF94
#byte TRIS_B	 = 0xF93
#byte TRIS_A	 = 0xF92

#byte LAT_E	 = 0xF8D
#byte LAT_D	 = 0xF8C
#byte LAT_C	 = 0xF8B
#byte LAT_B	 = 0xF8A
#byte LAT_A	 = 0xF89

#byte PORT_E	 = 0xF84
#byte PORT_D	 = 0xF83
#byte PORT_C	 = 0xF82
#byte PORT_B	 = 0xF81
#byte PORT_A	 = 0xF80


//some useful bits:


#byte OSCTUNE = 0xF9B
#bit PLLEN = OSCTUNE.6




//spi
#bit  smp =  SSPSTAT.7
#bit  cke =  SSPSTAT.6
#bit  bf =  SSPSTAT.0

#bit  sspen = SSPCON1.5
#bit  ckp = SSPCON1.4
#bit  sspm3 = SSPCON1.3
#bit  sspm2 = SSPCON1.2
#bit  sspm1 = SSPCON1.1
#bit  sspm0 = SSPCON1.0


//a to d
#bit  chs3 = ADCON0.5
#bit  chs2 = ADCON0.4
#bit  chs1 = ADCON0.3
#bit  chs0 = ADCON0.2
#bit  go_done = ADCON0.1  // GO/#DONE   set high, read til low
#bit  adon = ADCON0.0

#bit  vcfg1 = ADCON1.5
#bit  vcfg0 = ADCON1.4
#bit  pcfg3 = ADCON1.3
#bit  pcfg2 = ADCON1.2
#bit  pcfg1 = ADCON1.1
#bit  pcfg0 = ADCON1.0

#bit  adfm = ADCON2.7 //sets left or right justification for 12 bit result
////////////////.////////////////////.////////////////////.////////////////.//////////////////


//#bit MMC_TRANSISTOR =port_c.0
#bit MMCSS	= lat_c.1 //this is chip select for the MMC/SD
#bit XCS		= lat_b.4
#bit XDCS		=lat_b.3
#bit XCLK_l		= lat_c.3
#bit XCLK_p		= port_c.3
#bit XCLK_TRIS= tris_c.3
#bit XDI_p		= port_c.4//this is the tricky one that goes both ways, software spi and hardware spi
#bit XDI_l		= lat_c.4//this is the tricky one that goes both ways, software spi and hardware spi
#bit XDO	 	= port_c.0//not used, most likely
#bit MMC_TRANSISTOR= lat_c.0
#bit XDREQ		= port_b.2
#bit XRESET 	= lat_b.1
//#bit MOSI		=port_c.5
#bit XDI_TRIS	=tris_c.4


#byte JUMPERS	= port_a   //and-out the upper nibble. only use bits 0-3
#bit LED = lat_b.5

#byte SLAVE_PORT	=	port_d
//#bit SLAVE_RE		=	port_e.0
//#bit SLAVE_WE	=	port_e.1
//#bit SLAVE_CS		=	port_e.2

#bit SERIAL_INTERRUPT = port_b.0
#bit SERIAL_INTERRUPT_LAT = lat_b.0
#bit SERIAL_INTERRUPT_TRIS = tris_b.0

#bit SERIAL_OUT		=	lat_c.6
#bit SERIAL_IN			=	port_c.7
#bit PLAYINGPIN		=	lat_c.2
#bit PLAYINGPIN_TRIS		=	tris_c.2

//#bit DO_TRACK 		= port_c.2
//#bit DO_VOLUME_L 	= port_b.6
//#bit DO_VOLUME_R 	= port_b.7

#bit DO_TRACK 		= port_e.0
#bit GO_NOGO		 	= port_e.1
#bit TRUERANDOM 	= port_e.2

#bit ANALOGINPUT		= port_a.5

//button control defines:
#bit TRACKUP			= port_d.0
#bit TRACKDOWN		= port_d.1
#bit VOLUMEUP		= port_d.2
#bit VOLUMEDOWN	= port_d.3
#bit PAUSE				= port_d.4
#bit ZEROSTOP			= port_d.5


//JUMPER SETTINGS
//W=highspeed jumper

#define	FOURFOUR				0b001	//XY-
#define	PARALLEL				0b100	//-YZ

#define	SERIAL_PINNED		0b010	//X-Z
//#define	SERIAL_NOPIN			0b011 	//X--
#define	SERIAL_FULL_NOPIN	0b110	//--Z

#define	SHUFFLE				0b101	//-Y-
#define	BUTTONS				0b111	//---  this is default, handheld mp3 player style
#define	PIN_TO_PLAY			0b000	//XYZ



#define	NONAME	0
#define	PRINTALLOW	1

#define mmc_high_speed 0b00100000
#define mmc_med_speed 0b00100001
#define mmc_low_speed 0b00100010

#define RPMAX 150


#define LEDON 0
#define LEDOFF 1



///////////////////prototypes
void king_of_france (char message);
void blinkenlight(int32 duration, int32 number);
char rand(void);
#inline
void init(void);

void  find_bpb(void);
void printout_rootdir(void);
int32 readfat(int32 fatoffset);
#inline
int32 numbergetter(int16 name);
void find_highest_song_number(void);
int32 get_next_shuffle(void);
int32 get_next_shuffle_super(void);
void TRNG(void);
int16 true_random(int1 clip);

char  getsongstart(int32 filenumber, int1 kludge);
char  song(int32 startsector,int32 endsector);
void play_range(int32 start, int32 end);

void serialhandler(void);
void serialhandler_full(void);

int mmc_init(int1 report);
//int sd_init();

#inline
int mmc_response(unsigned char response);
#inline
int mmc_response_masked(unsigned char mask);

void  mmc_skip(char count);
void mmc_read(void);
int mmc_get_status();
char mmc_open_block(int32 block_number);
char mmc_get_byte(void);
void mmc_close_block(void);
void mmc_cancel_block(void);
#inline
int mmc_read_block_to_serial(int32 block_number,int1 mode);
#inline
int mmc_read_block_to_vs1011(int32 block_number);

int16 vs_command(char inout,address,a,b);
#inline
void resetvs1011_hard(void);
void resetvs1011_soft(void);
void morezeroes(int1 halted);
char vs_spi_write(char aa);

void analogvolumeload(void);
///////////////////////end prototypes


/////////////////////globals
char data_lo, data_hi;

char bigbufferA[32];
char bigbufferB[32];
char bigbufferC[32];
char bigbufferD[32];
char bigbufferE[32];
char bigbufferF[32];

char mode;

char volume_left, volume_right;
char nextcommand;
//char inter_vol_static;
char volcont;
//char portfunction;
char bust;
int1 songkill;
int1 newsong;
int1 pintoplaygo;
int1 pending;
int1 loopflag;
char BPB_SecPerClus;
int16 bpbstart;
//int16 RootDirSectors;
int16 FirstDataSector;
int16 BPB_ResvdSecCnt;
int16 BPB_BytsPerSec;
//int32 FirstRootDirSecNum; //;, songstarter, songlength, currentsong;

int32 songstarter;
int32 songlength;
int32 currentsong;
int32 fatstart, datsec;
int32 ccl, pccl, root_ccl;
int32 root_p_cl;
int32 highestsong;
//int32 BPB_RootEntCnt;
//int32 BPB_FATSz16;
int32 BPB_FATSz32;
int32 BPB_RootClus;
char BPB_NumFATs;

char offset;
char column_pointer[16];
char column_pointer_super[256];
//char testteats[256];
//char pointerh, pointerl, globalpointer;
char random_byte;//global variable for the random number generator (keeps the last value around for seeding purposes?)

int16 recentlyplayed[RPMAX];
char recentlyplayedpointer;
int1 tophalf;
char rphigh;

//int16 vs_insanity;
//char vs_insanity;
int32 shuffle_acum;//peeker;

char throttle;

int1 PAUSEFLAG;

int1 pintoplayflag;
//int1 sdmode;

char analogvolume;
int1 analogvolumeflag;

int1 autoadvance, autorewind;

////////////////////end globals
BYTE CONST RNDL [256]= {208,75,137,110,218,247,165,23,248,216,204,59,118,35,193,29,83,158,93,57,224,194,3,207,205,149,203,22,129,171,10,
142,251,99,249,244,144,16,17,238,37,197,111,151,4,192,104,7,95,150,67,101,69,15,132,186,87,156,206,27,166,20,182, 
163,92,198,119,213,180,61,152,39,185,19,58,45,33,64,24,160,40,222,172,131,227,221,105,108,177,183,73,43,157,12,199,
113,9,245,126,130,106,120,210,215,31,167,246,76,139,195,239,50,229,169,55,122,212,30,112,6,85,141,82,236,21,32,89,
219,173,188,127,74,214,187,190,176,62,123,189,121,184,148,225,34,1,196,28,49,46,79,77,54,117,240,175,242,140,231,
13,128,25,97,146,228,179,116,70,53,178,18,254,78,109,155,241,233,81,255,159,202,102,2,174,14,147,234,60,8,84,52,98,
100,230,145,211,125,11,181,164,26,90,134,161,36,71,223,143,232,136,63,68,51,96,243,66,65,235,47,154,237,252,72,209,
114,138,115,86,250,5,94,133,42,44,88,226,103,253,124,168,91,48,217,56,80,135,220,107,191,201,162,153,38,170,41,200,0};

////////////////////////////////////////////////////////////////////////////////////////////////
#int_timer0
void volume_handler(void){
char g;
	g='0';
	if(volcont==0){
		if(analogvolumeflag==1){
			volume_left=analogvolume;
			volume_right=analogvolume;
		}else{
			if ((!VOLUMEUP)){
				if (volume_left<255){
					volume_left++;
				}
				if (volume_right<255){
					volume_right++;
				}
			}
			if ((!VOLUMEDOWN)){
				if (volume_left>0){
					volume_left--;
				}
				if (volume_right>0){
					volume_right--;
				}
			}
		}
	}
	if(mode==PIN_TO_PLAY){
		volume_left=	analogvolume;
		volume_right=analogvolume;
	}
	if(mode==FOURFOUR){
		volume_left=SLAVE_PORT;
		volume_left &=0b11110000;
		volume_right=volume_left;
	}
	if (mode==SHUFFLE){
		if(analogvolumeflag==1){
			volume_left=analogvolume;
			volume_right=analogvolume;
		}else{
			volume_left=SLAVE_PORT;
			volume_right=SLAVE_PORT;
		}
	}
//	if(XDREQ){vs_insanity++;}
//	if(!XDREQ){vs_insanity=0;}
}
#int_timer1
void TRNG(void){
	;;
}
#INT_RDA  
void serialinterrupt(void){
//	if(mode==SERIAL_NOPIN){
//		serialhandler();
//	}
	if(mode==SERIAL_FULL_NOPIN){
		serialhandler_full();
	}
}	            

#int_ext
void strobe_line(void) {

//	putc('L');
//	if(mode==SERIAL_PINNED){
//		disable_interrupts(INT_EXT);
//		serialhandler_full();
//		INT0IF=0;
//		enable_interrupts(INT_EXT);
//	}

	if(mode==PARALLEL){
		if(DO_TRACK){
			putc('t');
			currentsong=SLAVE_PORT;
			songkill=1;
			newsong=1;
			pending=1;
		}
		if(!DO_TRACK){
			putc('v');
			volume_left=SLAVE_PORT;
			volume_right=volume_left;
		}
	}

	if(mode==FOURFOUR){
		putc(SLAVE_PORT);
		currentsong=SLAVE_PORT;
		currentsong &= 0b00001111;
			songkill=1;
			newsong=1;
			pending=1;
	}

	if (mode==SHUFFLE){
			songkill=1;
	}
}


#zero_ram

void main(void){

char flag, result, n, rpl, serialstuff;
int1 flaglet;

int32 truenumber;
int32 truenumber_b;

	init();
	PLAYINGPIN_TRIS=0;
	PLAYINGPIN=0;
	ADCON1=0b00001111;

	mode = JUMPERS;
	mode &= 0b00000111;
	if(bit_test(JUMPERS,3)){throttle=mmc_low_speed;}
	else{throttle=mmc_high_speed;};

	ADCON1=0b00001010;//this must be AFTER the lines above, or else the jumper pins will be analog at the wrong time
	ADCON2=0b10001000;
	INTEDG0=0;
	RBPU=0;
	INT0IF=0;
	T0CON=0b10000010;
	MMC_TRANSISTOR=1;//for p-channel, this would be off
	xreset=0;//hard reset
	delay_ms(50);
	putc(13);putc(10);
	putc(13);putc(10);
	king_of_france(0);
	putc(13);putc(10);
	putc(13);putc(10);
	printf("Firmware V1.3.1 CC 2001-2008 Raphael Abrams, www.teuthis.com");
	putc(13);putc(10);
	putc(13);putc(10);

	resetvs1011_hard();
	xcs=1;
	xdcs=1;
	PAUSEFLAG=0;
	bust=0;
	volcont=0;
	volume_right=0xdf;
	volume_left=0xdf;

	analogvolume=0;
	analogvolumeflag=0;
	autoadvance=0;
	autorewind=0;
	serialstuff=0;
	nextcommand='0';
	result=1;
	for (n=0;(result==1)&(n<10);n++){
		result=mmc_init(1);
	}
	if (result==1){printf("problem initializing card, going to sleep"); while(1){sleep();}}

	find_bpb();
	if(BPB_BytsPerSec!=0x200){
		printf("problem reading BPB");	putc(13);putc(10);
		find_bpb();
	}
	printf("file system found");	putc(13);putc(10);
	currentsong=0x01;
	find_highest_song_number();
	printf("root dir size is %u",highestsong);putc(13);putc(10);
	root_ccl= BPB_RootClus;
	flag='s';
	loopflag=0;
	newsong=1;
	result=0x00;
	songkill=0;
	pending=0;
	enable_interrupts(GLOBAL);
	disable_interrupts(INT_TIMER0);

	if(highestsong>(RPMAX*2)){
		rphigh=RPMAX;
	}else{
		rphigh=highestsong/2;
	}	
	for(n=0;n<rphigh;n++){
		recentlyplayed[n]=0xffff;
	}
	putc(13);putc(10);
	putc(13);putc(10);
	if(throttle==mmc_high_speed){
		printf("SPI throttle= FAST! this mode does not support all cards!");
	}else{
		printf("SPI throttle= SLOW");
	}
	putc(13);putc(10);
	analogvolumeload();//load up the analog volume register
	if(mode==SERIAL_FULL_NOPIN){
		putc(13);putc(10);
		printf("MODE is SERIAL_FULL_NOPIN");	putc(13);putc(10);
		putc(13);putc(10);
		volcont=1;
		newsong=0;
		currentsong=0xffff;
		truenumber=numbergetter(currentsong);
		while(1){
			while((loopflag==0)&&(newsong==0)){
				delay_ms(100);
				putc('.');
			}
			truenumber=numbergetter(currentsong);
			if (truenumber>0){
				do{
					if (truenumber>4){getsongstart(truenumber-5,NONAME);}
					if (truenumber>3){getsongstart(truenumber-4,NONAME);}
					if (truenumber>2){getsongstart(truenumber-3,NONAME);}
					if (truenumber>1){getsongstart(truenumber-2,NONAME);}
					if (truenumber>0){getsongstart(truenumber-1,NONAME);}
					getsongstart(truenumber,PRINTALLOW);
					putc(13);putc(10);
					flag=song( songstarter,(songstarter+(songlength/512)) );
				}while((loopflag==1)&&(newsong==0));
				delay_ms(1);
			}
		}
	}

	if(mode==PIN_TO_PLAY){
		printf("MODE is PIN TO PLAY");	putc(13);putc(10);
//		while(SLAVE_PORT==0xff){;}
//		currentsong = SLAVE_PORT;
		newsong=0;
		while(1){
			volcont=1;
//			newsong=1;
			if(newsong==0){
				currentsong = SLAVE_PORT;
			}
			truenumber=0;
				if(currentsong==(255-1)){
					truenumber_b = 0;
				}
				if(currentsong==(255-2)){
					truenumber_b = 1;
				}
				if(currentsong==(255-4)){
					truenumber_b = 2;
				}
				if(currentsong==(255-8)){
					truenumber_b = 3;
				}
				if(currentsong==(255-16)){
					truenumber_b = 4;
				}
				if(currentsong==(255-32)){
					truenumber_b = 5;
				}
				if(currentsong==(255-64)){
					truenumber_b = 6;
				}
				if(currentsong==(255-128)){
					truenumber_b = 7;
				}
				if(currentsong !=255){
					truenumber=numbergetter(truenumber_b);
				}
//			}	
			if(truenumber != 0){
				getsongstart(truenumber,NONAME);
				newsong=0;
				flag=song( songstarter,(songstarter+(songlength/512)) );
//				pintoplayflag=0;
//				while(newsong==0){;}
			}else{newsong=0;};
		}
	}
	if(mode==FOURFOUR){
		printf("MODE is FOUR-FOUR");	putc(13);putc(10);
		enable_interrupts(int_ext);
		enable_interrupts(int_timer0);
		volcont=1;
		while(!pending){;}
		while(1){
			truenumber=numbergetter(currentsong);
			if(truenumber>0){
				if (truenumber>4){getsongstart(truenumber-5,NONAME);}
				if (truenumber>3){getsongstart(truenumber-4,NONAME);}
				if (truenumber>2){getsongstart(truenumber-3,NONAME);}
				if (truenumber>1){getsongstart(truenumber-2,NONAME);}
				if (truenumber>0){getsongstart(truenumber-1,NONAME);}
				getsongstart(truenumber,PRINTALLOW);
				flag=song( songstarter,(songstarter+(songlength/512)) );
			}
			while(!pending){;}
			pending=0;
		}
	}

	if (mode==PARALLEL){
		printf("MODE is PARALLEL PORT");	putc(13);putc(10);
		enable_interrupts(int_ext);
		enable_interrupts(int_timer0);
		volcont=1;
		while(!pending){;}
		while(1){
			truenumber=numbergetter(currentsong);
			if(truenumber>0){
				if (truenumber>4){getsongstart(truenumber-5,NONAME);}
				if (truenumber>3){getsongstart(truenumber-4,NONAME);}
				if (truenumber>2){getsongstart(truenumber-3,NONAME);}
				if (truenumber>1){getsongstart(truenumber-2,NONAME);}
				if (truenumber>0){getsongstart(truenumber-1,NONAME);}
				getsongstart(truenumber,PRINTALLOW);
				flag=song( songstarter,(songstarter+(songlength/512)) );
			}
			while(!pending){;}
			pending=0;
		}
	}

	if (mode==SHUFFLE){
		if(!DO_TRACK){ //this is to test for analog volume mode, ignore the name of the pin
			analogvolumeflag=1;
		}
		printf("MODE is SHUFFLE/RANDOM");	putc(13);putc(10);
		if((GO_NOGO==0)){
			while(SERIAL_INTERRUPT){;}
		}
		while(1){
			shuffle_acum=0;
			volcont=2;
			enable_interrupts(int_ext);
			for(n=0;n<255;n++){
				column_pointer_super[n]=0;		
			}
				column_pointer_super[255]=0;
				random_byte=235;
			 offset=23;//arbitrary choices here...
			do{
				if(!TRUERANDOM){
					currentsong=get_next_shuffle_super();
					flaglet=1;
				}else{
					currentsong=true_random(1);
				//	printf("____%LX____",currentsong);				
					for(rpl=0;rpl<rphigh;rpl++){
						if(recentlyplayed[rpl]==(int16)currentsong){
							flaglet=0;
							break;
						}else{
							flaglet=1;
						}
					}
				}
//				for(rpl=0;rpl<rphigh;rpl++){
//					if(recentlyplayed[rpl]==(int16)currentsong){
//						flaglet=0;
//						break;
//					}else{
//						flaglet=1;
//					}
//				}
				if(flaglet==1){
					if (currentsong>4){getsongstart(currentsong-5,NONAME);}
					if (currentsong>3){getsongstart(currentsong-4,NONAME);}
					if (currentsong>2){getsongstart(currentsong-3,NONAME);}
					if (currentsong>1){getsongstart(currentsong-2,NONAME);}
					if (currentsong>0){getsongstart(currentsong-1,NONAME);}
					result=getsongstart(currentsong,PRINTALLOW);
					if(result==0x01){
//						printf("playing: %lX ",currentsong);	putc(13);putc(10);//	putc(13);putc(10);
//						printf("songstarter: %lX ",songstarter);	putc(13);putc(10);//	putc(13);putc(10);
						flag=song( songstarter,(songstarter+(songlength/512)) );
						if((GO_NOGO==0)&&(flag=='s')){
							while(SERIAL_INTERRUPT){;}
						}
					}

					
					if(TRUERANDOM){
						for(rpl=rphigh-2;rpl<rphigh;){
							recentlyplayed[rpl+1]=recentlyplayed[rpl];
							rpl--;
						}
						recentlyplayed[0]=(int16)currentsong;
					}
					putc(';');
				}							
			}while(shuffle_acum<64999);
		}
	}

	printf("MODE is DEFAULT");	putc(13);putc(10);
	if(GO_NOGO==0){
		while(1){	
			if(kbhit()){
				serialstuff=getc();
				BREAK;
			}
			if(!TRACKUP){
				BREAK;
			}
			if(!TRACKDOWN){
				BREAK;
			}
		}
	}
	if(!DO_TRACK){ //this is to test for analog volume mode, ignore the name of the pin
		analogvolumeflag=1;
	}

	while(1){
			////auto track increment. press and hold... single increment/decrement of track is within the "song" function
		if (!TRACKUP){
			for(n=120;n>0;n--){
				delay_ms(3);
				if (TRACKUP){
					break;
				}
			}
			while (!TRACKUP){
				delay_ms(1);
				n++;
				if (n==30){
					n=0;
					currentsong++;
					if (currentsong>highestsong){currentsong=0x01;}//putc('s');}
					//putc('f');
				}
			}
		}
		if (!TRACKDOWN){
			for(n=120;n>0;n--){
				delay_ms(3);
				if (TRACKDOWN){
					break;
				}
			}
			while (!TRACKDOWN){
				delay_ms(1);
				n++;
				if (n==30){
					n=0;
					currentsong--;
					if (currentsong==0x00){currentsong=highestsong;}// putc('s');}
					//putc('f');
				}
			}
		}    ////end AUTO TRACK INCREMENT	

//begin alternate serial control version of standard buttons mode. 
//	disable_interrupts(GLOBAL);
//	disable_interrupts(INT_RDA);

		if(kbhit()){
			serialstuff=getc();
		//	printf("serialstuff in main ");putc(serialstuff);
		}
		if(serialstuff=='n'){
			currentsong++;
			if (currentsong>highestsong){currentsong=0x01;}//putc('s');}
			serialstuff=0;
		}
		else if(serialstuff=='p'){
			currentsong--;
			if (currentsong==0x00){currentsong=highestsong;}// putc('s');}
			serialstuff=0;
		}
		else if(serialstuff=='u'){
				if (volume_left<255){
					volume_left++;
				}
				if (volume_right<255){
					volume_right++;
				}
		}		
		else if(serialstuff=='d'){
				if (volume_left>0){
					volume_left--;
				}
				if (volume_right>0){
					volume_right--;
				}
		}
		else if (serialstuff=='9'){
			currentsong=0x09;
			serialstuff=0;
		}
		else if (serialstuff=='M'){
			currentsong=0x020;
			serialstuff=0;
		}

//end alternate serial control

		if ((TRACKUP)&&(TRACKDOWN)){
			n=0;
			result=getsongstart(currentsong,NONAME);
			if(result==0x02){
				currentsong=0x01;
				flag='s';                    //got to end of card, going back to track one
			}
			else if(result==0x01){
				if (currentsong>4){getsongstart(currentsong-5,NONAME);}
				if (currentsong>3){getsongstart(currentsong-4,NONAME);}
				if (currentsong>2){getsongstart(currentsong-3,NONAME);}
				if (currentsong>1){getsongstart(currentsong-2,NONAME);}
				if (currentsong>0){getsongstart(currentsong-1,NONAME);}
				getsongstart(currentsong,PRINTALLOW);
				flag=song( songstarter,(songstarter+(songlength/512)) );
				if((GO_NOGO==0)&&(flag=='s')){
					do{
						if(kbhit()){
							serialstuff=getc();
							BREAK;
							printf("fooferaw 5");
//							if(serialstuff=='n'){flag='s';BREAK;}
//							if(serialstuff=='p'){flag='p';BREAK;}
						}
						if(!TRACKUP){flag='s';BREAK;}
						if(!TRACKDOWN){flag='p';BREAK;}
					}while(1);//((TRACKUP)&&(TRACKDOWN));
				}
			}
			if ((flag=='n')||(flag=='s')){
					currentsong++;             //auto advanced
			}
			else if(flag=='z'){currentsong=0x01;flag='s';}
			else if(flag=='9'){currentsong=0x09;flag='s';}
			else if(flag=='M'){currentsong=0x20;flag='s';}

			else if (flag=='p'){
				if (currentsong>1){
					currentsong--;        //previous track: attempting
				}
				else{currentsong=highestsong;}//loop around to the last track on the card
			}
		}
	}
}/////////////////////////////////////////end buttons mode///end main() function////////////////////////////////////



void serialhandler(void){
char n;
	disable_interrupts(GLOBAL);
	disable_interrupts(INT_RDA);
	n=getchar();
	putc(n);
	if(n>=192){
		n -= 192;
		n *= 4;
		volume_left=n;
		volume_right=n;
	}
	else if(n==184){
		PAUSEFLAG=(!PAUSEFLAG);//toggle pause on/off
	}
	else if(n==183){
		loopflag=0;//looping on
	}
	else if(n==182){
		loopflag=1;//looping off
	}
	else if(n==181){
		songkill=1;
		newsong=0;
		PAUSEFLAG=0;
	}
		if(n<=180){
//		printf(" Track getter: 0x%4LX",n);putc(13);putc(10);
		songkill=1;
		newsong=1;
		PAUSEFLAG=0;
		nextcommand='0';
	}	
//	vs_insanity=0;
	enable_interrupts(GLOBAL);
	enable_interrupts(INT_RDA);
}

void serialhandler_full(void){
char n;
int16 temporary, temporary_b;
	disable_interrupts(GLOBAL);
	disable_interrupts(INT_RDA);
	n=getchar();
	putc(n);
	if(nextcommand=='l'){
		volume_left=n;
		nextcommand='0';
	}
	else if(nextcommand=='r'){
		volume_right=n;
		nextcommand='0';
	}
	else if(nextcommand=='v'){
		volume_right=n;
		volume_left=n;
		nextcommand='0';
	}
	else if(nextcommand=='0'){
		if(n=='l'){
			nextcommand='l';
		}
		else if(n=='r'){
			nextcommand='r';
		}
		else if(n=='v'){
			nextcommand='v';
		}
		else if(n=='a'){
			loopflag=0;  //printf("looping off...");putc(13);putc(10);
			nextcommand='0';
		}
		else if(n=='b'){
			loopflag=1;  //printf("looping on!");putc(13);putc(10);
			nextcommand='0';
		}
		else if(n=='k'){
			songkill=1; //stop song alltogether
			newsong=0;
			nextcommand='0';
			PAUSEFLAG=0;
		}

		else if(n=='t'){
			n=getchar();	
			putc(n);  ////n -=48;//add this for fake ascii
			temporary = n;
			temporary = temporary<< 8;
			n=getchar();	
			putc(n); //   //n -=48;//add this for fake ascii
			temporary += n;
			temporary_b=temporary; ////putc(temporary_b+48);//fake ascii echo?
			songkill=1;
			newsong=1;
			currentsong=temporary_b;
			PAUSEFLAG=0;
			nextcommand='0';
		}
		else if(n=='p'){
			PAUSEFLAG=(!PAUSEFLAG);
		}
	}
//	vs_insanity=0;
	enable_interrupts(GLOBAL);
	enable_interrupts(INT_RDA);
}



///////////////////////////////////begin number getter:////////////

#inline
int32 numbergetter(int16 name){
	char Emm,Pee,Three, subsect;
	unsigned char l,m,h,ll,mm,hh;
	unsigned char xl,xm,xll,xmm;
	int32 ccltemp, eocmark;
	int32 cancelizer;
	int32  x, y, z, filenumber;
	char takeup;
	takeup=0;

	xm=name/10000;
	cancelizer=(int32)xm*10000;
	xl=(name-cancelizer)/1000;
	cancelizer=cancelizer+((int32)xl*1000);
	h=(name-cancelizer)/100;
	cancelizer=cancelizer+((int32)h*100);
	m=(name-cancelizer)/10;
	cancelizer=cancelizer+((int32)m*10);
	l=name-cancelizer;
	filenumber=0;

	do{
		root_ccl= BPB_RootClus;
		y=filenumber/16;
		z=y;
		y=y/BPB_SecPerClus;
		x=y*BPB_SecPerClus;
		x=z-x;	
		while(y>0){
			root_p_cl=root_ccl;
			root_ccl=readfat(root_p_cl);
			y--;
		}
		ccltemp=root_ccl-2;
		ccltemp=ccltemp * (int32)BPB_SecPerClus;
		ccltemp=ccltemp + (int32)datsec;
		ccltemp=ccltemp + (int32)x;
		mmc_open_block(ccltemp); 
		for (subsect=16;subsect>0;subsect--){
			mmc_read();
			takeup++;
			if((data_lo==0x00)||(data_lo == 0xe5)){//these are either blank entries or deleted ones. probably should escape from the loop if they are blank, but just to be sure we'll look to the end.
				mmc_skip(15); 
				takeup+=15;
			}else{
				xmm=data_lo-48;
				xll=data_hi-48;
				mmc_read();
				takeup++;
			 	hh=data_lo-48;
				mm=data_hi-48;
				mmc_read();
				takeup++;
			 	ll=data_lo-48;
				if ((xl==xll) && (xm==xmm) && (l==ll) && (m==mm) && (h==hh)){
					mmc_read();
					mmc_read();
					Emm=data_lo;
					Pee=data_hi;
					mmc_read();
					takeup+=3;
					Three=data_lo;
					if (((Emm=='m') || (Emm=='M')|| (Emm=='W')|| (Emm=='w')) && ((Pee=='P') || (Pee=='p')|| (Pee=='A')|| (Pee=='a')) && (Three=='3')||(Three=='V')||(Three=='v')){
						mmc_skip(256-takeup);
						mmc_close_block();
						return(filenumber);
					}
				}else{
					mmc_skip(3);
				}
				mmc_skip(10);
			}
			filenumber++;
		}
		eocmark=root_ccl & 0x0FFFFFFF;		
		mmc_close_block();
	}while((filenumber <= highestsong) && (eocmark<0x0ffffff0)); //	printf("file does not exist!");putc(13);putc(10);
	delay_cycles(1);
	putc('-');
	return (0x00); //complete failure! (the file does not exist even after searching entire root directory cluster chain) (fat 32 mode)
}
///////////////////////////////////


char getsongstart(int32 filenumber, int1 kludge){

char longname[65];
char sum, i,ii, j;
char clipper;//, clip;
char Emm,Pee,Three;
int32 Largeccl, ccltemp, eocmark;
int16  x, y, z;
char takeup;
	takeup=0;
	eocmark=0;
		root_ccl= BPB_RootClus;
		y=filenumber/16;
		z=y;
		y=y/BPB_SecPerClus;
		x=y*BPB_SecPerClus;
		x=z-x;	
		while(y>0){
			root_p_cl=root_ccl;
			root_ccl=readfat(root_p_cl);  //printf("current root cluster:%4LX   ",root_ccl);
			eocmark=root_ccl & 0x0FFFFFFF;
			y--;
		}
		if (eocmark>0x0fffffef){
			return (0x02);
		}
		ccltemp=root_ccl-2;
		ccltemp=ccltemp * (int32)BPB_SecPerClus;
		ccltemp=ccltemp + (int32)datsec;
		ccltemp=ccltemp + (int32)x;
		mmc_open_block(ccltemp); 
		takeup=((filenumber & 0x000F)*16);
		mmc_skip(takeup);
						
		for(i=0;i<32;){//load bigbuffer with directory entry information for later use. Also load shadow buffers for long filename support.
			mmc_read();
			takeup++;
			bigbufferF[i]=bigbufferE[i];//these buffers are globals. they are used to hold data about each file as it is read, over several calls to this function
			bigbufferE[i]=bigbufferD[i];
			bigbufferD[i]=bigbufferC[i];
			bigbufferC[i]=bigbufferB[i];
			bigbufferB[i]=bigbufferA[i];
			bigbufferA[i]=data_lo;
			i++;
			bigbufferF[i]=bigbufferE[i];
			bigbufferE[i]=bigbufferD[i];
			bigbufferD[i]=bigbufferC[i];
			bigbufferC[i]=bigbufferB[i];
			bigbufferB[i]=bigbufferA[i];
			bigbufferA[i]=data_hi;
			i++;
		}		//done loading bigbuffer with directory entry information for later use
		mmc_skip(255-takeup);
		mmc_read();	mmc_read();
		mmc_close_block();
		if((bigbufferA[0]!=0x00)&&(bigbufferA[0] != 0xe5)){//these are either blank entries or deleted ones.
			sum=0;                           //for long filename checksum calculation. start with a zero.
			for(ii=0;ii<21;ii++){
				longname[ii]=255;
			}
			for (sum = i = 0; i < 11; i++) {
					sum = (((sum & 1) << 7) | ((sum & 0xfe) >> 1)) + bigbufferA[i];
			}
			clipper=0;
			if (sum==bigbufferB[13]){
				longname[0]=bigbufferB[1];
				longname[1]=bigbufferB[3];
				longname[2]=bigbufferB[5];
				longname[3]=bigbufferB[7];
				longname[4]=bigbufferB[9];
				longname[5]=bigbufferB[14];
				longname[6]=bigbufferB[16];
				longname[7]=bigbufferB[18];
				longname[8]=bigbufferB[20];
				longname[9]=bigbufferB[22];
				longname[10]=bigbufferB[24];
				longname[11]=bigbufferB[28];
				longname[12]=bigbufferB[30];
				clipper=13;
				for (ii=0;ii<13;ii++){
					if (longname[ii]==0){clipper=ii;}
				}
				if (sum==bigbufferC[13]){
					longname[13]=bigbufferC[1];
					longname[14]=bigbufferC[3];
					longname[15]=bigbufferC[5];
					longname[16]=bigbufferC[7];
					longname[17]=bigbufferC[9];
					longname[18]=bigbufferC[14];
					longname[19]=bigbufferC[16];
					longname[20]=bigbufferC[18];
					longname[21]=bigbufferC[20];
					longname[22]=bigbufferC[22];
					longname[23]=bigbufferC[24];
					longname[24]=bigbufferC[28];
					longname[25]=bigbufferC[30];
					clipper=25;
					for (ii=13;ii<26;ii++){
						if (longname[ii]==0){clipper=ii;}
					}
					if (sum==bigbufferD[13]){
						longname[26]=bigbufferD[1];
						longname[27]=bigbufferD[3];
						longname[28]=bigbufferD[5];
						longname[29]=bigbufferD[7];
						longname[30]=bigbufferD[9];
						longname[31]=bigbufferD[14];
						longname[32]=bigbufferD[16];
						longname[33]=bigbufferD[18];
						longname[34]=bigbufferD[20];
						longname[35]=bigbufferD[22];
						longname[36]=bigbufferD[24];
						longname[37]=bigbufferD[28];
						longname[38]=bigbufferD[30];
						clipper=38;
						for (ii=26;ii<39;ii++){
							if (longname[ii]==0){clipper=ii;}
						}

					if (sum==bigbufferE[13]){
						longname[39]=bigbufferE[1];
						longname[40]=bigbufferE[3];
						longname[41]=bigbufferE[5];
						longname[42]=bigbufferE[7];
						longname[43]=bigbufferE[9];
						longname[44]=bigbufferE[14];
						longname[45]=bigbufferE[16];
						longname[46]=bigbufferE[18];
						longname[47]=bigbufferE[20];
						longname[48]=bigbufferE[22];
						longname[49]=bigbufferE[24];
						longname[50]=bigbufferE[28];
						longname[51]=bigbufferE[30];
						clipper=51;
						for (ii=39;ii<52;ii++){
							if (longname[ii]==0){clipper=ii;}
						}
					}

					if (sum==bigbufferF[13]){
						longname[52]=bigbufferF[1];
						longname[53]=bigbufferF[3];
						longname[54]=bigbufferF[5];
						longname[55]=bigbufferF[7];
						longname[56]=bigbufferF[9];
						longname[57]=bigbufferF[14];
						longname[58]=bigbufferF[16];
						longname[59]=bigbufferF[18];
						longname[60]=bigbufferF[20];
						longname[61]=bigbufferF[22];
						longname[62]=bigbufferF[24];
						longname[63]=bigbufferF[28];
						longname[64]=bigbufferF[30];
						clipper=64;
						for (ii=52;ii<65;ii++){
							if (longname[ii]==0){clipper=ii;}
						}
					}
				}
			}
		}else{
			longname[0]=bigbufferA[0];		
			longname[1]=bigbufferA[1];
			longname[2]=bigbufferA[2];
			longname[3]=bigbufferA[3];
			longname[4]=bigbufferA[4];
			longname[5]=bigbufferA[5];
			longname[6]=bigbufferA[6];
			longname[7]=bigbufferA[7];
			longname[8]='.';
			longname[9]=bigbufferA[8];
			longname[10]=bigbufferA[9];
			longname[11]=bigbufferA[10];
			clipper=12;
		}
	}
//////////////////////?????///////////////////
	if(bigbufferA[0]==0x00){
		return(0x00);
	}
	if(bigbufferA[0] != 0xe5){
		Emm=	bigbufferA[8];		//Emm=data_lo;
		Pee=bigbufferA[9];		//Pee=data_hi;
		Three=bigbufferA[10];		//Three=data_lo;
		if (((Emm=='m') || (Emm=='M')|| (Emm=='W')|| (Emm=='w')) && ((Pee=='P') || (Pee=='p')|| (Pee=='A')|| (Pee=='a')) && (Three=='3')||(Three=='V')||(Three=='v')){
			Largeccl= ((int16)bigbufferA[21]<<8)+bigbufferA[20];
			Largeccl=Largeccl<<16;
			ccl= ((int32)bigbufferA[27]<<8)+bigbufferA[26];
			ccl = ccl+ Largeccl;   //add on the high bytes of the cluster address
			songstarter=( ((((int32)bigbufferA[26]+((int32)bigbufferA[27]<<8))-0)  *BPB_SecPerClus) +bpbstart+Firstdatasector);
			*(((char*)&songlength)+0)=bigbufferA[28];
 			*(((char*)&songlength)+1)=bigbufferA[29];
			*(((char*)&songlength)+2)=bigbufferA[30];
			*(((char*)&songlength)+3)=bigbufferA[31];
			if(kludge){
				putc(13);putc(10);
				for (j=0;j<clipper;j++){
					putc(longname[j]);
				}	
				putc(13);putc(10);
			}	
			return(0x01);
		}
	}
	return(0x00);
}


char	song(int32 startsector,int32 endsector){
	int32 ccltemp,eocmark,totalsectors, totalsectorsdone;

	char insanity;
	char x, serialstuff;
	x=0;
	insanity=0;
	totalsectorsdone=0;
	vs_command(0x02,0x0b,254,254);
//	morezeroes(1);
	resetvs1011_soft();
//	vs_command(0x02,0x0b,254,254);
//	morezeroes(1);
	vs_command(0x02,0x0b,255-volume_right,255-volume_left);
	totalsectors = (int32)endsector - (int32)startsector;
//	printf("start %LX, end %LX, total=%LX",startsector, endsector, totalsectors);
	startsector	=	startsector/BPB_SecPerClus;
	endsector	=	endsector/BPB_SecPerClus;
	enable_interrupts(INT_TIMER0);
	songkill = 0;
	serialstuff=0;
//	vs_insanity=0;	

	PLAYINGPIN_TRIS=0;
	PLAYINGPIN=1;

//	ccltemp=ccl-2;
//	ccltemp=ccltemp * (int32)BPB_SecPerClus;
//	ccltemp=ccltemp + (int32)datsec;
//	ccltemp=ccltemp + (int32)x;
//	mmc_read_block_to_serial(ccltemp,0);
//	mmc_read_block_to_serial(ccltemp,1);
	do{
		for (x=0;x<BPB_SecPerClus;x++){
			totalsectorsdone++;
//			printf("%LX",totalsectorsdone);putc(13);
//			if(totalsectorsdone>=totalsectors){	vs_command(0x02,0x0b,254,254);BREAK;};
			ccltemp=ccl-2;
			ccltemp=ccltemp * (int32)BPB_SecPerClus;
			ccltemp=ccltemp + (int32)datsec;
			ccltemp=ccltemp + (int32)x;
			if(totalsectorsdone<totalsectors){
				if(mmc_read_block_to_vs1011(ccltemp)==1){;//this function call is where data goes to the decoder
//					printf("sector retry...");
					putc('!');
					if(mmc_read_block_to_vs1011(ccltemp)==1){
						putc('@');
//						printf("RETRY...");
//						if(mmc_read_block_to_vs1011(ccltemp)==1){
//							printf("hot!");mmc_init(0);	
							if(mmc_read_block_to_vs1011(ccltemp)==1){
								printf("lost a sector!");
							}
//						}
					}
				}
			}
//			if(vs_insanity>20){printf("vsinsane!!!!!!!");resetvs1011_hard(); vs_insanity=0;}

			if((mode==BUTTONS)){
//just for serial volume in player  mode
				if(kbhit()){
					serialstuff=getc();
//					printf("serialstuff in main ");putc(serialstuff);
					 if(serialstuff=='u'){
						if (volume_left<255){
							volume_left++;
						}
						if (volume_right<255){
							volume_right++;
						}
					}		
					else if(serialstuff=='d'){
						if (volume_left>0){
							volume_left--;
						}
						if (volume_right>0){
							volume_right--;
						}
					}				
				}
//end just for serial volume in player  mode
				if (serialstuff=='9'){
					disable_interrupts(INT_TIMER0); 
					PLAYINGPIN=0;
					vs_command(0x02,0x0b,254,254);
					morezeroes(0);
					putc('9');
					return('9');
				}
				if (serialstuff=='M'){
					disable_interrupts(INT_TIMER0); 
					PLAYINGPIN=0;
					vs_command(0x02,0x0b,254,254);
					morezeroes(0);
					putc('M');
					return('M');
				}
				if ((!TRACKUP)||(serialstuff=='n')){
					disable_interrupts(INT_TIMER0);
					PLAYINGPIN=0; 
					morezeroes(0);
					vs_command(0x02,0x0b,254,254);
					return('n');
				}
				if ((!TRACKDOWN)||(serialstuff=='p')){
					if (currentsong>0){
						disable_interrupts(INT_TIMER0);
						PLAYINGPIN=0;
						vs_command(0x02,0x0b,254,254);
						morezeroes(0);
						return('p');
					}
				}
				if((!ZEROSTOP)||(serialstuff=='z')){
					volume_left=0xd0;
					volume_right=0xd0;
					disable_interrupts(INT_TIMER0); 
					PLAYINGPIN=0;
					vs_command(0x02,0x0b,254,254);
					morezeroes(0);
					return('z');
				}
			}
			if(mode==PIN_TO_PLAY){
				if(SLAVE_PORT != 0xFF){if(pintoplaygo){
					pintoplaygo=0;		
					currentsong = SLAVE_PORT;
					songkill=0;
					newsong=1;
					PLAYINGPIN=0;
			//		morezeroes(0);
			//		morezeroes(0);
			//		morezeroes(0);
			//		vs_command(0x02,0x0b,254,254);
					return('k');
//					pintoplaygo=1;
				}}
				else{pintoplaygo=1;}
			}

			analogvolumeload();//load up the analog volume register
			while(PAUSEFLAG){
				delay_ms(10);
				analogvolumeload();
			}
			if(bust==1)			{disable_interrupts(INT_TIMER0);PLAYINGPIN=0; vs_command(0x02,0x0b,254,254); return('e');}
			if(songkill==1)		{disable_interrupts(INT_TIMER0); 
				songkill=0;
				printf("_song stopped by user_");
				putc(13);putc(10);
				PLAYINGPIN=0;
				vs_command(0x02,0x0b,254,254);
				morezeroes(0);
				return('k');
			}
		}
//		eocmark=ccl & 0x0FFFFFFF;
		if(startsector<endsector+10){//+10 for safety, can be taken out if it proves to be irrelevant
			pccl=ccl;
			ccl=readfat(pccl);
		}

//		printf("ccl=%08LX",ccl);
		vs_command(0x02,0x0b,255-volume_right,255-volume_left);
		if((mode==BUTTONS)){
			if((!PAUSE)||(serialstuff=='h')){
				delay_ms(20);
				while (!PAUSE){delay_ms(20);}
				while (PAUSE){
					//just for serial volume in player  mode
					if(kbhit()){
						serialstuff=getc();
						//printf("serialstuff in main ");putc(serialstuff);
						if(serialstuff=='h'){ 
							serialstuff='0';
							break;
						}
					 	else if(serialstuff=='u'){
							if (volume_left<255){
								volume_left++;
							}
							if (volume_right<255){
								volume_right++;
							}
						}		
						else if(serialstuff=='d'){
							if (volume_left>0){
								volume_left--;
							}
							if (volume_right>0){
								volume_right--;
							}
						}				
					}
					//end just for serial volume in player  mode
					delay_ms(20);
					if(bust==1){
						PLAYINGPIN=0;
						vs_command(0x02,0x0b,254,254);
						morezeroes(0);
						return('e');
					}
				}
				while (!PAUSE){;}
				delay_ms(20);
			}
		}

		eocmark=ccl & 0x0FFFFFFF;
	}while(eocmark<0x0fffffef);
//	delay_ms(3000);
//	led=LEDON;
//	delay_ms(3000);
//	led=LEDOFF;
//	morezeroes(1);
//	vs_command(0x02,0x0b,254,254);
	disable_interrupts(INT_TIMER0);
	newsong=0;
	PLAYINGPIN=0;
	return('s');
}

void analogvolumeload(void){
	ADCON0=0b00010001; //channel 0, ad on
	ADCON1=0b00001010;
	ADCON2=0b00111110;
	GODONE=1;
	while(GODONE){;}
	analogvolume=ADRESH;
//	analogvolume=200;
	if(analogvolume<1){analogvolume=1;}
//	ADCON0=0b00010001; //channel 4, ad on
}


////////////////////////////////////////////////////////////////////////////////////////////////
void play_range(int32 start, int32 end){
	for(;start<end;start++){
		mmc_read_block_to_vs1011(start);
	}
}




///////////// ///////////// ////////////////// ///////////????????????//////////////
void find_bpb(){
int32 bigtemp;
	bpbstart=0;
	mmc_open_block(0);
	mmc_skip(0xE3);
	mmc_read();
	bpbstart=((int16) data_hi<<8);
	bpbstart+=data_lo;
//	mmc_cancel_block();
	mmc_skip(27);
//	mmc_skip(35);
	mmc_close_block();
	mmc_open_block((int32) bpbstart);

	mmc_skip(5);
	mmc_read();
	BPB_bytspersec=data_hi;	//11
	mmc_read();
	BPB_bytspersec=BPB_bytspersec+((int16) data_lo<<8); //11.12
	BPB_SecPerClus=data_hi;	//13
	mmc_read();
	BPB_ResvdSecCnt=((int16) data_hi<<8)+data_lo;  //14.15
	mmc_read();

	BPB_NumFATs=data_lo;  //16      //BPB_RootEntCnt=data_hi;  //17
	mmc_read();//	BPB_RootEntCnt=BPB_RootEntCnt+((int16) data_lo<<8);  //17.18
	mmc_read();
	mmc_read();//	BPB_FATSz16=data_lo+((int16) data_hi<<8);  //22.23

	mmc_skip(6);
	mmc_read();
	BPB_FATSz32=0;
	BPB_FATSz32=data_lo+((int32) data_hi<<8);  //36.37
	bigtemp=BPB_FATSz32;
	mmc_read();
	BPB_FATSz32=data_lo+((int32) data_hi<<8);  //36.37.38.39
	BPB_FATSz32=BPB_FATSz32 <<16;
	BPB_FATSz32=bigtemp+BPB_FATSz32;
	mmc_read();
	mmc_read();
	mmc_read();
	BPB_RootClus=data_lo+((int32) data_hi<<8);  //44.45
	bigtemp=BPB_RootClus;

	mmc_read();
	mmc_skip(231);
//	mmc_cancel_block();
	mmc_close_block();
	BPB_RootClus=data_lo+((int32) data_hi<<8);  //44.45.46.47
	BPB_RootClus=BPB_RootClus<<16;
	BPB_RootClus=BPB_RootClus+bigtemp;
	root_ccl=BPB_RootClus;

	fatstart=bpbstart+BPB_ResvdSecCnt;
	FirstDataSector = BPB_ResvdSecCnt + (BPB_NumFATs * BPB_FATSz32);// + RootDirSectors;
	datsec = (int32)bpbstart + (int32)firstdatasector;
	putc(13);putc(10);
	printf("some info about card:"); putc(13);putc(10);
	printf(" bpbst:%4LX  ",bpbstart);						putc(13);putc(10); 
	printf(" BPB_bpsec:%4LX  ",BPB_bytspersec);			putc(13);putc(10); 
	printf(" BPB_ScPeC:%4LX  ",BPB_SecPerClus);		putc(13);putc(10); 
	printf(" BPB_RsvdScCt:%4LX  ",BPB_ResvdSecCnt);	putc(13);putc(10); 
	printf(" BPB_NmFAT:%4LX  ",BPB_NumFATs);		putc(13);putc(10); 
	printf(" fatstart: %4LX  ",fatstart);						putc(13);putc(10); 
	printf(" FirstDataSector:%4LX  ",FirstDataSector);		putc(13);putc(10);

}


void find_highest_song_number(void){
int32 clust_high;
int32 eocmark;
	root_ccl= BPB_RootClus;
	clust_high=0;
	do{
		root_p_cl=root_ccl;
		root_ccl=readfat(root_p_cl);
//		printf("cluster seek:%4LX   ",root_ccl);
		eocmark=root_ccl & 0x0FFFFFFF;
		clust_high++;
	}while(eocmark<0x0fffffef);
	highestsong=clust_high * BPB_SecPerClus;
	highestsong=	highestsong*16;
	highestsong--;
}

//-----------------------------------//this is limited to 2048 directory entries
int32 get_next_shuffle(void){
int16 a, work;
char b, c;
	work=0xffff;
	while(work>highestsong){
		b = rand();
		b = b/16;
		if (column_pointer[b]<255){
			c = column_pointer[b]++;
			c=c+offset;
			work=RNDL[c];	//printf("work is: %lX ",work);
			a = (int16) b * 256;
			work=work+a;//for a final number
		}
	}
	return (work);
}

//-----------------------------------//this is limited to 65536 directory entries

int16 true_random(int1 clip){
int16 dingle;
int16 bigpseudo;
int1 hold;
char x,highestbit;
	highestbit=0;
	ADCON0=0b00010001;
	ADCON1=0b00001010;
	ADCON2=0b10111110;
	for(x=0; x<16;x++){
		if (bit_test(highestsong,x)){
			highestbit=x;
		}
	}
	do{

		bigpseudo=rand();
		bigpseudo*= 256;
		bigpseudo+= rand();
		delay_cycles(1);

		for(x=15; x>highestbit;x--){
			bit_clear(bigpseudo,x);
		}
		dingle=0;
		x=0;
		while(x<=highestbit){
			GODONE=1;
			while(GODONE){;}
			if(bit_test(ADRESL,0)){
				hold=1;
			}else{hold=0;};

			delay_us(3);
			GODONE=1;
			while(GODONE){;}
			if(bit_test(ADRESL,0) != hold){
				if(hold){
					bit_set(dingle,x);
				}
				x++;
			}

		}
		dingle=dingle ^ bigpseudo;
	}while((dingle>highestsong)&(clip));
	putc('.');
//	printf("ding is: %lX ",dingle);
	ADCON0=0b00000001;
	return (dingle);
}

int32 get_next_shuffle_super(void){
int16 a, work;
char b, c;
	work=0xffff;
	while(work>highestsong){
		shuffle_acum++;
		b = rand()-1;
		
	//	printf("sh__%x",b);
		if (column_pointer_super[b]<255){
			c = column_pointer_super[b]++;
//			c=c+offset;
			work=(int16) RNDL[c];	//printf("work is: %lX ",work);
			a = (int16) b * 256;
			work=work+a;
		}
		if (shuffle_acum>65000){return(0);};
	}
//	printf("work is: %lX ",work);
	return (work);
}




char rand(void)	{
	char sum;
	sum = 0;
	// This calculates parity on the selected bits (mask = 0xb4).
	if(random_byte & 0x80){ sum   = 1;}
	if(random_byte & 0x20){ sum ^= 1;}
	if(random_byte & 0x10){ sum ^= 1;}
	if(random_byte & 0x04){ sum ^= 1;}
	random_byte <<= 1;
	random_byte |= sum;
	return(random_byte);
	
}

int32 readfat(int32 fatoffset){
int16 temp;
int32 tempb;
char los;

	temp=0;
	fatoffset=fatoffset*2;
	los = *(((char*)&fatoffset)+0);	//the bottom byte of the address goes directly to a word in the FAT
	fatoffset=fatoffset / 256; 
	fatoffset+=fatstart;
	if(mmc_open_block(fatoffset)==1){
		putc('^');
//		printf("fat retry...");
		if(mmc_open_block(fatoffset)==1){
			putc('&');
			mmc_init(0);
//			printf("fat RETRY...");
			if(mmc_open_block(fatoffset)==1){
				putc('*');
				mmc_init(0);
//				printf("fat LOOKS BAD...");
				if(mmc_open_block(fatoffset)==1){
					printf("problem reading fat table, quitting chain. Sorry!");
					return 0xffffffff;
					songkill=1;
				}
			}
		}
	}
	mmc_skip(los);
	mmc_read();
	temp = ((int16) data_hi * 256)+ (int16) data_lo;
	mmc_read();//for fat32, four bytes per entry
	tempb=0;
	tempb = ((int16) data_hi * 256)+ (int16) data_lo;
	tempb=tempb<<16;
	tempb=tempb+(int32) temp;
	mmc_skip(255-(los));//trouble???
	mmc_read();
	mmc_read();
	mmc_close_block();
	return tempb;
}
//-----------------------------------

////////////////////////////////////////////////////////////////////////////////////////////////
void resetvs1011_hard(void){
	MMCSS=1;	xcs=1;	xdcs=1;
	SSPEN=0;
	delay_ms(50);
		do{;
			xreset=0;//hard reset
			delay_us(250);
			xreset=1;
			delay_ms(50);
			delay_ms(50);
		}while(!XDREQ);
		do{;
		delay_cycles(500);
		vs_command(0x02,0x00,0x00,0x04);
		delay_ms(10);
		}while(!XDREQ);
		do{;
			delay_ms(50);
			delay_ms(50);
		}while(!XDREQ);
	delay_ms(1);
	vs_command(0x02,0x00,0x08,0x00);//go into new mode (w/o pin sharing, w/o sdi tests)
	vs_command(0x02,0x0b,255-volume_right,255-volume_left);//set volume
	delay_ms(1);
//	vs_command(0x02,0x03,0x31,0x2b);
		puts("VS1011: hard reset");
}


//-----------------------------------
void resetvs1011_soft(void){
	vs_command(0x02,0x00,0x08,0x04);//go into new mode (w/o pin sharing, w/o sdi tests)
	delay_ms(1);
	vs_command(0x02,0x0b,255-volume_right,255-volume_left);//set volume
	delay_ms(1);
//	vs_command(0x02,0x03,0x31,0x2b);
	delay_ms(1);
//	morezeroes(0);
}
///////////////////////////////////////////////////////////////////////////////////////////////
void morezeroes(int1 halted){
int16 c;
	xdcs=0;
	for(c=4096;c>0;c--){
		vs_spi_write(0);
		while ((halted)&(!XDREQ)){;}
	}
//	putc('m');	putc('o');	putc('r');	putc('e');
	xdcs=1;
}
////////////////////////////////////////////////////////////////////////////////////////////////
char vs_spi_write(char aa){
char h;
char i;
	h=0;
	i=7;
	XCLK_l=0;delay_cycles(10);
	do{ 
		XDI_l=bit_test(aa,i);	
		XCLK_l=1;
		delay_us(10);
		if(XDO==1){bit_set(h,i);}	
		XCLK_l=0;
		delay_us(10);
	}while((i--)>0);
	return (h);
}
////////////////////////////////////////////////////////////////////////////////////////////////
//#inline
int16 vs_command(char inout, address,a,b){//for sending non-song related data to the decoder..
int16 result;
	SSPCON1 = 0x00; //sspconkludge?
	MMCSS=1;
	xdcs=1;
	XDI_TRIS=0;
	xcs=0;
	XCLK_TRIS=0;
	vs_spi_write(inout); 
	vs_spi_write(address);//load the buffer with address
	*(((char*)&result)+1)=vs_spi_write(a);			//load buffer with hi  data(ignored on read command)
	*(((char*)&result)+0)=vs_spi_write(b);			//load buffer with data low value
	xcs=1;				//deselect the vs1011 command registers (go into sdi mode)
	XDI_TRIS=1;
	SSPCON1=throttle;//used to be 0b00100001
	return (result);
}
////////////////////////////////////////////////////////////////////////////////////////////////
void mmc_read(void){
	data_lo=SPI_READ(0xFF);
	data_hi=SPI_READ(0xFF);
}
/////////////////////////////////////////////////////////////////////////////////////////////////
void mmc_skip(char count){
	for (;count>0;count--){
		SPI_WRITE(0xFF);
		SPI_WRITE(0xFF);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////
char mmc_open_block(int32 block_number){
char tries,i;
int32 block_number_temp;

	for(tries=0;tries<10;tries++){
		SSPCON1=throttle;
		block_number_temp=block_number*512;
//		PUTC(13);PUTC(10);
//		printf("opening boolach");
//		printf("try:%u",tries);
		SPI_WRITE(0xFF);
		SPI_WRITE(0xFF);
		MMCSS=0;                     // set MMCSS = 0 (on)
//		SPI_WRITE(0xFF);
//		SPI_WRITE(0xFF);
		SPI_WRITE(0x51);                // send mmc read single block command
		SPI_WRITE(*(((char*)&block_number_temp)+3)); // arguments are address
		SPI_WRITE(*(((char*)&block_number_temp)+2));
		SPI_WRITE(*(((char*)&block_number_temp)+1));
		SPI_WRITE(0x00);
		SPI_WRITE(0xFF);                // checksum is no longer required but we always send 0xFF
		if((mmc_response(0x00))==0){//((mmc_response(0x00))==0){
			if((mmc_response(0xFE))==0){
				return(0);	
			}
	//		printf("NOFENOFE");
			for(i=0;i<255;i++){
				SPI_WRITE(0xFF);
				SPI_WRITE(0xFF);
			}
		}
		MMCSS=1;            // set MMCSS = 1 (off)
		SPI_WRITE(0xFF);// give mmc the clocks it needs to finish off
		SPI_WRITE(0xFF);// give mmc the clocks it needs to finish off
//		SPI_WRITE(0xFF);// give mmc the clocks it needs to finish off
//		SPI_WRITE(0xFF);// give mmc the clocks it needs to finish off

//		printf("minorcrash%d",tries);
//		if((throttle==mmc_high_speed)&(tries>3)){throttle=mmc_med_speed; printf("now med speed");}
//		else if((throttle==mmc_med_speed)&(tries>5)){throttle=mmc_low_speed; printf("now low speed");}
//		mmc_init(0);
	}
//	puts("MMC crash in: mmc open block.\n\r");
	return 1;
}

void mmc_close_block(void){
//printf("close block");
	SPI_READ(0xFF);                 // CRC bytes that are not needed
	SPI_READ(0xFF);
	MMCSS=1;            // set MMCSS = 1 (off)
	SPI_WRITE(0xFF);                // give mmc the clocks it needs to finish off
       SPI_WRITE(0xff);
//       SPI_WRITE(0xff);
  //     SPI_WRITE(0xff);
}


void mmc_cancel_block(void){
//char rep;

//for(rep=0;rep<=1;rep++){
	mmcss=1;
	SPI_WRITE(0xFF);	
	SPI_WRITE(0xFF);
	mmcss=0;
	SPI_WRITE(0xFF);
//	printf("cancel block");
	SPI_WRITE(0x4C);                // send mmc cancel block command
	SPI_WRITE(0); //no arguments
	SPI_WRITE(0);
	SPI_WRITE(0);
	SPI_WRITE(0);
	SPI_WRITE(0xFF);
	mmc_response(0x00);
		MMCSS=1;
		SPI_WRITE(0xFF); //	printf(" MMC block cancelled");	putc(13);putc(10);
	       SPI_WRITE(0xff);
//	mmc_get_status();
	//	return;
	
//}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
int mmc_init(int1 report){	//Initialises the MMC into SPI mode and sets block size
//char p;
int i, ii, tries;

for(tries=0;tries<10;tries++){
	MMC_TRANSISTOR=1;
//	MMCSS=0;
//	SSPCON1=0;
	XCLK_l=0;
	delay_ms(30);	
	SSPSTAT |= 0x40;                          // set CKE = 1 - clock idle low
	SSPCON1=mmc_low_speed; //was 0b00100001
	MMCSS=1;			//                    // set MMCSS = 1 (off)
	MMC_TRANSISTOR=0;
	delay_ms(30);
	for(i=0;i<10;i++){                       // initialise the MMC card into SPI mode by sending clks on
	        SPI_WRITE(0xFF);
	}
	MMCSS=0;				                     // set MMCSS = 0 (on) tells card to go to spi mode when it receives reset
	SPI_WRITE(0x40);                        // send reset command
 	SPI_WRITE(0x00);
 	SPI_WRITE(0x00);
 	SPI_WRITE(0x00);
 	SPI_WRITE(0x00);
	SPI_WRITE(0x95);                        // precalculated checksum as we are still in MMC mode
	if(report){
		puts(" \n\r");
		puts("MMC/SD init: Sent 0x40 (cmd0) SPI\n\r");
	}
	if(mmc_response(0x01)==0){// return 1;     // if = 1 then there was a timeout waiting for 0x01 from the mmc (bad!)
		if(report){
			puts("MMC/SD init: Got response \n\r");
		}
		i = 0;
		ii=0;
		do{     // must keep sending command if response
			if(report){
				putc(13);putc(10);
				printf("SD init: command55...");putc(13);putc(10);
			}
			MMCSS=1;
	
		       SPI_WRITE(0xff);
		       SPI_WRITE(0xff);
	
			MMCSS=0;
			SPI_WRITE(0x77);//command 55                // send mmc command one to bring out of idle state
		 	SPI_WRITE(0x00);
		 	SPI_WRITE(0x00);
		 	SPI_WRITE(0x00);
		 	SPI_WRITE(0x00);
			SPI_WRITE(0xF0);                // checksum is no longer required but we always send 0xFF
			while(mmc_response_masked(0b10000000)==1){
				i++;
				if(i>10){
					ii = 0;
						if(report){
							printf("not an SD, maybe an mmc?");putc(13);putc(10);
						}
					do{
						MMCSS=1;
		       			SPI_WRITE(0xff);
					       SPI_WRITE(0xff);
						MMCSS=0;
						SPI_WRITE(0x41);                // send mmc command one to bring out of idle state
					 	SPI_WRITE(0x00);
					 	SPI_WRITE(0x00);
					 	SPI_WRITE(0x00);
					 	SPI_WRITE(0x00);
					       SPI_WRITE(0xFF);                // checksum is no longer required but we always send 0xFF
	      			 		ii++;
						if(report){
							printf(".%u",ii);
						}
						if (mmc_response_masked(0b10000001)==0){
							if(report){
								printf("yes, it's an mmc!");putc(13);putc(10);
							}
							goto jump;
						}
					}while(ii < 255);     // must keep sending command if response
				}
			}	
	
		       SPI_WRITE(0xff);
		       SPI_WRITE(0xff);
		       SPI_WRITE(0xff);
		       SPI_WRITE(0xff);
	
			SPI_WRITE(0x69);//command 41, not command 1                // send mmc command one to bring out of idle state
		 	SPI_WRITE(0x00);
		 	SPI_WRITE(0x00);
		 	SPI_WRITE(0x00);
		 	SPI_WRITE(0x00);
			SPI_WRITE(0xFF);                // checksum is no longer required but we always send 0xFF
		       i++;
			if(report){	
				printf("...%ud ",i);// putc(13);putc(10);
			}
			if (mmc_response_masked(0b10000001)==0){goto jump;}
		}while(i < 255);//we want a zero here, right?
		if(report){
			printf("failed out of idle---");putc(13);putc(10);
		}
		return 1;
//		putc(13);putc(10);
	
jump:
		if(report){
			putc(13);putc(10);
			puts("MMC/SD init: Got out of idle response \n\r");putc(13);putc(10);
		}
		MMCSS=1;                    // set MMCSS = 1 (off)
		SPI_WRITE(0xFF);                        // extra clocks to allow mmc to finish off what it is doing
	       SPI_WRITE(0xff);
	       SPI_WRITE(0xff);
	       SPI_WRITE(0xff);
		MMCSS=0;                     // set MMCSS = 0 (on)
	       SPI_WRITE(0xff);
		SPI_WRITE(0x50);                // block size command
		SPI_WRITE(0x00);
		SPI_WRITE(0x00);
		SPI_WRITE(0x02);                // high block length bits - 512 bytes
		SPI_WRITE(0x00);                // low block length bits
		SPI_WRITE(0xFF);                // checksum is no longer required but we always send 0xFF
		if((mmc_response(0x00))==1) return 1; //bad!
		SPI_WRITE(0xff);                // 
		SPI_WRITE(0xff);                // 
		MMCSS=1;            //off
		if(report){
			puts("MMC/SD init: Got set block length response. Done.\n\r");putc(13);putc(10);
		}	
		SPI_WRITE(0xff);                // 
		SPI_WRITE(0xff);                // 
		sspcon1=throttle;
		return 0; //good
	}
}
}
	



////////////////////////////////////////////////////////////////////////////////////////////////
int mmc_get_status(){	// Get the status register of the MMC, for debugging
char  p;
	xcs=1;
	xdcs=1;
	MMCSS=0;                     // set MMCSS = 0 (on)
	SPI_WRITE(0x7a);                // 0x58?
	for(p=4;p>0;p--){
		SPI_WRITE(0x00);
	}
	SPI_WRITE(0xFF);                // checksum is no longer required but we always send 0xFF
	mmc_response(0x00);
	mmc_response(0xff);
	MMCSS=1;                    // set MMCSS = 1 (off)
	SPI_WRITE(0xFF);
	SPI_WRITE(0xFF);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////
#inline
int mmc_response(unsigned char response){	//reads the MMC until we get the response we want or timeout
int16 count;        		// 16bit repeat, it may be possible to shrink this to 8 bit but there is not much point
char temp;
	for(count=520;count>0;count--){
		temp = SPI_READ(0xFF);
		if(temp == response){
			return (0);
		}
	}
	return 1;
}


#inline
int mmc_response_masked(unsigned char mask){	//reads the MMC until we get the response we want or timeout
int16 count;        		// 16bit repeat, it may be possible to shrink this to 8 bit but there is not much point
char temp;
	for(count=520;count>0;count--){
		temp = SPI_READ(0xFF);
//		printf("__%uk", temp);
		temp =temp & mask;
		if(temp==0){
			return (0);
		}
	}
	return 1;
}


////////////////////////////////////////////////////////////////////////////////////////////////
int mmc_read_block_to_serial(int32 block_number, int1 mode){	//read block from the MMC and outputs each byte to RS232. just for fun
unsigned long i;
	mmc_open_block(block_number);
	for(i=0;i<512;i++){
		if(mode){
			printf("%x_",SPI_READ(0xFF));               // we should now receive 512 bytes
		}else{
			putc(SPI_READ(0xFF));
		}
	}
	mmc_close_block();
	puts("\n\r----End of read block----\n\r");
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////
#define KLEE 32

#inline
int mmc_read_block_to_vs1011(int32 block_number){
unsigned long i, ii;//, j;

//mmc_read_block_to_serial(block_number); return 0;

	SSPCON1=throttle;//used to be 0b00100001
//	printf("open vs1011 block");
	block_number*=2;
		SPI_WRITE(0xFF);
		SPI_WRITE(0xFF);
		MMCSS=0;                     // set MMCSS = 0 (on)
//		SPI_WRITE(0xFF);
//		SPI_WRITE(0xFF);
	SPI_WRITE(0x51);                // send mmc read single block command
	SPI_WRITE(*(((char*)&block_number)+2)); // arguments are address
	SPI_WRITE(*(((char*)&block_number)+1));
	SPI_WRITE(*(((char*)&block_number)+0));
	SPI_WRITE(0x00);
	SPI_WRITE(0xFF);                // checksum is no longer required but we always send 0xFF
	
	if((mmc_response(0x00))==0) {
		if((mmc_response(0xFE))==0){
//			XCLK_l=0;
//			SSPCON1 = 0x00;
			XDCS=0;//allow vs1011 to see the spi data
//			SSPCON1=0b00100000;
			for(i=16;i>0;i--){
				while(!XDREQ){;}
				led=LEDON;
				for(ii=0;ii<32;ii++){
			//	putc(SPI_read(0xff));
				SPI_write(0xff);
/*delay_cycles(KLEE);		XCLK_l=1;
delay_cycles(KLEE);		XCLK_l=0; //doing it this way (without looping) to kill off a few cycles per byte
delay_cycles(KLEE);		XCLK_l=1;
delay_cycles(KLEE);		XCLK_l=0;//if you really want to go crazy, do this whole thing 32 times
delay_cycles(KLEE);		XCLK_l=1;
delay_cycles(KLEE);		XCLK_l=0;// that would save you about 1000 clock cycles or so, per block
delay_cycles(KLEE);		XCLK_l=1;
delay_cycles(KLEE);		XCLK_l=0;
delay_cycles(KLEE);		XCLK_l=1;
delay_cycles(KLEE);		XCLK_l=0;
delay_cycles(KLEE);		XCLK_l=1;
delay_cycles(KLEE);		XCLK_l=0;
delay_cycles(KLEE);		XCLK_l=1;
delay_cycles(KLEE);		XCLK_l=0;
delay_cycles(KLEE);		XCLK_l=1;
delay_cycles(KLEE);		XCLK_l=0;
*/
				}
				led=LEDOFF;
			}
			XdCS=1;//turn off vs1011 chip select
somere:
//			delay_cycles(10);
			SSPCON1=throttle;//used to be 0b00100001	SETUP_SPI(SPI_MASTER | SPI_H_TO_L | SPI_CLK_DIV_4 | SPI_SS_DISABLED);
			SPI_READ(0xFF);                 // CRC bytes that are not needed, so we just use 0xFF
			SPI_READ(0xFF);
			MMCSS=1;            // set MMCSS = 1 (off)
			SPI_WRITE(0xFF);// give mmc the clocks it needs to finish off
			SPI_WRITE(0xFF);
			return 0;
		}
	//	printf("no FE_vs ");			putc(13);putc(10);
//		for(i=0;i<185;i++){
//			SPI_WRITE(0xFF);
//			SPI_WRITE(0xFF);
//		}
		SPI_WRITE(0xFF);
		SPI_WRITE(0xFF);
		MMCSS=1;            // set MMCSS = 1 (off)
		SPI_WRITE(0xFF);// give mmc the clocks it needs to finish off
		SPI_WRITE(0xFF);
		return 1;
	}
//	printf("no zed_vs  ");			putc(13);putc(10);
	SPI_WRITE(0xFF);
	SPI_WRITE(0xFF);
	MMCSS=1;            // set MMCSS = 1 (off)
	SPI_WRITE(0xFF);// give mmc the clocks it needs to finish off
	SPI_WRITE(0xFF);
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////
void blinkenlight(int32 duration, int32 number){
	duration /=2;
	for(;number>0;number--){	
		LED=LEDON;
		delay_ms(duration);
		LED=LEDOFF;
		delay_ms(duration);
	}
}

#inline
void init(void){
	lat_c=0x00;
	tris_a=0b01111111;
	tris_b=0b11000101;
	tris_c=0b10010100;//c0 has to be output for mmc transistor...
	tris_d=0b11111111;
	tris_e=0b00000111;//startup NOT in parallel slave port mode
	PLLEN = 1;    // Enable PLL
//	ADCON0=0b00010001; //channel 4, ad on
	ADCON0=0b00000001; //channel 0, ad on
}
void king_of_france (char message){
	if (message==0){
		delay_ms(800);
//		putc(254); putc(1); putc(254); putc(2);
		printf("I am the King ");
//		putc(254); putc(192);
		printf("of France!");
		delay_ms(400);
	}
	if (message==1){
		delay_ms(800);
		putc(254); putc(1); putc(254); putc(2);
		printf("who ate");
		putc(254); putc(192);
		printf("all the pie?!");
		delay_ms(400);
	}
}
