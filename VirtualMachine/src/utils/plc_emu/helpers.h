#include <stdio.h>
#include <signal.h>
#include <time.h>
// #include <asm/io.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

#define VERSION 1.4//minor debug. Instruction List!
#define IOPAGE 1
#define EXITPAGE 2
#define EDITPAGE 3
#define FILEPAGE 4
#define EDIT_MODE 5
#define HELPPAGE 6
#define BYTE unsigned char
#define MAXSTR	1024
#define MAXBUF 65536
#define NICKLEN	16


#define ERROR 		-1
#define ERR_BADCHAR	-2
#define ERR_BADFILE	-3
#define ERR_BADOPERAND	-4
#define ERR_BADINDEX	-5
#define ERR_BADCOIL	-6
#define ERR_BADOPERATOR	-7

#define KEY_TAB		9

#define LANG_LD		0
#define LANG_IL		1
#define LANG_ST		2

struct digital_input
{
    BYTE I;//contact value
    BYTE RE;//rising edge
    BYTE FE;//falling edge
    char nick[NICKLEN];//nickname    
};
struct digital_output
{
    BYTE Q;//contact
    BYTE SET;//set
    BYTE RESET;//reset
    char nick[NICKLEN];//nickname
};
struct timer
{//struct which represents  a timer state at a given cycle
    long S;	//scale; S=1000=>increase every 1000 cycles. STEP= 10 msec=> increase every 10 sec
    long sn;	//internal counter used for scaling
    long V;	//value
    BYTE Q;	//output
    long P;	//Preset value
    BYTE ONDELAY;//1=on delay, 0 = off delay
    BYTE START;//start command: must be on to count
    //BYTE RESET;//down command: sets V = 0
    char nick[NICKLEN];
};

struct blink
{//struct which represents a blinker
    BYTE Q; //output
    long S;	//scale; S=1000=>toggle every 1000 cycles. STEP= 10 msec=> toggle every 10 sec
    long sn;//internal counter for scaling
    char nick[NICKLEN];
};

struct mvar
{
    long V;
    BYTE RO;	//1 if read only;
    BYTE DOWN;	//1: can be used as a down counter
    BYTE PULSE;		//pulse for up/downcounting
    BYTE EDGE;		//edge of pulse
    BYTE SET;		//set pulse
    BYTE RESET;		//reset pulse
    char nick[NICKLEN];    //nickname
};
struct PLC_regs
{
	BYTE *inputs;//input values
	BYTE *outputs;//output values
	BYTE *edgein;	//edges
	BYTE *maskin;	//masks used to force values
	BYTE *maskout;
	BYTE *maskin_N;
	BYTE *maskout_N;
	BYTE command;//serial command from plcpipe
	BYTE response;//response to named pipe
	BYTE status;//0 = stopped, 1= running
	struct digital_input * di;
	struct digital_output * dq;
	struct timer * t;
	struct blink * s;
	struct mvar * m;
};

struct PLC_regs plc;

int Use_comedi,Step,Sigenable,Pagelen,Pagewidth,Nt,Ns,Nm,Di,Dq,Base,Wr_offs,Rd_offs,Comedi_file,Comedi_subdev_i,Comedi_subdev_q;
// const char Pipe[MAXSTR];
// const char Responsefile[MAXSTR];
// const char Hw[MAXSTR];
// const char com_nick[256][16];//comments for up to 256 serial commands

char Lines[MAXBUF][MAXSTR];//ladder lines
int Lineno;	//actual no of active lines 
int Pos[MAXBUF];//cursor position in each line
BYTE Val[MAXBUF];//current resolved value of each line. NEW: if it is final, set to 3.
char Labels[MAXBUF][MAXSTR];


