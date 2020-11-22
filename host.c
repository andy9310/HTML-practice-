#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
void err_sys(const char* x) 
{ 
    perror(x); 
    exit(1); 
}
typedef struct player_final{
    int player_id;
    int score;
    int rank;
}PF;
int compare(const void *a, const void *b){
      PF c = *(PF *)a;
      PF d = *(PF *)b;
      if(c.score < d.score) {return 1;}               //傳回 -1 代表 a < b
      else if (c.score == d.score) {return 0;}      //傳回   0 代表 a = b
      else return -1;                          //傳回  1 代表 a>b
}
int compare2(const void *a, const void *b){
      PF c = *(PF *)a;
      PF d = *(PF *)b;
      if(c.player_id < d.player_id) {return -1;}               //傳回 -1 代表 a < b
      else if (c.player_id == d.player_id) {return 0;}      //傳回   0 代表 a = b
      else return 1;                          //傳回  1 代表 a>b
}
int main(int argc, char* argv[]){
    pid_t pid,pid2;
    int key = atoi(argv[2]);
    int depth = atoi(argv[3]);
    int root_host = atoi(argv[1]);
    int pipe1_read_from_parent[2];
    int pipe2_write_to_parent[2];
    if(depth == 0){
        char fifoname[16];
        sprintf(fifoname,"fifo_%s.tmp",argv[1]);
        FILE* fd_roothost = fopen(fifoname,"r");
        if(pipe(pipe1_read_from_parent) < 0) err_sys("pipe1 error");
        if(pipe(pipe2_write_to_parent) < 0) err_sys("pipe2 error");
        if((pid = fork()) < 0) err_sys("fork error");
        else if(pid > 0) {              //// parent
            int pipe3_read_from_parent[2];
            int pipe4_write_to_parent[2];
            if(pipe(pipe3_read_from_parent) < 0) err_sys("pipe3 error");
            if(pipe(pipe4_write_to_parent) < 0) err_sys("pipe4 error");
            if((pid2 = fork()) < 0) err_sys("fork error");
            else if(pid2 > 0){ /// parent
                close(pipe1_read_from_parent[0]);
                close(pipe2_write_to_parent[1]);
                close(pipe3_read_from_parent[0]);
                close(pipe4_write_to_parent[1]);
                ////傳資料
                FILE* fd_root = fopen("fifo_0.tmp","w"); // open fifo
                while(true){
                    int player_id[8];
                     // open fifo
                    fscanf(fd_roothost,"%d %d %d %d %d %d %d %d",&(player_id[0]),&(player_id[1]),&(player_id[2]),&(player_id[3]),&(player_id[4]),&(player_id[5]),&(player_id[6]),&(player_id[7]));
                    //fprintf(stderr,"0: %d %d %d %d\n",player_id[0],player_id[1],player_id[2],player_id[3]);
                    //fprintf(stderr,"0: %d %d %d %d\n",player_id[4],player_id[5],player_id[6],player_id[7]);
                    FILE* fp = fdopen(pipe1_read_from_parent[1],"w"); 
                    FILE* fp2 = fdopen(pipe3_read_from_parent[1],"w");
                    FILE* fp3 = fdopen(pipe2_write_to_parent[0],"r");
                    FILE* fp4 = fdopen(pipe4_write_to_parent[0],"r");
                    fprintf(fp,"%d %d %d %d\n",player_id[0],player_id[1],player_id[2],player_id[3]);
                    //fprintf(stderr,"0: %d %d %d %d\n",player_id[0],player_id[1],player_id[2],player_id[3]);
                    fflush(fp);
                    fprintf(fp2,"%d %d %d %d\n",player_id[4],player_id[5],player_id[6],player_id[7]);
                    //fprintf(stderr,"0: %d %d %d %d\n",player_id[4],player_id[5],player_id[6],player_id[7]);
                    fflush(fp2);
                    
                    if(player_id[0] == -1){
                        break;
                    }
                    PF tmplist[8];
                    for(int i = 0;i < 8;i++){
                        tmplist[i].player_id = player_id[i];
                        tmplist[i].score = 0;
                        tmplist[i].rank = 0;
                    }
                    int count = 0;
                    while(count != 10){ 
                        count++;
                        int winner,winner2,bid,bid2,newwinner;
                        fscanf(fp3,"%d %d",&winner,&bid);
                        fscanf(fp4,"%d %d",&winner2,&bid2);
                        //fprintf(stderr,"depth 0: %d %d %d %d\n",winner,bid,winner2,bid2);
                        if(bid > bid2){ 
                            newwinner = winner;
                            for(int i = 0;i < 8;i++){
                                if(tmplist[i].player_id == newwinner){
                                    tmplist[i].score++;
                                }
                            }
                        }
                        else{
                            newwinner = winner2;
                            for(int i = 0;i < 8;i++){
                                if(tmplist[i].player_id == newwinner){
                                    tmplist[i].score++;
                                }
                            }
                        }
                    }
                    //傳給auction
                    
                    qsort(tmplist,8,sizeof(PF),compare);//sort by score 大排到小
                    /*for(int i = 0;i < 8;i++){
                        fprintf(stderr,"tmp: %d\n",tmplist[i].score);
                    }*/
                    for(int i = 0;i < 8;i++){
                        if(i!=0 && tmplist[i].score == tmplist[i-1].score){
                            tmplist[i].rank = tmplist[i-1].rank;
                        }
                        else{
                            tmplist[i].rank = i+1;
                        }
                        
                    }
                    qsort(tmplist,8,sizeof(PF),compare2); //sort by id 小排到大
                    fprintf(fd_root,"%d\n",key);
                    for(int i = 0;i < 8;i++){
                        fprintf(fd_root,"%d %d\n",tmplist[i].player_id,tmplist[i].rank);
                    }
                    fflush(fd_root);
                }
                /*wait(NULL);
                wait(NULL);*/
                pid_t child_pid;
                int status;
                while((child_pid = wait(&status)) != -1){}
                close(pipe3_read_from_parent[1]);
                close(pipe4_write_to_parent[0]);
                close(pipe1_read_from_parent[1]);
                close(pipe2_write_to_parent[0]);
                fclose(fd_roothost);
                exit(0);
            }
            else if(pid2 == 0){   
                close(pipe1_read_from_parent[0]);
                close(pipe2_write_to_parent[1]);              
                close(pipe1_read_from_parent[1]);
                close(pipe2_write_to_parent[0]);
                close(pipe3_read_from_parent[1]);
                close(pipe4_write_to_parent[0]);
                dup2(pipe4_write_to_parent[1],1);
                dup2(pipe3_read_from_parent[0],0);
                execl("./host","./host",argv[1],argv[2],"1",NULL);
            }
            /////
            
        }
        else if(pid == 0){  /// first child
            close(pipe1_read_from_parent[1]);
            close(pipe2_write_to_parent[0]);
            dup2(pipe2_write_to_parent[1],1);
            dup2(pipe1_read_from_parent[0],0);
            execl("./host","./host",argv[1],argv[2],"1",NULL);
        }
    }
    ////////
    else if(depth == 1){ 
        //fprintf(stderr,"depth == 1\n");
        int pipe1_read_from_parent[2];
        int pipe2_write_to_parent[2];
        if(pipe(pipe1_read_from_parent) < 0) err_sys("pipe1 error");
        if(pipe(pipe2_write_to_parent) < 0) err_sys("pipe2 error");

        if((pid = fork()) < 0) err_sys("fork error");
        else if(pid > 0) {              //// parent
            
            int pipe3_read_from_parent[2];
            int pipe4_write_to_parent[2];
            if(pipe(pipe3_read_from_parent) < 0) err_sys("pipe3 error");
            if(pipe(pipe4_write_to_parent) < 0) err_sys("pipe4 error");
            if((pid2 = fork()) < 0) err_sys("fork error");
            else if(pid2 > 0){ //still parent 
                close(pipe1_read_from_parent[0]);
                close(pipe2_write_to_parent[1]);
                close(pipe3_read_from_parent[0]);
                close(pipe4_write_to_parent[1]);
                ////傳資料
                FILE* fp = fdopen(pipe1_read_from_parent[1],"w"); 
                FILE* fp2 = fdopen(pipe3_read_from_parent[1],"w");
                FILE* fp3 = fdopen(pipe2_write_to_parent[0],"r");
                FILE* fp4 = fdopen(pipe4_write_to_parent[0],"r");
                while(true){
                     // 讀上面傳下來的player_id
                    int player_id[4] = {0};
                    scanf("%d %d %d %d",&(player_id[0]),&(player_id[1]),&(player_id[2]),&(player_id[3]));
                    //fprintf(stderr,"1: %d %d %d %d\n",player_id[0],player_id[1],player_id[2],player_id[3]);
                    /*if(player_id[0] == -1){
                        break;
                    }*/
                    
                    fprintf(fp,"%d %d\n",player_id[0],player_id[1]);
                    //fprintf(stderr,"%d %d\n",player_id[0],player_id[1]);
                    fflush(fp);
                    fprintf(fp2,"%d %d\n",player_id[2],player_id[3]);
                    //fprintf(stderr,"%d %d\n",player_id[2],player_id[3]);
                    fflush(fp2);
                    
                    if(player_id[0] == -1){
                        break;
                    }
                    int count = 0;
                    while(count != 10){  /// 10 times
                        count++;
                        int winner,winner2,bid,bid2,newwinner;
                        fscanf(fp3,"%d %d",&winner,&bid);
                        fscanf(fp4,"%d %d",&winner2,&bid2);
                        if(bid > bid2){ /// 往上傳
                            newwinner = winner;
                            printf("%d %d\n",newwinner,bid);
                            fflush(stdout);
                        }
                        else{
                            newwinner = winner2;
                            printf("%d %d\n",newwinner,bid2);
                            fflush(stdout);
                        }
                    }
                }
                /*wait(NULL);
                wait(NULL);*/
                pid_t child_pid;
                int status;
                while((child_pid = wait(&status)) != -1){}
                close(pipe3_read_from_parent[1]);
                close(pipe4_write_to_parent[0]);
                close(pipe1_read_from_parent[1]);
                close(pipe2_write_to_parent[0]);
                exit(0);
            }
            else if(pid2 == 0){ 
                close(pipe1_read_from_parent[0]);
                close(pipe2_write_to_parent[1]);                
                close(pipe1_read_from_parent[1]);
                close(pipe2_write_to_parent[0]);
                close(pipe3_read_from_parent[1]);
                close(pipe4_write_to_parent[0]);
                dup2(pipe4_write_to_parent[1],1);
                dup2(pipe3_read_from_parent[0],0);
                execl("./host","./host",argv[1],argv[2],"2",NULL);
            }
            /////
        }
        else if(pid == 0){  /// first child
            close(pipe1_read_from_parent[1]);
            close(pipe2_write_to_parent[0]);
            dup2(pipe2_write_to_parent[1],1);
            dup2(pipe1_read_from_parent[0],0);
            execl("./host", "./host",argv[1],argv[2],"2",NULL);
        }
    }
    /////
    else if(depth == 2){    ///// leaf host 
        //fprintf(stderr,"depth == 2\n");
        while(true){
        // 讀上面傳下來的player_id
        int player_id[2] = {0};
        scanf("%d %d",&(player_id[0]),&(player_id[1]));
        if(player_id[0] == -1){
            break;
        }
        int pipe1_read_from_parent[2];
        int pipe2_write_to_parent[2];
        if(pipe(pipe1_read_from_parent) < 0) err_sys("pipe1 error");
        if(pipe(pipe2_write_to_parent) < 0) err_sys("pipe2 error");

        if((pid = fork()) < 0) err_sys("fork error");
        else if(pid > 0){              //// parent
            
            int pipe3_read_from_parent[2];
            int pipe4_write_to_parent[2];
            if(pipe(pipe3_read_from_parent) < 0) err_sys("pipe3 error");
            if(pipe(pipe4_write_to_parent) < 0) err_sys("pipe4 error");

            ///// second child
            if((pid2 = fork()) < 0) err_sys("fork error");
            else if(pid2 > 0){ /// still parent
                close(pipe1_read_from_parent[0]);
                close(pipe2_write_to_parent[1]);
                close(pipe3_read_from_parent[0]);
                close(pipe4_write_to_parent[1]);
                int count = 0;
                FILE* fp3 = fdopen(pipe2_write_to_parent[0],"r");
                FILE* fp4 = fdopen(pipe4_write_to_parent[0],"r");
                while(count != 10){
                    count++;
                    int winner=0,winner2=0,bid=0,bid2=0,newwinner;
                    fscanf(fp3,"%d %d",&winner,&bid);
                    fscanf(fp4,"%d %d",&winner2,&bid2);
                    if(bid > bid2){ /// 往上傳
                        newwinner = winner;
                        printf("%d %d\n",newwinner,bid);
                        fflush(stdout);
                    }
                    else{
                        newwinner = winner2;
                        printf("%d %d\n",newwinner,bid2);
                        fflush(stdout);
                    }
                }
                /// 10 times finish , close pipe
                close(pipe3_read_from_parent[1]);
                close(pipe4_write_to_parent[0]);
            }
            else if(pid2 == 0){   
                close(pipe1_read_from_parent[0]);
                close(pipe2_write_to_parent[1]);              
                close(pipe1_read_from_parent[1]);
                close(pipe2_write_to_parent[0]);
                close(pipe3_read_from_parent[1]);
                close(pipe4_write_to_parent[0]);
                dup2(pipe4_write_to_parent[1],1);
                dup2(pipe3_read_from_parent[0],0);
                char b[100];
                sprintf(b,"%d",player_id[1]);
                execl("./player", "./player",b,NULL);
            }
            /////
            close(pipe1_read_from_parent[1]);
            close(pipe2_write_to_parent[0]);
        }
        else if(pid == 0){  /// first child
            close(pipe1_read_from_parent[1]);
            close(pipe2_write_to_parent[0]);
            dup2(pipe2_write_to_parent[1],1);
            dup2(pipe1_read_from_parent[0],0);
            char a[100];
            sprintf(a,"%d",player_id[0]);
            execl("./player", "./player",a,NULL);
        }
        }
        exit(0);
    }
}