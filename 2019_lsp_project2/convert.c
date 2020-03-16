#include "convert.h"

void convert(Optind opt)
{
	int i = 0,prt;
	FILE * c_file; // c 파일 
	FILE * java_file; // java 파일
	int java_line,c_line;
	size_t java_size,c_size;
	char string[STR_SIZE]; // java 파일 읽기 위한 문자열
	struct stat statbuf_c,statbuf_java;
	pid_t pid,child;

	memset((char *)string,0,STR_SIZE);

	if((java_file = fopen(fname_java,"r")) == NULL)// java 파일 읽기
	{
		fprintf(stderr,"%s open error\n", fname_java);
		exit(1);
	}
	if((c_file = fopen(fname_c,"w+")) == NULL)
	{
		fprintf(stderr,"%s open error\n", fname_c);
		exit(1);
	}

	header_check(java_file,c_file);
	fclose(c_file);
	java_line = count_lines(java_file);

	/* 변환 수행 */
	
	if(opt.opt_r)
	{
		pid = fork();

		if(pid == 0)
		{
			convert_java(java_file,pid);
		}
		else
		{
			wait(NULL);
			exit(1);
		}
	}
	pid = -1;
	if(!opt.opt_r)
		convert_java(java_file,pid);
	make_makefile();
	printf("\n%s convert Success!\n\n", fname_c);

	if((c_file = fopen(fname_c,"r")) == NULL)
	{
		fprintf(stderr,"fopen error for %s\n", fname_c);
		exit(1);
	}
	
	if(opt.opt_j) // print java code
	{
		printf("<< J option : %s >>\n\n", fname_java);
		code_print(java_file);
	}

	if(opt.opt_c) // print c code
	{
		printf("<< C option : %s >>\n\n", fname_c);
		code_print(c_file);
		for(i = 0; i < FILE_MAX; i++)
		{
			if(fname_append[i][0] == '\0')
				break;
			else
			{
				FILE * file;
				if((file = fopen(fname_append[i],"r")) == NULL)
				{
					fprintf(stderr,"fopen error for %s\n", fname_append[i]);
					exit(1);
				}
				printf("<< C option : %s >>\n\n", fname_append[i]);
				code_print(file);
				fclose(file);
			}
		}
	}

	if(opt.opt_f) // the size of java or c code
	{
		if(stat(fname_c,&statbuf_c) < 0)
		{
			fprintf(stderr,"stat error for %s\n", fname_c);
			exit(1);
		}
		if(stat(fname_java,&statbuf_java) < 0) // java파일 정보 얻기
		{
			fprintf(stderr,"stat error for %s\n", fname_java);
			exit(1);
		}
		c_size = statbuf_c.st_size;
		java_size = statbuf_java.st_size; // java파일 크기 측정
		printf("%s file size is %ld bytes.\n", fname_java, java_size);
		printf("%s file size is %ld bytes.\n", fname_c, c_size);
		for(i = 0; i < FILE_MAX; i++)
		{
			if(fname_append[i][0] != '\0')
			{
				struct stat statbuf_temp;
				if(stat(fname_append[i],&statbuf_temp) < 0)
				{
					fprintf(stderr,"stat error for %s\n", fname_append[i]);
					exit(1);
				}
				size_t temp_size = statbuf_temp.st_size;
				printf("%s file size is %ld bytes.\n\n", fname_append[i], temp_size);
			}
			else
				break;
		}
	}

	if(opt.opt_l)
	{
		c_line = count_lines(c_file);
		printf("%s has %d lines.\n", fname_java, java_line);
		printf("%s has %d lines.\n", fname_c, c_line);
		for(i = 0; i < FILE_MAX; i++)
		{
			if(fname_append[i][0] != '\0')
			{
				FILE * temp;
				if((temp = fopen(fname_append[i],"r")) == NULL)
				{
					fprintf(stderr,"fopen error for %s\n", fname_append[i]);
					exit(1);
				}
				int line = count_lines(temp);
				printf("%s has %d lines.\n", fname_append[i], line);
			}
		}
	}

	if(opt.opt_p) // print converted syntax
	{
		prt = 1;
		printf("\n<< P option : Converted List >>\n\n");

		for(i = 0; i < FUNC_MAX; i++)
		{
			int j = 0;
			if(fconvert[i][0] == '\0')
				break;
			while(fconvert[i][j] != ' ')
				j++;
			fconvert[i][j] = '-';
			fconvert[i][j+1] = '>';

			printf("%2d %s\n", prt++, fconvert[i]);
		}
	}

	/* 모든 작업 수행 후 나머지 작업 수행 */

	fclose(java_file);
	fclose(c_file);
}

