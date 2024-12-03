#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#include "fonctions.h"

//on initialise la structure d'une carte 
void init_carte(struct carte *carte){
    carte->couleur = '0';
    carte->valeur = '0';
}

//on creer l'ensemble des cartes, le jeu de tarot
void creer_jeu(struct carte jeu[78]){
    int i = 0;
    int j = 0;
    char couleurs[5] = {'C', 'K', 'P', 'T', NULL}; //K= carreau, P= pique, T= trefle, C= coeur
    char valeurs[14] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '10', 'V', 'C', 'D', 'R'};
    char atouts[22] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '10', '11', '12', '13', '14', '15', '16', '17', '18', '19', '20', '21', '*'}; // * = excuse
    for(i = 0; i < 4; i++){
        for(j = 0; j < 14; j++){
            jeu[i*14+j].couleur = couleurs[i];
            jeu[i*14+j].valeur = valeurs[j];
        }
    }
    //on ajoute les atouts
    for(i = 0; i < 22; i++){
        jeu[56+i].couleur = NULL;
        jeu[56+i].valeur = atouts[i];
    }
}

int main(){
    struct carte jeu[78];
    creer_jeu(jeu);
    for(int i = 0; i < 78; i++){
        printf("Carte %d: %c%c\n", i, jeu[i].couleur, jeu[i].valeur);
    }
    return 0;
}

