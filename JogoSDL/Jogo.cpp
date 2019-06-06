// Universidade do Estado de Mato Grosso - UNEMAT
// Bacharelado em Sistemas de Informação
// Professor Tales Nereu Bogoni - tales@unemat.br
// Exemplo de Jogo2d para ser melhorado na disciplina de Algoritmos II
// Para complementar o jogo podem ser utilizados os tutoriais disponíveis em http://lazyfoo.net/tutorials/SDL/

#include "SDL2/SDL.h"
#include "SDL2/SDL_opengl.h"
#include "SDL2/SDL_image.h"
#include "SDL2/SDL_mixer.h"
#include <stdio.h>
#define LARGURA 800
#define ALTURA 600

//The music that will be played
Mix_Music *music = NULL;
Mix_Chunk *explosao = NULL;

SDL_Window *janelaprincipal = NULL; // nome da janela principal do jogo
SDL_Surface *content = NULL; // área de trabalho da janela

SDL_Renderer *visualizacao;

//usando para regenciar as teclas de interação com o jogo
typedef struct Teclas{
    bool esquerda;
    bool direita;
    bool espaco;
} ;

Teclas teclas;

typedef struct Mouse{
    int x, y;
    bool esquerdo, direito;
};

Mouse mouse;

void inicializaMouse(){
    mouse.x=0;
    mouse.y=0;
    mouse.direito=false;
    mouse.esquerdo=false;
}



void inicializaTeclas(){
    teclas.esquerda=false;
    teclas.direita=false;
    teclas.espaco=false;
}


typedef struct {
    SDL_Rect area; // cria um retângulo area.x, area.y, area.w (largurra), area.h (altura)
    float velocidade; // velocidade que o objeto se move
    SDL_Texture* textura=NULL; // textura da imagem
    bool vivo;
    float angulo=0;
} Objeto;

Objeto nave;
Objeto background;
Objeto tiro;
Objeto inimigo;

bool tironatela;

//diretivas do jogo
bool fim=false; // encerra o jogo com true

void carregaObjeto(Objeto *o, const char *caminho_da_imagem){
    o->area.w=64; //largura da nave
    o->area.h=64; //altura da nave
    SDL_Surface* imagem = IMG_Load(caminho_da_imagem); // le a imagem
    if( imagem == NULL ){
        printf( "Erro ao carregar nave %s\n", SDL_GetError() );
    }else {
        o->textura = SDL_CreateTextureFromSurface(visualizacao, imagem); // cria a textura
        SDL_FreeSurface(imagem); // apaga a imagem da tela
    }
}

void carregaObjetos()
{
    //Carrega as imagens
    carregaObjeto(&background,"imagens\\background.png");
    background.area.x=0;
    background.area.y=0;
    background.area.w=LARGURA;
    background.area.h=ALTURA;
    carregaObjeto(&nave,"imagens\\minhanave.png");
    nave.area.x=LARGURA/2; //posicao x do meio da janela
    nave.area.y=ALTURA - nave.area.h - 1; // posiciona no fim da tela
    nave.velocidade = 5.0f;
    carregaObjeto(&tiro,"imagens\\tiro.png");
    tiro.area.w=32;
    tiro.area.h=64;
    tiro.velocidade=10.0f;
    carregaObjeto(&inimigo,"imagens\\ovni.bmp");
    inimigo.area.x=LARGURA/4; //posicao x do meio da janela
    inimigo.area.y=120; // posiciona no fim da tela
    inimigo.vivo=false;
}

bool init()
{
    bool success = true;

    //Inicializa a SDL
    if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0 )
    {
        printf( "Erro ao carregar a SDL: %s\n", SDL_GetError() );
        success = false;
    }
    else
    {
        //Cria a janela principal
        janelaprincipal = SDL_CreateWindow( "Meu Jogo", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                   LARGURA, ALTURA, SDL_WINDOW_SHOWN );
        if( janelaprincipal == NULL )
        {
            printf( "Erro na criação da janela: %s\n", SDL_GetError() );
            success = false;
        }
        else
        {
            visualizacao = SDL_CreateRenderer(janelaprincipal, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        }

        //Initialize SDL_mixer
        if( Mix_OpenAudio( 22050, MIX_DEFAULT_FORMAT, 2, 4096 ) == -1 )
        {
            success = false;
        }
        else{
            //Load the music
            music = Mix_LoadMUS( "EricClapton.mp3" );

            //If there was a problem loading the music
            if( music == NULL )
            {
                 success = false;
            }
        }
    }
    return success;

}

