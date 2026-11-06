#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "audio.h"

// Dimensões dos objetos da cena:
#define COMPRIMENTO_MESA 6.0f     // 2,54 m.
#define LARGURA_MESA     3.0f     // 1,27 m.

#define ALTURA_TAMPO     1.80f    // 0,76 m.
#define ESPESSURA_TAMPO  0.12f    // 0,048 m.

#define LARGURA_BORDA    0.3000f     // 0,12 m.
#define ALTURA_BORDA     0.2500f     // 0,10 m.
#define RAIO_BOLA           0.0675f  // 0,028575 m.
#define RAIO_CACAPA_CANTO   0.135f   // 0,05715 m.
#define RAIO_CACAPA_LATERAL 0.150f   // 0,0635 m.
#define COMPRIMENTO_TACO     3.47f   // 1,47 m.
#define ESPESSURA_TACO       0.035f  // 0,015 m.
#define RAIO_SELECAO          0.095f

// Dimensões do ambiente (pés da mesa, piso, mesas de bar e lâmpadas):
#define LARGURA_PE          0.40f    // Pé da mesa de sinuca (0,17 m).
#define LADO_AZULEJO        1.00f    // Azulejo quadrado (0,42 m).
#define NUM_AZULEJOS        24       // Piso de 24 x 24 azulejos.
#define ALTURA_MESA_BAR     1.78f    // 0,75 m.
#define RAIO_MESA_BAR       0.80f    // Tampo redondo.
#define ALTURA_ASSENTO      1.05f    // Assento da cadeira (0,44 m).
#define NUM_LAMPADAS        3        // Lâmpadas penduradas sobre a mesa de sinuca.
#define ALTURA_LAMPADA      4.40f    // Altura da lâmpada (centro do bulbo).
#define ALTURA_ABAJUR       0.45f
#define RAIO_ABAJUR         0.55f
#define ALTURA_TRILHO       7.00f    // Trilho de onde os fios das lâmpadas pendem.

// Dimensões das paredes e janelas (o salão tem o mesmo tamanho do piso):
#define ALTURA_PAREDE       8.00f    // 3,4 m.
#define DIVISAO_PAREDE      1.00f    // Tamanho das subdivisões da parede (para a luz variar ao longo dela).
#define NUM_JANELAS         3        // Janelas em cada parede.
#define LARGURA_JANELA      3.00f
#define ALTURA_JANELA       2.80f
#define ALTURA_PEITORIL     2.40f    // Altura da base da janela até o chão.
#define ESPESSURA_MOLDURA   0.14f

// Conjuntos de mesa e cadeiras de plástico (modelos 3D carregados de arquivos OBJ):
#define DISTANCIA_CADEIRA   1.70f    // Distância do centro da mesa ao centro de cada cadeira.

// Parâmetros da mira e da força da tacada.
#define DISTANCIA_MIN_TACO   0.02f // Quase encostado na bola branca.
#define DISTANCIA_MAX_TACO   0.90f
#define PASSO_ANGULO_TACO    2.5f
#define PASSO_DISTANCIA_TACO 0.05f
#define PASSO_IMPACTO         0.01f
#define DESVIO_MAXIMO_IMPACTO (0.65f * RAIO_BOLA)

// Parâmetros da animação e colisão.
#define NUM_BOLAS            16
#define BOLA_BRANCA          0
#define VELOCIDADE_MINIMA    0.30f
#define VELOCIDADE_MAXIMA    10.00f
#define ATRITO               0.988f
#define ELASTICIDADE_BOLAS   0.92f
#define PERDA_NA_BORDA       0.70f
#define VELOCIDADE_PARADA    0.03f
#define INTERVALO_ANIMACAO   16

// Parâmetros da câmera virtual (ajustando a sensibilidade da movimentação da câmera):
#define PASSO_ANGULO 4.0f
#define PASSO_ZOOM   0.40f
#define PI           3.14159265f
// Posição inicial da câmera virtual pensando na primeira tacada!
float anguloHorizontal = -90.0f; 
float anguloVertical = 45.0f;   
float distanciaCamera = 9.0f;

// Posição X de cada lâmpada pendurada sobre a mesa (todas em Z = 0).
float lampadaX[NUM_LAMPADAS] = { -1.8f, 0.0f, 1.8f };

// Posição de cada janela ao longo da parede (em ordem crescente; a mesma para as quatro paredes).
float janelaX[NUM_JANELAS] = { -7.0f, 0.0f, 7.0f };

// Um modelo 3D carregado de um arquivo OBJ, guardado como lista de exibição (desenho já compilado).
typedef struct
{
    GLuint lista;
    int carregado; // 1 se o arquivo foi lido com sucesso.
} Modelo;

Modelo modeloMesaPlastico;
Modelo modeloCadeiraPlastico;

// Posição, velocidade e cor de cada bola da mesa.
float posicaoX[NUM_BOLAS];
float posicaoZ[NUM_BOLAS];
float velocidadeX[NUM_BOLAS];
float velocidadeZ[NUM_BOLAS];
float corR[NUM_BOLAS];
float corG[NUM_BOLAS];
float corB[NUM_BOLAS];
int bolaAtiva[NUM_BOLAS];
int pontos = 0;

// Estado inicial do taco e da animação.
float anguloTaco = 0.0f;
float distanciaTaco = 0.20f;
int bolaEmMovimento = 0;
int bolaBrancaSelecionada = 0;
float desvioImpacto = 0.0f;

// Uma maneira prática de desenhar uma peça retangular centralizada.
void Caixa(float x, float y, float z, float comprimento, float altura, float largura, float r, float g, float b) 
{
    glPushMatrix();
        glTranslatef(x, y, z); // Centro da caixa.
        glScalef(comprimento, altura, largura); // Escala para o tamanho desejado.
        glColor3f(r, g, b); // Cor da caixa.
        glutSolidCube(1.0); // Desenha um cubo unitário primeiro, que será escalado para o tamanho desejado.
    glPopMatrix();
}

// Desenha a mesa de sinuca com tampo verde e quinas de madeira.
void DesenhaMesa(void)
{
    float ALTURA_QUINA = ALTURA_TAMPO + ESPESSURA_TAMPO / 2.0f; // Altura do centro das bordas de madeira (quinas).

    // Tampo verde da mesa de sinuca.
    Caixa(0.0f, ALTURA_TAMPO - ESPESSURA_TAMPO / 2.0f, 0.0f, 
          COMPRIMENTO_MESA, ESPESSURA_TAMPO, LARGURA_MESA,
          0.0f, 0.35f, 0.08f); // Verde escuro.

    // Quinas
    Caixa(0.0f, ALTURA_QUINA,  LARGURA_MESA / 2.0f + LARGURA_BORDA / 2.0f, // Quina frontal.
          COMPRIMENTO_MESA + 2.0f * LARGURA_BORDA, ALTURA_BORDA, LARGURA_BORDA,
          0.30f, 0.13f, 0.04f); // Marrom escuro.

    Caixa(0.0f, ALTURA_QUINA, -LARGURA_MESA / 2.0f - LARGURA_BORDA / 2.0f, // Quina traseira.
          COMPRIMENTO_MESA + 2.0f * LARGURA_BORDA, ALTURA_BORDA, LARGURA_BORDA,
          0.30f, 0.13f, 0.04f); // Marrom escuro.

    Caixa( COMPRIMENTO_MESA / 2.0f + LARGURA_BORDA / 2.0f, ALTURA_QUINA, 0.0f, // Quina lateral direita.
          LARGURA_BORDA, ALTURA_BORDA, LARGURA_MESA,
          0.30f, 0.13f, 0.04f); // Marrom escuro.

    Caixa(-COMPRIMENTO_MESA / 2.0f - LARGURA_BORDA / 2.0f, ALTURA_QUINA, 0.0f, // Quina lateral esquerda.
          LARGURA_BORDA, ALTURA_BORDA, LARGURA_MESA,
          0.30f, 0.13f, 0.04f); // Marrom escuro.

}

