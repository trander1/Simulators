#include "plclib.h"
// #include "project.h"
extern const char errmsg[][256];
int cur,page;
int in,in_win_buf,out,out_win_buf,tim,tim_win_buf,blink,blink_win_buf;
int mvar,mvar_win_buf,edit,edit_buf,conf,conf_buf,ld,ld_win_buf,file_buf;
int fileselect, del_buf,help,help_buf;

int pagelen,Update,enable;
//Config variables

struct PLC_regs * old;
struct pollfd PLC_com[1];
int rfd;//response file descriptor
int win_flag=FALSE;//0->inputs, 1->outputs 2->mvars, 3->timers, 4->blinkers
struct timeval t;
int save_flag = 0;//0 loads, 1 saves
int Language = LANG_LD;
const char Lang_str[3][18] = {"Ladder Diagram",
			      "Instruction List",
			      "Structured Text"};
			      
int init_config(const char * filename)
{
	FILE * fcfg;
	char line[256], name[128], path[1024], val[64];
//        filename = "plc.config";
	memset(path, 0, 1024);
	sprintf(path, "%s",filename);    
	if(fcfg = fopen(path, "r"))
	{
		memset(line, 0, 128);
		memset(name, 0, 128);
		memset(val, 0, 16);
		while(fgets(line, 256, fcfg))
		{
			sscanf(line, "%s\t%s", name, val);
			if(!strcmp(name,"USE_COMEDI"))
				Use_comedi = atoi(val);
			if(!strcmp(name,"STEP"))
				Step = atoi(val);
			if(!strcmp(name,"SIGENABLE"))
				Sigenable = atoi(val);
			if(!strcmp(name,"PAGELEN"))
				Pagelen = atoi(val);
			if(!strcmp(name,"PAGEWIDTH"))
				Pagewidth = atoi(val);
			if(!strcmp(name,"NT"))
				Nt = atoi(val);
			if(!strcmp(name,"NS"))
				Ns = atoi(val);
			if(!strcmp(name,"NM"))
				Nm = atoi(val);
			if(!strcmp(name,"DI"))
				Di = atoi(val);
			if(!strcmp(name,"DQ"))
				Dq = atoi(val);
			if(!strcmp(name,"BASE"))
				Base = atoi(val);
			if(!strcmp(name,"WR_OFFS"))
				Wr_offs = atoi(val);
			if(!strcmp(name,"RD_OFFS"))
				Rd_offs = atoi(val);
			if(!strcmp(name,"COMEDI_FILE"))
				Comedi_file = atoi(val);
			if(!strcmp(name,"COMEDI_SUBDEV_É"))
				Comedi_subdev_i = atoi(val);
			if(!strcmp(name,"COMEDI_SUBDEV_Q"))
				Comedi_subdev_q = atoi(val);
			if(!strcmp(name,"HW"))
				sprintf(Hw,"%s",val);
			if(!strcmp(name,"PIPE"))
				sprintf(Pipe,"%s",val);			
			if(!strcmp(name,"RESPONSE"))
				sprintf(Responsefile,"%s",val);
			memset(line, 0, 256);
			memset(name, 0, 128);
		} 
		fclose(fcfg);
		if(Use_comedi>=0
			&& Step>0
			&& Sigenable > 29
			&& Pagelen > 23
			&& Pagewidth > 79
			&& Nt >= 0
			&& Ns >= 0
			&& Nm >= 0
			&& Di >= 0
			&& Dq >= 0
			&& Wr_offs >= 0
			&& Rd_offs >= 0
			&& Base > 0)
			return OK;
	}return ERR;
}

int timeval_subtract (struct timeval *result,struct timeval *x,struct timeval *y)
{     /* Subtract the `struct timeval' values X and Y,
        storing the result in RESULT.
        Return 1 if the difference is negative, otherwise 0.  */          
    /* Perform the carry for the later subtraction by updating y. */
    if (x->tv_usec < y->tv_usec)
    {
         int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
         y->tv_usec -= 1000000 * nsec;
         y->tv_sec += nsec;
    }
    if (x->tv_usec - y->tv_usec > 1000000) 
    {
        int nsec = (x->tv_usec - y->tv_usec) / 1000000;
        y->tv_usec += 1000000 * nsec;
        y->tv_sec -= nsec;
    }		     
       /* Compute the time remaining to wait.
          tv_usec is certainly positive. */
       result->tv_sec = x->tv_sec - y->tv_sec;
       result->tv_usec = x->tv_usec - y->tv_usec; 
       /* Return 1 if result is negative. */
       return x->tv_sec < y->tv_sec;
}
void sigenable()
{
    enable=enable?0:1;
}

void init_help()
{
	FILE * f;
	char line[MAXSTR], helpline[MAXSTR];
	buf_clear(help_buf);
	if(f = fopen("./help", "r"))
        {
			while(fgets(line, 256, f))
			{//read help file
				sprintf(helpline," %s",line);
				app_line(help_buf,helpline);
			}
		fclose(f);    
	}
}

