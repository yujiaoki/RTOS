 /* writertest.c
  */
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>



// 自作strcat
char* connect_string(char* s1, const char* s2)
{
    // ループ変数
    int i = 0, j = 0;
    // s1の末尾を探索
    for(; s1[i] != '\0'; i++);
    // s2の先頭から順にコピー
    for(; s2[j] != '\0'; j++)
    {
        s1[i + j] = s2[j];
    }
    // 終端文字をコピー
    s1[i + j] = s2[j];
    return s1;
}

int main() {
	char s1[16] = "hoge";
	// 結合する文字
	char s2[] = "hage";
	// 文字の結合
	connect_string(s1, s2); 
	printf("%s",s1);
}