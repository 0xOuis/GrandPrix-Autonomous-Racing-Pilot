#ifndef PHYSICS_H
#define PHYSICS_H

#include "common.h"

/* Carburant consommé par un mouvement (valeur négative) */
int computeFuelCost(int dvx, int dvy, int vx, int vy);

/* Norme² de la vitesse : vx*vx + vy*vy */
int speedNorm2(int vx, int vy);

/* 1 si le mouvement est légal, 0 sinon */
int isValidMove(const Map *map, const Car *car,
                int dvx, int dvy,
                const Car *others, int nbOthers);

/* Applique dvx,dvy à la voiture (modifie car en place) */
void applyAcceleration(Car *car, int dvx, int dvy);

/* Remplit out[][2] avec les accélérations légales, écrit le count */
/* useBoost=1 pour inclure les coups avec dvx ou dvy = ±2          */
void listLegalAccelerations(const Map *map, const Car *car,
                            const Car *others, int nbOthers,
                            int out[][2], int *count,
                            int useBoost);

#endif
