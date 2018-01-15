#include<stdio.h>
#include<string.h>
#include<stdlib.h>
int calc_spaces(char *c);
void wtf(FILE *,char *,char *,int);
void calc_hex(char *hex,char *arg);
void alt_rev(char *str);
int get_target(char *,char *,char *,int);
int find_reg(char *oper);
int check_addr(char *oper,char *disp);
int search_symbol(char *sym_addr,char *arg);

char hexes[8][10]={"000","001","010","011","100","101","110","111"};
char r_8[9][10]={"AL","CL","DL","BL","AH","CH","DH","BH","1"};
char r_16[9][10]={"AX","CX","DX","BX","SP","BP","SI","DI","2"};
char r_32[9][10]={"EAX","ECX","EDX","EBX","ESP","EBP","ESI","EDI","3"};
char r_seg[9][10]={"ES","CS","SS","DS","FS","GS","$$","$$","4"};
char rm_16[9][10]={"[BX+SI]","[BX+DI]","[BP+SI]","[BP+DI]","[SI]","[DI]","[BP]","[BX]","5"};
char rm_32[9][10]={"[EAX]","[ECX]","[EDX]","[EBX]","$$$$","[EBP]","[ESI]","[EDI]","6"};

int main(){
    FILE *f_in,*sym_table,*mem_map,*mem_final;
    int lc,s_flag,spaces,comma_index,space_index,num,status;
    char *inst=malloc(sizeof(char)*150);
    char cnt[10],asm_dir[30],cs_addr[5],ds_addr[5],label[25],op[25],arg[50],hex[100],taddr[10];
    char oper1[50],oper2[50],dbit[2],wbit[2],mod1[2],mod2[2],disp[10];
    int type1,type2,reg1,reg2,tmp;
    memset(label, '\0', sizeof(label));
    memset(op, '\0', sizeof(op));
    memset(dbit, '\0', sizeof(dbit));
    memset(wbit, '\0', sizeof(wbit));
    memset(mod1, '\0', sizeof(mod1));
    memset(mod2, '\0', sizeof(mod2));
    memset(arg, '\0', sizeof(arg));
    memset(hex, '\0', sizeof(hex));
    memset(disp, '\0', sizeof(disp));
    memset(cnt, '\0', sizeof(cnt));
    memset(taddr, '\0', sizeof(taddr));
    memset(oper1, '\0', sizeof(oper1));
    memset(oper2, '\0', sizeof(oper2));
    sym_table=fopen("symbol_table.txt","w");
    mem_map=fopen("memory_map.txt","w");
    mem_final=fopen("memory_map_final.txt","w");
    f_in=fopen("asm_in.txt","r");
    if(f_in==NULL){
        printf("\nERROR_1 : Input assembly file not found");
    }else{
        while(!feof(f_in)){
            fgets( inst , 150 , f_in );
            //check for assembler directive
            strupr(inst);
            inst[strlen(inst)-1]='\0';
            if(inst[0]=='.'){
                memset(asm_dir, '\0', sizeof(asm_dir));
                inst=inst+1;
                strcpy(asm_dir,inst);
                if(strcmp(asm_dir,"CODE")==0){
                    lc=0;
                    s_flag=1;
                }else if(strcmp(asm_dir,"DATA")==0){
                    lc=0;
                    s_flag=0;
                }else if(strcmp(asm_dir,"EXIT")==0){

                }else if(strcmp(asm_dir,"STARTUP")==0){

                }else{
                    if(strncmp(asm_dir,"MODEL",5)==0){
                        inst=inst+6;
                        strcpy(asm_dir,inst);
                        if(strcmp(asm_dir,"SMALL")==0){
                            strcpy(cs_addr,"3000");
                            strcpy(ds_addr,"0000");
                        }else if(strcmp(asm_dir,"TINY")==0){
                            strcpy(cs_addr,"3000");
                        }else{
                            printf("\nERROR_2 : Assembler directive: \"%s\" does not exist",asm_dir);
                        }
                    }
                    else{
                        printf("\nERROR_3 : Assembler directive: \"%s\" does not exist",asm_dir);
                    }
                }
            }else{ //check for pseudo op or machine op
                spaces=calc_spaces(inst);
                if(spaces>1){
                    if(s_flag==1){ // code segment
                        //printf("cs\n");
                        memset(label, '\0', sizeof(label));
                        space_index=strcspn(inst," ");
                        strncpy(label,inst,space_index);
                        wtf(sym_table,label,cs_addr,lc);
                        inst=inst+strlen(label)+1;
                    }else if(s_flag==0){ //data segment
                        //printf("ds\n");
                        if(spaces==2 && inst[strlen(inst)-1]==')'){
                        }else{
                            memset(label, '\0', sizeof(label));
                            space_index=strcspn(inst," ");
                            strncpy(label,inst,space_index);
                            wtf(sym_table,label,ds_addr,lc);
                            inst=inst+strlen(label)+1;
                        }
                    }
                }
                if(s_flag==0){// data segment
                    memset(op, '\0', sizeof(op));
                    space_index=strcspn(inst," ");
                    strncpy(op,inst,space_index);
                    inst=inst+strlen(op)+1;
                    strcpy(arg,inst);
                    spaces=calc_spaces(inst);
                    if(spaces==0){
                        if(strcmp(op,"DB")==0){
                            calc_hex(hex,arg);
                            alt_rev(hex);
                            wtf(mem_map,hex,ds_addr,lc);
                            lc=lc+1;
                        }else if(strcmp(op,"DW")==0){
                            calc_hex(hex,arg);
                            alt_rev(hex);
                            wtf(mem_map,hex,ds_addr,lc);
                            lc=lc+2;
                        }else if(strcmp(op,"DD")==0){
                            calc_hex(hex,arg);
                            alt_rev(hex);
                            wtf(mem_map,hex,ds_addr,lc);
                            lc=lc+4;
                        }else if(strcmp(op,"DQ")==0){
                            calc_hex(hex,arg);
                            alt_rev(hex);
                            wtf(mem_map,hex,ds_addr,lc);
                            lc=lc+8;
                        }else{
                            printf("\nERROR_4 : Inavlid Op Code : \"%s\"",op);
                        }
                    }else if(spaces==1){
                        memset(cnt, '\0', sizeof(cnt));
                        space_index=strcspn(inst," ");
                        strncpy(cnt,inst,space_index);
                        inst=inst+strlen(cnt)+5;
                        inst[strlen(inst)-1]='\0';
                        strcpy(arg,inst);
                        num=str_to_num(cnt);
                        while(num--){
                            if(strcmp(op,"DB")==0){
                                calc_hex(hex,arg);
                                alt_rev(hex);
                                wtf(mem_map,hex,ds_addr,lc);
                                lc=lc+1;
                            }else if(strcmp(op,"DW")==0){
                                calc_hex(hex,arg);
                                alt_rev(hex);
                                wtf(mem_map,hex,ds_addr,lc);
                                lc=lc+2;
                            }else if(strcmp(op,"DD")==0){
                                calc_hex(hex,arg);
                                alt_rev(hex);
                                wtf(mem_map,hex,ds_addr,lc);
                                lc=lc+4;
                            }else if(strcmp(op,"DQ")==0){
                                calc_hex(hex,arg);
                                alt_rev(hex);
                                wtf(mem_map,hex,ds_addr,lc);
                                lc=lc+8;
                            }
                        }
                    }
                }else{ // code segment
                    memset(op, '\0', sizeof(op));
                    space_index=strcspn(inst," ");
                    strncpy(op,inst,space_index);
                    inst=inst+strlen(op)+1;
                    strcpy(arg,inst);
                    if(strcmp(op,"MOV")==0){
                        memset(oper1, '\0', sizeof(oper1));
                        memset(oper2, '\0', sizeof(oper2));
                        memset(disp, '\0', sizeof(disp));
                        memset(dbit, '\0', sizeof(dbit));
                        memset(wbit, '\0', sizeof(wbit));
                        memset(mod1, '\0', sizeof(mod1));
                        memset(mod2, '\0', sizeof(mod2));
                        comma_index=strcspn(inst,",");
                        strncpy(oper1,inst,comma_index);
                        inst=inst+strlen(oper1)+1;
                        strcpy(oper2,inst);
                        tmp=find_reg(oper1);
                        type1=tmp/10;
                        reg1=tmp%10;
                        tmp=find_reg(oper2);
                        type2=tmp/10;
                        reg2=tmp%10;
                        if(reg1==-1){
                            fclose(sym_table);
                            if(check_addr(oper1,disp)==1){
                                type1=7;
                            }else if(search_symbol(disp,oper1)==1){
                                type1=8;
                                printf("ERROR_5 : First operator is label -> %s\n",oper1);
                            }else{
                                printf("ERROR_6 : Invalid Reg. -> %s\n",oper1);
                            }
                            sym_table=fopen("symbol_table.txt","a");
                        }
                        if(reg2==-1){
                            fclose(sym_table);
                            if(check_addr(oper2,disp)==1){
                                type2=7;
                            }else if(search_symbol(disp,oper2)==1){
                                type2=8;
                            }else{
                                printf("ERROR_7 : Invalid Reg. -> %s\n",oper2);
                            }
                            sym_table=fopen("symbol_table.txt","a");
                        }
                        if(type1==4 && type2==4){
                            printf("ERROR_8 : Both reg cant be segment -> %s,%s\n",oper1,oper2);
                        }else if(type1+type2>=10 && (type1!=8 && type2!=8)){
                             printf("ERROR_9 : Both reg cant be memory refernce -> %s,%s\n",oper1,oper2);
                        }else if(type1==1 &&(type2==2 || type2==3 || type2==4)){
                            printf("ERROR_10 : Reg size mismatch -> %s,%s\n",oper1,oper2);
                        }else if(type2==1 &&(type1==2 || type1==3 || type1==4)){
                            printf("ERROR_11 : Reg size mismatch -> %s,%s\n",oper1,oper2);
                        }else if(type1==2 &&(type2==1 || type2==3)){
                            printf("ERROR_12 : Reg size mismatch -> %s,%s\n",oper1,oper2);
                        }else if(type2==2 &&(type1==1 || type1==3)){
                            printf("ERROR_13 : Reg size mismatch -> %s,%s\n",oper1,oper2);
                        }else if(type1==3 &&(type2==1 || type2==2 || type2==4)){
                            printf("ERROR_14 : Reg size mismatch -> %s,%s\n",oper1,oper2);
                        }else if(type2==3 &&(type1==1 || type1==2 || type1==4)){
                            printf("ERROR_15 : Reg size mismatch -> %s,%s\n",oper1,oper2);
                        }else if(type1==4 &&(type2==1 || type2==3)){
                            printf("ERROR_16 : Reg size mismatch -> %s,%s\n",oper1,oper2);
                        }else if(type2==4 &&(type1==1 || type1==3)){
                            printf("ERROR_17 : Reg size mismatch -> %s,%s\n",oper1,oper2);
                        }else if(type1==4 && reg1==1){
                            printf("ERROR_18 : Destination reg cannot be code seg. -> %s,%s\n",oper1,oper2);
                        }else{
                            if(type1==4 || type2==4){
                                strcpy(hex,"01100110100011");
                                strcpy(wbit,"0");
                                if(type1==4){
                                    strcpy(dbit,"1");
                                    if(type2==5 || type2==6 || type2==7){
                                        strcpy(mod1,"0");
                                        strcpy(mod2,"0");
                                    }else{
                                        strcpy(mod1,"1");
                                        strcpy(mod2,"1");
                                    }
                                }else{
                                    strcpy(dbit,"0");
                                    if(type1==5 || type1==6 || type1==7){
                                        strcpy(mod1,"0");
                                        strcpy(mod2,"0");
                                    }else{
                                        strcpy(mod1,"1");
                                        strcpy(mod2,"1");
                                    }
                                    tmp=reg1;
                                    reg1=reg2;
                                    reg2=tmp;
                                }
                                strcat(hex,dbit);
                                strcat(hex,wbit);
                                strcat(hex,mod1);
                                strcat(hex,mod2);
                                strcat(hex,hexes[reg1]);
                                strcat(hex,hexes[reg2]);
                                sprintf(hex,"%X",strtol(hex,NULL,2));
                                if(type1==7)
                                    tmp=check_addr(oper1,disp);
                                else if(type2==7)
                                    tmp=check_addr(oper2,disp);
                                alt_rev(disp);
                                strcat(hex,disp);
                                wtf(mem_map,hex,cs_addr,lc);
                                lc=lc+strlen(hex)/2;
                            }else{
                                if(type1==2 || type2==2){
                                    if(type1==5 || type2==5)
                                        strcpy(hex,"0110011101100110100010");
                                    else
                                        strcpy(hex,"01100110100010");
                                }else if(type1==5 || type2==5){
                                    strcpy(hex,"01100111100010");
                                }else{
                                    strcpy(hex,"100010");
                                }
                                if(type1==1 || type2==1){
                                    strcpy(wbit,"0");
                                }else{
                                    strcpy(wbit,"1");
                                }
                                if(type1==5 || type1==6){
                                    strcpy(dbit,"0");
                                }else{
                                    strcpy(dbit,"1");
                                }
                                if(type1>=5 || type2>=5){
                                    strcpy(mod1,"0");
                                    strcpy(mod2,"0");
                                }else{
                                    strcpy(mod1,"1");
                                    strcpy(mod2,"1");
                                }
                                if(type1>=7){
                                    reg1=6;
                                    tmp=reg1;
                                    reg1=reg2;
                                    reg2=tmp;
                                }
                                if(type2>=7){
                                    reg2=6;
                                }
                                strcat(hex,dbit);
                                strcat(hex,wbit);
                                strcat(hex,mod1);
                                strcat(hex,mod2);
                                strcat(hex,hexes[reg1]);
                                strcat(hex,hexes[reg2]);
                                sprintf(hex,"%X",strtol(hex,NULL,2));
                                if(type1==7)
                                    tmp=check_addr(oper1,disp);
                                else if(type2==7)
                                    tmp=check_addr(oper2,disp);
                                alt_rev(disp);
                                strcat(hex,disp);
                                wtf(mem_map,hex,cs_addr,lc);
                                lc=lc+strlen(hex)/2;
                            }
                        }
                    }else if(op[0]=='J'){
                        strcpy(hex,"74");
                        fclose(sym_table);
                        status=get_target(taddr,arg,cs_addr,lc);
                        sym_table=fopen("symbol_table.txt","a");
                        if(status==0){
                            strcat(hex,"????????");
                            strcat(hex,arg);
                        }else{
                            strcat(hex,taddr);
                        }
                        wtf(mem_map,hex,cs_addr,lc);
                        lc=lc+5;
                    }else if(strcmp(op,"END")==0){
                        lc=lc+0;
                        //pass 2 begins
                        fclose(mem_map);
                        mem_map=fopen("memory_map.txt","r");
                        char tmp_addr[10],tmp_label[25],local_label[25],new_addr[25];
                        int local_i,local_j,t,o,l;
                        while(!feof(mem_map)){
                            memset(tmp_addr, '\0', sizeof(tmp_addr));
                            memset(tmp_label, '\0', sizeof(tmp_label));
                            memset(local_label, '\0', sizeof(local_label));
                            memset(new_addr, '\0', sizeof(new_addr));
                            fscanf( mem_map , "%s" , tmp_addr );
                            fscanf( mem_map , "%s" , tmp_label );
                            local_j=0;
                            if(tmp_label[2]=='?'){
                                local_i=10;
                                while(tmp_label[local_i]!='\0'){
                                    local_label[local_j++]=tmp_label[local_i++];
                                }
                                if(search_symbol(disp,local_label)==1){
                                    t=strtol(disp,NULL,16);
                                    l=strtol(tmp_addr,NULL,16);
                                    o=t-(l+5);
                                    sprintf(new_addr,"%X",o);
                                    strrev(new_addr);
                                    for(local_i=0;new_addr[local_i]!='\0';local_i++){}
                                    while(local_i<8){
                                        strcat(new_addr,"0");
                                        local_i++;
                                    }
                                    strrev(new_addr);
                                    strrev(new_addr);
                                    strcat(new_addr,"47");
                                    strrev(new_addr);
                                    fprintf( mem_final , "%s %s\n" , tmp_addr , new_addr);
                                }else{
                                    printf("\nERROR_20 : No label exists -> \"%s\"",local_label);
                                }
                            }else{
                                fprintf( mem_final , "%s %s\n" , tmp_addr , tmp_label);
                            }
                        }
                        fclose(mem_map);
                        fclose(mem_final);
                    }else if(strcmp(op,"EQU")==0){

                    }else if(strcmp(op,"BYTE PTR")==0){

                    }else if(strcmp(op,"WORD PTR")==0){

                    }else if(strcmp(op,"DWORD PTR")==0){

                    }else if(strcmp(op,"QWORD PTR")==0){

                    }else{
                        printf("\nERROR_20 : Inavlid Op Code : \"%s\"",op);
                    }
                }
            }
        }
    }
    fclose(f_in);
    fclose(sym_table);
    return 0;
}

