#include "plclib.h"
#include "parser.h"
#include <ctype.h>
const char errmsg[8][256]= 
{  
    "Something went terribly wrong!",
    "Invalid Symbol!",
    "Wrong File!",
    "Invalid Operand!",
    "Invalid Numeric!",
    "Invalid Output!",
    "Invalid Command!",
    ""    
};
const char il_commands[N_IL_INSN][4]={"",")","RET","JMP","CAL","S","R","AND","OR","XOR","LD","ST","ADD","SUB","MUL","DIV","GT","GE","EQ","NE","LT","LE"};
//TODO: IL byte operations
//TODO: 
//{ operator should toggle state of coil
//) negative contact
//[ set
//] reset

/******************parse ladder files!**********************/
/*
1.read a text file and store it in array Lines.
2.parse each unresolved line i in Lines[i] up to '+','(',0, blank, '|'.
    a. parse grammar.
	i.read next character
	ii. increase line position counter from Pos[i]
	iii.if unaccepted character, return error. else return uccepted character number.
	
    b. if blank or 0,'|', empty value for the line. if 0 set it resolved.
    c. if '(', 
	i. see if it is a coil. (expect Q,S,R,T,D,M,W followed by number)
	ii. resolve coils value from the correspondant value 
	iii. mark line as resolved by setting Pos[i] = -1.
    d. if '-' do nothing and go to next character
    e. if '+' stop and continue with next line.
    f. if ! negate next operand by setting normally closed mode
    g. otherwise operand is expected(i,q,f,r,m,t,c,b)
	if operand
	    read number starting at Pos[i]+1
	    if there is a number
		get the number
		get its number of digits
		compare number with max no for operand
		get value from operand[number]
		if in normally closed mode negate value
		AND it with current value	
3.reset normally closed mode
(At this point, all lines have been resolved,or paused at a '+', with one value in their stack.
Pos[i] is holding the position of each lines cursor.)
	
4.  parse vertically
    cursor=0
    while there are unresolved lines(minmin returns non negative)
	resolve lines
	j = get min i for min Pos[i] , Pos[i] > cursor
	cursor = Pos[j]
	curline = j
	start at line j
	
	parse downwards:
	get character at Lines[++j][cursor]
	    a. if '|' do nothing
	    b. if '+'
		if new_node 
		    curline=j;
		    new_node=false 
		else store Val[i], Val[j] = Val[i] OR Val[j]
	    c. otherwise do nothing and set new_node    	
*/
extern Di, Dq, Nm, Nt, Ns, Pagelen;
int extract_number(char * line, int start)
{//read characters from string line 
    int i,n;
	//starting @ position start	
    if(start >= strlen(line))
		return ERROR;
    n=0;
    for(i=start;isdigit(line[i]);i++)
		n=10*n+(line[i] - '0');
    if(i==start)
		//no digits read
		return ERROR;	
    return n;
	//return number read or error 
}
int read_char(char * line, int idx)
{//read ONE character from line[idx]
//parse grammatically:
    int r=0;
	// printf("idx->%d, line[idx]->%c\n",idx,line[idx]);
    if(idx > strlen(line))
		return ERROR;
	if(line[idx]==0
		|| line[idx]==';'
		|| line[idx]=='\n'
		|| line[idx]=='\r')
		return END;
    if(line[idx]==' '
		|| line[idx]=='.'
		|| line[idx]=='#'
		|| line[idx]=='\t' )
		return BLANK;
    if(isdigit(line[idx]))
		return line[idx] - '0';
    switch(line[idx])
    {
		case '('://Coil
			r=COIL;
			break;
		case '-'://horizontal line
			r=AND;
			break;
		case '|'://vertical line
			r=OR;
			break;
		case '!'://normally clozed
			r=NOT;
			break;
		case '+'://
			r=NODE;
			break;
		case 'Q'://dry contact output
			r=CONTACT;
			break;
	//	case 'S'://set output
		case '[': //NU!
			r=SET;
			break;
	//	case 'R'://reset output
		case ']'://NU!
			r=RESET;
			break;
		case 'T'://start timer
			r=START;
			break;
	//	case 'D'://down timer DEPRECATED. USED FOR COIL NEGATION
		case ')':
			r=DOWN;
			break;
		case 'M'://pulse to counter
			r=PULSEIN;
			break;
		case 'W'://write response
			r=WRITE;
			break;
		case 'i'://input
			r=INPUT;
			break;
		case 'q'://output value
			r=OUTPUT;
			break;
		case 'f'://falling edge
			r=FALLING;
			break;
		case 'r'://rising Edge
			r=RISING;
			break;
		case 'm'://pulse of counter
			r=MEMORY;
			break;
		case 't'://timer.q
			r=TIMEOUT;
			break;
		case 'c'://read command
			r=COMMAND;
			break;
		case 'b'://blinker
			r=BLINKOUT;
			break;
		default:
			r=ERR_BADCHAR;//error
	}
	//return value or error	
	return r;
}

