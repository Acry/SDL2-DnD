//BEGIN HEAD
//BEGIN DESCRIPTION

/* Drag and Drop 4
 * see:
 * https://github.com/Acry/curl_jansson
 * for more details about curl and the JSON parser jansson.
 * 
 */

/* Reflecting latest Shatertoy JSON changes
 * API seems currently unavailable.
 * 
 * Code does: load a texture needed by shader from shadertoy
 * see: https://www.shadertoy.com/api
 * If json exists, don't download, parse from file.
 * If texture exists, download is skipped also.
 * 
 *
 * example:
 * ./shader_drop Ms2SWW	# with texture
 * ./shader_drop Xds3Rr	# with sound -> should tell not supported (for now)
 * 
 */
//END   DESCRIPTION

//BEGIN INCLUDES
//system headers
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>
#include <jansson.h>
//local headers
#include "helper.h"
//END   INCLUDES

//BEGIN CPP DEFINITIONS
#define WHITE 	255,255,255,255
#define BLACK 	0,0,0,255
#define RED   	255,0,0,255
#define WW 	550
#define WH 	(WW/16)*12

#define CONFFILE	"api_key"
#define BUFFER_SIZE  	(256 * 1024)  /* 256 KB */

// concat base and api
#define URL_BASE	"https://www.shadertoy.com"
#define URL_API		""URL_BASE"/api/v1/shaders/%s?key=%s"
#define URL_SIZE     	512
//END   CPP DEFINITIONS

//BEGIN DATASTRUCTURES
struct write_result
{
	char *data;
	int   pos;
};
//END	DATASTRUCTURES

//BEGIN GLOBALS
int ww=WW;
int wh=WH;


//BEGIN VISIBLES
SDL_Surface    *temp_surface	= NULL;

SDL_Texture    *logo		= NULL;
SDL_Rect 	logo_dst;

SDL_Texture    *test_result	= NULL;
SDL_Rect	test_rect = {0,0,250,250};

//END 	VISIBLES

SDL_Point	mouse;
char* dropped_filedir;
//END   GLOBALS

//BEGIN FUNCTION PROTOTYPES
void assets_in	(void);
void assets_out	(void);
int handle_droptext(char *);
char *get_shader_shorthash(char *);
// read file into array
char 	*read_file(char *);

// read file into array without newline
char 	*read_conf(char *);

// load json
char 	*request_shader_without_key(const char *);
char 	*request(const char *);
size_t   write_response(void *, size_t , size_t , void *);

// load and write image if file doesn't exist
int 	 save_image( char *, char *);
//END	FUNCTION PROTOTYPES

//END 	HEAD

//BEGIN MAIN FUNCTION
int main(int argc, char *argv[])
{

(void)argc;
(void)argv;

//BEGIN INIT
init();
assets_in();

//BEGIN WINDOW
SDL_SetWindowPosition(Window,0,0);
SDL_SetWindowSize(Window,ww,wh);
SDL_SetWindowTitle(Window, "Shadertoy shader download");
SDL_ShowWindow(Window);
//END WINDOW

SDL_Event event;
int running = 1;
//END   INIT


#include <stdio.h>
//BEGIN MAIN LOOP
while(running){

	//BEGIN EVENT LOOP
	while(SDL_PollEvent(&event)){
		switch (event.type) {
			case SDL_QUIT:
				running = 0;
				break;
			case SDL_DROPTEXT:
// 				dropped_filedir = event.drop.file;
				handle_droptext(event.drop.file);
				break;

			case SDL_MOUSEMOTION:
				break;
			default:
				break;
		}
		
		if(event.type == SDL_MOUSEBUTTONDOWN){
			if(event.button.button == SDL_BUTTON_RIGHT){
				;
			}
			if(event.button.button == SDL_BUTTON_MIDDLE){
				;
			}
			if(event.button.button==SDL_BUTTON_LEFT){
				;
			}
		}
		if(event.type == SDL_KEYDOWN ){
			switch(event.key.keysym.sym ){
				case SDLK_ESCAPE:
					running =0;
					break;

				case SDLK_r:
				case SDLK_BACKSPACE:
					break;

				case SDLK_p:	
				case SDLK_SPACE:
					break;
					
				default:
					break;
			}
		}
	}
	//END   EVENT LOOP
	//BEGIN RENDERING
	SDL_SetRenderDrawColor(Renderer, WHITE);
	SDL_RenderClear(Renderer);

	SDL_RenderCopy(Renderer, logo, NULL, &logo_dst);
	if (test_result	!= NULL)
		SDL_RenderCopy(Renderer, test_result, NULL, &test_rect);
	

	SDL_RenderPresent(Renderer);
	//END   RENDERING
}
//END   MAIN LOOP

assets_out();
exit_();
return EXIT_SUCCESS;

}
//END   MAIN FUNCTION