// Desenha os quatro pés da mesa de sinuca, nos cantos, apoiados no chão.
void DesenhaPesMesa(void)
{
    // Os pés sobem até a parte de baixo das quinas de madeira.
    float alturaPe = ALTURA_TAMPO + ESPESSURA_TAMPO / 2.0f - ALTURA_BORDA / 2.0f;
    float xPe = COMPRIMENTO_MESA / 2.0f + LARGURA_BORDA - LARGURA_PE / 2.0f;
    float zPe = LARGURA_MESA / 2.0f + LARGURA_BORDA - LARGURA_PE / 2.0f;
    int i;
    int j;
    float sinalX;
    float sinalZ;

    for (i = 0; i < 2; i++)
    {
        for (j = 0; j < 2; j++)
        {
            sinalX = (i == 0) ? -1.0f : 1.0f;
            sinalZ = (j == 0) ? -1.0f : 1.0f;

            Caixa(sinalX * xPe, alturaPe / 2.0f, sinalZ * zPe, // Pé.
                  LARGURA_PE, alturaPe, LARGURA_PE,
                  0.30f, 0.13f, 0.04f); // Marrom escuro.

            Caixa(sinalX * xPe, 0.05f, sinalZ * zPe, // Sapata na base do pé.
                  LARGURA_PE + 0.10f, 0.10f, LARGURA_PE + 0.10f,
                  0.15f, 0.07f, 0.02f); // Marrom mais escuro.
        }
    }
}

// Desenha o chão de azulejos: um fundo de rejunte e azulejos alternados (xadrez) por cima.
void DesenhaPiso(void)
{
    int i;
    int j;
    float metade = NUM_AZULEJOS * LADO_AZULEJO / 2.0f;
    float folga = 0.02f; // Metade da largura do rejunte.
    float x0;
    float z0;
    float x1;
    float z1;

    glNormal3f(0.0f, 1.0f, 0.0f); // Todo o piso aponta para cima.

    // Rejunte.
    glColor3f(0.22f, 0.22f, 0.22f);
    glBegin(GL_QUADS);
        glVertex3f(-metade, 0.0f, -metade);
        glVertex3f(-metade, 0.0f,  metade);
        glVertex3f( metade, 0.0f,  metade);
        glVertex3f( metade, 0.0f, -metade);
    glEnd();

    // Azulejos, levemente acima do rejunte para não brigar no Z-Buffer.
    glBegin(GL_QUADS);
    for (i = 0; i < NUM_AZULEJOS; i++)
    {
        for (j = 0; j < NUM_AZULEJOS; j++)
        {
            if ((i + j) % 2 == 0)
                glColor3f(0.88f, 0.86f, 0.80f); // Azulejo claro.
            else
                glColor3f(0.30f, 0.33f, 0.36f); // Azulejo escuro.

            x0 = -metade + i * LADO_AZULEJO + folga;
            z0 = -metade + j * LADO_AZULEJO + folga;
            x1 = x0 + LADO_AZULEJO - 2.0f * folga;
            z1 = z0 + LADO_AZULEJO - 2.0f * folga;

            glVertex3f(x0, 0.004f, z0);
            glVertex3f(x0, 0.004f, z1);
            glVertex3f(x1, 0.004f, z1);
            glVertex3f(x1, 0.004f, z0);
        }
    }
    glEnd();
}

// Desenha um cilindro vertical (com tampa em cima) a partir da altura yBase.
void CilindroVertical(float x, float yBase, float z, float raio, float altura, float r, float g, float b)
{
    GLUquadric *quadric = gluNewQuadric();

    glColor3f(r, g, b);
    glPushMatrix();
        glTranslatef(x, yBase, z);
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f); // O cilindro da GLU nasce no eixo Z; aqui ele sobe pelo eixo Y.
        gluCylinder(quadric, raio, raio, altura, 24, 1);
        glTranslatef(0.0f, 0.0f, altura);
        gluDisk(quadric, 0.0, raio, 24, 1); // Tampa de cima.
    glPopMatrix();

    gluDeleteQuadric(quadric);
}

// Desenha uma cadeira de madeira com assento vermelho, de frente para +Z (antes de girar).
void DesenhaCadeira(float x, float z, float angulo)
{
    int i;
    float sinalX;
    float sinalZ;

    glPushMatrix();
        glTranslatef(x, 0.0f, z);
        glRotatef(angulo, 0.0f, 1.0f, 0.0f);

        // Assento.
        Caixa(0.0f, ALTURA_ASSENTO - 0.05f, 0.0f, 0.90f, 0.10f, 0.90f, 0.55f, 0.08f, 0.08f);

        // Quatro pernas.
        for (i = 0; i < 4; i++)
        {
            sinalX = (i & 1) ? 1.0f : -1.0f;
            sinalZ = (i & 2) ? 1.0f : -1.0f;
            Caixa(sinalX * 0.38f, (ALTURA_ASSENTO - 0.10f) / 2.0f, sinalZ * 0.38f,
                  0.08f, ALTURA_ASSENTO - 0.10f, 0.08f, 0.35f, 0.18f, 0.07f);
        }

        // Encosto: dois postes e um painel (atrás, no lado -Z).
        Caixa(-0.38f, 1.55f, -0.38f, 0.08f, 1.10f, 0.08f, 0.35f, 0.18f, 0.07f);
        Caixa( 0.38f, 1.55f, -0.38f, 0.08f, 1.10f, 0.08f, 0.35f, 0.18f, 0.07f);
        Caixa(0.0f, 1.85f, -0.38f, 0.70f, 0.35f, 0.06f, 0.35f, 0.18f, 0.07f);
    glPopMatrix();
}

// Desenha uma mesa de bar redonda (com uma coluna central) e duas cadeiras frente a frente.
void DesenhaMesaBar(float x, float z, float angulo)
{
    glPushMatrix();
        glTranslatef(x, 0.0f, z);
        glRotatef(angulo, 0.0f, 1.0f, 0.0f);

        CilindroVertical(0.0f, 0.0f, 0.0f, 0.45f, 0.06f, 0.20f, 0.20f, 0.20f); // Base no chão.
        CilindroVertical(0.0f, 0.06f, 0.0f, 0.07f, ALTURA_MESA_BAR - 0.14f, 0.25f, 0.25f, 0.25f); // Coluna.
        CilindroVertical(0.0f, ALTURA_MESA_BAR - 0.08f, 0.0f, RAIO_MESA_BAR, 0.08f, 0.50f, 0.30f, 0.14f); // Tampo.

        DesenhaCadeira(0.0f, -1.20f, 0.0f);   // Cadeira de um lado, olhando para a mesa.
        DesenhaCadeira(0.0f,  1.20f, 180.0f); // Cadeira do outro lado.
    glPopMatrix();
}

// Lê um índice de vértice de uma face OBJ, nos formatos "v", "v/vt", "v//vn" ou "v/vt/vn".
// Devolve 0 quando não há mais índices na linha. Índices negativos contam a partir do fim.
int LeIndiceFace(char **texto, int numVertices, int numNormais, int *vertice, int *normal)
{
    char *s = *texto;
    char *fim;
    long v;
    long n = 0;

    while (*s == ' ' || *s == '\t')
        s++;
    if (*s == '\0' || *s == '\n' || *s == '\r')
        return 0;

    v = strtol(s, &fim, 10);
    if (fim == s)
        return 0;
    s = fim;

    if (*s == '/')
    {
        s++;
        if (*s != '/') // Pula o índice de textura, que não usamos.
        {
            strtol(s, &fim, 10);
            s = fim;
        }
        if (*s == '/')
        {
            s++;
            n = strtol(s, &fim, 10);
            if (fim != s)
                s = fim;
            else
                n = 0;
        }
    }

    if (v < 0)
        v = numVertices + v + 1;
    if (n < 0)
        n = numNormais + n + 1;

    *vertice = (int)v - 1;
    *normal = (n > 0) ? (int)n - 1 : -1; // -1 = a face não traz normal.
    *texto = s;
    return 1;
}