void init_emu()
{
    int i;
    enable_bus();
    plc.inputs = (BYTE *)malloc(Di);
    plc.outputs = (BYTE *)malloc(Dq);
    plc.edgein = (BYTE *)malloc(Di);
    plc.maskin = (BYTE *)malloc(Di);
    plc.maskout = (BYTE *)malloc(Dq);
    plc.maskin_N = (BYTE *)malloc(Di);
    plc.maskout_N = (BYTE *)malloc(Dq);
    plc.di = (struct digital_input*)malloc(8*Di*sizeof(struct digital_input));
    plc.dq = (struct digital_output*)malloc(8*Dq*sizeof(struct digital_output));
    plc.t = (struct timer *)malloc(Nt*sizeof(struct timer));
    plc.s = (struct blink *)malloc(Ns*sizeof(struct blink));
    plc.m = (struct mvar *)malloc(Nm*sizeof(struct mvar));
    memset(plc.inputs,0,Di);
    memset(plc.outputs,0,Dq);
    memset(plc.maskin,0,Di);
    memset(plc.maskout,0,Dq);
    memset(plc.maskin_N,0,Di);
    memset(plc.maskout_N,0,Dq);
    memset(plc.di,0,8*Di*sizeof(struct digital_input));
    memset(plc.dq,0,8*Dq*sizeof(struct digital_output));
    memset(plc.t,0,Nt*sizeof(struct timer));
    memset(plc.s,0,Ns*sizeof(struct blink));
    memset(plc.m,0,Nm*sizeof(struct timer));

    old=(struct PLC_regs *)malloc(sizeof(struct PLC_regs));

    old->inputs = (BYTE *)malloc(Di);
    old->outputs = (BYTE *)malloc(Dq);
    old->maskin = (BYTE *)malloc(Di);
    old->edgein = (BYTE *)malloc(Di);
    old->maskout = (BYTE *)malloc(Dq);
    old->maskin_N = (BYTE *)malloc(Di);
    old->maskout_N = (BYTE *)malloc(Dq);
    old->di = (struct digital_input*)malloc(8*Di*sizeof(struct digital_input));
    old->dq = (struct digital_output*)malloc(8*Dq*sizeof(struct digital_output));
    old->t = (struct timer *)malloc(Nt*sizeof(struct timer));
    old->s = (struct blink *)malloc(Ns*sizeof(struct blink));
    old->m = (struct mvar *)malloc(Nm*sizeof(struct mvar));
    
    memcpy(old->inputs,plc.inputs,Di);
    memcpy(old->outputs,plc.outputs,Dq);
    memcpy(old->m,plc.di,Nm*sizeof(struct mvar));
    memcpy(old->t,plc.dq,Nt*sizeof(struct blink));
    memcpy(old->s,plc.dq,Ns*sizeof(struct timer));
    plc.command=0;
    plc.status=TRUE;
    win_flag=TRUE;
    Update=TRUE;
    enable = 1;
    signal(Sigenable,sigenable);
    PLC_init();
    cur = 0;
    
}
void TimeHeader(int page)
{
	char t[30],*p;
	char str[48];
	time_t now;

	time(&now);
	strcpy(t,ctime(&now));
	t[19]='\0';
	p=t+10;
	sprintf(str, " PLC-EMUlator v%4.2f %14s ", VERSION, p);
	draw_header(str);
}
void LoadTimers()
{
    int i;
    char s[32],color;
    buf_clear(tim_win_buf);
    for(i = 0; i < Nt;i++)	
    {
	if(plc.t[i].Q)
	    color='.';//red
	else
	    color='#';//green
	sprintf(s,"%cT%dx%d\t %d/%d %s",color,i,plc.t[i].S,plc.t[i].V,plc.t[i].P,plc.t[i].nick);
	//printf("%s\n",s);
	app_line(tim_win_buf,s);
    }
}
void LoadBlinkers()
{
    int i;
    char s[32],color;
    buf_clear(blink_win_buf);
    for(i = 0; i < Ns;i++)	
    {
	if(plc.s[i].Q)
	    color='.';//red
	else
	    color='#';//green
	sprintf(s,"%cS%dx%d\t %s",color, i, plc.s[i].S, plc.s[i].nick);
	//printf("%s\n",s);
	app_line(blink_win_buf,s);
    }
}

void LoadMvars()
{
    int i;
    char s[32],color;
    buf_clear(mvar_win_buf);
    for(i = 0; i < Nm;i++)	
    {
	if(plc.m[i].PULSE)
	    color='#';//green
	else if(plc.m[i].RO)//locked
	    color='.';//red
	else color=' ';
	sprintf(s,"%cM%d.\t %d %s",color,i,plc.m[i].V,plc.m[i].nick);
	//printf("%s\n",s);
	app_line(mvar_win_buf,s);
    }
}

