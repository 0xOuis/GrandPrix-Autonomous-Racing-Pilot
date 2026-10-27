#include <stdio.h>
#include <assert.h>
#include "common.h"
#include "physics.h"

/* ---- Helpers pour construire une Map de test ---- */

/* Construit une carte simple 10x5 avec une ligne droite
   ##########
   ##########
   ====######   <- arrivée
   ##########
   ##########
*/
static void makeSimpleMap(Map *m)
{
    int x, y;
    m->width  = 10;
    m->height = 5;
    for (y = 0; y < m->height; y++)
        for (x = 0; x < m->width; x++)
            m->cells[y][x] = '#';
    /* ligne d'arrivée en y=2 */
    for (x = 0; x < 4; x++)
        m->cells[2][x] = '=';
    /* du sable en (5,2) */
    m->cells[2][5] = '~';
}

/* Implémentation minimale des utilitaires de parser.c
   (pour que les tests compilent sans parser.c) */
char getCell(const Map *m, int x, int y)
{
    if (x < 0 || x >= m->width || y < 0 || y >= m->height)
        return '.';
    return m->cells[y][x];
}
int isInBounds(const Map *m, int x, int y)
{ return x >= 0 && x < m->width && y >= 0 && y < m->height; }
int isPassable(const Map *m, int x, int y)
{
    char c = getCell(m, x, y);
    return c == '#' || c == '=' || c == '~';
}
int isSand(const Map *m, int x, int y)
{ return getCell(m, x, y) == '~'; }
int isFinish(const Map *m, int x, int y)
{ return getCell(m, x, y) == '='; }

/* ================================================
   TESTS
   ================================================ */

static void test_speedNorm2(void)
{
    printf("Test speedNorm2... ");
    assert(speedNorm2(0, 0) == 0);
    assert(speedNorm2(3, 4) == 25);   /* norme = 5 exactement */
    assert(speedNorm2(5, 0) == 25);
    assert(speedNorm2(4, 4) == 32);   /* > 25, invalide */
    printf("OK\n");
}

static void test_computeFuelCost(void)
{
    printf("Test computeFuelCost... ");
    /* Arrêt complet : dvx=0,dvy=0,vx=0,vy=0 → 0/2 = 0 */
    assert(computeFuelCost(0, 0, 0, 0) == 0);

    /* dvx=1,dvy=0,vx=0,vy=0 → -(1+0)/2 = 0 (tronqué) */
    assert(computeFuelCost(1, 0, 0, 0) == 0);

    /* dvx=1,dvy=1,vx=0,vy=0 → -(2)/2 = -1 */
    assert(computeFuelCost(1, 1, 0, 0) == -1);

    /* dvx=0,dvy=0,vx=2,vy=0 → -(0 + 3*4)/2 = -6 */
    assert(computeFuelCost(0, 0, 2, 0) == -6);

    /* dvx=1,dvy=0,vx=2,vy=0 → -(1 + 12)/2 = -6 */
    assert(computeFuelCost(1, 0, 2, 0) == -6);

    /* dvx=0,dvy=0,vx=3,vy=0 → -(0 + 27)/2 = -13 */
    assert(computeFuelCost(0, 0, 3, 0) == -13);
    printf("OK\n");
}

static void test_isValidMove_basic(void)
{
    Map m;
    Car me;
    printf("Test isValidMove (basique)... ");

    makeSimpleMap(&m);

    /* Voiture à (2,2), vitesse nulle */
    me.x = 2; me.y = 2;
    me.vx = 0; me.vy = 0;
    me.gas = 100; me.boosts = 5;

    /* Accélération (1,0) → nouvelle vitesse (1,0) → nouvelle pos (3,2) : piste */
    assert(isValidMove(&m, &me, 1, 0, NULL, 0) == 1);

    /* Accélération (0,0) → reste en place : valide */
    assert(isValidMove(&m, &me, 0, 0, NULL, 0) == 1);

    printf("OK\n");
}

static void test_isValidMove_outofbounds(void)
{
    Map m;
    Car me;
    printf("Test isValidMove (hors-carte)... ");

    makeSimpleMap(&m);

    /* Voiture au bord gauche (0,2), vitesse (-1,0) */
    me.x = 0; me.y = 2;
    me.vx = -1; me.vy = 0;
    me.gas = 100; me.boosts = 0;

    /* dvx=0 → nouvelle vitesse (-1,0) → pos (-1,2) hors carte → invalide */
    assert(isValidMove(&m, &me, 0, 0, NULL, 0) == 0);

    printf("OK\n");
}