// Carrega um arquivo OBJ (vértices, normais e faces) e o compila numa lista de exibição.
// Precisa ser chamada depois de glutCreateWindow. Devolve 1 se deu certo, 0 se falhou.
int CarregaModelo(Modelo *modelo, const char *arquivo)
{
    FILE *f = fopen(arquivo, "r");
    char linha[512];
    float *vertices;
    float *normais;
    int nv = 0;
    int nn = 0;
    int lidosV = 0;
    int lidosN = 0;
    int iv[16];
    int in[16];
    int n;
    int k;
    char *texto;
    float ux, uy, uz, wx, wy, wz, nx, ny, nz, tam;

    modelo->carregado = 0;
    if (f == NULL)
    {
        printf("Aviso: nao foi possivel abrir o modelo '%s'.\n", arquivo);
        return 0;
    }

    // Primeira passada: conta vértices e normais para reservar a memória.
    while (fgets(linha, sizeof(linha), f) != NULL)
    {
        if (strncmp(linha, "v ", 2) == 0)
            nv++;
        else if (strncmp(linha, "vn ", 3) == 0)
            nn++;
    }
    rewind(f);

    vertices = (float *)malloc(3 * (nv > 0 ? nv : 1) * sizeof(float));
    normais = (float *)malloc(3 * (nn > 0 ? nn : 1) * sizeof(float));
    if (vertices == NULL || normais == NULL)
    {
        free(vertices);
        free(normais);
        fclose(f);
        return 0;
    }

    // Segunda passada: guarda os dados e desenha cada face dentro da lista de exibição.
    modelo->lista = glGenLists(1);
    glNewList(modelo->lista, GL_COMPILE);
    glBegin(GL_TRIANGLES);

    while (fgets(linha, sizeof(linha), f) != NULL)
    {
        if (strncmp(linha, "v ", 2) == 0)
        {
            sscanf(linha + 2, "%f %f %f", &vertices[3 * lidosV], &vertices[3 * lidosV + 1], &vertices[3 * lidosV + 2]);
            lidosV++;
        }
        else if (strncmp(linha, "vn ", 3) == 0)
        {
            sscanf(linha + 3, "%f %f %f", &normais[3 * lidosN], &normais[3 * lidosN + 1], &normais[3 * lidosN + 2]);
            lidosN++;
        }
        else if (strncmp(linha, "f ", 2) == 0)
        {
            texto = linha + 2;
            n = 0;
            while (n < 16 && LeIndiceFace(&texto, lidosV, lidosN, &iv[n], &in[n]))
                n++;

            // Faces com mais de três lados viram um leque de triângulos.
            for (k = 1; k + 1 < n; k++)
            {
                int a = iv[0];
                int b = iv[k];
                int c = iv[k + 1];

                if (a < 0 || b < 0 || c < 0 || a >= lidosV || b >= lidosV || c >= lidosV)
                    continue;

                if (in[0] >= 0 && in[k] >= 0 && in[k + 1] >= 0)
                {
                    // Normais suaves, uma por vértice.
                    glNormal3fv(&normais[3 * in[0]]);
                    glVertex3fv(&vertices[3 * a]);
                    glNormal3fv(&normais[3 * in[k]]);
                    glVertex3fv(&vertices[3 * b]);
                    glNormal3fv(&normais[3 * in[k + 1]]);
                    glVertex3fv(&vertices[3 * c]);
                }
                else
                {
                    // Sem normais no arquivo: calcula a normal da face (produto vetorial).
                    ux = vertices[3 * b] - vertices[3 * a];
                    uy = vertices[3 * b + 1] - vertices[3 * a + 1];
                    uz = vertices[3 * b + 2] - vertices[3 * a + 2];
                    wx = vertices[3 * c] - vertices[3 * a];
                    wy = vertices[3 * c + 1] - vertices[3 * a + 1];
                    wz = vertices[3 * c + 2] - vertices[3 * a + 2];
                    nx = uy * wz - uz * wy;
                    ny = uz * wx - ux * wz;
                    nz = ux * wy - uy * wx;
                    tam = sqrtf(nx * nx + ny * ny + nz * nz);
                    if (tam > 0.0f)
                        glNormal3f(nx / tam, ny / tam, nz / tam);
                    glVertex3fv(&vertices[3 * a]);
                    glVertex3fv(&vertices[3 * b]);
                    glVertex3fv(&vertices[3 * c]);
                }
            }
        }
    }

    glEnd();
    glEndList();

    free(vertices);
    free(normais);
    fclose(f);

    modelo->carregado = 1;
    return 1;
}

// Carrega os modelos usados na cena (os arquivos ficam na pasta "modelos", junto do programa).
void CarregaModelos(void)
{
    CarregaModelo(&modeloMesaPlastico, "mesa_plastico.obj");
    CarregaModelo(&modeloCadeiraPlastico, "cadeira_plastico.obj");
}

// Desenha um modelo 3D no chão, na posição x e z, girado em torno do eixo vertical.
void DesenhaModelo(Modelo *modelo, float x, float z, float angulo, float r, float g, float b)
{
    if (modelo->carregado == 0)
        return;

    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE); // Peças finas (como os braços da cadeira) recebem luz dos dois lados.
    glPushMatrix();
        glTranslatef(x, 0.0f, z);
        glRotatef(angulo, 0.0f, 1.0f, 0.0f);
        glColor3f(r, g, b);
        glCallList(modelo->lista);
    glPopMatrix();
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
}

// Desenha uma mesa quadrada de plástico com quatro cadeiras viradas para ela.
void DesenhaConjuntoPlastico(float x, float z, float angulo)
{
    int i;

    glPushMatrix();
        glTranslatef(x, 0.0f, z);
        glRotatef(angulo, 0.0f, 1.0f, 0.0f);

        DesenhaModelo(&modeloMesaPlastico, 0.0f, 0.0f, 0.0f, 0.95f, 0.78f, 0.05f); // Mesa amarela.

        // A cadeira do arquivo olha para -Z: colocada em +Z, ela fica de frente para a mesa.
        // Girando o conjunto de 90 em 90 graus, as quatro cadeiras cercam a mesa.
        for (i = 0; i < 4; i++)
        {
            glPushMatrix();
                glRotatef(90.0f * i, 0.0f, 1.0f, 0.0f);
                DesenhaModelo(&modeloCadeiraPlastico, 0.0f, DISTANCIA_CADEIRA, 0.0f, 0.95f, 0.78f, 0.05f);
            glPopMatrix();
        }
    glPopMatrix();
}

// Posiciona quatro conjuntos de mesa e cadeiras ao redor da mesa de sinuca.
void DesenhaMesasBar(void)
{
    if (modeloMesaPlastico.carregado == 1 && modeloCadeiraPlastico.carregado == 1)
    {
        // Conjuntos de plástico (modelos 3D), com pequenas rotações para não ficarem todos iguais.
        DesenhaConjuntoPlastico( 6.5f, -4.8f,  15.0f);
        DesenhaConjuntoPlastico( 6.5f,  4.8f, -10.0f);
        DesenhaConjuntoPlastico(-6.5f, -4.8f,  30.0f);
        DesenhaConjuntoPlastico(-6.5f,  4.8f,   0.0f);
    }
    else
    {
        // Se os arquivos não foram encontrados, usa as mesas e cadeiras feitas com caixas.
        DesenhaMesaBar( 6.5f, -4.8f, 90.0f);
        DesenhaMesaBar( 6.5f,  4.8f,  0.0f);
        DesenhaMesaBar(-6.5f, -4.8f,  0.0f);
        DesenhaMesaBar(-6.5f,  4.8f, 90.0f);
    }
}