void header_check(FILE * jfile,FILE * cfile)
{
	char * ptr;
	char name[NAME_SIZE];
	char string[STR_SIZE];
	char cprt[NAME_SIZE][STR_SIZE];
	int i = 0,j,count = 0,index = 0;

	for(i = 0; i < NAME_SIZE; i++)
		memset((char *)cprt[i],0,STR_SIZE);

	fseek(jfile,0,SEEK_SET);
	fseek(cfile,0,SEEK_SET);
	memset((char *)string,0,STR_SIZE);

	while(fgets(string,STR_SIZE,jfile) != NULL) // 한 줄씩 읽기
	{
		string[strlen(string)-1] = '\0'; // 마지막은 널문자 처리

		if((ptr = strstr(string,"class ")) != NULL)
		{
			FILE * fp;
		
			if(!judge_class(string))
			{
				j = 0;
				for(i = 0; fname_append[i][0] != '\0'; i++);
				ptr += strlen("class ");
				memset((char *)name,0,NAME_SIZE);
				name[j] = *(ptr);
				j++; ptr++;
				while(*(ptr) != ';' && *(ptr) != '{')
				{
					name[j] = *(ptr);
					j++; ptr++;
				}
				sprintf(name,"%s.c",name);
				strcpy(fname_append[i],name);
				if((fp = fopen(fname_append[i],"w+")) == NULL)
				{
					fprintf(stderr,"fopen error for %s\n", name);
					exit(1);
				}
				else
				{
					if(strstr(string,"{") != NULL)
						count = 1;
					else
					{
						fgets(string,STR_SIZE,jfile);
						while(strstr(string,"{") == NULL);
							fgets(string,STR_SIZE,jfile);
						if(strstr(string,"{") != NULL)
							count = 1;
					}
					while(fgets(string,STR_SIZE,jfile) != NULL)
					{
						string[strlen(string)-1] = 0;
						if(strstr(string,"{") != NULL)
							count++;
						if(strstr(string,"}") != NULL)
						{
							count--;
							if(!count)
								break;
						}
						else if(strstr(string,"new ") != NULL) 
							header_write(fp,"exit");
						else if(strstr(string,"System.out.print") != NULL) 
							header_write(fp,"printf");
						else if(strstr(string,"Scanner") != NULL) 
							header_write(fp,"scanf");
						else if(strstr(string,"FileWriter") != NULL) 
							header_write(fp,"write");
						else if(strstr(string,"FileReader") != NULL) 
							header_write(fp,"read");
						else if(strstr(string,"File") != NULL) 
							header_write(fp,"open");
						else if(strstr(string,"final ") != NULL)
						{
							char * ptr;
							char temp[STR_SIZE];
							char expr[STR_SIZE];

							memset((char *)temp,0,STR_SIZE);
							memset((char *)expr,0,STR_SIZE);
	
							if((ptr = strstr(string,"int ")) != NULL)
							{
								ptr += strlen("int ");
								i = 0;
								while(*(ptr) != ';' && *(ptr) != '=')
								{
									temp[i] = *(ptr);
									i++; ptr++;
								}
								sprintf(cprt[index],"#define %s", temp);
								if(*(ptr) == '=')
								{
									ptr++;
									i = 0;
									while(*(ptr) != '\0' && *(ptr) != ';')
									{
										expr[i] = *(ptr);
										ptr++; i++;
									}
									expr[i] = '\n';
									sprintf(cprt[index],"%s%s",cprt[index],expr);
									index++;
								}
							}
						}//else if "final "
					}
				}
				fprintf(fp,"\n");
				for(i = 0; cprt[i][0] != '\0'; i++)
					fprintf(fp,"%s", cprt[i]);
				fclose(fp);
			}
		}
		if(strstr(string,"new ") != NULL)
		{
			header_write(cfile,"exit");
		}
		if(strstr(string,"System.out.") != NULL) // printf
		{
			if(strstr(string,"printf") != NULL)
				header_write(cfile,"printf");
		}
		if(strstr(string,"Scanner") != NULL) // scanf
		{
			if(strstr(string,"import java.util.Scanner") != NULL)
				header_write(cfile,"scanf");
		}
		if(strstr(string,"FileWriter") != NULL) // write
		{
			header_write(cfile,"write");
		}
		if(strstr(string,"String") != NULL) // string
		{
			header_write(cfile,"string");
		}
		if(strstr(string,"FileReader") != NULL) // read
		{
			header_write(cfile,"read");
		}
		if(strstr(string,"File") != NULL) // open
		{
			if(strstr(string,"import java.io.File") != NULL)
				header_write(cfile,"open");
		}

		memset((char *)string,0,STR_SIZE);
	}
	fseek(jfile,0,SEEK_SET);
}