void LoadInputs()
{
    int i;
    char s[32],color,bit;
    buf_clear(in_win_buf);
    for(i = 0; i < 8*Di;i++)	
    {
	if(!(((plc.maskin[i/8])>>(i%8))%2)
	&&!(((plc.maskin_N[i/8])>>(i%8))%2))
	{

	    if(((plc.inputs[i/8])>>(i%8))%2)
	    {
		color='#';//green
		bit='1';
	    }
	    else 
	    {
		color=' ';
		bit='0';
	    }
	}
	else
	{
	    color='.';//red
	    if(((plc.maskin[i/8])>>(i%8))%2 )
		bit='1';
	    else if(((plc.maskin_N[i/8])>>(i%8))%2)
		bit='0';
	}
	sprintf(s,"%cI%d.\t %c %s",color,i,bit,plc.di[i].nick);
	//printf("%s\n",s);
	app_line(in_win_buf,s);
    }
}
void LoadOutputs()
{
    int i;
    char s[32],color,bit;
    buf_clear(out_win_buf);
    for(i = 0; i < 8*Dq;i++)	
    {
	if(!(((plc.maskout[i/8])>>(i%8))%2)
	&&!(((plc.maskout_N[i/8])>>(i%8))%2))
	{
	    
	    if(((plc.outputs[i/8])>>(i%8))%2)
	    {
		bit='1';
		color='#';//green
	    }
	    else 
	    {
		color=' ';
		bit='0';
	    }
	}
	else
	{
	    color='.';//red
	    if(((plc.maskout[i/8])>>(i%8))%2 )
		bit='1';
	    else if(((plc.maskout_N[i/8])>>(i%8))%2)
		bit='0';
	}
	sprintf(s,"%cQ%d.\t %c %s",color,i,bit,plc.dq[i].nick);
	//printf("%s\n",s);
	app_line(out_win_buf,s);
    }
}
void LoadLD()
{
    int i;
    buf_clear(ld_win_buf);
    for(i=0;i< Lineno;i++)
	app_line(ld_win_buf,Lines[i]);
    
}
int IO_Page()
{
    static int redraw=TRUE;
    int c,i,ret,ch;
    i= 0;
    static int win_sticky;
    char str[48];
    if(Update)
    {
	redraw=TRUE;
	Update=FALSE;
    }
    if (redraw)
    {
	if(plc.status%2)//running
	    sprintf(str,"Hardware:%s Language:%s RUNNING",Hw, Lang_str[Language]);
	else
	    sprintf(str,"Hardware:%s Language:%s STOPPED",Hw, Lang_str[Language]);
	draw_footer(str);
//	draw_info_line(1,"F1/2:Force 1/0|F3:Unforce|F4:Run|F5:Edit|F6:Lock|F7:Load|F8:Save|F9:Help|F10:Quit");

	wdraw(in);
	wdraw(out);
	wdraw(tim);
	wdraw(mvar);
	wdraw(blink);
	wdraw(ld);
		
	LoadInputs();
	LoadOutputs();
	LoadMvars();
	LoadTimers();
	LoadBlinkers();
	LoadLD();
	
	wshowall_c(in,in_win_buf);
	wshowall_c(out,out_win_buf);
	wshowall_c(mvar,mvar_win_buf);
	wshowall_c(tim,tim_win_buf);
	wshowall_c(blink,blink_win_buf);
	wshowall_c(ld,ld_win_buf);
	redraw=FALSE;
    }
    ret=IOPAGE;
    if(enable)
    {
	if(win_flag==DI)
	{
	    draw_info_line(1,"F1/2:Force 1/0|F3:Unforce|F4:Run/Stop|F5:Edit|F7:Load|F8:Save|F9:Help|F10:Quit");
	    i = wselect(in, in_win_buf);
	    win_set(in,i);
	}
	else if(win_flag==DQ)
	{
	    draw_info_line(1,"F1/2:Force 1/0|F3:Unforce|F4:Run/Stop|F5:Edit|F7:Load|F8:Save|F9:Help|F10:Quit");
	    i = wselect(out, out_win_buf);
	    win_set(out,i);
	}
	else if(win_flag==COUNTER)
	{
	    draw_info_line(1,"F1/2:Toggle Pulse|F4:Run/Stop|F5:Edit|F6:Lock|F7:Load|F8:Save|F9:Help|F10:Quit");    
	    i = wselect(mvar, mvar_win_buf);
	    win_set(mvar,i);
	}
	else if(win_flag==TIMER)
	{
	    draw_info_line(1,"F1:Start|F2:Pause|F3:Reset|F4:Run/Stop|F5:Edit|F7:Load|F8:Save|F9:Help|F10:Quit");
	    i = wselect(tim, tim_win_buf);
	    win_set(tim,i);
	}
	else if(win_flag==BLINKER)
	{
	    draw_info_line(1,"F4:Run/Stop|F5:Edit|F7:Load|F8:Save|F9:Help|F10:Quit");
	    i = wselect(blink, blink_win_buf);
	    win_set(blink,i);
	}
	else if(win_flag==5)
	{
	   if(!plc.status%2)//stopped	    
	    {	//wedit
		draw_info_line(1,"F4:Run and execute |F7:Load|F8:Save|F9:Help|F10:Quit");
		return EDIT_MODE;
	    }
	    else
	    {
		draw_info_line(1,"F4:Stop and edit |F7:Load|F8:Save|F9:Help|F10:Quit");
		i = wselect(ld, ld_win_buf);
		win_set(ld,i);
	    }
	}
	c = lastchar();
	Update=TRUE;
	switch(c)
	{
    //arrows change window
	case KEY_RIGHT:
	    if(win_flag<5)
		win_flag++;
	    break;
	case KEY_LEFT:
	    if(win_flag>0)
		win_flag--;
	    break;
	case KEY_TAB://hotkey for edit window
	    if(win_flag<5)
	    {	
		win_sticky = win_flag;
		win_flag=5;
	    }
	    else 
		win_flag = win_sticky;
	    break;

	case KEY_F(1):    //F1 forces 1
	    if(win_flag==DI)
	    {
		plc.maskin[i/8]|=(1<<i%8);
		plc.maskin_N[i/8]&=~(1<<i%8);
	    }
	    else if(win_flag==DQ)
	    {
		plc.maskout[i/8]|=(1<<i%8);
		plc.maskout_N[i/8]&=~(1<<i%8);
	    }    
	    else if(win_flag==COUNTER)
		set(&plc,COUNTER,i);
	    else if(win_flag==TIMER)
		set(&plc,TIMER,i);
	    redraw=TRUE;

	    break;
	case KEY_F(2):    //F2 forces 0
	    if(win_flag==DI)
	    {
		plc.maskin[i/8]&=~(1<<i%8);
		plc.maskin_N[i/8]|=(1<<i%8);
	    }
	    else if(win_flag==DQ)
	    {
		plc.maskout[i/8]&=~(1<<i%8);
		plc.maskout_N[i/8]|=(1<<i%8);
	    }    
	    else if(win_flag==COUNTER)
		reset(&plc,COUNTER,i);
	    else if(win_flag==TIMER)
		reset(&plc,TIMER,i);

	    redraw=TRUE;
	    break;
	case KEY_F(3):    //F3 unforces
	    if(win_flag==DI)
	    {
		plc.maskin[i/8]&=~(1<<i%8);
		plc.maskin_N[i/8]&=~(1<<i%8);
	    }
	    else if(win_flag==DQ)
	    {		
		plc.maskout[i/8]&=~(1<<i%8);
		plc.maskout_N[i/8]&=~(1<<i%8);
	    }
	    else if(win_flag==TIMER)
		down_timer(&plc,i);
	    redraw=TRUE;
	    break;    
	case KEY_F(4):    //F4 runs/stops
	    if(!plc.status%2)//stopped
		plc.status|=TRUE;
	    else --plc.status;//running
	    redraw=TRUE;
	    break;
	case KEY_F(5)://edit
		cur = i;
		ret=EDITPAGE;
	    break;
	case KEY_F(6)://toggle lock
	    if(win_flag==2)
		plc.m[i].RO=(plc.m[i].RO)?0:1;
	    redraw=TRUE;
	    break;
	
        case KEY_F(7):	
	    ret=FILEPAGE;
	    save_flag = 0;
	    break;
	case KEY_F(8):	
	    ret=FILEPAGE;
	    save_flag=TRUE;
	    break;
	    
	case KEY_F(9):	
	    ret=HELPPAGE;
	    break;

        case KEY_F(10):	
	    ret=EXITPAGE;
	    break;
	default:
	    break;
	}
    }
    return ret;
}
int ExitPage()
{
	static int redraw=TRUE;
	int c;
	if (redraw)
	{
		wdraw(conf);
		wshowall(conf,conf_buf);
		redraw=FALSE;
	}
	grgetch();
	c=lastchar();
	if (c=='n' || c=='N') return(0); 
	if (c=='y' || c=='Y') 
	{
		redraw=TRUE;
		win_clear(conf);
		return(IOPAGE);
	}
	return(EXITPAGE);
}
int HelpPage()
{
	int c,i=0;
	static int redraw_help=TRUE;
	if (redraw_help)
	{
		wdraw(help);
		wshowall_c(help,help_buf);
		redraw_help=FALSE;
//	win_set(help,i);
	}
	i = wselect(help, help_buf);
	win_set(help,i);

	c=lastchar();

	if(c==KEY_ESC)
	{
	    redraw_help=TRUE;
	    return(IOPAGE);
	}
	return(HELPPAGE);
}