// Desenha uma lâmpada pendurada: fio, abajur verde, e o bulbo aceso (sem iluminação).
void DesenhaLampada(float x, float z)
{
    GLUquadric *quadric = gluNewQuadric();
    float bordaAbajur = ALTURA_LAMPADA + 0.05f; // Borda de baixo do abajur.
    float topoAbajur = bordaAbajur + ALTURA_ABAJUR;

    // Fio que liga o abajur ao trilho.
    Caixa(x, (topoAbajur + ALTURA_TRILHO) / 2.0f, z,
          0.03f, ALTURA_TRILHO - topoAbajur, 0.03f,
          0.08f, 0.08f, 0.08f);

    // Abajur em forma de cone.
    glColor3f(0.08f, 0.30f, 0.14f);
    glPushMatrix();
        glTranslatef(x, bordaAbajur, z);
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
        gluCylinder(quadric, RAIO_ABAJUR, 0.12, ALTURA_ABAJUR, 24, 1);
    glPopMatrix();

    // A luz em si não recebe sombreamento: desliga a iluminação para ela parecer acesa.
    glDisable(GL_LIGHTING);

    glColor3f(1.0f, 0.92f, 0.60f); // Brilho na boca do abajur.
    glPushMatrix();
        glTranslatef(x, bordaAbajur, z);
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
        gluDisk(quadric, 0.0, RAIO_ABAJUR, 24, 1);
    glPopMatrix();

    glColor3f(1.0f, 0.97f, 0.80f); // Bulbo.
    glPushMatrix();
        glTranslatef(x, ALTURA_LAMPADA, z);
        glutSolidSphere(0.14f, 16, 16);
    glPopMatrix();

    glEnable(GL_LIGHTING);
    gluDeleteQuadric(quadric);
}

// Desenha o trilho e as lâmpadas penduradas sobre a mesa de sinuca.
void DesenhaLampadas(void)
{
    int i;

    Caixa(0.0f, ALTURA_TRILHO, 0.0f, 4.60f, 0.08f, 0.08f, 0.12f, 0.12f, 0.12f); // Trilho.

    for (i = 0; i < NUM_LAMPADAS; i++)
        DesenhaLampada(lampadaX[i], 0.0f);
}

// Desenha um retângulo de parede (no plano z = 0, virado para +Z), dividido em pedaços menores.
// Precisa ser chamado entre glBegin(GL_QUADS) e glEnd().
void QuadParede(float x0, float y0, float x1, float y1)
{
    int nx;
    int ny;
    int i;
    int j;
    float dx;
    float dy;
    float xa;
    float ya;

    if (x1 <= x0 || y1 <= y0)
        return;

    nx = (int)ceilf((x1 - x0) / DIVISAO_PAREDE);
    ny = (int)ceilf((y1 - y0) / DIVISAO_PAREDE);
    dx = (x1 - x0) / nx;
    dy = (y1 - y0) / ny;

    for (i = 0; i < nx; i++)
    {
        for (j = 0; j < ny; j++)
        {
            xa = x0 + i * dx;
            ya = y0 + j * dy;

            // Sentido anti-horário visto de +Z (necessário para o descarte de faces).
            glVertex3f(xa,      ya,      0.0f);
            glVertex3f(xa + dx, ya,      0.0f);
            glVertex3f(xa + dx, ya + dy, 0.0f);
            glVertex3f(xa,      ya + dy, 0.0f);
        }
    }
}

// Move o desenho para uma das quatro paredes, sempre com a face interna virada para o centro do salão.
void PosicionaParede(int indice)
{
    float metade = NUM_AZULEJOS * LADO_AZULEJO / 2.0f;

    switch (indice)
    {
        case 0: // Parede do fundo (z negativo).
            glTranslatef(0.0f, 0.0f, -metade);
            break;
        case 1: // Parede da frente (z positivo).
            glTranslatef(0.0f, 0.0f, metade);
            glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
            break;
        case 2: // Parede da esquerda (x negativo).
            glTranslatef(-metade, 0.0f, 0.0f);
            glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
            break;
        case 3: // Parede da direita (x positivo).
            glTranslatef(metade, 0.0f, 0.0f);
            glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
            break;
    }
}

// Desenha uma parede com os vãos das janelas (sem os vãos, o resto é preenchido).
void Parede(void)
{
    int i;
    float metade = NUM_AZULEJOS * LADO_AZULEJO / 2.0f;
    float baseJanela = ALTURA_PEITORIL;
    float topoJanela = ALTURA_PEITORIL + ALTURA_JANELA;
    float xAnterior = -metade; // Onde termina o último pedaço de parede desenhado.

    glNormal3f(0.0f, 0.0f, 1.0f);
    glColor3f(0.86f, 0.80f, 0.68f); // Bege.

    glBegin(GL_QUADS);
        QuadParede(-metade, 0.0f, metade, baseJanela);               // Faixa abaixo das janelas.
        QuadParede(-metade, topoJanela, metade, ALTURA_PAREDE);      // Faixa acima das janelas.

        // Pedaços de parede entre as janelas.
        for (i = 0; i < NUM_JANELAS; i++)
        {
            QuadParede(xAnterior, baseJanela, janelaX[i] - LARGURA_JANELA / 2.0f, topoJanela);
            xAnterior = janelaX[i] + LARGURA_JANELA / 2.0f;
        }
        QuadParede(xAnterior, baseJanela, metade, topoJanela);
    glEnd();
}

// Desenha a moldura, o travessão em cruz e o peitoril de uma janela (no plano da parede).
void DesenhaJanela(float x)
{
    float base = ALTURA_PEITORIL;
    float topo = ALTURA_PEITORIL + ALTURA_JANELA;
    float centroY = (base + topo) / 2.0f;
    float e = ESPESSURA_MOLDURA;

    // Quatro lados da moldura.
    Caixa(x, base + e / 2.0f, 0.06f, LARGURA_JANELA, e, 0.22f, 0.92f, 0.92f, 0.90f);
    Caixa(x, topo - e / 2.0f, 0.06f, LARGURA_JANELA, e, 0.22f, 0.92f, 0.92f, 0.90f);
    Caixa(x - LARGURA_JANELA / 2.0f + e / 2.0f, centroY, 0.06f, e, ALTURA_JANELA, 0.22f, 0.92f, 0.92f, 0.90f);
    Caixa(x + LARGURA_JANELA / 2.0f - e / 2.0f, centroY, 0.06f, e, ALTURA_JANELA, 0.22f, 0.92f, 0.92f, 0.90f);

    // Travessões (cruz) dividindo o vidro em quatro.
    Caixa(x, centroY, 0.06f, 0.08f, ALTURA_JANELA, 0.12f, 0.92f, 0.92f, 0.90f);
    Caixa(x, centroY, 0.06f, LARGURA_JANELA, 0.08f, 0.12f, 0.92f, 0.92f, 0.90f);

    // Peitoril saindo para dentro do salão.
    Caixa(x, base - 0.04f, 0.20f, LARGURA_JANELA + 0.40f, 0.08f, 0.50f, 0.85f, 0.85f, 0.82f);
}

// Desenha as quatro paredes e as molduras das janelas.
void DesenhaParedes(void)
{
    int i;
    int j;

    // Descarta as faces de trás: quando a câmera está fora do salão (zoom máximo),
    // a parede some e continuamos vendo o interior.
    glEnable(GL_CULL_FACE);
    for (i = 0; i < 4; i++)
    {
        glPushMatrix();
            PosicionaParede(i);
            Parede();
        glPopMatrix();
    }
    glDisable(GL_CULL_FACE);

    for (i = 0; i < 4; i++)
    {
        glPushMatrix();
            PosicionaParede(i);
            for (j = 0; j < NUM_JANELAS; j++)
                DesenhaJanela(janelaX[j]);
        glPopMatrix();
    }
}

