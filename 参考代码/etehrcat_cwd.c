#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(void)
{
        FILE *fp = NULL;
        int file_size;
        char cmd[50000]={""};
        char buffer[10240];
        char result[40000]={"suibianshiyixia"};
        char cmd_1[100]={"curl -d \"xml_message="};
        char cmd_2[10]={"\"http://"};
        char cmd_3[20]={"192.168.168.2:8080"};
        char cmd_4[20]={"/api/v1/upload"};
        //while(1) {
            // fp = NULL;
            // fp = popen("ethercat xml -p 0,1", "r");
            // if(!fp) {
            //         perror("popen");
            //         exit(EXIT_FAILURE);
            // }
            // printf("=====================================================================================");
            // while (!feof(fp))
            // {
            //     if (fgets(buffer,4096,fp))
            //     {
            //         strcat(result,buffer);
            //     }
            //     if (sizeof(result)>40000) break;
            // }
            // printf("output:%s\n" , result );
            // pclose(fp);
            //sleep(1);
       //}
        strcat(cmd,cmd_1);
        strcat(cmd,result);
        strcat(cmd,cmd_2);
        strcat(cmd,cmd_3);
        strcat(cmd,cmd_4);
        system(cmd);
        printf(cmd);
}