//BEGIN FUNCTIONS
void assets_in(void)
{

	//BEGIN LOGO
	temp_surface = IMG_Load("./assets/gfx/logo.png");
	logo = SDL_CreateTextureFromSurface(Renderer, temp_surface);
	SDL_QueryTexture(logo, NULL, NULL, &logo_dst.w, &logo_dst.h);
	logo_dst.x=(ww/2)-(logo_dst.w/2);
	logo_dst.y=(wh/2)-(logo_dst.h/2);
	//END 	LOGO

}

void assets_out(void)
{
	SDL_DestroyTexture(logo);
	if (test_result	!= NULL)
		SDL_DestroyTexture(test_result);
}

char * read_conf(char *filename)
{
	long length 	= 0;
	char *result 	= NULL;
	FILE *file 	= fopen(filename, "r");
	
	if(file) {
		int status = fseek(file, 0, SEEK_END);
		if(status != 0) {
			fclose(file);
			return NULL;
		}
		length = ftell(file);
		status = fseek(file, 0, SEEK_SET);
		if(status != 0){
			fclose(file);
			return NULL;
		}
		result = malloc((length) * sizeof(char));
		//substitute newline against string termination
		if(result) {
			size_t actual_length = fread(result, sizeof(char), length , file);
			result[actual_length-1] = '\0';
		} 
		fclose(file);
		return result;
	}
	fprintf(stderr,"Couldn't read %s\n", filename);
	return NULL;
}

size_t write_response(void *ptr, size_t size, size_t nmemb, void *stream)
{
	struct write_result *result = (struct write_result *)stream;
	
	if(result->pos + size * nmemb >= BUFFER_SIZE - 1){
		fprintf(stderr, "error: too small buffer\n");
		return 0;
	}
	
	memcpy(result->data + result->pos, ptr, size * nmemb);
	result->pos += size * nmemb;
	
	return size * nmemb;
}

char *request(const char *url)
{
	CURL *curl 		= NULL;
	CURLcode status;
	struct curl_slist *headers = NULL;
	char *data = NULL;
	long code;
	
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	if(!curl)
		goto error;
	
	data = malloc(BUFFER_SIZE);
	if(!data)
		goto error;
	
	struct write_result write_result = {
		.data = data,
		.pos = 0
	};
	
	curl_easy_setopt(curl, CURLOPT_URL, url);
	headers = curl_slist_append(headers, "User-Agent: Acry Shadertoy API crawler");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_response);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &write_result);
	
	status = curl_easy_perform(curl);
	if(status != 0){
		fprintf(stderr, "error: unable to request data from %s:\n", url);
		fprintf(stderr, "%s\n", curl_easy_strerror(status));
		goto error;
	}
	
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
	if(code != 200){
		fprintf(stderr, "error: server responded with code %ld\n", code);
		goto error;
	}
	
	curl_easy_cleanup(curl);
	curl_slist_free_all(headers);
	curl_global_cleanup();
	
	/* zero-terminate the result */
	data[write_result.pos] = '\0';
	
	return data;
	
	error:
	if(data)
		free(data);
	if(curl)
		curl_easy_cleanup(curl);
	if(headers)
		curl_slist_free_all(headers);
	curl_global_cleanup();
	return NULL;
}