// Desenha o vidro das janelas, semitransparente. Deve ser o último objeto desenhado da cena.
void DesenhaVidros(void)
{
    int i;
    int j;
    float base = ALTURA_PEITORIL;
    float topo = ALTURA_PEITORIL + ALTURA_JANELA;
    float esquerda;
    float direita;

    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE); // O vidro não escreve no Z-Buffer, para não esconder o que está atrás dele.

    glColor4f(0.60f, 0.80f, 1.00f, 0.25f); // Azul claro transparente.

    for (i = 0; i < 4; i++)
    {
        glPushMatrix();
            PosicionaParede(i);
            glBegin(GL_QUADS);
            for (j = 0; j < NUM_JANELAS; j++)
            {
                esquerda = janelaX[j] - LARGURA_JANELA / 2.0f;
                direita = janelaX[j] + LARGURA_JANELA / 2.0f;
                glVertex3f(esquerda, base, 0.0f);
                glVertex3f(direita,  base, 0.0f);
                glVertex3f(direita,  topo, 0.0f);
                glVertex3f(esquerda, topo, 0.0f);
            }
            glEnd();
        glPopMatrix();
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

// Desenha uma bola colorida na posição x e z da mesa.
void DesenhaBola(float x, float z, float r, float g, float b)
{
    glPushMatrix();
        glTranslatef(x, ALTURA_TAMPO + RAIO_BOLA, z); // Coloca a bola sobre o tampo.
        glColor3f(r, g, b); // Cor da bola.
        glutSolidSphere(RAIO_BOLA, 24, 24); // Desenha a esfera sólida.
    glPopMatrix();
}

// Define a posição e a cor de uma bola, deixando-a inicialmente parada.
void DefineBola(int indice, float x, float z, float r, float g, float b)
{
    // Inicializa a posição, velocidade e cor da bola.
    posicaoX[indice] = x;
    posicaoZ[indice] = z;
    velocidadeX[indice] = 0.0f;
    velocidadeZ[indice] = 0.0f;
    corR[indice] = r;
    corG[indice] = g;
    corB[indice] = b;
    bolaAtiva[indice] = 1;
}

// Define a bola branca e o triângulo de bolas coloridas na posição inicial.
void InicializaBolas(void)
{
    float inicioX = 1.20f; // Posição da primeira bola do triângulo (estimado).
    float avancoX = 1.732f * RAIO_BOLA; // Distância entre as fileiras do triângulo (1.732 = raiz quadrada de 3).
    float avancoZ = 2.0f * RAIO_BOLA; // Distância entre bolas da mesma fileira (ou seja, estão encostadas).

    DefineBola(0, -1.5f, 0.0f, 0.95f, 0.95f, 0.95f);                              // Bola branca.

    // Triângulo de bolas com cores tradicionais:
    DefineBola(1, inicioX, 0.0f, 1.0f, 1.0f, 0.0f);                               // Bola 1: Amarela.

    DefineBola(2, inicioX + avancoX, -0.5f * avancoZ, 1.0f, 1.0f, 0.0f);          // Bola 9: amarela.
    DefineBola(3, inicioX + avancoX,  0.5f * avancoZ, 0.0f, 0.0f, 1.0f);          // Bola 2: azul.

    DefineBola(4, inicioX + 2.0f * avancoX, -avancoZ, 1.0f, 0.0f, 0.0f);          // Bola 3: vermelha.
    DefineBola(5, inicioX + 2.0f * avancoX, 0.0f, 0.05f, 0.05f, 0.05f);           // Bola 8: preta.
    DefineBola(6, inicioX + 2.0f * avancoX,  avancoZ, 0.0f, 0.0f, 1.0f);          // Bola 10: azul.

    DefineBola(7, inicioX + 3.0f * avancoX, -1.5f * avancoZ, 1.0f, 0.0f, 0.0f);   // Bola 11: vermelha.
    DefineBola(8, inicioX + 3.0f * avancoX, -0.5f * avancoZ, 0.50f, 0.0f, 0.50f); // Bola 4: roxa.
    DefineBola(9, inicioX + 3.0f * avancoX,  0.5f * avancoZ, 0.50f, 0.0f, 0.50f); // Bola 12: roxa.
    DefineBola(10, inicioX + 3.0f * avancoX, 1.5f * avancoZ, 1.0f, 0.50f, 0.0f);  // Bola 5: laranja.

    DefineBola(11, inicioX + 4.0f * avancoX, -2.0f * avancoZ, 0.0f, 0.50f, 0.0f); // Bola 6: verde.
    DefineBola(12, inicioX + 4.0f * avancoX, -1.0f * avancoZ, 1.0f, 0.50f, 0.0f); // Bola 13: laranja.
    DefineBola(13, inicioX + 4.0f * avancoX, 0.0f, 0.50f, 0.0f, 0.0f);            // Bola 7: vinho.
    DefineBola(14, inicioX + 4.0f * avancoX, 1.0f * avancoZ, 0.0f, 0.50f, 0.0f);  // Bola 14: verde.
    DefineBola(15, inicioX + 4.0f * avancoX, 2.0f * avancoZ, 0.50f, 0.0f, 0.0f);  // Bola 15: vinho.
}

// Desenha as dezesseis bolas nas posições atuais.
void DesenhaBolas(void)
{
    int i;

    for (i = 0; i < NUM_BOLAS; i++)
        if (bolaAtiva[i] == 1)
            DesenhaBola(posicaoX[i], posicaoZ[i], corR[i], corG[i], corB[i]);
}

// Desenha o taco atrás da bola branca, na direção escolhida.
void DesenhaTaco(void)
{
    // Cilindro representando o taco, posicionado atrás da bola branca.
    float anguloRad = anguloTaco * PI / 180.0f;
    float direcaoX = cosf(anguloRad);
    float direcaoZ = sinf(anguloRad);
    float lateralX = -direcaoZ;
    float lateralZ = direcaoX;
    float profundidade = sqrtf(RAIO_BOLA * RAIO_BOLA - desvioImpacto * desvioImpacto);
    // Calcula a posição da ponta do taco, considerando a distância da bola branca.
    // Isso vai permiti simular a força da tacada de acordo com a distância da ponta do taco até a bola branca.
    float pontaX = posicaoX[BOLA_BRANCA] - direcaoX * (profundidade + distanciaTaco) + lateralX * desvioImpacto;
    float pontaZ = posicaoZ[BOLA_BRANCA] - direcaoZ * (profundidade + distanciaTaco) + lateralZ * desvioImpacto;

    glPushMatrix();
        glTranslatef(pontaX, ALTURA_TAMPO + RAIO_BOLA, pontaZ); // Coloca a ponta atrás da bola.
        glRotatef(-anguloTaco, 0.0f, 1.0f, 0.0f); // Gira o taco em torno do eixo vertical.
        glTranslatef(-COMPRIMENTO_TACO / 2.0f, 0.0f, 0.0f); // Centraliza o taco a partir da ponta.
        glScalef(COMPRIMENTO_TACO, ESPESSURA_TACO, ESPESSURA_TACO);
        glColor3f(0.65f, 0.38f, 0.15f); // Cor de madeira.
        glutSolidCube(1.0); // Representa o taco com uma caixa fina.
    glPopMatrix();
}

// Desenha um anel amarelo para indicar que a bola branca foi selecionada.
void DesenhaSelecaoBranca(void)
{
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 0.0f);
    glLineWidth(2.0f);

    glPushMatrix();
        glTranslatef(posicaoX[BOLA_BRANCA], ALTURA_TAMPO + 0.008f, posicaoZ[BOLA_BRANCA]);
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        glutWireTorus(0.006f, RAIO_SELECAO, 8, 24);
    glPopMatrix();

    glLineWidth(1.0f);
    glEnable(GL_LIGHTING);
}

// Desenha uma caçapa como um círculo escuro sobre o tampo.
void DesenhaCacapa(float x, float z, float raio)
{
    // Cria um objeto quadric (uma superfície curva)para desenhar o disco da caçapa.
    GLUquadric *quadric = gluNewQuadric();

    glPushMatrix();
        glTranslatef(x, ALTURA_TAMPO + 0.003f, z); // Posição da caçapa sobre o tampo.
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f); // Coloca o círculo na posição horizontal.
        glColor3f(0.03f, 0.03f, 0.03f); // Cor escura para representar o buraco.
        gluDisk(quadric, 0.0, raio, 24, 1); // Desenha um círculo preenchido.
    glPopMatrix();

    gluDeleteQuadric(quadric);
}