void header_write(FILE * cfile,char * str) // 파일에 헤더파일 쓰기
{
	int i,index;
	off_t offset;

	if((index = header_index(str)) == -1)
		return;
	else
	{
		char temp[NAME_SIZE];

		for(i = 0; i < strlen(header_array[index]); i++)
		{
			if(header_array[index][i] == '#')
			{
				parsing_head(header_array[index],temp,i);
				if(!header_exist(cfile,temp))
				{
					fseek(cfile,0,SEEK_END);
					fprintf(cfile,"%s\n", temp);
					fflush(cfile);
				}
			}
		}
	}
}

int header_index(char * str) // header_array 인덱스 번호 리턴
{
	int i;

	for(i = 0; i < HEADER_INDEX; i++)
	{
		if(strstr(header_array[i],str) != NULL)
			return i;
	}
	return -1; // 없는 경우
}

void parsing_head(char * str,char * des,int index) // 파싱
{
	int i;
	memset((char *)des,0,NAME_SIZE);

	for(i = index; str[i] != '>' && str[i] != '\0'; i++)
		des[i-index] = str[i];
	if(str[i] == '>')
		des[i-index] = str[i];
}

bool header_exist(FILE * cfile,char * str) // c file에 이미 헤더를 넣었는가?
{
	char string[STR_SIZE];
	memset((char *)string,0,STR_SIZE);
	
	fseek(cfile,0,SEEK_SET);
	while(fgets(string,STR_SIZE,cfile) != NULL)
	{
		string[strlen(string)-1] = '\0';
		if(strstr(string,str) != NULL)
			return TRUE; // it exists
		memset((char *)string,0,STR_SIZE);
	}
	fseek(cfile,0,SEEK_SET);
	return FALSE; // it does not exists
}

