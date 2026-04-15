#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>

#define WEB_SUCCESS 0
#define WEB_ERROR -1
#define MAXLEN 1024


char* getcgidata(FILE* fp, char* requestmethod)
{
	char* input;
	int len;
	int size = MAXLEN;
	int i = 0;

	if (!strcmp(requestmethod, "GET"))
	{
		input = getenv("QUERY_STRING");
		return input;
	}
	else if (!strcmp(requestmethod, "POST"))
	{
		len = atoi(getenv("CONTENT_LENGTH"));
		input = (char*)malloc(sizeof(char)*(size + 1));

		if (len == 0)
		{
			input[0] = '\0';
			return input;
		}

		while(1)
		{
			input[i] = (char)fgetc(fp);
			if (i == size)
			{
				input[i+1] = '\0';
				return input;
			}

			--len;
			if (feof(fp) || (!(len)))
			{
				i++;
				input[i] = '\0';
				return input;
			}
			i++;

		}
	}
	return NULL;
}

int main (int argc , char *argv[])
{
	int ret = WEB_SUCCESS;
	char* cgistr = NULL;
	char* req_method = NULL;
	char user[20];
	char pwd[20];
	//printf("Content-type: text/html;charset='utf-8'\n\n");
	req_method = getenv("REQUEST_METHOD");
	cgistr = getcgidata(stdin, req_method);
	sscanf(cgistr,"name=%[^&]&pwd=%s",user,pwd);

	printf("11112:%s\n",cgistr);
	printf("name:%s---------pwd:%s\n",user,pwd);
	int login = 1;
	if(strcmp(user,"admin")==0 && strcmp(pwd,"admin")==0)
	{
		login = 1;
		printf("Set-Cookie: isLogin=%d;path=/; \r\n",login);
		printf("Content-type: text/html;charset='utf-8'\n\n");
		printf("<script>window.location.href = '../settingMain.html'</script>\n");
	}
	else
	{
		login = 0;
		printf("Set-Cookie: isLogin=%d;path=/; \r\n",login);
		printf("Content-type: text/html;charset='utf-8'\n\n");
		printf("<script>window.location.href = '../index.html'</script>\n");
	}
	return ret;
}