int resolve_operand(struct PLC_regs * p,int type, int idx)
{
    int r=ERR_BADINDEX;	
    switch(type)
    {
		case INPUT://input
			if(idx>=0
				&& idx<Di*8)
				r = resolve(p,DI,idx);
			break;
		case OUTPUT://output value
			if(idx>=0
				&& idx<Dq*8)
				r = resolve(p,DQ,idx);
			break;
		case FALLING://falling edge
			if(idx>=0
				&& idx<Di*8)
				r = fe(p,DI,idx);
			break;
		case RISING://rising Edge
			if(idx>=0
				&& idx<Di*8)
				r = re(p,DI,idx);			
			break;
		case MEMORY://memory variable
			if(idx>=0
				&& idx<Nm)
				r = resolve(p,COUNTER,idx);
			break;
		case TIMEOUT://timer.q
			if(idx>=0
				&& idx<Nt)
				r = resolve(p,TIMER,idx);
			break;
		case COMMAND://read command serially: maximum 1 instance of this can be true
			r=(Command==idx)?1:0;
			break;
		case BLINKOUT://blinker
			if(idx>=0
				&& idx<Ns)
				r = resolve(p,BLINKER,idx);
			break;
		default: r= ERROR;//this should never happen
    }
    return r;
}
int resolve_coil(struct PLC_regs * p, int type,int idx,int val)
{
    int r=OK;
    switch(type)
    {
	case CONTACT:
	    if(idx>=0
			&& idx<Dq*8)
			r = contact(p,DQ,idx,val);
	    else r=ERR_BADINDEX;	    
	    break;
/*	case SET:
	    if(idx>=0
	    && idx<Dq*8)
	    {
		if(val)
		    r = set(p,DQ,idx);
	    }
	    else r=ERR_BADINDEX;	    
	    break;
	case RESET:
	    if(idx>=0
	    && idx<Dq*8)
	    {	
		if(val)
		    r = reset(p,DQ,idx);
	    }
	    else r=ERR_BADINDEX;	    
	    break;      DEPRECATED */
	case START:
	    if(idx>=0
	    && idx<Nt)
	    {
		if(val)
		    r = set(p,TIMER,idx);
	    }
	    else r=ERR_BADINDEX;	    
	    break;
	case DOWN:
	    if(idx>=0
	    && idx<Nt)
	    {
		if(val)
		    r = down_timer(p,idx);
	    }
	    else r=ERR_BADINDEX;	    
	    break;
	case PULSEIN:
	    if(idx>=0
	    && idx<Nm)
		r = contact(p,COUNTER,idx,val);
	    else r=ERR_BADINDEX;	    
	    break;
	case WRITE:
	    Response=idx;//unimplemented
	    break;
	default: r= ERR_BADCOIL;//this should never happen
    }
    return r;
}
int resolve_set(struct PLC_regs * p, int type,int idx, int val)
{
    int r=OK;
    switch(type)
    {
	case CONTACT:
	    if(idx>=0
	    && idx<Dq*8)
	    {	
		if(val)
		    r = set(p,DQ,idx);
	    }
	    else r=ERR_BADINDEX;	    
	    break;
	case START:
	    if(idx>=0
	    && idx<Nt)
	    {	
		if(val)
		    r = set(p,TIMER,idx);
	    }
	    else r=ERR_BADINDEX;	    
	    break;
	case PULSEIN:
	    if(idx>=0
	    && idx<Nm)
	    {
		if(val)
		    r = set(p,COUNTER,idx);
	    }
	    else r=ERR_BADINDEX;	    
	    break;
/*	case WRITE:
	    Response=idx;//unimplemented
	    break;*/
	default: r= ERR_BADCOIL;//this should never happen
    }
    return r;
}
int resolve_reset(struct PLC_regs * p, int type,int idx, int val)
{
    int r=OK;
    switch(type)
    {
	case CONTACT:
	    if(idx>=0
	    && idx<Dq*8)
	    {
		if(val)
		    r = reset(p,DQ,idx);
	    }
	    else r=ERR_BADINDEX;	    
	    break;
	case START:
	    if(idx>=0
	    && idx<Nt)
	    {
	    	if(val)
		    r = down_timer(p,idx);
	    }
	    else r=ERR_BADINDEX;	    
	    break;
	case PULSEIN:
	    if(idx>=0
	    && idx<Nm)
	    {
	    	if(val)
		    r = reset(p,COUNTER,idx);
	    }
	    else r=ERR_BADINDEX;	    
	    break;
/*	case WRITE:
	    Response=idx;//unimplemented
	    break;*/
	default: r= ERR_BADCOIL;//this should never happen
    }
    return r;
}