// Desenha quatro caçapas nos cantos e duas nas laterais.
void DesenhaCacapas(void)
{
    float xCanto = COMPRIMENTO_MESA / 2.0f - RAIO_CACAPA_CANTO / 2.0f;
    float zCanto = LARGURA_MESA / 2.0f - RAIO_CACAPA_CANTO / 2.0f;
    float zLateral = LARGURA_MESA / 2.0f - RAIO_CACAPA_LATERAL / 2.0f;

    DesenhaCacapa( xCanto,  zCanto, RAIO_CACAPA_CANTO); // Canto direito frontal.
    DesenhaCacapa( xCanto, -zCanto, RAIO_CACAPA_CANTO); // Canto direito traseiro.
    DesenhaCacapa(-xCanto,  zCanto, RAIO_CACAPA_CANTO); // Canto esquerdo frontal.
    DesenhaCacapa(-xCanto, -zCanto, RAIO_CACAPA_CANTO); // Canto esquerdo traseiro.
    DesenhaCacapa(0.0f,  zLateral, RAIO_CACAPA_LATERAL); // Lateral frontal.
    DesenhaCacapa(0.0f, -zLateral, RAIO_CACAPA_LATERAL); // Lateral traseira.
}

// Atualiza o título da janela com a quantidade de bolas coloridas encaçapadas.
void AtualizaTitulo(void)
{
    char titulo[50];

    snprintf(titulo, sizeof(titulo), "Sinuca 3D - Pontos: %d", pontos);
    glutSetWindowTitle(titulo);
}

// Verifica se o centro de uma bola entrou na abertura de uma caçapa.
int BolaNaCacapa(int indice, float xCacapa, float zCacapa, float raioCacapa)
{
    float dx = posicaoX[indice] - xCacapa;
    float dz = posicaoZ[indice] - zCacapa;

    // Compara as distâncias ao quadrado para não precisar calcular raiz quadrada.
    return dx * dx + dz * dz < raioCacapa * raioCacapa;
}

// Remove uma bola colorida da mesa ou reposiciona a bola branca.
void EncaçapaBola(int indice)
{
    velocidadeX[indice] = 0.0f;
    velocidadeZ[indice] = 0.0f;

    if (indice == BOLA_BRANCA)
    {
        // A branca não vale ponto: ela volta para sua posição inicial.
        posicaoX[indice] = -1.5f;
        posicaoZ[indice] = 0.0f;
    }
    else
    {
        // Bola inativa não é mais desenhada, atualizada nem testada em colisões.
        bolaAtiva[indice] = 0;
        pontos++; // Cada bola colorida encaçapada vale um ponto.
        printf("Ponto! Total: %d\n", pontos);
        AtualizaTitulo();
    }
}

// Testa cada bola ativa contra as seis caçapas da mesa.
void VerificaCacapas(void)
{
    int i;
    float xCanto = COMPRIMENTO_MESA / 2.0f - RAIO_CACAPA_CANTO / 2.0f;
    float zCanto = LARGURA_MESA / 2.0f - RAIO_CACAPA_CANTO / 2.0f;
    float zLateral = LARGURA_MESA / 2.0f - RAIO_CACAPA_LATERAL / 2.0f;

    for (i = 0; i < NUM_BOLAS; i++)
    {
        // Não é necessário testar bolas que já foram encaçapadas.
        if (bolaAtiva[i] == 0)
            continue;

        // A bola é encaçapada ao entrar em qualquer uma das seis aberturas.
        if (BolaNaCacapa(i,  xCanto,  zCanto, RAIO_CACAPA_CANTO) ||
            BolaNaCacapa(i,  xCanto, -zCanto, RAIO_CACAPA_CANTO) ||
            BolaNaCacapa(i, -xCanto,  zCanto, RAIO_CACAPA_CANTO) ||
            BolaNaCacapa(i, -xCanto, -zCanto, RAIO_CACAPA_CANTO) ||
            BolaNaCacapa(i, 0.0f,  zLateral, RAIO_CACAPA_LATERAL) ||
            BolaNaCacapa(i, 0.0f, -zLateral, RAIO_CACAPA_LATERAL))
        {
            EncaçapaBola(i);
        }
    }
}

// Configura a iluminação da cena.
void ConfiguraIluminacao(void)
{
    // Testar diferentes valores para a luz e o material, para ver o que fica melhor!
    GLfloat posicao[] = { 0.0f, 6.0f, 0.0f, 1.0f };      // Em cima da mesa, iluminando de cima para baixo.
    GLfloat ambiente[] = { 0.20f, 0.20f, 0.20f, 1.0f };  // Luz fraca que chega em todos os lados.
    GLfloat difusa[] = { 0.25f, 0.25f, 0.25f, 1.0f };    // Luz geral do salão (a principal agora vem das lâmpadas).
    GLfloat especular[] = { 0.15f, 0.15f, 0.15f, 1.0f }; // Brilho refletido pela luz.
    GLfloat difusaLampada[] = { 0.20f, 0.18f, 0.13f, 1.0f };     // Luz amarelada das lâmpadas.
    GLfloat especularLampada[] = { 0.15f, 0.13f, 0.10f, 1.0f };
    GLfloat posicaoLampada[4];
    int i;
    GLfloat materialEspecular[] = { 0.20f, 0.20f, 0.20f, 1.0f }; // Reflexo discreto do material.
    GLfloat brilho[] = { 20.0f }; // Concentração do brilho.

    // Ativando a iluminação e configurando os parâmetros da luz e do material.
    glEnable(GL_LIGHTING); 
    glEnable(GL_LIGHT0); 
    glEnable(GL_COLOR_MATERIAL); 
    glEnable(GL_NORMALIZE); 
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE); // Frente e verso, por causa das peças finas dos modelos.

    // Configurando os parâmetros da luz e do material.
    glLightfv(GL_LIGHT0, GL_POSITION, posicao); 
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambiente);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, difusa); 
    glLightfv(GL_LIGHT0, GL_SPECULAR, especular);
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialEspecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, brilho);

    // Uma fonte de luz (GL_LIGHT1, 2, 3...) para cada lâmpada pendurada sobre a mesa.
    for (i = 0; i < NUM_LAMPADAS; i++)
    {
        posicaoLampada[0] = lampadaX[i];
        posicaoLampada[1] = ALTURA_LAMPADA;
        posicaoLampada[2] = 0.0f;
        posicaoLampada[3] = 1.0f;

        glEnable(GL_LIGHT1 + i);
        glLightfv(GL_LIGHT1 + i, GL_POSITION, posicaoLampada);
        glLightfv(GL_LIGHT1 + i, GL_DIFFUSE, difusaLampada);
        glLightfv(GL_LIGHT1 + i, GL_SPECULAR, especularLampada);
    }
}

// Configura a projeção da cena, definindo o campo de visão e a proporção da janela.
void ConfiguraProjecao(int largura, int altura)
{
    if (altura == 0)
        altura = 1; // Evita divisão por zero.

    glViewport(0, 0, largura, altura); // Define a região da janela onde a cena será exibida.
    glMatrixMode(GL_PROJECTION); // Seleciona a matriz de projeção.
    glLoadIdentity(); // Reinicia a matriz de projeção.
    gluPerspective(45.0, (float)largura / (float)altura, 0.1, 30.0); // Campo de visão, proporção, perto e longe.
}

// Calcula a posição da câmera em volta do centro da mesa.
void ConfiguraCamera(void)
{
    // Converte os ângulos de graus para radianos para movimentação suave.
    float horizontalRad = anguloHorizontal * PI / 180.0f;
    float verticalRad = anguloVertical * PI / 180.0f;
    float cameraX;
    float cameraY;
    float cameraZ;

    cameraX = distanciaCamera * cosf(verticalRad) * sinf(horizontalRad);
    cameraY = 1.0f + distanciaCamera * sinf(verticalRad);
    cameraZ = distanciaCamera * cosf(verticalRad) * cosf(horizontalRad);

    gluLookAt(cameraX, cameraY, cameraZ,
              0.0, 1.0, 0.0,
              0.0, 1.0, 0.0);
}