void desenhaNave(){
    SDL_Point centro;
    centro.x=nave.area.w/2;
    centro.y=nave.area.h/2;
    SDL_RenderCopyEx(visualizacao, nave.textura, NULL, &nave.area, nave.angulo, &centro, SDL_FLIP_NONE );
    //SDL_RenderCopy(visualizacao,nave.textura,NULL, &nave.area);
}

void desenhaBackgound(){
    SDL_RenderCopy(visualizacao,background.textura,NULL, &background.area);
}

void desenhaTiro() {
    SDL_RenderCopy(visualizacao,tiro.textura,NULL, &tiro.area);
    tiro.area.y-=tiro.velocidade; // movimenta o tiro
    if(tiro.area.y<0) // ate sair da tela
        tironatela=false;
}

void desenhaInimigo() {
    if(inimigo.vivo)
        SDL_RenderCopy(visualizacao,inimigo.textura,NULL, &inimigo.area);
}

void close()
{
    SDL_FreeSurface( content );
    content = NULL;
    SDL_DestroyWindow( janelaprincipal );
    janelaprincipal= NULL;
    IMG_Quit();
    SDL_DestroyRenderer(visualizacao);
    SDL_DestroyTexture(nave.textura);
    SDL_DestroyTexture(background.textura);
    SDL_DestroyTexture(tiro.textura);
    SDL_DestroyTexture(inimigo.textura);
    Mix_FreeMusic( music );
    SDL_Quit(); // fecha a SDL
}

void display(){
    SDL_RenderClear(visualizacao); //limpa a tela
    desenhaBackgound(); // desenha o fundo
    desenhaNave(); // mostra os objetos
    if(tironatela)
        desenhaTiro();; // animação do tiro
    desenhaInimigo();
    SDL_RenderPresent(visualizacao);
}

void executaAcao()
{
    if(teclas.direita)
        nave.area.x+=nave.velocidade;
    if(teclas.esquerda)
        nave.area.x-=nave.velocidade;
    if(teclas.espaco){
        tironatela=true;
        teclas.espaco=false; // apenas um tiro de cada vez
        tiro.area.x=nave.area.x;
        tiro.area.y=nave.area.y;
    }
}

int main( int argc, char* args[] )
{
    inicializaTeclas();
    inicializaMouse();
    tironatela=false;
    //Eventos
    SDL_Event evento;

    //Inicializa a SDL
    if( !init() )
    {
        printf( "Falha na inicialização!\n" );
    }
    else
    {
        carregaObjetos();
        Mix_PlayMusic( music, -1 );
        explosao = Mix_LoadWAV( "explode.wav" );


        while(!fim){
            while( SDL_PollEvent( &evento ))
            {
                switch(evento.type){
                    case SDL_QUIT :
                        fim = true;
                        break;
                    case SDL_KEYDOWN: // tecla pressionada
                        if(evento.key.keysym.sym == SDLK_LEFT)
                            teclas.esquerda=true;
                        if (evento.key.keysym.sym == SDLK_RIGHT)
                            teclas.direita=true;
                        if (evento.key.keysym.sym == SDLK_SPACE)
                            if(!tironatela)
                            {
                                teclas.espaco=true;
                                Mix_PlayChannel( -1, explosao, 0 );
                            }

                        if(evento.key.keysym.sym ==SDLK_a)
                            nave.angulo -= 5;
                        if(evento.key.keysym.sym ==SDLK_d)
                            nave.angulo += 5;
                        break;
                    case SDL_KEYUP: // tecla solta
                        if(evento.key.keysym.sym == SDLK_LEFT)
                            teclas.esquerda=false;
                        if (evento.key.keysym.sym == SDLK_RIGHT)
                            teclas.direita=false;
                        if (evento.key.keysym.sym == SDLK_SPACE)
                            teclas.espaco=false;
                        break;
                    case SDL_MOUSEMOTION: // movimento do mouse
                        mouse.x=evento.motion.x;
                        mouse.y=evento.motion.y;
                        if(mouse.x>nave.area.x){
                            teclas.direita=true;
                            teclas.esquerda=false;
                        }
                        else if(mouse.x <nave.area.x)
                        {
                            teclas.direita=false;
                            teclas.esquerda=true;
                        }
                        break;
                    case SDL_MOUSEBUTTONDOWN: // pressionar botão esquerdo
                        if(evento.button.button == SDL_BUTTON_LEFT)
                            teclas.espaco=true;
                        break;
                }
            }
            executaAcao();
            display();
        }
    }

    //Libera a memória
    close();

    return 0;
}
