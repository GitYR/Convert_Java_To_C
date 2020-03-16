#include "convert.h"

void main_exception(int argc,char * argv[]) // main함수의 exception 처리
{
	FILE * temp;

	if(argc < 2)
	{
		fprintf(stderr,"Usage : %s <FILENAME> [OPTION]\n", argv[0]);
		exit(1);
	}
	if(strstr(argv[1],".java") == NULL)
	{
		fprintf(stderr,"<FILENAME> must have a \".java\" extension!\n");
		exit(1);
	}

	memset((char *)fname_java,0,NAME_SIZE);
	sprintf(fname_java,"%s", argv[1]);

	if((temp = fopen(fname_java,"r")) == NULL)
	{
		fprintf(stderr,"The file <%s> cannot open.\n", fname_java);
		exit(1);
	}
	else
		fclose(temp);
}

void convert_init(Optind * opt,int argc,char * argv[]) // convert를 위한 초기화
{
	int i,j,debug;

	opt->opt_j = opt->opt_c = opt->opt_p = opt->opt_f = opt->opt_l = opt->opt_r = 0;

	memset((char *)fname,0,NAME_SIZE);
	memset((char *)fname_c,0,NAME_SIZE);
	memset((char *)fname_makefile,0,NAME_SIZE);

	/* 파일 이름 추출 및 저장 */

	for(i = strlen(argv[1]) - 1; i >= 0; i--) // java파일이 절대경로 일 수 있기 때문에 파싱
	{
		if(argv[1][i] == '/')
		{
			i++;
			break;
		}
	}
	if(i == -1)
		i = 0;

	for(j = i; argv[1][j] != '.'; j++)
		fname[j-i] = argv[1][j];

	sprintf(fname_c,"%s.c", fname);
	sprintf(fname_makefile,"%s_Makefile", fname);

	/* p option을 위한 배열 초기화 */

	for(i = 0; i < FUNC_MAX; i++)
		memset((char *)fconvert[i],0,STR_SIZE);

	/* 헤더 파일을 위한 테이블 배열 */

	for(i = 0; i < HEADER_INDEX; i++)
		memset((char *)header_array[i],0,STR_SIZE);

	sprintf(header_array[0],"open #include <fcntl.h> #include <sys/stat.h> #include <sys/types.h>");
	sprintf(header_array[1],"read #include <unistd.h>");
	sprintf(header_array[2],"write #include <unistd.h>");
	sprintf(header_array[3],"printf #include <stdio.h>");
	sprintf(header_array[4],"scanf #include <stdio.h>");
	sprintf(header_array[5],"string #include <string.h>");
	sprintf(header_array[6],"exit #include <stdlib.h>");

	/* 메인 클래스 외 다른 클래스를 발견할 때 파일을 생성하기 위한 용도 */

	for(i = 0; i < FILE_MAX; i++)
		memset((char *)fname_append[i],0,NAME_SIZE);
}

void ssu_runtime(struct timeval * begin,struct timeval * end)
{
	end -> tv_sec -= begin -> tv_sec;

	if(end -> tv_usec < begin -> tv_usec)
	{
		end -> tv_sec--;
		end -> tv_usec += SECOND_TO_MICRO;
	}
	end -> tv_usec -= begin -> tv_usec;
	printf("Runtime : %ld:%06ld(sec:usec)\n", end -> tv_sec, end -> tv_usec);
}