// Função de desenho da cena.
void Desenha(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Limpa a imagem e as profundidades anteriores.

    glMatrixMode(GL_MODELVIEW); // Seleciona a matriz de visualização.
    glLoadIdentity(); // Reinicia a matriz de visualização.

    // Posição do observador, ponto observado e direção vertical.
    ConfiguraCamera();

    // Configura a iluminação da cena. 
    ConfiguraIluminacao();

    // Desenha os objetos da cena.
    DesenhaPiso();
    DesenhaParedes();
    DesenhaMesa(); 
    DesenhaPesMesa();
    DesenhaMesasBar();
    DesenhaLampadas();
    DesenhaCacapas();
    DesenhaBolas(); 
    if (bolaBrancaSelecionada == 1)
        DesenhaSelecaoBranca();
    if (bolaEmMovimento == 0 && bolaBrancaSelecionada == 1)
    {
        DesenhaTaco();
    }

    DesenhaVidros(); // Por último, pois é transparente.

    glutSwapBuffers(); // Troca o buffer desenhado pelo que será exibido.
}

// Função chamada quando a janela é redimensionada.
void Redimensiona(int largura, int altura)
{
    ConfiguraProjecao(largura, altura); // Atualiza a projeção quando a janela muda de tamanho.
}

// Seleciona a bola branca quando o clique do mouse cai dentro de seu raio.
void SelecionaBolaBranca(int botao, int estado, int x, int y)
{
    GLint viewport[4];
    GLdouble matrizModelo[16];
    GLdouble matrizProjecao[16];
    GLdouble pertoX, pertoY, pertoZ;
    GLdouble longeX, longeY, longeZ;
    GLdouble fator;
    GLdouble pontoX, pontoZ;
    float dx, dz;

    if (botao != GLUT_LEFT_BUTTON || estado != GLUT_DOWN || bolaEmMovimento == 1)
        return;

    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetDoublev(GL_MODELVIEW_MATRIX, matrizModelo);
    glGetDoublev(GL_PROJECTION_MATRIX, matrizProjecao);

    // Converte o clique da janela em uma reta no espaço 3D da cena.
    gluUnProject(x, viewport[3] - y, 0.0, matrizModelo, matrizProjecao, viewport,
                 &pertoX, &pertoY, &pertoZ);
    gluUnProject(x, viewport[3] - y, 1.0, matrizModelo, matrizProjecao, viewport,
                 &longeX, &longeY, &longeZ);

    if (fabs(longeY - pertoY) < 0.0001)
        return;

    // Encontra onde essa reta cruza o plano horizontal do centro da bola branca.
    fator = (ALTURA_TAMPO + RAIO_BOLA - pertoY) / (longeY - pertoY);
    pontoX = pertoX + fator * (longeX - pertoX);
    pontoZ = pertoZ + fator * (longeZ - pertoZ);
    dx = (float)pontoX - posicaoX[BOLA_BRANCA];
    dz = (float)pontoZ - posicaoZ[BOLA_BRANCA];

    bolaBrancaSelecionada = (dx * dx + dz * dz <= RAIO_BOLA * RAIO_BOLA);
    glutPostRedisplay();
}

// Restaura bolas, pontuação, mira e seleção para uma nova partida.
void ReiniciaPartida(void)
{
    InicializaBolas();
    pontos = 0;
    anguloTaco = 0.0f;
    distanciaTaco = 0.20f;
    desvioImpacto = 0.0f;
    bolaEmMovimento = 0;
    bolaBrancaSelecionada = 0;
    AtualizaTitulo();
}

// Controla a direção e a distância do taco com as setas do teclado.
void TeclasEspeciais(int tecla, int x, int y)
{
    (void)x;
    (void)y;

    if (bolaEmMovimento == 1 || bolaBrancaSelecionada == 0)
        return;

    switch (tecla)
    {
        case GLUT_KEY_LEFT:
            anguloTaco -= PASSO_ANGULO_TACO; // Gira a mira para a esquerda.
            if (anguloTaco < 0.0f)
                anguloTaco += 360.0f;
            break;

        case GLUT_KEY_RIGHT:
            anguloTaco += PASSO_ANGULO_TACO; // Gira a mira para a direita.
            if (anguloTaco >= 360.0f)
                anguloTaco -= 360.0f;
            break;

        case GLUT_KEY_UP:
            distanciaTaco -= PASSO_DISTANCIA_TACO; // Aproxima a ponta e reduz a força.
            if (distanciaTaco < DISTANCIA_MIN_TACO)
                distanciaTaco = DISTANCIA_MIN_TACO;
            break;

        case GLUT_KEY_DOWN:
            distanciaTaco += PASSO_DISTANCIA_TACO; // Afasta a ponta e aumenta a força.
            if (distanciaTaco > DISTANCIA_MAX_TACO)
                distanciaTaco = DISTANCIA_MAX_TACO;
            break;
    }

    glutPostRedisplay(); // Atualiza a posição do taco na janela.
}

// Atira a bola branca na direção do taco e com a força escolhida.
void AtiraBolaBranca(void)
{
    float forca;
    float velocidade;
    float anguloRad;

    if (bolaEmMovimento == 1 || bolaBrancaSelecionada == 0)
        return;

    forca = (distanciaTaco - DISTANCIA_MIN_TACO) /
            (DISTANCIA_MAX_TACO - DISTANCIA_MIN_TACO);
    velocidade = VELOCIDADE_MINIMA + forca * (VELOCIDADE_MAXIMA - VELOCIDADE_MINIMA);
    anguloRad = anguloTaco * PI / 180.0f;

    velocidadeX[BOLA_BRANCA] = velocidade * cosf(anguloRad);
    velocidadeZ[BOLA_BRANCA] = velocidade * sinf(anguloRad);
    velocidadeX[BOLA_BRANCA] += velocidade * (desvioImpacto / RAIO_BOLA) * 0.12f * -sinf(anguloRad);
    velocidadeZ[BOLA_BRANCA] += velocidade * (desvioImpacto / RAIO_BOLA) * 0.12f * cosf(anguloRad);
    bolaEmMovimento = 1;
}

// Trata a colisão de duas bolas com a mesma massa.
void ColideBolas(int primeira, int segunda)
{
    // Vetor que aponta do centro da primeira bola para a segunda.
    float dx = posicaoX[segunda] - posicaoX[primeira];
    float dz = posicaoZ[segunda] - posicaoZ[primeira];
    float distancia = sqrtf(dx * dx + dz * dz);
    float distanciaMinima = 2.0f * RAIO_BOLA; // Centros de bolas encostadas.

    // Só existe colisão quando os centros estão a menos de dois raios de distância.
    if (distancia > 0.0f && distancia < distanciaMinima)
    {
        // Vetor normal: direção exata em que uma bola empurra a outra.
        float normalX = dx / distancia;
        float normalZ = dz / distancia;
        float sobreposicao = distanciaMinima - distancia;
        float velocidadeRelativa;
        float impulso;

        // Separa as bolas para que elas não fiquem uma dentro da outra.
        posicaoX[primeira] -= normalX * sobreposicao / 2.0f;
        posicaoZ[primeira] -= normalZ * sobreposicao / 2.0f;
        posicaoX[segunda] += normalX * sobreposicao / 2.0f;
        posicaoZ[segunda] += normalZ * sobreposicao / 2.0f;

        // Mede se elas se aproximam ou se afastam na direção do impacto.
        velocidadeRelativa = (velocidadeX[segunda] - velocidadeX[primeira]) * normalX +
                              (velocidadeZ[segunda] - velocidadeZ[primeira]) * normalZ;

        // Só troca velocidade se as bolas estiverem se aproximando.
        if (velocidadeRelativa < 0.0f)
        {
            // Impulso transfere a velocidade entre bolas de mesma massa.
            impulso = -(1.0f + ELASTICIDADE_BOLAS) * velocidadeRelativa / 2.0f;
            velocidadeX[primeira] -= impulso * normalX;
            velocidadeZ[primeira] -= impulso * normalZ;
            velocidadeX[segunda] += impulso * normalX;
            velocidadeZ[segunda] += impulso * normalZ;
        }
    }
}