int resolve_lines(struct PLC_regs * p)
{
    int i,j,c,type,r,n_mode,idx,digits,operand;
    char * l;
    char tst[2560], tst2[80];
/*
    parse horizontally each unresolved line i in Lines[i] up to '+','(',0.
*/
    tst[0] = 0;
    
    r = 0;
    for(i=0;i<Lineno;i++)
    {
		n_mode=0;//normally open mode
		l = Lines[i];
		printf("l=%s\n",l);
		c = AND;//default character = '-'
		// sprintf(tst,"LINE %d of %d:",i,Lineno);
		while(Pos[i]>RESOLVED
			&& c!=NODE)
		{//loop
			c = read_char(l,Pos[i]);
			printf("C=  %c\n",l[Pos[i]]);
			switch(c)
			{
				case ERR_BADCHAR:
					r = c;
					Pos[i]=RESOLVED;
					break;
				case END://if 0 , this sould be a coil
					Pos[i]=RESOLVED;
					break;
				case BLANK:case OR://if blank or '|', empty value for the line.
					Val[i]=FALSE|FINAL;
					Pos[i]++;
					// printf("O:Val[%d]=%d,Pos[%d]=%d\n",i,Val[i],i,Pos[i]);
					break;
				case NOT:
					n_mode=1;//normally closed mode
				case AND:
					Pos[i]++;
					break;
				case NODE:
					//  Pos[i]++;//pause until resolved
					if(Val[i]>=FINAL)
						Val[i]-=FINAL;
					printf("N:Val[%d]=%d,Pos[%d]=%d\n",i,Val[i],i,Pos[i]);
					break;
				case COIL://see if it is a coil.(NEW: ()[] (expect Q,T,M,W followed by number)
				case SET:
				case RESET:
				case DOWN:
					Pos[i]++;
					type = c;
					c = read_char(l,Pos[i]);
					if(c>=CONTACT
						&& c< END)
					{
						operand = c;
						c = read_char(l,Pos[i]);	
						Pos[i]++;
						idx = extract_number(l,Pos[i]);
						if(idx>=0)
						{
							// sprintf(tst2,"LINE %d VALUE %d",i,Val[i]);
							// strcat(tst,tst2);
							printf(tst);
							//	draw_info_line(4+Pagelen/2,tst);
							if(Val[i]>=FINAL)
							{
								switch(type)
								{
									case COIL:
										r = resolve_coil(p,operand,idx,Val[i]&TRUE);
										break;
									case DOWN://NEW: negative logic output
										r = resolve_coil(p,operand,idx,Val[i]&TRUE?FALSE:TRUE);
										break;
									case SET:
										r = resolve_set(p,operand,idx,Val[i]&TRUE);
										break;
									case RESET:
										r = resolve_reset(p,operand,idx, Val[i]&TRUE);
										break;
								
								}
								Pos[i] = -FINAL;
							}
							Pos[i]=RESOLVED;
						}
						else 
							r = ERR_BADINDEX;
					}
					else
					r =  ERR_BADCOIL;
					break;
				default://otherwise operand is expected(i,q,f,r,m,t,c,b)
					if(	c>=INPUT
						&& c< CONTACT)
					{//valid input symbol
						operand = c;
						c = read_char(l,Pos[i]);						
						Pos[i]++;						
						idx = extract_number(l,Pos[i]);
						if(idx>=0)
						{							
							if(idx>100)
								digits=2;
							else if(idx>10)
								digits=1;
							else digits = 0;
							Pos[i]+=digits;							
							r = resolve_operand(p,operand,idx);
							printf("resolve=%d\n",r);
							// if(r)
							// {
							if(n_mode)
							{
								r = r?FALSE:TRUE;
								n_mode = FALSE;
							}
							Val[i] &= FINAL|r;
							printf("C:Val[%d]=%d\n",i,Val[i]);
							// }
						}	
						else 
							r = ERR_BADINDEX;	
					}
					else
					{
						r =  ERR_BADOPERAND;
					}
					Pos[i]++;
			}//end switch
			if(r<0)
			{//error
				Pos[i]=RESOLVED;
				Lines[i][0]='.';
				Val[i] = FALSE;
				return r;
			}
		
		}//end while
		if(Val[i]&=TRUE)
			Lines[i][0]='#';
		else 
			Lines[i][0]=' ';

    }//end for
/*(At this point, all lines have been resolved,or paused at a '+'*/
    return OK;
}

