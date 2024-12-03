#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>

#include "fonctions.h"  // Si cette fonction est définie ailleurs


// Initialisation de la structure d'une carte
void init_carte(struct carte *carte){
    carte->couleur = '0';
    carte->valeur[0] = '0';
    carte->valeur[1] = '\0';  // La chaîne de caractères doit être terminée par un caractère nul
    carte->point = 0.0;
}

// Création de l'ensemble des cartes, le jeu de tarot
void creer_jeu(struct carte jeu[78]){
    int i = 0;
    int j = 0;
    char couleurs[4] = {'C', 'K', 'P', 'T'};  // K = carreau, P = pique, T = trefle, C = coeur
    char* valeurs[14] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "V", "C", "D", "R"};  // Valeurs sous forme de chaînes de caractères
    float points[14] = {0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 1.5, 2.5, 3.5, 4.5};  // Points associés à chaque valeur
    char* atouts[22] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "20", "21", "*"}; // '*' = excuse
    float points_atouts[22] = {4.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 4.5, 4.5};  // Points associés à chaque atout
    // Création des cartes de couleurs
    for(i = 0; i < 4; i++){
        for(j = 0; j < 14; j++){
            jeu[i * 14 + j].couleur = couleurs[i];
            strcpy(jeu[i * 14 + j].valeur, valeurs[j]);  // Copie de la valeur dans la structure
            jeu[i * 14 + j].point = points[j];
        }
    }

    // Ajout des atouts
    for(i = 0; i < 22; i++){
        jeu[56 + i].couleur = ' ';  // Couleur vide représentée par un espace
        strcpy(jeu[56 + i].valeur, atouts[i]);  // Copie de la valeur de l'atout dans la structure
        jeu[56 + i].point = points_atouts[i];
    }
}

void creer_paquet (struct paquet *p){
    p->nb_cartes = 78;
    creer_jeu(p->jeu);
}

bool est_atout(struct carte *carte){
    if (carte->couleur == ' '){
        return true;
    }
    return false;
}

bool possede_couleur (struct carte *carte, char couleur){
    if (carte->couleur == couleur){
        return true;
    }
    return false;
}

bool accepter_carte(struct carte *cartePrecedente, struct carte *carteActuelle){
    //si precedent nul accepte
    if (cartePrecedente == NULL){
        return true;
    }
    //si couleur identique accepte
    if (cartePrecedente->couleur == carteActuelle->couleur){
        return true;
    }

    //si on a pas de carte de la meme couleur et que c'est un atout
    if (! possede_couleur(cartePrecedente, carteActuelle->couleur) && est_atout(carteActuelle)){
        return true;
    }

    //si on a pas de carte de la meme couleur et qu'on a pas d'atout
    if (! possede_couleur(cartePrecedente, carteActuelle->couleur) && ! possede_couleur (cartePrecedente, ' ')){
        return true;
    }

    // si on a un atout et que la carte actuelle est un atout, on regarde bien que la carte actuelle est plus forte
    if (est_atout(cartePrecedente) && est_atout(carteActuelle)){
        if (carteActuelle->point > cartePrecedente->point){
            return true;
        }
    }
}


int main(){
    struct carte jeu[78];
    creer_jeu(jeu);


    //creer paquet
    struct paquet p;
    creer_paquet(&p);
    for (int i = 0; i < p.nb_cartes; i++){
        printf("Carte %d: %c %s %f\n", i, p.jeu[i].couleur, p.jeu[i].valeur, p.jeu[i].point);
    }

    return 0;
}