void convert_java(FILE * jfile,pid_t pid) // 변경
{
	FILE * cfile;
	char * ptr;
	char jsave[FILE_SIZE];
	char csave[FILE_SIZE];
	char cstr[STR_SIZE];
	char jstr[STR_SIZE];
	char ctab[STR_SIZE];
	int i,count = 0;
	int jline,cline;
	bool check;

	memset((char *)jstr,0,STR_SIZE);
	memset((char *)cstr,0,STR_SIZE);
	memset((char *)jsave,0,FILE_SIZE);
	memset((char *)csave,0,FILE_SIZE);

	jline = cline = 1;
	fseek(jfile,0,SEEK_SET);

	strcat(jsave,"----------\n");
	sprintf(jstr,"%s\n", fname_java);
	strcat(jsave,jstr);
	strcat(jsave,"----------\n");
	memset((char *)jstr,0,STR_SIZE);

	while(fgets(jstr,STR_SIZE,jfile) != NULL)
	{
		char temp[STR_SIZE];
		memset((char *)temp,0,STR_SIZE);
		sprintf(temp,"%2d %s", jline++, jstr);
		strcat(jsave,temp);
		jstr[strlen(jstr)-1] = 0;

		if(pid == 0)
		{
			system("clear");
			printf("%s", jsave);
			printf("%s", csave);
			sleep(1);
		}

		if((ptr = strstr(jstr,"class ")) != NULL)
		{
			if(judge_class(jstr)) // 메인 클래스
			{
				cline = 1;
				if((cfile = fopen(fname_c,"a+")) == NULL)
				{
					fprintf(stderr,"fopen error for %s\n", fname_c);
					exit(1);
				}
				else
				{
					memset((char *)csave,0,FILE_SIZE);
					strcat(csave,"----------\n");
					sprintf(cstr,"%s\n", fname_c);
					strcat(csave,cstr);
					strcat(csave,"----------\n");
					memset((char *)cstr,0,STR_SIZE);

					while(fgets(cstr,STR_SIZE,cfile) != NULL)
					{
						char tmp[STR_SIZE];
						memset((char *)tmp,0,STR_SIZE);
						sprintf(tmp,"%2d %s", cline++, cstr);
						strcat(csave,tmp);
					}
					bool start = TRUE;
					fseek(cfile,0,SEEK_END);
					fprintf(cfile,"\n");
					fflush(cfile);
					
					memset((char *)jstr,0,STR_SIZE);
					memset((char *)cstr,0,STR_SIZE);
					memset((char *)ctab,0,STR_SIZE);
					count = 0;	

					while(fgets(jstr,STR_SIZE,jfile) != NULL)
					{
						memset((char *)temp,0,STR_SIZE);
						sprintf(temp,"%2d %s", jline++, jstr);
						strcat(jsave,temp);
						jstr[strlen(jstr)-1] = 0;
						check = make_cfile(jstr,cstr,&count);

						if(strstr(cstr,"{") != NULL)
						{
							for(i = 0; i < count -1; i++)
								strcat(ctab,"\t");
							strcat(ctab,cstr);
							fprintf(cfile,"%s",ctab);
						}
						else
						{
							for(i = 0; i < count; i++)
								strcat(ctab,"\t");
							strcat(ctab,cstr);
							fprintf(cfile,"%s", ctab);
						}
						
						memset((char *)temp,0,STR_SIZE);
						if(strstr(ctab,"\n") != NULL)
							sprintf(temp,"%2d %s",cline++, ctab);
						else
							sprintf(temp,"%2d %s\n", cline++, ctab);
						strcat(csave,temp);

						if(pid == 0)
						{
							system("clear");
							printf("%s", jsave);
							printf("%s", csave);
							sleep(1);
						}


						if(!start && !count)
							break;
						if(start == TRUE)
							start = FALSE;
						
						memset((char *)jstr,0,STR_SIZE);
						memset((char *)cstr,0,STR_SIZE);
						memset((char *)ctab,0,STR_SIZE);
						fflush(cfile);
					}
					count = 0;
					fclose(cfile);
				}
			}
			else // 부속 클래스
			{
				cline = 1;
				int i = 0;
				char fname_tmp[NAME_SIZE];

				memset((char *)fname_tmp,0,NAME_SIZE);
				if((ptr = strstr(jstr,"class")) != NULL)
					ptr += strlen("class ");
				fname_tmp[i] = *(ptr);
				ptr++; i++;
				while(*(ptr) != '{' && *(ptr) != '\0')
				{
					fname_tmp[i] = *(ptr);
					ptr++; i++;
				}
				sprintf(fname_tmp,"%s.c", fname_tmp);
				for(i = 0; i < FILE_MAX; i++)
				{
					if(!strcmp(fname_append[i],fname_tmp))
						break;
				}
				if((cfile = fopen(fname_tmp,"a+")) == NULL)
				{
					fprintf(stderr,"fopen error for %s\n", fname_tmp);
					exit(1);
				}
				else
				{
					memset((char *)csave,0,FILE_SIZE);
					strcat(csave,"----------\n");
					sprintf(cstr,"%s\n", fname_tmp);
					strcat(csave,cstr);
					strcat(csave,"----------\n");
					memset((char *)cstr,0,STR_SIZE);

					while(fgets(cstr,STR_SIZE,cfile) != NULL)
					{
						char tmp[STR_SIZE];
						memset((char *)tmp,0,STR_SIZE);
						sprintf(tmp,"%2d %s", cline++, cstr);
						strcat(csave,tmp);
					}
					memset((char *)cstr,0,STR_SIZE);
					bool start = TRUE;
					fseek(cfile,0,SEEK_END);
					fprintf(cfile,"\n");
					fflush(cfile);
					if(strstr(jstr,"{") != NULL)
						count = 1;
					else
					{
						fgets(jstr,STR_SIZE,jfile);
						while(strstr(jstr,"{") == NULL)
							fgets(jstr,STR_SIZE,jfile);
						count = 1;
					}
					memset((char *)jstr,0,STR_SIZE);
					memset((char *)cstr,0,STR_SIZE);
					memset((char *)ctab,0,STR_SIZE);
				
					while(fgets(jstr,STR_SIZE,jfile) != NULL)
					{
						memset((char *)temp,0,STR_SIZE);
						sprintf(temp,"%2d %s", jline++, jstr);
						strcat(jsave,temp);
						jstr[strlen(jstr)-1] = 0;
						check = make_cfile(jstr,cstr,&count);
						if(!start && count == 0)
							break;
						
						if(strstr(cstr,"{") != NULL)
						{
							for(i = 0; i < count -2; i++)
								strcat(ctab,"\t");
							strcat(ctab,cstr);
							fprintf(cfile,"%s",ctab);
						}
						else
						{
							for(i = 0; i < count-1; i++)
								strcat(ctab,"\t");
							strcat(ctab,cstr);
							fprintf(cfile,"%s", ctab);
						}

						memset((char *)temp,0,STR_SIZE);
						if(strstr(ctab,"\n") != NULL)
							sprintf(temp,"%2d %s", cline++, ctab);
						else
							sprintf(temp,"%2d %s\n", cline++, ctab);
						strcat(csave,temp);

						if(pid == 0)
						{
							system("clear");
							printf("%s", jsave);
							printf("%s", csave);
							sleep(1);
						}

						if(start == TRUE)
							start = FALSE;

						memset((char *)jstr,0,STR_SIZE);
						memset((char *)cstr,0,STR_SIZE);
						memset((char *)ctab,0,STR_SIZE);
						fflush(cfile);
					}
					count = 0;
					fclose(cfile);
				}
			}
		}
		memset((char *)jstr,0,STR_SIZE);
	}
}

