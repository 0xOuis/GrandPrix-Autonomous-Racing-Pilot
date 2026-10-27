#ifndef COMMON_H
#define COMMON_H

#define MAX_W         200
#define MAX_H         200
#define MAX_SPEED     5
#define MAX_SPEED2    25   /* MAX_SPEED * MAX_SPEED */
#define MAX_SAND_SPD2 1    /* vitesse max dans le sable (norme² ≤ 1) */
#define BOOSTS_AT_START 5

/* ---- La carte ---- */
typedef struct {
    char cells[MAX_H][MAX_W];
    int  width;
    int  height;
} Map;

/* ---- Une voiture ---- */
typedef struct {
    int x, y;     /* position courante */
    int vx, vy;   /* vitesse courante  */
    int gas;      /* carburant restant */
    int boosts;   /* boosts restants   */
} Car;

/* ---- Utilitaires carte (définis dans parser.c) ---- */
char getCell   (const Map *map, int x, int y);
int  isInBounds(const Map *map, int x, int y);
int  isPassable(const Map *map, int x, int y);
int  isSand    (const Map *map, int x, int y);
int  isFinish  (const Map *map, int x, int y);

#endif