int minmin(int * arr, int min, int max)
{//for an array arr of integers ,return the smallest of indices i so that arr[i] =  min(arr) >= min 
    int i;
    int v = MAXSTR;//cant be more than length  of line
    int r = RESOLVED;
    for(i =max-1; i >=0;i--)
    {
	if(arr[i]<=v
	&& arr[i]>=min)
	{
	    v=arr[i];
	    r = i;
	}
    }
    return r;
}
//
int LD_task(struct PLC_regs * p)
{//main loop
    int r,i,j,k,tempval,new_node = 0;
    int cursor=0;//care only for unresolved lines
    int startline,curline = 0;
    char tst[2560];
    char tst2[64];
    //init Val,Pos
    int tester = 0;
    for(j = 0;j<Lineno;j++)
    {
	    Pos[j]=1;
	    Val[j]=TRUE|FINAL;
    }

//    while(curline>=0)
//    {//    while there are unresolved lines(minmin returns non negative)	
	r = resolve_lines(p);
	if(r<0)
	    return r;//error	    

	curline = minmin(Pos,cursor,Lineno);//find first unrfinalized line that has stopped at smallest cursor larger than current cursor
	if(curline < 0 )
	    return OK;//finished
	cursor = Pos[curline];
	/* parse vertically
		start at current line
	*/
	startline = curline;
	new_node = TRUE;
	for(j = 0;j<Lineno;j++)
	{
	    if(Pos[j]>RESOLVED)
	    {
			//	if(Val[j]<FINAL)
			sprintf(tst,"Checking line %d of %d (%d)@%d",j,Lineno,Val[j],Pos[j]);
			printf(tst);
			//	draw_info_line(4+Pagelen/2,tst);
			//	sleep(1);
			if(Val[j]<FINAL)
			{
				curline=j;
				cursor = Pos[j];
				// new_node = FALSE;
				sprintf(tst,"-->new node %d of %d (%s)@%d \n", curline,Lineno,Val[j]&TRUE?"TRUE":"FALSE",cursor);				
				printf(tst);
				// draw_info_line(4+Pagelen/2,tst);
				//sleep(1);
				for(i = curline;i<Lineno;i++)
				{
					if((Lines[i][cursor]!='|'
						&& Lines[i][cursor]!='+')
						|| i>=Lineno-1)//vertical line interrupted
					{	
						// new_node = TRUE;
						tempval = Val[curline]&TRUE;
						while(Stack!=NULL)
						{
							sprintf(tst,"OR (%s)  \n",Stack->operation?"TRUE":"FALSE");
							// strcat(tst,tst2);
							printf(tst);
							tempval=pop(tempval);
						}
						for(k = curline;(k<=i)&&(Lines[k][cursor]=='|'|| Lines[k][cursor]=='+') ;k++)
						{
							if( Lines[k][cursor]=='+')
							{								
								Val[k] = tempval|FINAL;
								Pos[k]++;
							}	
						}
						r = resolve_lines(p);
						if(r<0)
							return r;//error	    
							break;
					}
					else 
					if (Lines[i][cursor]=='+')
						{
							printf("Value Pushed\n");
							push(BOOL+IL_OR,Val[i]&TRUE);
						}
				}
				//	sleep(1);
				//Val[j]|=FINAL;
			}
	    }
	}//end for
//	tester++;
//    }//end while
    return OK;
}