bool judge_class(char * jstr)
{
	if(strstr(jstr,fname) != NULL)
		return TRUE;
	else
		return FALSE;
}

bool make_cfile(char * jstr,char * cstr,int * num)
{
	int i,j;
	int count = 0,ind;
	bool check = FALSE;

	remove_tab(jstr);
	
	if(strstr(jstr,"{") != NULL)
	{
		sprintf(cstr,"{\n");
		(*num)++;
		check = TRUE;
	}
	if(strstr(jstr,"}") != NULL)
	{
		char temp[STR_SIZE];

		memset((char *)temp,0,STR_SIZE);
		count = 0;

		for(i = 0; i < strlen(jstr); i++)
		{
			if(jstr[i] == '}')
			{
				count++;
				strcat(temp,"}");
			}
		}
		sprintf(cstr,"%s\n", temp);
	//	sprintf(cstr,"}\n");
		(*num) -= count;
	}
	if(strstr(jstr,"public static void main") != NULL)
	{
		if(strstr(jstr,"{") != NULL)
			sprintf(cstr,"int main(int argc,char * argv[]){\n");
		else
			sprintf(cstr,"int main(int argc,char * argv[])\n");
	}
	else if(strstr(jstr,"System.out.printf") != NULL)
	{
		char * ptr;
		char temp[STR_SIZE];

		memset((char *)temp,0,STR_SIZE);
		
		for(i = 0; i < strlen(jstr); i++)
		{
			if(jstr[i] == '"')
			{
				temp[0] = jstr[i];

				for(j = i+1; jstr[j] != ';'; j++)
					temp[j-i] = jstr[j];
				if((ptr = strstr(temp,".")) != NULL)
				{
					while(*(ptr) != ' ' && *(ptr) != ',' && *(ptr) != '\"')
					{
						*(ptr) = ' ';
						ptr--;
					}
				}
				sprintf(cstr,"printf(%s;\n",temp); 
				break;
			}
		}
		if((ind = isConverted("System.out.printf")) != -1)
		{
			strcat(fconvert[ind],"System.out.printf()  ");
			strcat(fconvert[ind],"printf()");
		}
	}
	else if(strstr(jstr,"if") != NULL)
	{
		if(strstr(jstr,"!= null") == NULL)
			sprintf(cstr,"%s\n", jstr);
		else
		{
			sprintf(cstr,"\n");
		}
	}
	else if(strstr(jstr,"else") != NULL)
	{
		sprintf(cstr,"%s\n", jstr);
	}
	else if(strstr(jstr,"new Scanner") != NULL)
	{
		memset((char *)cstr,0,STR_SIZE);
	}
	else if(strstr(jstr,"final ") != NULL)
	{
		memset((char *)cstr,0,STR_SIZE);
	}
	else if(strstr(jstr,"public ") != NULL)
	{
		char * ptr;
		char name[NAME_SIZE];

		memset((char *)name,0,NAME_SIZE);

		if((ptr = strstr(jstr,"public ")) != NULL)
		{
			ptr += strlen("public ");
			while(*(ptr) == ' ' && *(ptr) == '\0') { ptr++; }
			i = 0;
			while(*(ptr) != '\0' && *(ptr) != '\n')
			{
				name[i] = *(ptr);
				i++; ptr++;
			}
			if(strstr(jstr,"int") == NULL && strstr(jstr,"void") == NULL) // 생성자
			{
				sprintf(cstr,"void %s\n", name);
			}
			else
			{
				sprintf(cstr,"%s\n", name);
			}
		}
	}
	else if(strstr(jstr,"int ") != NULL || strstr(jstr,"int[") != NULL)
	{
		char * ptr;
		char size[NAME_SIZE];
		char name[NAME_SIZE];

		memset((char *)size,0,NAME_SIZE);
		memset((char *)name,0,NAME_SIZE);

		if((ptr = strstr(jstr,"[")) != NULL)
		{
			ptr++;

			if(*(ptr) == ']') // 선언임을 알 수 있음
			{
				ptr++;

				while(*(ptr++) != ' ' && *(ptr++) != '\0');
				if(*(ptr) != '\0' && *(ptr) != ';')
				{
					i = 0;
					name[i] = *(ptr);
					i++; ptr++;
					while(*(ptr) != ';' && *(ptr) != '\0')
					{
						name[i] = *(ptr);
						i++; ptr++;
					}
					sprintf(cstr,"int * %s;\n", name);
				}
			}
			else
			{
				if(strstr(jstr,"new ") != NULL)
				{
					for(i = 0; jstr[i] != ' ' &&  jstr[i] != '='; i++)
						name[i] = jstr[i];
					if((ptr = strstr(jstr,"[")) != NULL)
					{
						ptr++;
						i = 0;
						while(*(ptr) != ']' && *(ptr) != ';' && *(ptr) != '\0')
						{
							size[i] = *(ptr);
							ptr++; i++;
						}
						sprintf(cstr,"%s = (int *)calloc(%s,sizeof(int));\n", name, size);
					}
				}
			}
		}
		else
		{
			sprintf(cstr,"%s\n",jstr);
		}
	}
	else if(strstr(jstr,"double") != NULL)
	{
	}
	else if(strstr(jstr,"float") != NULL)
	{
	}
	else if(strstr(jstr,"short") != NULL)
	{
	}
	else if(strstr(jstr,".nextInt()") != NULL)
	{
		char temp[NAME_SIZE];
		char expr[STR_SIZE];
		memset((char *)temp,0,NAME_SIZE);
		memset((char *)expr,0,STR_SIZE);
		for(i = 0; jstr[i] != ' '; i++)
			temp[i] = jstr[i];
		strcpy(expr,"\%d");
		sprintf(cstr,"scanf(\"%s\", &%s);\n", expr, temp);

		if((ind = isConverted("nextInt()")) != -1)
		{
			strcat(fconvert[ind],"nextInt()  ");
			strcat(fconvert[ind],"scanf()");
		}
	}
	else if(strstr(jstr,"File ") != NULL)
	{
		char * ptr;
		char expr[STR_SIZE];
		char temp[STR_SIZE];

		memset((char *)temp,0,STR_SIZE);
		memset((char *)expr,0,STR_SIZE);

		for(i = 0; i < strlen(jstr); i++)
		{
			temp[i] = jstr[i];
			if(!strcmp(temp,"File"))
				break;
		}
		memset((char *)temp,0,STR_SIZE);
		j = i+1;
		for(i = j; jstr[i] != '='; i++) { temp[i-j] = jstr[i]; }

		for(i = 0; i < strlen(temp); i++)
		{
			if(temp[i] == ' ')
			{
				for(j = i; j < strlen(temp); j++)
					temp[j] = temp[j+1];
				i--;
			}
		}
		if((ptr = strstr(jstr,"(")) != NULL)
		{
			i = 1;
			char ch = *(ptr+i);
			expr[i-1] = ch;
			i++;
			while(ch != ')')
			{
				ch = *(ptr+i);
				if(ch == ')')
					break;
				expr[i-1] = ch;
				i++;
			}
			sprintf(cstr,"char * %s = %s;\n",temp, expr);
		}
	}
	else if(strstr(jstr,"FileWriter") != NULL)
	{
		char * ptr;
		char temp[STR_SIZE];
		char expr[STR_SIZE];
		char ctab[STR_SIZE];

		memset((char *)temp,0,STR_SIZE);
		memset((char *)expr,0,STR_SIZE);
		memset((char *)ctab,0,STR_SIZE);

		for(i = 0; i < *(num); i++)
			strcat(ctab,"\t");

		for(i = 0; i < strlen(jstr); i++)
		{
			temp[i] = jstr[i];
			if(!strcmp(temp,"FileWriter "))
				break;
		}
		memset((char *)temp,0,STR_SIZE);
		j = i+1;
		for(i = j; jstr[i] != '='; i++) { temp[i-j] = jstr[i]; }

		for(i = 0; i < strlen(temp); i++)
		{
			if(temp[i] == ' ')
			{
				for(j = i; j < strlen(temp); j++)
					temp[j] = temp[j+1];
				i--;
			}
		}
		sprintf(cstr,"%sint %s;\n\n", ctab,temp);
		if((ptr = strstr(jstr,"(")) != NULL)
		{
			i = 1;
			char ch = *(ptr+i);
			expr[i-1] = ch;
			i++;
			while(ch != ')')
			{
				ch = *(ptr+i);
				if(ch == ')')
					break;
				expr[i-1] = ch;
				i++;
			}
			if((ptr = strstr(expr,"true")) != NULL)
			{
				*ptr = 0;
				strcat(expr,"O_APPEND | O_WRONLY");
			}
			else if((ptr = strstr(expr,"false")) != NULL)
			{
				*ptr = 0;
				strcat(expr,"O_CREAT | O_TRUNC | O_WRONLY, 0666");
			}
			sprintf(cstr,"%s%sif((%s = open(%s)) < 0)\n%s{\n\t%sfprintf(stderr,\"open error!\\n\");\n\t%sreturn 1;\n\t}\n", cstr,ctab,temp,expr,ctab,ctab,ctab);
		}

		if((ind = isConverted("FileWriter()")) != -1)
		{
			strcat(fconvert[ind],"FileWriter()  ");
			strcat(fconvert[ind],"open()");
		}
	}
	else if(strstr(jstr,"write(") != NULL)
	{
		char * ptr;
		char temp[STR_SIZE];
		char expr[STR_SIZE];

		memset((char *)temp,0,STR_SIZE);
		memset((char *)expr,0,STR_SIZE);

		i = 0;
		remove_tab(jstr);
		for(i = 0; jstr[i] != '.'; i++)
			temp[i] = jstr[i];
		if((ptr = strstr(jstr,"\"")) != NULL)
		{
			i = 0;
			char ch = *(ptr+i);
			expr[i] = ch;
			i++;
			while(ch != ')')
			{
				ch = *(ptr+i);
				if(ch == ')')
					break;
				expr[i] = ch;
				i++;
			}
			sprintf(cstr,"write(%s,%s,strlen(%s));\n",temp,expr,expr);
		}
		if((ind = isConverted("write()")) != -1)
		{
			strcat(fconvert[ind],"write()  ");
			strcat(fconvert[ind],"write()");
		}
	}
	else if(strstr(jstr,"flush()") != NULL)
	{
		char * ptr;
		char temp[STR_SIZE];

		memset((char *)temp,0,STR_SIZE);
		remove_tab(jstr);
		for(i = 0; jstr[i] != '.'; i++)
			temp[i] = jstr[i];
		sprintf(cstr,"fsync(%s);\n", temp);
		if((ind = isConverted("flush()")) != -1)
		{
			strcat(fconvert[ind],"flush()  ");
			strcat(fconvert[ind],"fsync()");
		}
	}
	else if(strstr(jstr,"return") != NULL) // 리턴 뒤에 있는 문자 검사
	{
		char temp[NAME_SIZE];
		memset((char *)temp,0,NAME_SIZE);

		i = strlen("return");
		while(jstr[i] == ' ' && jstr[i] != ';') { i++; }

		if(jstr[i] == ';')
		{
			sprintf(cstr,"return 0;\n");
		}
		else
		{
			memset((char *)temp,0,NAME_SIZE);

			for(j = i; jstr[j] != ';'; j++)
				temp[j-i] = jstr[j];
			if(*temp == '\0')
					sprintf(cstr,"return 0;\n");
			else
				sprintf(cstr,"return %s;\n", temp);
		}
	}
	else if(strstr(jstr,".close") != NULL)
	{
		char temp[STR_SIZE];
		char expr[STR_SIZE];

		memset((char *)temp,0,STR_SIZE);
		memset((char *)expr,0,STR_SIZE);
		remove_tab(jstr);

		for(i = 0; jstr[i] != '.'; i++)
			temp[i] = jstr[i];
		for(j = i+1; jstr[j] != '('; j++)
			expr[j-i-1] = jstr[j];

		sprintf(cstr,"%s(%s);\n", expr, temp);
		if((ind = isConverted("close()")) != -1)
		{
			strcat(fconvert[ind],"close()  ");
			strcat(fconvert[ind],"close()");
		}
	}
	else if(strstr(jstr,".") != NULL) // 참조 연산자를 만나면
	{
		char * ptr;
		char temp[STR_SIZE];

		memset((char *)temp,0,STR_SIZE);
		ptr = strstr(jstr,".");
		ptr++; i = 0;
		while(*(ptr) != ';' && *(ptr) != '\0')
		{
			temp[i] = *(ptr);
			ptr++; i++;
		}
		if(strstr(temp,"\"") != NULL);
		else if(strstr(temp,"(") != NULL && strstr(temp,")") != NULL)
		{
			memset((char *)cstr,0,STR_SIZE);
			sprintf(cstr,"%s;\n", temp);
		}
		else
			sprintf(cstr,"%s\n", jstr);
	}
	else if(strstr(jstr,"new ") != NULL)
	{
		char * ptr;
		char temp[STR_SIZE];

		memset((char *)temp,0,STR_SIZE);

		if((ptr = strstr(jstr,"new ")) != NULL)
		{
			ptr += strlen("new ");
			i = 0;
			while(*(ptr) != ';' && *(ptr) != '\0')
			{
				if(*(ptr) == ' ')
				{
					ptr++;
					continue;
				}
				temp[i] = *(ptr);
				ptr++; i++;
			}
			sprintf(cstr,"%s;\n", temp);
		}
	}
	else
	{
		sprintf(cstr,"%s\n", jstr);
	}
	if(strstr(jstr,"}") != NULL)
	{
		if(strcmp(cstr,"}\n"))
		{
			if(strstr(cstr,"}") == NULL)
			{
				cstr[strlen(cstr)-1] = '}';
				cstr[strlen(cstr)] = '\n';
				cstr[strlen(cstr)] = 0;
			}
		}
	}
	return check;
}