int save_image( char *url, char *filename)
{
	// check if file exists:
	if( access( filename, F_OK ) != -1 ) {
		SDL_Log("exists, nothing to do!\n");
		return 0;
	} else {
		SDL_Log("does not exist, trying to download...\n");
		CURL *image; 
		image = curl_easy_init();
		
		if( image ){ 
			// Open file 
			FILE *fp;
			fp = fopen(filename, "wb");
			
			// set file non executable, one probably won't need this
			char buffer[128];
			snprintf(buffer, sizeof(buffer), "chmod -x %s", filename);
			system(buffer);
			
			if( fp == NULL )
				SDL_Log("no image for you");
			
			CURLcode imgresult;
			curl_easy_setopt(image, CURLOPT_URL, url);
			curl_easy_setopt(image, CURLOPT_WRITEFUNCTION, NULL); 
			curl_easy_setopt(image, CURLOPT_WRITEDATA, fp); 
			
			// Grab image 
			imgresult = curl_easy_perform(image); 
			if( imgresult ){ 
				SDL_Log("still no image for you"); 
			}
			
			curl_easy_cleanup(image); 
			fclose(fp); 
			return 0;
		}
		
	}
	
	return 1; 
}

char * read_file(char *filename)
{
	long length 	= 0;
	char *result 	= NULL;
	FILE *file 	= fopen(filename, "r");
	
	if(file) {
		int status = fseek(file, 0, SEEK_END);
		if(status != 0) {
			fclose(file);
			return NULL;
		}
		length = ftell(file);
		status = fseek(file, 0, SEEK_SET);
		if(status != 0){
			fclose(file);
			return NULL;
		}
		result = malloc((length+1) * sizeof(char));
		if(result) {
			size_t actual_length = fread(result, sizeof(char), length , file);
			result[actual_length++] = '\0';
		} 
		fclose(file);
		return result;
	}
	fprintf(stderr,"Couldn't read %s\n", filename);
	return NULL;
}

