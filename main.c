#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "http.h"
#include "cJSON.h"
#include "cJSON_Utils.h"

#define TRUE 255
#define FALSE 0

typedef int bool;

char test_question[] = "\
{\r\n\
  \"model\": \"mistral\",\r\n\
  \"prompt\": \"Why is the sky blue?\",\r\n\
  \"stream\": false\r\n\
}";

// to change ???
char ollama_ip[] = "127.0.0.1:11434";
char* model_name = "mistral";
double temp = 1.2;

int send_post_request_to_ai(int sd, struct http_url* url, char* prompt,char* context) {
	char buf[4096];
  char json[2048];

  snprintf(json, sizeof(json),
  "\
  {\r\n\
    \"model\": \"%s\",\r\n\
    \"prompt\": \"%s\",\r\n\
    \"stream\": false,\r\n\
    \"options\": {\r\n\
     \"temperature\": %f\r\n\
   },\r\n\
  }\r\n\
  ", model_name, prompt, temp);

	snprintf(buf, sizeof(buf),
		"\
POST /api/generate HTTP/1.1\r\n\
Host: %s\r\n\
User-Agent: curl/7.68.0\r\n\
Accept: */*\r\n\
Content-Length: %lu\r\n\
Content-Type: application/x-www-form-urlencoded\r\n\
\r\n\
%s\r\n\
\r\n\
", ollama_ip, strlen(json), json);

  puts(buf);

	if (http_send(sd, buf)) {
		perror("http_send");
		return -1;
	}

	return 0;
}

// returns context
char* speak_to_ollama(char* prompt,char* context)
{
  struct http_url *ollama_url;
  struct http_message msg;
  int socket;
  ollama_url = http_parse_url(ollama_ip);
  if(!(socket = http_connect(ollama_url)))
  {
    free(ollama_url);
    perror("http_connect");
    return FALSE;
  }
  memset(&msg, 0, sizeof(msg));
  if (!send_post_request_to_ai(socket, ollama_url,prompt,context)) {
    while (http_response(socket, &msg) > 0) {
      if (msg.content) {
        write(1, msg.content, msg.length);
      }
    }
  }

  free(ollama_url);
  close(socket);

if (msg.header.code != 200) {
  fprintf(
    stderr,
    "error: returned HTTP code %d\n",
    msg.header.code);
}

}

bool check_if_ollama_exists()
{
  struct http_message msg;
  int socket = http_request(ollama_ip);
  memset(&msg, 0, sizeof(msg));
	while (http_response(socket, &msg));
  puts("Testing Ollama:");
  puts(msg.content);
  close(socket);
  if(msg.header.code != 200)
  {
    fprintf(stderr, "Ollama not installed or broken!!!");
    return FALSE;
  }
  return TRUE;
}

void chat()
{
  puts("Asking gods why sky is blue to test the system:");
  speak_to_ollama("Why is sky blue?", "empty");
  puts("If the answer seems to be fine then you can continue:");
  char* context = NULL;
  char prompt[1024];
  printf("> ");
  fgets(prompt,sizeof(prompt),stdin);
  context = speak_to_ollama(prompt, "empty");
  while(TRUE)
  {
    printf("> ");
    fgets(prompt,sizeof(prompt),stdin);
    context = speak_to_ollama(prompt, context);
  }
}

int main(int argc, char** argv)
{
  if(!check_if_ollama_exists())
    return -1;
  chat();
	return 0;
}

//TheNeverMan 2025
