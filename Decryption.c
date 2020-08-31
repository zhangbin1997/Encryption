#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<time.h>
#include<openssl/md5.h>
#include"base64.h"
//#include"md5.h"

#define PREFIX_LEN 16
#define MAX_DATALEN 500
#define SUFFIX_LEN 100
#define SYSDATE_LEN 10

static unsigned char gs_md5_outcome[ MAX_DATALEN ] = {};
static char gs_curdate[ SYSDATE_LEN ] = {}; 

static void read_file(unsigned char read_buf[MAX_DATALEN]) 
{     //读文件内容
    int fd = -1, len = 0;
    char* path = "/etc/dcfile/license";

    fd = open(path,O_RDONLY);  
    if (fd == -1) {
        printf("open error\n");
        return;
    }
    if ((len = read(fd,read_buf,MAX_DATALEN)) == -1) {
        printf("已过期\n");
        return;
    }
    close(fd);
}

static void encrypt_date(unsigned char* prefix) 
{         //进行MD5加密
    MD5_CTX ctx;
    MD5_Init(&ctx); 
    if (!prefix) {
        printf("prefix is null\n");
        return;
    }
    MD5_Update(&ctx,prefix,strlen (prefix));
    MD5_Final(gs_md5_outcome,&ctx);
}

static void get_sysdate() 
{        //获取当前系统的年月日
    time_t seconds;
    struct tm* date = NULL;

    time(&seconds);       //等同于seoncds = time(NULL)
    date = localtime(&seconds);
    sprintf(gs_curdate,"%04d%02d%02d",date->tm_year + 1900,date->tm_mon + 1,date->tm_mday);
}

static int time_len(const char* dated)
{      //获取此时间距离2020.01.01的天数
    char date[10] = {};
    int month_len[13] = {0,31,28,31,30,31,30,31,31,30,31,30,30};
    int counts = 0, month, day, year, i;

    if (!dated) {
        printf("dated is null\n");
        return -1;
    }
    strncpy(date,dated,4);
    year = atoi(date);
    if (year > 2020) {
        counts += 365 * (year - 2020);
    }
    memset(date,0,sizeof (date));
    strncpy(date,dated+4,2);
    month = atoi(date);

    for (i = 1; i < month; ++i) {
        counts += month_len[i];
    }

    strncpy(date,dated+6,2);
    day = atoi(date);
    counts += day;
    return counts;
}

static int time_out(const char* dated,const char* cur)
{      //根据两个日期距离2020.01.01的天数差距来判断是否过期
    int dated_len = time_len(dated);
    int cur_len = time_len(cur);
    return dated_len - cur_len;
}

int main ()
{
    unsigned char* decode = NULL;
    unsigned char read_buf[MAX_DATALEN] = {}, str[MAX_DATALEN] = {};
    unsigned char prefix[MAX_DATALEN] = {}, suffix[SUFFIX_LEN] = {};
    int len, timeout;

    read_file(read_buf);

  //  strncpy (str,read_buf,strlen(read_buf));
  //  len = strlen(str);
    /*
    if (!base64_decode(read_buf,&decode)) {
        printf("read_buf is null\n");
    }
    */

    decode = base64_decode(read_buf);
    strncpy(prefix,decode,PREFIX_LEN);
    strncpy(suffix,decode+PREFIX_LEN,SUFFIX_LEN);
    free(decode);
    decode = NULL;
 //   base64_decode(prefix,&decode);
    decode = base64_decode(prefix);
    encrypt_date(decode);
    if (strcmp(gs_md5_outcome,suffix)) {
        printf("已过期\n");
        return -1;
    }
    strncpy(prefix,decode+4,PREFIX_LEN);
    get_sysdate();            
 //   strcpy(gs_curdate,"20220801");         手动更改证书有效日期
    timeout = time_out(prefix,gs_curdate);
    if (timeout < 0) {
        printf("证书有效日期为：%s\t已过期 %d 天\n",prefix,-timeout);
    } else {
        printf("证书有效日期为：%s\t还差 %d 天过期\n",prefix,timeout);
    }
    free(decode);
    return 0;  
}