int handle_droptext(char *url)
{
	// get shader shorthash
	char *shorthash=get_shader_shorthash(url);
	SDL_Log("%s",shorthash);
	
	// read API-Key
	char *key;
	key=read_conf(""CONFFILE);
	if(!key)
		return 1;
	SDL_Log("%s",key);
	
	// set URL
	char api_url[URL_SIZE];
	snprintf(api_url, URL_SIZE, URL_API, shorthash, key);
	SDL_Log("%s",api_url);

	// download shader.json
	char *text;
// 	text = request(api_url);
	text = request_shader_without_key(shorthash);
	if(!text)
		return 2;
	SDL_Log("Json download done");

	// save shader json
	FILE *fp;
	char buffer[64];
	snprintf(buffer, sizeof(buffer), "%s.json",shorthash);
	fp = fopen( buffer , "w" );
	fputs(text, fp);
	fclose(fp);
	
	// set file non executable, one probably won't need this
	char cmd[128];
	snprintf(cmd, sizeof(cmd), "chmod -x %s", buffer);
	system(cmd);

	// parse json & download asset(s)
	char *json_file;
	json_file = text;
	if(!json_file)
		return 3;
	
	// decode json
	json_error_t error;
	json_t *root;
	root = json_loads(json_file, 0, &error);
	if(!root){
		fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
		return 4;
	}

	if(json_is_array(root)){
		// extract shader name
		printf("root is an array\n");
		for(size_t i = 0; i < json_array_size(root); i++){
			json_t *data = json_array_get(root, i);
			json_t 	*info, *name;
			const char 	*requested_info;
			info 	= json_object_get(data, "info");
			name 	= json_object_get(info	, "name");
			requested_info = json_string_value(name);
			printf("Shader-Name: %s\n",requested_info);
			
			json_t 		*renderpass;
			renderpass 	= json_object_get(data, "renderpass");
			if(json_is_array(renderpass)){
				printf("renderpass is an array\n");
				for(size_t i = 0; i < json_array_size(renderpass); i++){
					json_t *data = json_array_get(renderpass, i);
					json_t *inputs = json_object_get(data, "inputs");
					if(json_is_array(inputs)){
						printf("inputs is an array\n");
						for(size_t i = 0; i < json_array_size(inputs); i++){
							json_t *data 	= json_array_get(inputs	, i);
							
							// getting type
							json_t *type;
							type = json_object_get(data, "type");
							
							// only texture support for now
							const char 	*requested_info;
							requested_info  = json_string_value(type);
							if (!strcmp("texture",requested_info)){
								printf("type: %s\n",requested_info);
								json_t *channel;
								channel= json_object_get(data, "channel");
								int chan = json_integer_value(channel);
								printf("on Channel: %d\n", chan);
								
								json_t *src;
								src = json_object_get(data, "filepath");
								requested_info  = json_string_value(src);
								char *str = strdup(requested_info);
								char *token = strtok(str, "/");
								char *last;
								while (token != NULL){
									last=token;
									token = strtok(NULL, "/");
								}
								
								char url[URL_SIZE];
								snprintf(url, URL_SIZE, URL_BASE"%s", requested_info);
								save_image(url,last);
								
								// load texture
								temp_surface = IMG_Load(last);
								test_result = SDL_CreateTextureFromSurface(Renderer, temp_surface);
								test_rect.x=(ww/2)-(test_rect.w/2);
								test_rect.y=(wh/2)-(test_rect.h/2);
							} else
								printf("Not a supported type!\n");
							
						}
					}
				}
			}
		}
	}
	free(text);
	json_decref(root);

return 0;
}

char * get_shader_shorthash(char *url)
{

char *token = strtok(url, "/");
char *shorthash;

while (token != NULL){
	shorthash=token;
	token = strtok(NULL, "/");
}

if (shorthash == NULL)
	return NULL;

return shorthash;

}

char *request_shader_without_key(const char *shader)
{
	char buffer[128];
	CURL *curl 			= NULL;
	CURLcode status;
	struct curl_slist *headers 	= NULL;
	char *data 			= NULL;
	long code;
	
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	if(!curl)
		goto error;
	
	data = malloc(BUFFER_SIZE);
	if(!data)
		goto error;
	
	struct write_result write_result = {
		.data = data,
		.pos = 0
	};
	
	curl_easy_setopt(curl, CURLOPT_URL, "https://www.shadertoy.com/shadertoy");
	
	headers = curl_slist_append(headers, "Referer: https://www.shadertoy.com/");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	
	// set postfields
	snprintf(buffer, sizeof(buffer), "s={\"shaders\":+[\"%s\"]}", shader);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS	, buffer);
	
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION	, write_response);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA	, &write_result);
	
	status = curl_easy_perform(curl);
	if(status != 0){
		fprintf(stderr, "error: unable to request data:\n");
		fprintf(stderr, "%s\n", curl_easy_strerror(status));
		goto error;
	}
	
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
	if(code != 200){
		fprintf(stderr, "error: server responded with code %ld\n", code);
		goto error;
	}
	
	curl_easy_cleanup(curl);
	curl_slist_free_all(headers);
	curl_global_cleanup();
	
	/* zero-terminate the result */
	data[write_result.pos] = '\0';
	
	return data;
	
	error:
	if(data)
		free(data);
	if(curl)
		curl_easy_cleanup(curl);
	if(headers)
		curl_slist_free_all(headers);
	curl_global_cleanup();
	return NULL;
}
//END   FUNCTIONS
