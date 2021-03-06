//BEGIN HEAD
//BEGIN DESCRIPTION

/* Drag and Drop 2
 *
 */

/* DEFINED PROGRESS GOALS
 * 
 * catch url via D&D
 * and tokenize it
 * 
 */
//END   DESCRIPTION

//BEGIN INCLUDES
//system headers
#include <math.h>
#include <string.h>
//local headers
#include "helper.h"
//END   INCLUDES

//BEGIN CPP DEFINITIONS
#define WHITE 	255,255,255,255
#define BLACK 	0,0,0,255
#define RED   	255,0,0,255
#define WW 	550
#define WH 	(WW/16)*12
//END   CPP DEFINITIONS

//BEGIN DATASTRUCTURES
//END	DATASTRUCTURES

//BEGIN GLOBALS
int ww=WW;
int wh=WH;

//BEGIN VISIBLES
SDL_Surface    *temp_surface	= NULL;

SDL_Texture    *logo		= NULL;
SDL_Rect 	logo_dst;
//END 	VISIBLES

SDL_Point	mouse;

//END   GLOBALS

//BEGIN FUNCTION PROTOTYPES
void assets_in	(void);
void assets_out	(void);
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
SDL_SetWindowTitle(Window, "SDL2 Template");
SDL_ShowWindow(Window);
//END WINDOW

SDL_Event event;
int running = 1;
//END   INIT
char* dropped_filedir;

#include <stdio.h>
//BEGIN MAIN LOOP
while(running){

	//BEGIN EVENT LOOP
	while(SDL_PollEvent(&event)){
		switch (event.type) {
			case SDL_QUIT:
				running = 0;
				break;
			case SDL_DROPFILE:
				dropped_filedir = event.drop.file;
				SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "File dropped on window", dropped_filedir, Window);
				SDL_free(dropped_filedir);
				break;
			case SDL_DROPTEXT:
				SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Text dropped on window", event.drop.file, Window);
				char *token = strtok(event.drop.file, "/");
				char *last;
				while (token != NULL){
					SDL_Log("%s\n", token);
					last=token;
					token = strtok(NULL, "/");
				}
				SDL_Log("%s\n", last);
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
}

//END   FUNCTIONS
