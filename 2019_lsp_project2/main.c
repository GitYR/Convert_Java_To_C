#include "convert.h"

int main(int argc,char * argv[])
{
	Optind opt;
	char option;
	struct timeval begin,end;

	gettimeofday(&begin,NULL); // 시간 측정 시작

	/* 예외 처리 */

	main_exception(argc,argv); 

	/* 기본 초기화 작업 */	
	
	convert_init(&opt,argc,argv); 

	/* 옵션 처리 */

	opterr = 0; // 에러는 출력하지 않음.

	while((option = getopt(argc,argv,"jcpflr")) != EOF)
	{
		switch(option) 
		{
			case 'j' :
				opt.opt_j = optind - 1;
				break;
			case 'c' :
				opt.opt_c = optind - 1;
				break;
			case 'p' :
				opt.opt_p = optind - 1;
				break;
			case 'f' :
				opt.opt_f = optind - 1;
				break;
			case 'l' :
				opt.opt_l = optind - 1;
				break;
			case 'r' :
				opt.opt_r = optind - 1;
				break;
			case '?' :
				fprintf(stderr,"Invalid option\n");
				exit(1);
		}
	}

	/* 본격적인 변환을 시작하는 함수 */

	convert(opt); 
	
	gettimeofday(&end,NULL); // 시간 측정 끝
	printf("\n");
	ssu_runtime(&begin,&end); // 시간 출력
	exit(0);
}
