#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#include "fonctions.h"  // Si cette fonction est définie ailleurs

int main(){
    struct carte jeu[78];
    creer_jeu(jeu);


    //creer paquet
    struct paquet p;
    creer_paquet(&p);
    for (int i = 0; i < p.nb_cartes; i++){
        printf("Carte %d : %c %s\n", i, p.jeu[i].couleur, p.jeu[i].valeur);
    }

    return 0;
}