int IL_task(struct PLC_regs * p)
{
    struct instruction op;
    int l,j,r = OK;
    char tst[Pagewidth];
    for(j = 0;j<Lineno;j++)
    {
	r = parse_il_line(Lines[j], &op, j);
	if(r<0)
	{
	    Lines[j][0] = '.';
	    break;
	}
	l = j;
	r = instruct(p, &op, &l);
/*	if(l!=j)
	{
	    sprintf(tst, "go from %d to %d",j,l);
	    draw_info_line(4+Pagelen/2,tst);
	}*/
	j = l;
	if(r<0)
	{
	    Lines[j][0] = '.';
	    break;
	}
    }
    return r;
}

/*********************************STACK****************************************/

void push(BYTE op, BYTE val)
{//push an opcode and a value into stack.
    struct opcode * p;
    //malloc a struct opcode pointer
    p = malloc(sizeof(struct opcode));
    //initialize
    p->operation = op;
    p->value = val;
    p->next = Stack;	
    //set stack head pointer to point at it
    Stack = p;
}

BYTE pop(BYTE val)
{//retrieve stack heads operation and operand, apply it to val and return result
    BYTE r = val; //return value
    struct opcode *p;
    if(Stack != NULL)//safety
    {
		r = operate(Stack->operation, Stack->value, val);//execute instruction
		p = Stack;
		Stack = Stack->next;//set stack head to point to next opcode in stack
		free(p);//free previous head.
    }
    return r;
}
/***************************INSTRUCTION LIST***********************************/
int parse_il_line(char * line, struct instruction * op, int pc)
{//    line format:[label:]<operator>[<modifier>[%<operand><byte>[/<bit>]]|<label>][;comment]
    char buf[MAXSTR];
    char tst[Pagewidth];
    int i, byte, bit;
    char * str;
    char * cursor;
    char op_buf[4];
    char label_buf[MAXSTR];
    unsigned int idx = 0;    
    BYTE modifier, operand,operator; 
    
    memset(label_buf,0, MAXSTR);
    memset(buf,0, MAXSTR);
    //1. read up to ';' or /n
    while(line[idx]!=0
    && line[idx]!='\n'
    && line[idx]!=';')
    {
//	buf[idx]=line[idx];
	idx++;
    }
    for(i = idx; i< MAXSTR;i++)
	line[i]=0;//trunc comments
    idx = 0;    	
    //2. find last ':', trunc up to there, store label.
    str = strrchr(line, ':');
    cursor = line;
    i = 0;
    if(str)
    {
	while(line+i+1 != str)
	    label_buf[i++] = line[i+1];    
	strcpy(Labels[pc],label_buf);
	
	strcpy(buf, str+1);
//printf("label:%s\n",label_buf);
    }
    else strcpy(buf, line+1);
    //3. find first ' ','!','('. store modifier (0 if not found)
    str = strchr(buf, '(');
    if(str)//push stack
	modifier = IL_PUSH;
    else
    {//negate
	str = strchr(buf, '!');	
	if(str)
	    modifier = IL_NEG;
	else
	{//normal
	    str = strchr(buf, ' ');
	    if(str)
		modifier = IL_NORM;
	    else
	    {//conditional branch
		str = strchr(buf, '?');		
		if(str)
		    modifier = IL_COND;
		else modifier = NOP;
	    }
	}
    }   
//printf("modifier:%d\n",modifier);
    //4. read operator from beginning to modifier. check if invalid, (return error)
    if(str)
    {
	cursor = buf;
	i = 0;
	memset(op_buf, 0, 4);
	while(cursor+i != str && i<4)
	    op_buf[i++] = cursor[i];
    }
    else 
    {
	if(strlen(buf)<4)
	    strcpy(op_buf, buf);
	else return ERR_BADOPERATOR;
    }
//printf("operator:%s\n",op_buf);        	
    operator = N_IL_INSN;
    for(i = 0;i<N_IL_INSN;++i)
    {
	if(!strcmp(op_buf,il_commands[i]))
	    operator = i;
    }    
//printf("(%d)\n",operator);
    if (operator==N_IL_INSN)
	return ERR_BADOPERATOR;

    if(operator > IL_CAL)
    {//5. if operator supports arguments, find first '%'. (special case: JMP (string). if not found return error

	str = strchr(buf, '%');
	if(!str)
	    return ERR_BADCHAR;
	operand = 0;	
	//6.  read first non-numeric char after '%'. if not found return error. store operand. chack if invalid (return error).
	if(isalpha(str[1]))
	    operand = read_char(str,1);
	else
	    return ERR_BADOPERAND;
//printf("operand:%d\n",operand);
    //7.  read first numeric chars after operand. store byte. if not found or out of range, return error.
	byte = extract_number(str,2);
	if(byte<0)
	    return ERR_BADINDEX;    
//printf("byte:%d\n",byte);	
    //8.  find '/'. if not found truncate, return.
	cursor = strchr(str,'/');
	bit = BOOL;
	if(cursor)
	{
	    if(!isdigit(cursor[1])
	    || cursor[1]>'7')
		return ERR_BADINDEX;
	    else bit = cursor[1]-'0';
//printf("bit:%d\n",bit);
	}
    //9.  if found, read next char. if not found, or not numeric, or >7, return error. store bit.
    }// otherwise truncate and return
    else if(operator==IL_JMP)
    {	
	str = strchr(buf, ' ');
	if(!str)
	    return ERR_BADOPERAND;
	strcpy(op->label,str+1);
    }
    op->operation = operator;
    op->modifier = modifier;
    op->operand = operand;
    op->byte = byte;
    op->bit = bit;
    return pc++;
}

