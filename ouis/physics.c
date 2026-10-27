#include "physics.h"
#include "follow_line.h"  /* fourni dans l'archive GDC */
#include <stdlib.h>       /* abs() */

/* ================================================================
   speedNorm2 — retourne vx² + vy²
   On compare TOUJOURS les carrés pour éviter sqrt (flottant).
   Ex : vx=3, vy=4 → 9+16=25 = MAX_SPEED², c'est la limite exacte.
   ================================================================ */
int speedNorm2(int vx, int vy)
{
    return vx * vx + vy * vy;
}

/* ================================================================
   computeFuelCost — formule exacte du sujet
   ∆c = -floor( (dvx² + dvy²) + 3*(vx² + vy²) ) / 2

   PIÈGE C : la division entière en C tronque vers zéro, pas vers -∞.
   -7 / 2 = -3 en C, mais floor(-7/2) = -4 en maths.
   Solution : calculer le numérateur (positif), diviser, puis nier.
   ================================================================ */
int computeFuelCost(int dvx, int dvy, int vx, int vy)
{
    int numerateur = (dvx * dvx + dvy * dvy) + 3 * (vx * vx + vy * vy);
    return -(numerateur / 2);
}

/* ================================================================
   isValidMove — vérifie TOUTES les contraintes avant d'accepter
   un mouvement. Retourne 1 si légal, 0 sinon.

   Vérifications dans l'ordre :
   1) L'accélération est dans les bornes valides
   2) La nouvelle vitesse ne dépasse pas la limite
   3) La case d'arrivée est dans les bornes de la carte
   4) Le chemin (cases intermédiaires) est libre de tout obstacle
   5) Pas de collision avec les autres voitures
   ================================================================ */
int isValidMove(const Map *map, const Car *car,
                int dvx, int dvy,
                const Car *others, int nbOthers)
{
    int new_vx, new_vy, new_x, new_y;
    int norm2;
    int maxAllowed2;
    Pos2Dint start, stop, p;
    InfoLine line;
    int i;

    /* 1) Bornes de l'accélération : dvx,dvy ∈ {-2,-1,0,1,2}
       (la validation boost/normal est faite dans listLegalAccelerations) */
    if (dvx < -2 || dvx > 2 || dvy < -2 || dvy > 2)
        return 0;

    /* 2) Calcul de la nouvelle vitesse */
    new_vx = car->vx + dvx;
    new_vy = car->vy + dvy;
    norm2  = speedNorm2(new_vx, new_vy);

    /* La limite dépend du terrain sur lequel on EST actuellement */
    maxAllowed2 = isSand(map, car->x, car->y) ? MAX_SAND_SPD2 : MAX_SPEED2;

    if (norm2 > maxAllowed2)
        return 0;

    /* 3) Nouvelle position */
    new_x = car->x + new_vx;
    new_y = car->y + new_vy;

    /* La position d'arrivée doit être dans la carte */
    if (!isInBounds(map, new_x, new_y))
        return 0;

    /* 4) Parcourir les cases intermédiaires avec follow_line
       On vérifie chaque case entre (car->x, car->y) et (new_x, new_y).
       La case de départ est ignorée (on y est déjà), on commence au
       point SUIVANT. */

    start.x = car->x;  
    start.y = car->y;
    stop.x  = new_x;   
    stop.y  = new_y;
    
    initLine(start.x, start.y, stop.x, stop.y, &line);

    while (nextPoint(&line, &p, +1) > 0) {
        /* Ignorer la case de départ */
        if (p.x == car->x && p.y == car->y)
            continue;

        /* La case doit être praticable */
        if (!isPassable(map, p.x, p.y))
            return 0;
        

        /* Vérifier la limite de vitesse dans le sable sur le trajet */
        if (isSand(map, p.x, p.y) && norm2 > MAX_SAND_SPD2)
            return 0;

        /* 5) Collision avec chaque autre voiture sur les cases intermédiaires */
        for (i = 0; i < nbOthers; i++) {
            if (p.x == others[i].x && p.y == others[i].y)
                return 0;
        }
    }

    return 1;
}

/* ================================================================
   applyAcceleration — met à jour la voiture après un mouvement
   ATTENTION : n'appeler que si isValidMove a retourné 1 !
   La consommation de carburant est toujours appliquée (sujet p.9).
   ================================================================ */
void applyAcceleration(Car *car, int dvx, int dvy)
{
    /* Le coût est calculé avec la vitesse AVANT modification */
    car->gas += computeFuelCost(dvx, dvy, car->vx, car->vy);

    /* Pénalité sable : -1 de carburant si on est sur du sable */
    /* Note : la Partie 1 devra appeler ça après avoir lu la carte */
    if (abs(dvx) == 2 || abs(dvy) == 2) {
        car->boosts -= 1;
    } //on retire un boost 

    car->vx += dvx;
    car->vy += dvy;
    car->x  += car->vx;
    car->y  += car->vy;// on met a jour la vitesse et la position

   
}


/* ================================================================
   listLegalAccelerations — énumère tous les coups légaux
   Remplit out[i][0]=dvx, out[i][1]=dvy pour chaque coup valide.
   useBoost=1 : inclure les coups avec |dvx|=2 ou |dvy|=2.
   useBoost=0 : seulement dvx,dvy ∈ {-1,0,1}.
   ================================================================ */
void listLegalAccelerations(const Map *map, const Car *car,
                            const Car *others, int nbOthers,
                            int out[][2], int *count,
                            int useBoost)
{
    int dvx, dvy;
    int isBoostMove;
    *count = 0;

    for (dvx = -2; dvx <= 2; dvx++) {
        for (dvy = -2; dvy <= 2; dvy++) {

            /* Détermine si c'est un coup "boost" (|dvx|=2 ou |dvy|=2) */
            isBoostMove = (abs(dvx) == 2 || abs(dvy) == 2);

            /* Ignorer les coups boost si on ne peut/veut pas booster */
            if (isBoostMove && !useBoost)
                continue;

            /* Ignorer les coups boost si plus de boosts disponibles */
            if (isBoostMove && car->boosts <= 0)
                continue;

            if (isValidMove(map, car, dvx, dvy, others, nbOthers)) {
                out[*count][0] = dvx;
                out[*count][1] = dvy;
                (*count)++;
            }
        }
    }
}
