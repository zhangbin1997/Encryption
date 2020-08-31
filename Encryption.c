#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<time.h>
#include<openssl/md5.h>
#include"base64.h"
//#include"md5.h"

#define SYSDATE_LEN 10
#define MAX_DATALEN 500

static unsigned char gs_curdate[SYSDATE_LEN]  = {};
static unsigned char gs_md5_outcome[20]  = {};
static unsigned char gs_jointed_string[MAX_DATALEN] = {};

/*
static void get_sysdate ()   //获取当前系统的年月日
{        
    time_t seconds;
    struct tm* date = NULL;
    time (&seconds) ;      //等同于seoncds = time(NULL)
    date = localtime (&seconds) ;
    sprintf (gs_curdate,"%04d%02d%02d",date->tm_year + 1900,date->tm_mon + 1,date->tm_mday) ;
}
*/

static void encrypt_date(unsigned char* prefix) 
{       //进行MD5加密
    MD5_CTX ctx;
    MD5_Init(&ctx); 
    if (!prefix) {
		printf("prefix is null\n");
		return;
	}
    MD5_Update(&ctx,prefix,strlen (prefix));
    MD5_Final(gs_md5_outcome,&ctx);
}

static unsigned char* joint_string() 
{         
/*
  前缀加上日期拼接成一个字符串A,对字符串A进行MD5加密生成字符串B，对字符串A进行Base64加密生成字符串C，
  然后拼接字符串C,B生成字符串D，返回字符串D 。
*/
    unsigned char prefix[MAX_DATALEN] = "abcd";
    unsigned char* base64 = NULL, *encrypted_date = NULL;
    encrypted_date = (unsigned char*)malloc(sizeof(unsigned char)*16);
    if (!encrypted_date) {
    	printf("encrypted_date is null\n");
    	return NULL;
    }
    strcat(prefix,gs_curdate);

 //   base64_encode(prefix,&base64);       //base64长度为16
    base64 = base64_encode(prefix);

  //  unsigned char* encrypted_date;
    encrypt_date(prefix);           //MD5长度为16
 //   strncpy (encrypted_date,gs_md5_outcome,strlen(gs_md5_outcome));
    strncpy (gs_jointed_string,base64,strlen(base64));
  //  strcat (gs_jointed_string,encrypted_date);

    strcat (gs_jointed_string,gs_md5_outcome);
    free(base64);         //base64_encode/decode中没有释放
    free(encrypted_date);
    return gs_jointed_string;
}

static void write_file (unsigned char* str) 
{          //将内容写入文件
    int fd = -1;
    char* path = "/etc/dcfile/license";
 //   unsigned char buf[MAX_DATALEN] = {};
    if (!str) {
		printf("str is null\n");
		return;
	}
  //  strncpy(buf,str,strlen(str));
    fd = open(path,O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);       //设置文件权限为644
    if (fd == -1) {
        printf("create file failed\n");
        return;
    }
    if (write(fd,str,strlen(str)) == -1) {
        printf("write error\n");
        return;
    }
    close(fd);
}

static int is_digit(const char* src) 
{
	if (!src) {
		printf("src is null\n");
		exit(-1);
	}
    int len = strlen(src), i;
    char judge[3] = {};
    if (len != 8) {
        printf("请输入有效的证书格式\n");
		exit(-1);
    }
    strncpy(judge,src + 4,2);
    if (atoi(judge) > 12) {
        printf("请输入有效的月份\n");
		exit(-1);
    }
    strncpy(judge,src + 6,2);
    if (atoi(judge) > 31) {
        printf("请输入有效的日期\n");
        exit(-1);
    }

    for (i = 0; i < len; ++i) {
        if (!isdigit(src[i])) {
            printf("请输入有效的证书格式\n");
            exit(-1);
        }
    }
    return 0;
}

int  main(int argc,char* argv[])
{
    unsigned char* str = NULL;
    unsigned char* base64 = NULL;
    if (argc < 2) {
        printf ("请输入正确的操作命令 : \n");
        return -1;
    }          
    strncpy(gs_curdate,argv[1],strlen(argv[1]));      
    is_digit(gs_curdate);
//  get_sysdate();
    joint_string();
 //   base64_encode(gs_jointed_string,&base64);
    base64 = base64_encode(gs_jointed_string);
    write_file(base64);
    free(base64);
    return 0;
}