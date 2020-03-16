#ifndef __CONVERTER_HEADER__
#define __CONVERTER_HEADER__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#define TRUE 1
#define FALSE 0
#define NAME_SIZE 256
#define STR_SIZE 512
#define FILE_SIZE 4096
#define HEADER_INDEX 10
#define FUNC_MAX 10
#define FILE_MAX 10
#define SECOND_TO_MICRO 1000000
#define S_MODE ( S_IRWXU | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH )

typedef _Bool bool;

typedef struct _optind
{
	int opt_j; // 변환할 java 코드 출력
	int opt_c; // 변환할 c 코드 출력
	int opt_p; // java 함수들을 대응하는 c 프로그램 함수와 같이 출력 
	int opt_f; // java 프로그램과 c 프로그램 파일의 파일 크기 출력
	int opt_l; // java 프로그램과 c 프로그램 파일의 라인 수 출력
	int opt_r; // 변환되는 과정을 한 줄마다 출력
}Optind;

/* 사용할 전역 변수 */

char fname[NAME_SIZE]; // 프로그램 이름
char fname_java[NAME_SIZE]; // 자바 코드 파일
char fname_c[NAME_SIZE]; // c 코드 파일
char fname_makefile[NAME_SIZE]; // Makefile 
char header_array[HEADER_INDEX][STR_SIZE]; // header array table
char fname_append[FILE_MAX][NAME_SIZE]; // 메인 클래스 외에 다른 클래스가 있을 때 처리하는 용도
char fconvert[FUNC_MAX][STR_SIZE]; // p option 

/* SUPPORT MAIN -> setting.c */

void main_exception(int argc,char * argv[]); // 기본 예외처리

void convert_init(Optind * opt,int argc,char *argv[]); // 옵션 초기화 및 기타 초기화 작업

void ssu_runtime(struct timeval * begin,struct timeval * end); // 시간 출력 함수

void make_header_table(void); // 헤더 테이블 만들기

/* CONVERT JAVA CODE INTO C CODE -> convert.c */

void convert(Optind opt); // 변환 시작

void parsing_head(char * str,char * des,int index); // header 테이블 파싱

void header_check(FILE * jfile,FILE * cfile); // 헤더 체크 함수

bool header_exist(FILE * cfile,char * str); // 위 header_check함수에서 header를 쓰면 이미 써져 있는지 검사

int header_index(char * str); // header_array에서 str에 해당하는 인덱스 리턴

int number_of_header(char * str); // '#' 기준으로 개수 판단

void header_write(FILE * cfile,char * str); // 파일에 헤더 쓰기

void convert_java(FILE * jfile,pid_t pid); // r 옵션 안하는 변환

bool judge_class(char * jstr); // 클래스 검사

bool make_cfile(char * jstr,char * cstr,int * num); // 클래스를 c 파일로 변환

void make_makefile(void); // Makefile 만들기

int isConverted(char * str); // 이미 변환-저장된 함수면 -1 리턴 저장안된 함수면 입력할 fconvert의 인덱스 반환

void remove_tab(char * jstr);

int count_lines(FILE * file); // 파일의 라인 수 세기

void code_print(FILE * file); // 코드 출력

#endif