static void test_isValidMove_speedlimit(void)
{
    Map m;
    Car me;
    printf("Test isValidMove (vitesse limite)... ");

    makeSimpleMap(&m);

    /* Voiture à (5,0), vitesse (5,0) = max */
    me.x = 5; me.y = 0;
    me.vx = 5; me.vy = 0;
    me.gas = 100; me.boosts = 0;

    /* dvx=0 → vitesse inchangée (5,0) → norme²=25 ≤ 25 : OK */
    /* (si la destination est dans la carte et praticable) */

    /* dvx=1 → vitesse (6,0) → norme²=36 > 25 → invalide */
    assert(isValidMove(&m, &me, 1, 0, NULL, 0) == 0);

    /* dvx=-1 → vitesse (4,0) → norme²=16 ≤ 25 → valide si piste libre */
    /* (destination (9,0) sur la carte ? oui) */
    assert(isValidMove(&m, &me, -1, 0, NULL, 0) == 1);

    printf("OK\n");
}

static void test_isValidMove_sand(void)
{
    Map m;
    Car me;
    printf("Test isValidMove (sable)... ");

    makeSimpleMap(&m);

    /* Voiture à (4,2) sur la piste, vitesse (1,0)
       → nouvelle pos (5,2) = sable, nouvelle vitesse (1,0), norme²=1 : OK */
    me.x = 4; me.y = 2;
    me.vx = 1; me.vy = 0;
    me.gas = 100; me.boosts = 0;
    assert(isValidMove(&m, &me, 0, 0, NULL, 0) == 1); /* arrive dans sable à v=1 */

    /* Si vitesse (2,0) et on entre dans le sable → norme²=4 > 1 → invalide */
    me.vx = 2; me.vy = 0;
    assert(isValidMove(&m, &me, 0, 0, NULL, 0) == 0);

    printf("OK\n");
}

static void test_listLegalAccelerations(void)
{
    Map m;
    Car me;
    int moves[25][2];
    int count = 0;

    printf("Test listLegalAccelerations... ");

    makeSimpleMap(&m);

    /* Voiture au milieu (5,2), vitesse nulle, pas de boost */
    me.x = 5; me.y = 2;
    me.vx = 0; me.vy = 0;
    me.gas = 100; me.boosts = 0;

    listLegalAccelerations(&m, &me, NULL, 0, moves, &count, 0);

    /* Depuis le centre avec v=0, beaucoup de coups normaux valides */
    printf("(%d coups legaux) ", count);
    assert(count > 0);
    assert(count <= 9); /* au max 9 sans boost */

    printf("OK\n");
}

static void test_applyAcceleration(void)
{
    Map m;
    Car me;
    printf("Test applyAcceleration... ");

    makeSimpleMap(&m);

    me.x = 2; me.y = 2;
    me.vx = 0; me.vy = 0;
    me.gas = 100; me.boosts = 5;

    /* dvx=1, dvy=0 : coût = -(1²+0² + 3*(0²+0²))/2 = 0 */
    applyAcceleration(&me, 1, 0);

    assert(me.vx == 1);
    assert(me.vy == 0);
    assert(me.x  == 3);   /* 2 + vx_new(1) */
    assert(me.y  == 2);
    assert(me.gas == 100); /* coût = 0 car numérateur = 1, 1/2 = 0 */

    /* Tour 2 : dvx=1,dvy=0, vx=1,vy=0 → coût = -(1 + 3)/2 = -2 */
    applyAcceleration(&me, 1, 0);
    assert(me.vx == 2);
    assert(me.x  == 5);   /* 3 + 2 */
    assert(me.gas == 98);  /* 100 - 2 */

    printf("OK\n");
}

int main(void)
{
    printf("=== Tests physics.c ===\n\n");

    test_speedNorm2();
    test_computeFuelCost();
    test_isValidMove_basic();
    test_isValidMove_outofbounds();
    test_isValidMove_speedlimit();
    test_isValidMove_sand();
    test_listLegalAccelerations();
    test_applyAcceleration();

    printf("\n=== Tous les tests OK ===\n");
    return 0;
}