// Faz uma bola quicar nas quatro bordas da área jogável.
void ColideBorda(int indice)
{
    // O centro da bola deve ficar dentro da mesa, deixando um raio até a borda.
    float limiteX = COMPRIMENTO_MESA / 2.0f - RAIO_BOLA;
    float limiteZ = LARGURA_MESA / 2.0f - RAIO_BOLA;

    if (posicaoX[indice] > limiteX || posicaoX[indice] < -limiteX)
    {
        if (posicaoX[indice] > limiteX)
            posicaoX[indice] = limiteX;
        else
            posicaoX[indice] = -limiteX;

        velocidadeX[indice] = -velocidadeX[indice] * PERDA_NA_BORDA; // Inverte e perde energia.
    }

    if (posicaoZ[indice] > limiteZ || posicaoZ[indice] < -limiteZ)
    {
        if (posicaoZ[indice] > limiteZ)
            posicaoZ[indice] = limiteZ;
        else
            posicaoZ[indice] = -limiteZ;

        velocidadeZ[indice] = -velocidadeZ[indice] * PERDA_NA_BORDA; // Inverte e perde energia.
    }
}

// Atualiza posição, atrito e colisões das bolas a cada 16 milissegundos.
void AtualizaBolas(int valor)
{
    int i;
    int j;
    int algumaBolaEmMovimento = 0;

    (void)valor; // A GLUT exige esse parâmetro, mas não precisamos dele.

    // Só calculamos física enquanto ao menos uma bola estiver em movimento.
    if (bolaEmMovimento == 1)
    {
        for (i = 0; i < NUM_BOLAS; i++)
        {
            if (bolaAtiva[i] == 0)
                continue;

            // Nova posição = posição atual + velocidade x tempo.
            posicaoX[i] += velocidadeX[i] * 0.016f;
            posicaoZ[i] += velocidadeZ[i] * 0.016f;
            ColideBorda(i); // Corrige a posição se a bola bateu na borda.
        }

        // Compara todos os pares de bolas ativas uma única vez.
        for (i = 0; i < NUM_BOLAS; i++)
            for (j = i + 1; j < NUM_BOLAS; j++)
                if (bolaAtiva[i] == 1 && bolaAtiva[j] == 1)
                    ColideBolas(i, j);

        // Depois dos movimentos e choques, verifica se alguma bola entrou em caçapa.
        VerificaCacapas();

        for (i = 0; i < NUM_BOLAS; i++)
        {
            if (bolaAtiva[i] == 0)
                continue;

            velocidadeX[i] *= ATRITO; // Atrito reduz a velocidade horizontal.
            velocidadeZ[i] *= ATRITO; // Atrito reduz a velocidade no eixo Z da mesa.

            // Velocidades muito pequenas são consideradas paradas.
            if (fabsf(velocidadeX[i]) < VELOCIDADE_PARADA)
                velocidadeX[i] = 0.0f;
            if (fabsf(velocidadeZ[i]) < VELOCIDADE_PARADA)
                velocidadeZ[i] = 0.0f;

            if (velocidadeX[i] != 0.0f || velocidadeZ[i] != 0.0f)
                algumaBolaEmMovimento = 1;
        }

        // Quando todas param, o taco volta a aparecer.
        bolaEmMovimento = algumaBolaEmMovimento;
    }

    glutPostRedisplay(); // Pede que a janela desenhe as novas posições.
    glutTimerFunc(INTERVALO_ANIMACAO, AtualizaBolas, 0); // Agenda a próxima atualização.
}

// Controla os ângulos e a distância da câmera virtual.
void Teclado(unsigned char tecla, int x, int y)
{
    (void)x; // Posição horizontal do mouse, não usada neste momento.
    (void)y; // Posição vertical do mouse, não usada neste momento.

    // Controles de câmera com as teclas WASD para girar e inclinar, QE para zoom e R para resetar.
    switch (tecla)
    {
        case ' ':
            AtiraBolaBranca(); // Executa a tacada com a força escolhida.
            break;

        case 'j':
        case 'J':
            if (bolaEmMovimento == 0 && bolaBrancaSelecionada == 1 && desvioImpacto > -DESVIO_MAXIMO_IMPACTO)
            {
                desvioImpacto -= PASSO_IMPACTO; // Move o impacto para a esquerda do taco.
                if (desvioImpacto < -DESVIO_MAXIMO_IMPACTO)
                    desvioImpacto = -DESVIO_MAXIMO_IMPACTO;
            }
            break;

        case 'l':
        case 'L':
            if (bolaEmMovimento == 0 && bolaBrancaSelecionada == 1 && desvioImpacto < DESVIO_MAXIMO_IMPACTO)
            {
                desvioImpacto += PASSO_IMPACTO; // Move o impacto para a direita do taco.
                if (desvioImpacto > DESVIO_MAXIMO_IMPACTO)
                    desvioImpacto = DESVIO_MAXIMO_IMPACTO;
            }
            break;

        case 'k':
        case 'K':
            desvioImpacto = 0.0f; // Retorna o impacto para o centro da bola.
            break;

        case 't':
        case 'T':
            ReiniciaPartida(); // Reinicia a mesa e a pontuação.
            break;

        case 'a':
        case 'A':
            anguloHorizontal -= PASSO_ANGULO; // Gira a câmera para a esquerda (negativo).
            break;

        case 'd':
        case 'D':
            anguloHorizontal += PASSO_ANGULO; // Gira a câmera para a direita (positivo).
            break;

        case 'w':
        case 'W':
            if (anguloVertical < 75.0f)
                anguloVertical += PASSO_ANGULO; // Eleva a câmera (positivo).
            break;

        case 's':
        case 'S':
            if (anguloVertical > 5.0f)
                anguloVertical -= PASSO_ANGULO; // Abaixa a câmera (negativo).
            break;

        case 'e':
        case 'E':
            if (distanciaCamera > 4.5f)
                distanciaCamera -= PASSO_ZOOM; // Aproxima a câmera (negativo).
            break;

        case 'q':
        case 'Q':
            if (distanciaCamera < 16.0f)
                distanciaCamera += PASSO_ZOOM; // Afasta a câmera (positivo).
            break;

        case 'r':
        case 'R':
            anguloHorizontal = -90.0f; // Retorna para a visão inicial.
            anguloVertical = 25.0f;
            distanciaCamera = 8.0f;
            break;

    }

    glutPostRedisplay(); // Pede para a janela desenhar a cena novamente.
}

// Função principal do programa.
int main(int argc, char **argv)
{
    glutInit(&argc, argv); // Inicializa a GLUT.
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); // Buffer duplo, cores RGB e Z-Buffer.
    glutInitWindowSize(900, 600); 
    glutCreateWindow("Sinuca 3D"); 

    iniciarAudio("tarocheda.wav");
    atexit(pararAudio);

    glClearColor(0.55f, 0.55f, 0.55f, 1.0f); // Cor de fundo meio cinza.
    glEnable(GL_DEPTH_TEST); // Ativa o teste de profundidade.
    CarregaModelos(); // Lê os modelos 3D (precisa vir depois de criar a janela).

    glutDisplayFunc(Desenha); // Desenhar!
    glutReshapeFunc(Redimensiona); // Redimensionar a janela.
    glutKeyboardFunc(Teclado); // Controlar a câmera com o teclado.
    glutSpecialFunc(TeclasEspeciais); // Controlar o taco com as setas.
    glutMouseFunc(SelecionaBolaBranca); // Selecionar a bola branca com o mouse.
    InicializaBolas(); // Define as posições iniciais das bolas.
    AtualizaTitulo(); // Mostra a pontuação inicial no título da janela.
    glutTimerFunc(INTERVALO_ANIMACAO, AtualizaBolas, 0); // Inicia a atualização das colisões.
    glutMainLoop(); // Mantém a janela aberta e processa eventos.

    return 0;
}