int EditPage(int i)
{//edit comments, value if memory/timer/blinker, scale if timer/blinker, preset & up/down if timer 
	static int redraw=TRUE;
	int c;
	static int maxrow;
	static char buf[16]="";
	static int row=0;
	static int col=0;
	int x,y,n;

	if(plc.status%2)//if running 
	    --plc.status;//running
	
	if (redraw)
	{//init window
		redraw=FALSE;
		wdraw(edit);
		draw_footer("Esc:Cancel  Enter:Enter");
		wshowall(edit,edit_buf);
		redraw=FALSE;

		switch(win_flag)
		{
		    case 0://inputs
			    maxrow=TRUE;
			    win_puts(edit,1,1,"Comment        :");
			    sprintf(buf,"%s",plc.di[i].nick);
			    win_puts(edit,1,20,buf);
			    break;
		    case 1://outputs
			    maxrow=1;
			    win_puts(edit,1,1,"Comment        :");
			    sprintf(buf,"%s",plc.dq[i].nick);
			    win_puts(edit,1,20,buf);
			    break;
		    case 2://memory
			    maxrow=3;
			    win_puts(edit,1,1,"Comment        :");
			    sprintf(buf,"%s",plc.m[i].nick);
			    win_puts(edit,1,20,buf);
			    win_puts(edit,2,1,"Value          :");
			    sprintf(buf,"%d",plc.m[i].V);
			    win_puts(edit,2,20,buf);
			    win_puts(edit,3,1,"Downcounting     :");
			    sprintf(buf,"%d",plc.m[i].DOWN);
			    win_puts(edit,3,20,buf);
			    break;
		    case 3://timers
			    maxrow=5;
			    win_puts(edit,1,1,"Comment        :");
			    sprintf(buf,"%s",plc.t[i].nick);
			    win_puts(edit,1,20,buf);
			    win_puts(edit,2,1,"Value          :");
			    sprintf(buf,"%d",plc.t[i].V);
			    win_puts(edit,2,20,buf);
			    win_puts(edit,3,1,"Preset         :");
			    sprintf(buf,"%d",plc.t[i].P);
			    win_puts(edit,3,20,buf);
			    win_puts(edit,4,1,"Cycles/count   :");
			    sprintf(buf,"%d",plc.t[i].S);
			    win_puts(edit,4,20,buf);
			    win_puts(edit,5,1,"ON/OFF delay   :");
			    sprintf(buf,"%d",plc.t[i].ONDELAY);
			    win_puts(edit,5,20,buf);
			    break;

		    case 4://blinkers
			    maxrow=2;
		    	    win_puts(edit,1,1,"Comment        :");
			    sprintf(buf,"%s",plc.s[i].nick);
			    win_puts(edit,1,20,buf);
			    win_puts(edit,2,1,"Cycles/count   :");
			    sprintf(buf,"%d",plc.s[i].S);
			    win_puts(edit,2,20,buf);
			    break;
		    case 5:
			    maxrow=2;
			    win_puts(edit,1,1,"Press Enter to switch");
			    win_puts(edit,2,1,"languages (PLC will");
			    win_puts(edit,3,1,"stop and the current");
			    win_puts(edit,4,1,"program will be lost!");
		    default:break;		
		    
		}

	}	
	x=20;
	col=0;
	y=row+1;
	n=10;

	switch(win_flag)
	{//update correct row every time
	    case 0://inputs
		if(row==0)	    
		    sprintf(buf,"%s",plc.di[i].nick);
		break;
	    case 1://outputs
		if(row==0)	    
		    sprintf(buf,"%s",plc.dq[i].nick);
		break;
	    case 2://memory
		switch(row)
		{
		    case 0:
			    sprintf(buf,"%s",plc.m[i].nick);
			    break;
		    case 1:
			    sprintf(buf,"%d",plc.m[i].V);
			    break;
		    case 2:
			    sprintf(buf,"%d",plc.m[i].DOWN);
			    break;
		    default:
			    break;
		}
		break;
	    case 3://timers
		switch(row)
		{
		    case 0:
			    sprintf(buf,"%s",plc.t[i].nick);
			    break;
		    case 1: 
			    sprintf(buf,"%d",plc.t[i].V);
			    break;
		    case 2:
			    sprintf(buf,"%d",plc.t[i].P);
			    break;
		    case 3:
			    sprintf(buf,"%d",plc.t[i].S);
			    break;
		    case 4:
			    sprintf(buf,"%d",plc.t[i].ONDELAY);
			    break;
		    default:

			    break;
		}
		break;
	    case 4://blinkers
		if(row==0)
	    	    sprintf(buf,"%s",plc.s[i].nick);
		if(row==1)
		    sprintf(buf,"%d",plc.s[i].S);
		break;
	    default:break;			    
	}
	win_gets(edit_buf,y,20,16,buf);
	switch(win_flag)
	{//update correct value with input
	    case 0://inputs
		if(row==0)	    
		    sprintf(plc.di[i].nick,"%s",buf);
		break;
	    case 1://outputs
		if(row==0)	    
		    sprintf(plc.dq[i].nick,"%s",buf);
		break;
	    case 2://memory
		switch(row)
		{
		    case 0:
			    sprintf(plc.m[i].nick,"%s",buf);
			    break;
		    case 1:
			    plc.m[i].V=atoi(buf);
			    break;
		    case 2:
			    plc.m[i].DOWN=atoi(buf)?1:0;
		    default:break;
		}
		break;
	    case 3://timers
		switch(row)
		{
		    case 0:
			    sprintf(plc.t[i].nick,"%s",buf);
			    break;
		    case 1: 
			    plc.t[i].V=atoi(buf);
			    break;
		    case 2:
			    plc.t[i].P=atoi(buf);
			    break;
		    case 3:
			    plc.t[i].S=atoi(buf);
			    break;
		    case 4:
			    plc.t[i].ONDELAY=atoi(buf)?1:0;
		    default:break;
		}
		break;
	    case 4://blinkers
		if(row==0)
			    sprintf(plc.s[i].nick,"%s",buf);
		if(row==1)
			    plc.s[i].S=atoi(buf);
		break;
	    
	    default:break;			    
	}
	c=lastchar();
	switch(c)
	{
		case KEY_UP: 	if (row>0) --row;
				break;
		case KEY_DOWN:	if (row<maxrow-1) ++row; 
				break;
		case KEY_LEFT: 	
				if (col>0) --col;
				break;
		case KEY_RIGHT:	
				if (col<16) ++col;
				break;
		case 10:
		case 13:
		case KEY_END:
				redraw=TRUE;
				win_clear(edit);
				plc.status|=1;//RUN mode 
				if(win_flag==5)
				{
				    if(Language==LANG_LD)
					Language=LANG_IL;
				    else Language=LANG_LD;
				    memset(Lines[i],0,MAXSTR);
				    memset(Labels[i],0,MAXSTR);
				    plc.status|=0;//RUN mode 
				}
				return(IOPAGE);

		case KEY_ESC:
				redraw=TRUE;
				win_clear(edit);
				plc.status|=1;//RUN mode 
				return(IOPAGE);
		
	}
	return(EDITPAGE);
}
int save_file(char * path)
{
    FILE * f;
    int i;	
	//open file for writing
		if((f=fopen(path,"w"))==NULL)
		{
		    return ERROR;
		}
		else
		{
		    for(i=0;i<Di*8;i++)
		    {
			if(plc.di[i].nick[0]!=0)
			    fprintf(f,"I\t%d\t%s\t\n", i, plc.di[i].nick);
		    }
		    for(i=0;i<Dq*8;i++)
		    {
			if(plc.dq[i].nick[0]!=0)
			    fprintf(f,"Q\t%d\t%s\t\n", i, plc.dq[i].nick);
		    }
		    		    
		    for(i=0;i<Nm;i++)
		    {
			if(plc.m[i].nick[0]!=0)
			    fprintf(f,"M\t%d\t%s\t\n", i, plc.m[i].nick);
			if(plc.m[i].V>0)
			    fprintf(f,"MEMORY\t%d\t%d\t\n", i, plc.m[i].V);
			if(plc.m[i].DOWN>0)
			    fprintf(f,"COUNT\t%d\tDOWN\t\n", i );
			if(plc.m[i].RO>0)
			    fprintf(f,"COUNTER\t%d\tOFF\t\n", i );
		    }
		    for(i=0;i<Nt;i++)
		    {	
			if(plc.t[i].nick[0]!=0)
			    fprintf(f,"T\t%d\t%s\t\n", i, plc.t[i].nick);
			if(plc.t[i].S>0)
			    fprintf(f,"TIME\t%d\t%d\t\n", i,plc.t[i].S );
			if(plc.t[i].P>0)
			    fprintf(f,"PRESET\t%d\t%d\t\n", i, plc.t[i].P);
			if(plc.t[i].ONDELAY>0)
			    fprintf(f,"DELAY\t%d\tON\t\n", i );
		    }
		    for(i=0;i<Ns;i++)
		    {	
			if(plc.s[i].nick[0]!=0)
			    fprintf(f,"B\t%d\t%s\t\n", i, plc.s[i].nick);
			if(plc.s[i].S>0)
			    fprintf(f,"BLINK\t%d\t%d\t\n", i,plc.s[i].S );
		    }
		    for(i=0;i<256;i++)
		    {	
			if(com_nick[i][0]!=0)
			    fprintf(f,"COM\t%d\t%s\t\n", i, com_nick[i]);
		    }
		    fprintf(f,"\n%s\n","LD");
		    for(i = 0;i<Lineno;i++)
			fprintf(f,"%s\n",Lines[i]);
		    fclose(f);
		}

}
int load_file(char * path,int ini)
{//ini =1 if file is loaded initially, i.e. messages should be printf'd not draw_info_line'd
	FILE * f;
	char * tab;
	char line[MAXSTR], name[128], val[16], msg[64], idx_str[4];
	int idx,r,lineno,i,j,k, found_start=FALSE;
	i = 0;
	//TODO for instruction list:
	if(f = fopen(path, "r"))
	{
		memset(line, 0, MAXSTR);
		memset(name, 0, 128);
		memset(val, 0, 16);
		disable_bus();
		init_emu();

		lineno=0;
		while(fgets(line, 256, f))
		{//read initialization values
			if((line[0]=='L'
				&& line[1]=='D')
				|| (line[0]=='I'
				&& line[1]=='L'))
			{//or 'IL' for IL
				found_start=TRUE;
			}
			else if(!found_start)
			{
				j = 0;
				k = 0;
				memset(name,0,128);
				memset(val,0,16);
				memset(idx_str,0,4);
				//read alpha characters
				while(isspace(line[j]))//ignore blanks
					j++;
				while(isalpha(line[j]))
					name[k++] = line[j++];
				k = 0;
				while(isspace(line[j]))//ignore blanks
					j++;
				while(isdigit(line[j]))
					idx_str[k++] = line[j++];
				while(isspace(line[j]))//ignore blanks
					j++;
				k = 0;
				while(	line[j]
						&&	k<16
						&&	line[j]!=10
						&&	line[j]!=13
						&&	line[j]!=';'
						&&	line[j]!='\t')
					val[k++] = line[j++];
				/*  sscanf(line, "%s\t%s\t%s", name,idx_str, val);*/
				idx = atoi(idx_str);
				lineno++;
				if(idx<0)
				{
					r = ERR_BADINDEX;
					break;
				}    
				else if(!strcmp(name,"I"))
				{
					if(idx>=8*Di)
					{
					r = ERR_BADINDEX;
					break;
					}    
					sprintf(plc.di[idx].nick,"%s",val);
				}
				else if(!strcmp(name,"Q"))
				{
					if(idx>=8*Dq)
					{
					r = ERR_BADINDEX;
					break;
					}    
					sprintf(plc.dq[idx].nick,"%s",val);
				}
				else if(!strcmp(name,"M"))
				{
					if(idx>=Nm)
					{
						r = ERR_BADINDEX;
						break;
					}    
					sprintf(plc.m[idx].nick,"%s",val);
				}
				else if(!strcmp(name,"T"))
				{
					if(idx>=Nt)
					{
						r = ERR_BADINDEX;
						break;
					}    
					sprintf(plc.t[idx].nick,"%s",val);
				}
				else if(!strcmp(name,"B"))
				{
					if(idx>=Ns)
					{
						r = ERR_BADINDEX;
						break;
					}    
					sprintf(plc.s[idx].nick,"%s",val);
				}
				else if(!strcmp(name,"COM"))
				{
					if(idx>=256)
					{
						r = ERR_BADINDEX;
						break;
					}    
					sprintf(com_nick[idx],"%s",val);
				}
				else if(!strcmp(name,"MEMORY"))
				{
					if(idx>=Nm)
					{
						r = ERR_BADINDEX;
						break;
					}    
					plc.m[idx].V=atol(val);
				}
				else if(!strcmp(name,"COUNT"))
				{
					if(idx>=Nm)
					{
						r = ERR_BADINDEX;
						break;
					}    
					if(!strcmp(val,"DOWN"))
					plc.m[idx].DOWN=TRUE;
					else 
					{
					r= ERR_BADOPERATOR;
					break;
					}
				}
				else if(!strcmp(name,"COUNTER"))
				{
					if(idx>=Nm)
					{
						r = ERR_BADINDEX;
						break;
					}    
					if(!strcmp(val,"OFF"))
					plc.m[idx].RO=TRUE;
					else
					{
					r= ERR_BADOPERATOR;
					break;
					}
				}
				else if(!strcmp(name,"TIME"))
				{
					if(idx>=Nt)
					{
						r = ERR_BADINDEX;
						break;
					}    
					plc.t[idx].S=atoi(val);
				}
				else if(!strcmp(name,"PRESET"))
				{
					if(idx>=Nt)
					{
						r = ERR_BADINDEX;
						break;
					}    
					plc.t[idx].P=atoi(val);
				}
				else if(!strcmp(name,"DELAY"))
				{
					if(idx>=Nt)
					{
						r = ERR_BADINDEX;
						break;
					}    
					if(!strcmp(val,"ON"))
					plc.t[idx].ONDELAY=TRUE;
					else 
					{
					r = ERR_BADOPERAND;
					break;
					}
				}
				else if(!strcmp(name,"BLINK"))			
				{
					if(idx>=Ns)
					{
						r = ERR_BADINDEX;
						break;
					}    
					plc.s[idx].S=atoi(val);
				}
				else if(name[0]!=';'
					&& isalnum(name[0])!=0
					&& strcmp(name,"LD"))
				{
					r = ERR_BADOPERAND;
					break;
				}
			}
			else
			{
				while(strchr(line,'\t')!=NULL)//tabs are not supported
				{
					tab = strchr(line,'\t');
					*tab = '.';
				}
				memset(Lines[i],0,MAXSTR);
				memset(Labels[i],0,MAXSTR);
				sprintf(Lines[i++],"%s",line);
				//i++;
			}
			r = OK;
			memset(line, 0, MAXSTR);
			memset(name, 0, 128);
		} 
		fclose(f);
		Lineno=i;
	}
	else r = ERR_BADFILE;
//	printf(msg,"");
	if(r<0)
	{
	    switch(r)
	    {
			case ERR_BADFILE:
				sprintf(msg,"Invalid filename:!");
				break;
			case ERR_BADINDEX:
					sprintf(msg,"%s:%d:Index out of bounds!",path, lineno);	
				break;
			case ERR_BADOPERAND:
					sprintf(msg,"%s:%d:Invalid word %s!",path, lineno,name);	
				break;
			case ERR_BADOPERATOR:
					sprintf(msg,"%s:%d:Invalid word %s!",path, lineno,val);	
				break;

			default:break;
	    }
	    if(ini==1)
			printf("%s\n",msg);
	    else
			draw_info_line(pagelen+1,msg);
	    return ERROR;
	}
	else
	{ 
	    return OK;
	}    
}
int FilePage()
{
	static int redraw=TRUE;
	static char path[256];
	int c;
	static char buf[256]="";
	int i;
	FILE * f;
	if (redraw)
	{
	    if(plc.status%2)//if running 
		--plc.status;//running
	    wdraw(fileselect);
	    draw_footer("Esc:Cancel  Enter:Enter");
	    wshowall(fileselect,file_buf);
	    redraw=FALSE;
	}
	win_gets(file_buf,1,1,256,buf);
	c=lastchar();
	if(c==10||c==13)
	{//enter
	    redraw=TRUE;
	    win_clear(conf);
	    sprintf(path, "%s",buf);
	    if(save_flag)
	    {//save to file
		    if(save_file(path)<0)
		    {
			draw_info_line(pagelen+1,"Invalid filename!");
			return FILEPAGE;
		    }
	    }	
	    else
	    {//init from file
		    if(load_file(path,0)<0)
			return FILEPAGE;
	    }
	    plc.status|=1;//RUN mode 
	    return(IOPAGE);
	}
	if(c==KEY_ESC)
	{
	    redraw=TRUE;
	    win_clear(fileselect);
	    plc.status|=1;//RUN mode 
	    return(IOPAGE);
	}
	return(FILEPAGE);
}