/*TODO: IL_task
parsing:
2. switch operator:
    valid ones:
	)	pop
	    only boolean:(bitwise if operand is byte)

	S 	set
	R 	reset
	    
	AND 
	OR
	XOR
	    any:
	LD 	load
	ST 	store
	ADD	
	SUB	
	MUL	
	DIV	
	GT	>
	GE	>=
	NE	<>
	EQ	==
	LE	<=
	LT	<
	JMP
	    unimplemented:
	CAL
	RET

3. switch modifier
    ) cannot have one
    S, R can only have ' '
    LD, ST cannot have (
    AND, OR, XOR can have all 3
    the rest can only have '('    
	if '!' set negate
	if '(' push stack
4. switch operand
    valid ones:
	DI	digital input
	R	rising edge
	F	falling edge
	DQ	digital output
	MH	memory high byte
	ML	memory low byte
	MP	pulse byte: 0000-SET-RESET-EDGE-VALUE
	B	blinker
	TQ	timer output
	TI	timer start
	W	serial output
	C	serial input
5. resolve operand byte: 0 to MAX
6. resolve operand bit: 0 to 8
7. execute command if no errors
*/
int instruct(struct PLC_regs *p, struct instruction *op, int *pc)
{
    static BYTE acc;//accumulator
    BYTE operation, boolflag = 0;
    char tst[Pagewidth];
    int i;
//modifier check
    if(op->operation>IL_XOR
    && op->operation<IL_ADD
    && op->modifier!=IL_NEG
    && op->modifier!=IL_NORM)//only negation
	return ERR_BADOPERATOR;

    if(op->operation>IL_ST// only push
    && op->modifier!=IL_PUSH
    && op->modifier!=IL_NORM)
	return ERR_BADOPERATOR;

    if(op->operation>IL_CAL
    && op->operation<IL_AND
    && op->modifier!=IL_NORM)//no modifier
	return ERR_BADOPERATOR;

    if(op->bit < 8)
	boolflag = 1;
    
//operand check

    if(op->operation==IL_SET || op->operation==IL_RESET || op->operation==IL_ST)
    {	
	if(op->operand < CONTACT)
	{
	    if(op->operand == OUTPUT) 
		op->operand =CONTACT;
	    else if(op->operand == MEMORY) 
		op->operand =PULSEIN;
	    else if(op->operand == TIMEOUT) 
		op->operand =START;
	    else return ERR_BADOPERAND;//outputs
	}
    }
    else
	if(op->operation > IL_CAL	
	&& (op->operand < INPUT || op->operand > CONTACT))
	    return ERR_BADOPERAND;//inputs
	    
    switch(op->operation)
    {
//IL OPCODES: no operand
	case  IL_POP: //POP		
	    acc = pop(acc);
	break;
	case  IL_RET:		//RET
//unimplemented yet: retrieve  previeous program , counter, set pc
	break;
//arithmetic LABEL
	case  IL_JMP:			//JMP
	for(i = 0;i<Lineno;i++)
	{
	    if(isalnum(Labels[i][0]))
	    {
		if(!strcmp(op->label,Labels[i]))
		{
//		    sprintf(tst, "Line: %d, Label: %s go from %d to %d",i,Labels[i],*pc, i);
//	    	    draw_info_line(4+Pagelen/2,tst);
		    *pc = i;
		}
	    }
	}    
//retrieve line number from label, set pc
	break;
//subroutine call (unimplemented)
	case  IL_CAL://retrieve subroutine line, set pc
	
	break;
//boolflagean, no modifier, outputs.
	case  IL_SET:	//S
	    switch(op->operand)
	    {
		case CONTACT://set output %QX.Y
		    if(!boolflag)//only gets called when bit is defined
			return ERR_BADOPERAND;
		    set(p,DQ,(op->byte)/8+op->bit);
		break;
		case START://bits are irrelevant
		    set(p,TIMER,op->byte);
		break;
		case PULSEIN://same here
		    set(p,COUNTER,op->byte);
		break;
		default: return ERR_BADOPERAND;
	    }
	break;
	case  IL_RESET :	//R
	    switch(op->operand)
	    {
		case CONTACT://set output %QX.Y
		    if(!boolflag)//only gets called when bit is defined
			return ERR_BADOPERAND;
		    reset(p,DQ,(op->byte)/8+op->bit);
		break;
		case START://bits are irrelevant
		    reset(p,TIMER,op->byte);
		break;
		case PULSEIN://same here
		    reset(p,COUNTER,op->byte);
		break;
		default: return ERR_BADOPERAND;
	    }
	break;
	case  IL_LD:	//LD
	    switch(op->operand)
	    {
		case OUTPUT://set output %QX.Y
		    if(!boolflag)//word
			acc = p->outputs[op->byte];
		    else acc = resolve(p,DQ,(op->byte)/8+op->bit);
		break;
		case INPUT://load input %IX.Y
		    if(!boolflag)//word
			acc = p->inputs[op->byte];
		    else acc = resolve(p,DI,(op->byte)/8+op->bit);
		break;
		case MEMORY:
		    if(!boolflag)//word
			acc = p->m[op->byte].V;
		    else acc = resolve(p,COUNTER,op->byte);
		break;
		case TIMEOUT:
		    if(!boolflag)//a convention: bit is irrelevant, but defining it means we are referring to t.Q, otherwise t.V
			acc = p->t[op->byte].V;
		    else acc = resolve(p,TIMER,op->byte);
		break;
		case BLINKOUT://bit is irrelevant
		    acc = resolve(p,BLINKER,op->byte);
		break;
		case COMMAND:
		    acc = p->command;
		break;
		case RISING://only boolean
	    	    if(!boolflag)//only gets called when bit is defined
			return ERR_BADOPERAND;
		    acc = re(p,DI,(op->byte)/8+op->bit);
		break;    
		case FALLING://only boolflagean
	    	    if(!boolflag)//only gets called when bit is defined
			return ERR_BADOPERAND;
		    acc = fe(p,DI,(op->byte)/8+op->bit);
		break;
		default:return ERR_BADOPERAND;
		break;
	    }
	    //if negate, negate acc
	    if(op->modifier==IL_NEG)
		acc = 255-acc;

	break;
	case  IL_ST:	//ST: output
	    //if negate, negate acc
	    if(op->modifier==IL_NEG)
		acc = 255-acc;
	    switch(op->operand)
	    {
		case CONTACT://set output %QX.Y
		    if(!boolflag)//word
			p->outputs[op->byte] = acc;
		    else contact(p,DQ,(op->byte)/8+op->bit, acc%2);
		break;
		case START://bits are irrelevant
		    contact(p,TIMER,op->byte, acc%2);
		break;
		case PULSEIN:
		    if(!boolflag)//word
			p->m[op->byte].V = acc;
		    else contact(p,COUNTER,op->byte, acc%2);
		break;
		case WRITE:
		    p->command = acc;
		default: return ERR_BADOPERAND;
	    }

//any operand, only push
	break;
	default://all others
		    if(op->modifier==IL_NEG)
			op->operand += NEGATE;
		    
		    if(op->modifier==IL_PUSH)
		    {
			push(op->operation, acc);
			op->operation = IL_LD;
			op->modifier=IL_NORM;
			instruct(p,op,pc);//recursiveness?
		    }
		    else acc = operate(op->operation, acc, op->operand);
	break;
    }
//        sprintf(tst, "Lines: %d, Operation: %d Operand:%d Mod:%d Accumulator:%d",Lineno, op->operation,op->operand,op->modifier,acc);
//	draw_info_line(4+Pagelen/2,tst);

    return acc;

}
BYTE operate(BYTE op, BYTE a, BYTE b)
{
    BYTE modularize,r=0;//return value
    modularize = 0;
    if(op & BOOL)
    {//BOOLean type instruction
		a&=TRUE;//consider only LSB
	/*	if(op & NEGATE)
		{//negate b
			op -= NEGATE;
				b=(b>0)?FALSE:TRUE;
		}
		else*/	
		b=(b>0)?TRUE:FALSE;//consider only LSB
		op -= BOOL;
		modularize = TRUE;
	/*	switch(op)
		{
			case IL_AND:
			r = a?b:FALSE;
			break;
			case IL_OR:
			r = (a+b>0)?TRUE:FALSE;
			break;
			case IL_XOR:
			r = (a+b==TRUE)?TRUE:FALSE;
			break;
		//    default://error. we'll see about that
		}*/
    }
    //{
	if(op & NEGATE)
	{//negate b
		op -= NEGATE;
		b=255-b;
	}
	switch(op)
	{
	    //boolean or bitwise, all modifiers, 
	    case  IL_AND:	//AND
		r = a&b;    
	    break;
	    
	    case  IL_OR:	//OR
		r = a|b;
    	    break;
	
	    case  IL_XOR:	//XOR
		r = a^b;
    	    break;
	    case  IL_ADD:
		if (a + b > 255)
		    r = a+b-255;
		else r = a+b;
	    break;
	    case  IL_SUB:
		if(a >= b)
		    r = a - b;
		else r = 256 - b + a;//2's complement
	    break;
	    case  IL_MUL:
		r = (a*b)%256;
	    break;
	    case  IL_DIV:
		r = a/b;
	    break;
	    case  IL_GT:
		r = (a>b);
	    break;
	    case  IL_GE:
		r = (a>=b);
	    break;
	    case  IL_EQ:
		r = (a==b);
    	    break;
	    case  IL_NE:
		r = (a!=b);    
	    break;
    	    case  IL_LT:
		r = (a<b);
	    break;
	    case  IL_LE:
		r = (a<=b);
	    break;
	    default:
//		return ERR_BADOPERAND;
	    break;
	}
//    }    
    if(modularize)
	return r%2;
    else return r;
}










