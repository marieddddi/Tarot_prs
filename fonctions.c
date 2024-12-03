#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#include "fonctions.h"  // Si cette fonction est définie ailleurs

// Exemple d'utilisation
int main() {
    struct paquet p;
    creer_paquet(&p);

    printf("Jeu complet:\n");
    for (int i = 0; i < p.nb_cartes; i++) {
        printf("Carte: %c%s, Points: %.1f\n", p.jeu[i].couleur, p.jeu[i].valeur, p.jeu[i].points);
    }

    printf("Score total du paquet: %.1f\n", calculer_score(&p));

    return 0;
}