void make_makefile(void)
{
	int i,j;
	FILE * fp;
	char * ptr;
	char main_object[NAME_SIZE];
	char sub_object[FILE_MAX][NAME_SIZE];
	char expr[STR_SIZE];
	char compile[STR_SIZE];

	memset((char *)main_object,0,NAME_SIZE);

	for(i = 0; i < FILE_MAX; i++)
		memset((char *)sub_object[i],0,NAME_SIZE);

	if((fp = fopen(fname_makefile,"w+")) == NULL)
	{
		fprintf(stderr,"fopen error for %s\n", fname_makefile);
		exit(1);
	}

	sprintf(main_object,"%s.o", fname);

	for(i = 0; i < FILE_MAX; i++)
	{
		if(fname_append[i][0] != '\0')
		{
			strncpy(sub_object[i],fname_append[i],strlen(fname_append[i])-1);
			strcat(sub_object[i],"o");
		}
		else 
			break;
	}
	memset((char *)expr,0,STR_SIZE);
	memset((char *)compile,0,STR_SIZE);
	sprintf(expr,"%s : %s", fname, main_object);
	sprintf(compile,"\tgcc -o %s %s", fname, main_object);

	for(i = 0; i < FILE_MAX; i++)
	{
		if(*(sub_object[i]) != '\0')
		{
			strcat(expr," "); strcat(compile," ");
			strcat(expr,sub_object[i]); strcat(compile,sub_object[i]);
		}
		else 
			break;
	}
	fprintf(fp,"%s\n", expr);
	fprintf(fp,"%s\n\n", compile);

	fprintf(fp,"%s : %s\n", main_object, fname_c);
	fprintf(fp,"\tgcc -c -o %s %s\n\n", main_object, fname_c);

	for(i = 0; i < FILE_MAX; i++)
	{
		if(*(sub_object[i]) != '\0')
		{
			fprintf(fp,"%s : %s\n", sub_object[i], fname_append[i]);
			fprintf(fp,"\tgcc -c -o %s %s\n\n", sub_object[i], fname_append[i]);
		}
		else
			break;
	}

	fflush(fp);
	fclose(fp);
}