int plc_func(int daemon)
{
    struct timeval tp,tn,dt;
    long timeout;    
    BYTE i_bit,com[2];
    int  q_bit,n,i,j,written,r=OK;

    int i_changed=FALSE;    
    int o_changed=FALSE;    
    int m_changed=FALSE;
    int t_changed=FALSE;
    int s_changed=FALSE;
    char test[16];
    if((plc.status)%2)//run
    {//read inputs		
	for(i = 0; i< Di;i++)
	{	//for each input byte
	    plc.inputs[i]=0;
	    for(j = 0; j < 8;j++)
	    {	//read n bit into in
		n = i*8+j;
		i_bit = 0;
		dio_read(n,&i_bit);
		plc.inputs[i]|= i_bit<<j;
	    }//mask them
	    plc.inputs[i] = (plc.inputs[i]|plc.maskin[i])&~plc.maskin_N[i]; 
	    if(plc.inputs[i]!=old->inputs[i])
		i_changed=TRUE;
	    plc.edgein[i]=(plc.inputs[i])^(old->inputs[i]);
	}  
        //manage timers
	for(i= 0; i < Nt; i++)
	{
	    if(plc.t[i].V < plc.t[i].P
	    && plc.t[i].START)
	    {
		if(plc.t[i].sn<plc.t[i].S)
		    plc.t[i].sn++;
		else
		{
		    t_changed=TRUE;
		    plc.t[i].V++;
		    plc.t[i].sn=0;

		}
		plc.t[i].Q=(plc.t[i].ONDELAY)?0:1;//on delay
	    }
	    else if( plc.t[i].START)
	    {
		plc.t[i].Q=(plc.t[i].ONDELAY)?1:0;//on delay
	    }
	}
	for(i= 0; i < Ns; i++)
	{
	    if(plc.s[i].S>0)
	    {//if set up
		if(plc.s[i].sn > plc.s[i].S)
		{
		    s_changed=TRUE;
		    plc.s[i].Q=(plc.s[i].Q)?0:1;//toggle
		    plc.s[i].sn=0;
		}
		else
	    	    plc.s[i].sn++; 
	    }
	}
	read_mvars(&plc);

//poll on plcpipe for command, for max STEP msecs
	PLC_com[0].fd=open(Pipe,O_NONBLOCK|O_RDONLY);
	PLC_com[0].events=POLLIN|POLLPRI;
	gettimeofday(&tn);//how much time passed for previous cycle?
	timeval_subtract(&dt,&tn,&t);
//	sprintf(test,"Refresh time approx:%d microseconds",dt.tv_usec);
//	draw_info_line(pagelen+ 2,test);
	while(dt.tv_usec>1000*Step)	//if timeout, add another circle;
	    dt.tv_usec-=1000*Step;
	timeout=Step-dt.tv_usec/1000;
	written=poll(PLC_com,1,timeout);
	gettimeofday(&tp);//how much time did poll wait?
	timeval_subtract(&dt,&tp,&tn);
	if(written)
	{
	    if(read(PLC_com[0].fd,com,2))
	    {
		if(com[0]==0)
		    com[0]=0;//NOP
		else plc.command=com[0]-48;
		sprintf(test,"LAST command:%d, %s",plc.command,com_nick[com[0]-48]);
		if(daemon==1)
		{
		    printf("%s\n",test);
		    sprintf(test,"");
		}
		else
		    draw_info_line(pagelen+1,test);
	    }
	}
	else if (written==0)
	    plc.command=0;
	else
	{
	    if(daemon==1)
		printf("PIPE ERROR\n");
	    else
		draw_info_line(pagelen+1,"PIPE ERROR");
	    plc.command=0;
	}

//	sprintf(test,"Poll time approx:%d milliseconds",dt.tv_usec/1000);
	close(PLC_com[0]);
        gettimeofday(&t);//start timing next cycle
	Command = plc.command;
	dec_inp(&plc); //read inputs	
	PLC_task(&plc);
	
	if(Language==LANG_LD)
	    r = LD_task(&plc);
	if(Language==LANG_IL)
	    r = IL_task(&plc);
	
	enc_out(&plc);    
	plc.response=Response;

    	plc.command=0;
	for(i = 0; i< Dq;i++)
	{	//write masked outputs
	    plc.outputs[i] = (plc.outputs[i]|plc.maskout[i])&~plc.maskout_N[i]; 
	    for(j = 0; j < 8;j++)
	    {//write n bit out
		n = 8*i+j;
		q_bit = (plc.outputs[i]>>j)%2;
    		dio_write(plc.outputs,n,q_bit);
    	    }
	    if(plc.outputs[i]!=old->outputs[i])
		o_changed=TRUE;
	}	
	for(i = 0;i< Nm; i++)//check counter pulses
	{
	    if(plc.m[i].PULSE!=old->m[i].PULSE)
	    {
		plc.m[i].EDGE=TRUE;
		m_changed=TRUE;
	    }
	}
	
	write_mvars(&plc);

	if(i_changed)
	{
//draw_info_line(4+Pagelen/2," Input changed!"); 	
	    memcpy(old->inputs,plc.inputs,Di);
	    Update=TRUE;
	    i_changed=FALSE;
	}
	if(o_changed)
	{
//draw_info_line(4+Pagelen/2," Output changed!"); 	
	    memcpy(old->outputs,plc.outputs,Dq);
	    Update=TRUE;
	    o_changed=FALSE;
	}	
	if(m_changed)
	{
	    memcpy(old->m,plc.m,Nm*sizeof(struct mvar));
	    Update=TRUE;
	    m_changed=FALSE;
	}
	if(t_changed)
	{
	    memcpy(old->t,plc.t,Nt*sizeof(struct timer));
	    Update=TRUE;
	    t_changed=FALSE;
	}
	if(s_changed)
	{
	    memcpy(old->s,plc.s,Ns*sizeof(struct blink));
	    Update=TRUE;
	    s_changed=FALSE;
	}
	//write out response
	if(plc.response)
	{
	    rfd=open(Responsefile,O_NONBLOCK|O_WRONLY);
	    write(rfd,plc.response,1);
	    close(rfd);
	    plc.response = 0;
	}
    }
    else 
	usleep(Step*1000);
    return r;
}
int EditMode()
{
	char line[MAXSTR];
	int i = 0;
	wedit(ld_win_buf,ld,NULL);//del_buf);
	win_flag = 0;
	memset(line,0,MAXSTR);
	for(i = 0;i<=Lineno;i++)
	{
	    memset(Lines[i],0,MAXSTR);
	    memset(Labels[i],0,MAXSTR);
	}
	i = 0;
	while (buf_cpline(ld_win_buf,i,line)>=0)
	{
	    line[0]='-';
	    sprintf(Lines[i],"%s",line);
	    i++;
	    memset(line,0,MAXSTR);
	}
	Lineno=i;
	return IOPAGE;
}
int main(int argc, char **argv)
{
    int i,p,j,n,errcode,daemon_flag=FALSE;
    int changed=FALSE;
    BYTE * in_p, * out_p;
    char str[128], confstr[128], inistr[128];

    strcpy(confstr,"plc.config");
    sprintf(inistr,"");
    for(i = 1; i<argc;i++)
    {
	if(!strcmp(argv[i],"-d"))
	    daemon_flag=TRUE;
	else
	{//check if previous arg was -i or -c
		if(!strcmp(argv[i-1],"-i"))
		    strcpy(inistr,argv[i]);
		else if(!strcmp(argv[i-1],"-c"))
		    strcpy(confstr,argv[i]);
		else
		{
		    if((strcmp(argv[i],"-i")
		    && strcmp(argv[i],"-c"))
		    || argc==i+1)
		    {
printf("Usage: plcemu [-i program file] [-c config file] [-d] \n\
Options:\n\
-i loads initially a text file with initialization values and LD/IL program, \n\
-c uses a configuration file other than plc.config \n\
-d runs PLC-EMU as daemon \n");
			return ERR;
		    }
		}	    
	}
    }
    if(init_config(confstr)<0)
    {
	printf ("Invalid configuration file\n");
	return ERR;
    }
    
    init_emu();
    
    if(inistr[0]
    && load_file(inistr,1)<0)
	printf("Invalid program file\n");

    if(daemon_flag==FALSE)
    {
	win_start();
	pagelen=Pagelen-3;	
	in=win_open(3,1,Pagelen/2,Pagewidth/4-1,
	"DIGITAL INPUTS");
	out=win_open(3,1+Pagewidth/4,Pagelen/2,Pagewidth/4-1,
	"DIGITAL OUTPUTS");
	mvar= win_open(3,1+Pagewidth/2,Pagelen/2,Pagewidth/4-1,
	"MEMORY COUNTERS");
	tim=win_open(3,1+3*Pagewidth/4,Pagelen/4-1,Pagewidth/4-2,
	"TIMERS");
	blink=win_open(4+Pagelen/4,1+3*Pagewidth/4,Pagelen/4-1,Pagewidth/4-2,
	"BLINKERS");
	ld = win_open(6+Pagelen/2,1,Pagelen/2-9,Pagewidth-2,
	"PLC TASK");
	edit=win_open(	10,15,8,40,
			" Configuration "
	    		);
	conf=win_open(	10,15,4,50,
			" Exit PLC-emu ? "
	    		);
	fileselect=win_open(	10,15,8,40,
			" Write a UNIX path"
	    		);
	help = win_open(6,1,Pagelen-10,80,"HELP");

	in_win_buf = buf_open();
	out_win_buf = buf_open();
	mvar_win_buf = buf_open();
	tim_win_buf = buf_open();
	blink_win_buf = buf_open();
	ld_win_buf = buf_open();
	edit_buf = buf_open();
	conf_buf = buf_open();
	strcpy(str," Quit?  wh(Y)? Why (N)ot? ");
	app_line(conf_buf,str);
    
	file_buf = buf_open();
//    del_buf = buf_open();
	help_buf = buf_open();
	page = 1;
        gettimeofday(&t);
	init_help();
	while(page)
	{
    	    TimeHeader(page);	    
    	    if (page==IOPAGE)
		page=IO_Page();
	    else if (page==EXITPAGE)
		page=ExitPage();
	    else if (page==EDITPAGE)
		page=EditPage(cur);
	    else if (page==FILEPAGE)
		page=FilePage();
	    else if(page==EDIT_MODE)
		page=EditMode();
	    else if(page==HELPPAGE)
		page=HelpPage();

	    errcode = plc_func(FALSE);   
	    if(errcode < 0)
	    {
		sprintf(str,"error code %d",-errcode);
    	    draw_info_line(4+Pagelen/2,errmsg[-1-errcode]); 
//		draw_info_line(4+Pagelen/2,str);
//	    plc.status=0;
	    }
//	else draw_info_line(4+Pagelen/2," "); 
	}
    }else
    {//daemon mode
	for(;;)
	{
	    if(plc.status>0)
	    {	
		errcode = plc_func(TRUE);
		if(errcode <0)
		{
		    printf("%s\n", errmsg[1-errcode]);
		    plc.status=0;
		}
	    }
	}
    }
    disable_bus();
    win_end();
}
