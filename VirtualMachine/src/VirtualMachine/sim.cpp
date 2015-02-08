#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

extern "C"{
#include "plclib.h"
#include "parser.h"
}

extern const char errmsg[][256];

main(){
    int idx,r,lineno,i,j,k, found_start=FALSE;
    struct PLC_regs * p;
	FILE *fp;
	char * tab;
	char line[MAXSTR], name[128], val[16], msg[64], idx_str[4];
	i=0;
	Di=4;
	Dq=4;
	
	char path[]="../src/VirtualMachine/program";	
	if (fp = fopen(path, "r")){
		memset(line, 0, MAXSTR);
		memset(name, 0, 128);
		memset(val, 0, 16);
        // exit(EXIT_FAILURE);
		while(fgets(line, 256, fp)){
			if((line[0]=='L' && line[1]=='D') || (line[0]=='I' && line[1]=='L')){//or 'IL' for IL
				 found_start=TRUE;
			}else{
				while(strchr(line,'\t')!=NULL)//tabs are not supported
				{
					tab = strchr(line,'\t');
					*tab = '.';
				}
				memset(Lines[i],0,MAXSTR);
				memset(Labels[i],0,MAXSTR);
				sprintf(Lines[i++],"%s",line);				
			}
			r = OK;
			memset(line, 0, MAXSTR);
			memset(name, 0, 128);               	
		}
		fclose(fp);
		Lineno=i;
	}	
	// printf("LINES=%s, No Lines=%d\n",Lines,Lineno);

	p=&plc;	
	r = LD_task(&plc);
	printf("return=%d\n",r);
	
    for(j = 0;j<Lineno;j++){
	    printf("Pos[%d]=%d",j,Pos[j]);
		printf("->Val[%d]=%d\n",j,Val[j]);	    
    }
}