int isConverted(char * str)
{
	int i = 0;

	for(i = 0; i < FUNC_MAX; i++)
	{
		if(fconvert[i][0] != '\0')
		{
			if(!strncmp(fconvert[i],str,strlen(str))) // 이미 존재함
				return -1;
		}
		else
			return i;
	}
	return -1;
}

void remove_tab(char * jstr)
{
	int i = 0,j;

	while(i < strlen(jstr))
	{
		if(jstr[i] == '\t')
		{
			for(j = i; j < strlen(jstr); j++)
				jstr[j] = jstr[j+1];
			i--;
		}
		i++;
	}
}

int count_lines(FILE * file)
{
	int count = 0;
	char string[STR_SIZE];

	memset((char *)string,0,STR_SIZE);
	fseek(file,0,SEEK_SET);
	while(fgets(string,STR_SIZE,file) != NULL)
		count++;
	fseek(file,0,SEEK_SET);
	return count;
}

void code_print(FILE * file) // 코드 출력
{
	int count = 1;
	char string[STR_SIZE];

	fseek(file,0,SEEK_SET);
	memset((char *)string,0,STR_SIZE);
	while(fgets(string,STR_SIZE,file) != NULL)
		printf("%2d %s", count++, string);
	fseek(file,0,SEEK_SET);
	printf("\n");
}