int search_symbol(char *sym_addr,char *arg){
    FILE *s_table;
    s_table=fopen("symbol_table.txt","r");
    char tmp_addr[10],tmp_label[25];
    int flag=0,s_index,i,target,local,offset;
    while(!feof(s_table) && !flag){
        memset(tmp_addr, '\0', sizeof(tmp_addr));
        memset(tmp_label, '\0', sizeof(tmp_label));
        fscanf( s_table , "%s" , tmp_addr );
        fscanf( s_table , "%s" , tmp_label );
        if(strcmp(tmp_label,arg)==0)
            flag=1;
    }
    if(flag){
        strcpy(sym_addr,tmp_addr);
    }
    fclose(s_table);
    return flag;
}
int check_addr(char *oper,char *disp){
    int status=-1,i,flag=0;
    memset(disp, '\0', sizeof(disp));
    if(oper[0]=='[' && oper[strlen(oper)-1]==']'){
        if(oper[strlen(oper)-2]=='H'){
            for(i=1;i<5 && !flag;i++){
                if((oper[i]>='A' && oper[i]<='F') || (oper[i]>='0' && oper[i]<='9')){
                    status=1;
                    disp[i-1]=oper[i];
                }else{
                    status=-1;
                    flag=1;
                    memset(disp, '\0', sizeof(disp));
                }
            }
        }
    }
    return status;
}
int find_reg(char *oper){
    int ans=-1,i;
    for(i=0;i<8 && ans==-1;i++){
        if(strcmp(r_8[i],oper)==0){
            ans=strtol(r_8[8],NULL,10)*10+i;
        }
    }
    for(i=0;i<8 && ans==-1;i++){
        if(strcmp(r_16[i],oper)==0){
            ans=strtol(r_16[8],NULL,10)*10+i;
        }
    }
    for(i=0;i<8 && ans==-1;i++){
        if(strcmp(r_32[i],oper)==0){
            ans=strtol(r_32[8],NULL,10)*10+i;
        }
    }
    for(i=0;i<8 && ans==-1;i++){
        if(strcmp(r_seg[i],oper)==0){
            ans=strtol(r_seg[8],NULL,10)*10+i;
        }
    }
    for(i=0;i<8 && ans==-1;i++){
        if(strcmp(rm_16[i],oper)==0){
            ans=strtol(rm_16[8],NULL,10)*10+i;
        }
    }
    for(i=0;i<8 && ans==-1;i++){
        if(strcmp(rm_32[i],oper)==0){
            ans=strtol(rm_32[8],NULL,10)*10+i;
        }
    }
    return ans;
}
int get_target(char *taddr,char *arg,char *saddr,int lc){
    FILE *s_table;
    s_table=fopen("symbol_table.txt","r");
    char tmp_addr[10],tmp_label[25],lc_str[5],local_addr[10];
    memset(tmp_addr, '\0', sizeof(tmp_addr));
    memset(local_addr, '\0', sizeof(local_addr));
    memset(lc_str, '\0', sizeof(lc_str));
    memset(tmp_label, '\0', sizeof(tmp_label));
    int flag=0,s_index,i,target,local,offset;
    while(!feof(s_table) && !flag){
        fscanf( s_table , "%s" , tmp_addr );
        fscanf( s_table , "%s" , tmp_label );
        if(strcmp(tmp_label,arg)==0)
            flag=1;
    }
    if(flag){
        sprintf(lc_str,"%X",lc);
        strrev(lc_str);
        for(i=0;lc_str[i]!='\0';i++){}
        while(i<4){
            strcat(lc_str,"0");
            i++;
        }
        strrev(lc_str);
        strcat(local_addr,saddr);
        strcat(local_addr,lc_str);
        target=strtol(tmp_addr,NULL,16);
        local=strtol(local_addr,NULL,16);
        offset=target-(local+5);
        sprintf(taddr,"%X",offset);
        /*printf("local: %s  target: %s  offset: %s\n\n",local_addr,tmp_addr,taddr);
        getch();*/
        strrev(taddr);
        for(i=0;taddr[i]!='\0';i++){}
        while(i<8){
            strcat(taddr,"0");
            i++;
        }
        strrev(taddr);
    }
    fclose(s_table);
    return flag;
}
int str_to_num(char *str){
    return strtol(str, NULL, 10);
}
void alt_rev(char *str){
    char temp[10];
    strcpy(temp,str);
    int i=strlen(temp)-2,j=0;
    while(i>=0){
        str[j++]=temp[i];
        str[j++]=temp[i+1];
        i=i-2;
    }
}
void calc_hex(char *hex,char *arg){ // only for psuedo ops
    char str[10];
    int i;
    memset(str, '\0', sizeof(str));
    if(arg[0]=='?'){
        strcpy(hex,"00000000");
    }else{
        sprintf(str,"%X",strtol(arg, NULL, 10));
        strrev(str);
        for(i=0;str[i]!='\0';i++){}
        while(i<8){
            strcat(str,"0");
            i++;
        }
        strrev(str);
        strcpy(hex,str);
    }
}
void wtf(FILE *fp,char *str,char *addr,int lc){
    char lc_str[5];
    int i;
    memset(lc_str, '\0', sizeof(lc_str));
    sprintf(lc_str,"%X",lc);
    strrev(lc_str);
    for(i=0;lc_str[i]!='\0';i++){}
    while(i<4){
        strcat(lc_str,"0");
        i++;
    }
    strrev(lc_str);
    fprintf( fp , "%s%s %s\n" , addr , lc_str , str);
}
int calc_spaces(char *c){
    int ans=0,i;
    for(i=0;i<strlen(c);i++){
        if(c[i]==' ')
            ans++;
    }
    return ans;
}
