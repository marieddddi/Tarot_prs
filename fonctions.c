#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

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

bool est_meme_couleur (struct carte *carte, char couleur){
    if (carte->couleur == couleur){
        return true;
    }
    return false;
}

bool possede_couleur(struct paquet *p, char couleur){
    for (int i = 0; i < p->nb_cartes; i++){
        if (p->jeu[i].couleur == couleur){
            return true;
        }
    }
    printf ("Vous ne possedez pas de carte de cette couleur\n");
    return false;
}

bool accepter_carte(struct carte *cartePrecedente, struct carte *carteActuelle, struct paquet *paquet){
    //si precedent nul accepte
    if (cartePrecedente == NULL){
        printf ("Carte precedente nulle\n");
        return true;
    }
    //si couleur identique et que ce n'est pas un atout on accepte
    if (! est_atout(carteActuelle) && est_meme_couleur(carteActuelle, cartePrecedente->couleur)){
        printf ("Couleur identique\n");
        return true;
    }

    //si on a pas de carte de la meme couleur et que c'est un atout
    if (! possede_couleur(paquet, cartePrecedente->couleur) && est_atout(carteActuelle)){
        printf ("Atout\n");
        return true;
    }

    //si on a pas de carte de la meme couleur et qu'on a pas d'atout
    if (! possede_couleur(paquet, cartePrecedente->couleur) && ! possede_couleur(paquet, ' ')){
        printf ("Pas de couleur et pas d'atout\n");
        return true;
    }

    // si on a un atout et que la carte actuelle est un atout, on regarde bien que la carte actuelle est plus forte
    if (est_atout(cartePrecedente) && est_atout(carteActuelle)){
        //on affiche la carte precedente et la carte actuelle
        printf ("Atout\n");
        printf ("Carte precedente: %c %s %f\n", cartePrecedente->couleur, cartePrecedente->valeur, cartePrecedente->point);
        printf ("Carte actuelle: %c %s %f\n", carteActuelle->couleur, carteActuelle->valeur, carteActuelle->point);
        if (carteActuelle->valeur[0] > cartePrecedente->valeur[0]){
            printf ("Atout plus fort\n");
            return true;
        }
        //si on peut mettre qu'un atout plus faible
        for (int i = 0; i < paquet->nb_cartes; i++){
            if (est_atout(&paquet->jeu[i]) && paquet->jeu[i].valeur[0] > cartePrecedente->valeur[0]){
                printf ("Atout plus fort\n");
                return false;
            }
        }
        printf ("Atout plus faible pas le choix\n");
        return true;
    }
    printf ("Vous ne pouvez pas jouer cette carte\n");
    return false;
}


float calculer_points(struct paquet *paquet){
    float points = 0;
    for (int i = 0; i < paquet->nb_cartes; i++){
        points += paquet->jeu[i].point;
    }
    return points;
}

//definir le plus grand contrat: si que 0 on refait un tour !
float contrat (char choix_contrat){
    if (strcmp(choix_contrat, "Passe") == 0){
        return 0;
    }
    if (strcmp(choix_contrat, "Petite") == 0){
        return 1;
    }
    if (strcmp(choix_contrat, "Garde") == 0){
        return 2;
    }
    return 3; //3 = erreur 
}

//dscore que l'on a fait, on doit voir si on a des bouts dans notre jeu -> a utiliser a la fin de la partie car on peut recuperer des bouts (1)
float score(struct paquet *paquet){
    int nb_bout = 0;
    float nb_points = 0;
    for (int i = 0; i < paquet->nb_cartes; i++){
        if ((paquet->jeu[i].valeur[0] == '1' && paquet->jeu[i].couleur == ' ') || (paquet->jeu[i].valeur[0] == '21' && paquet->jeu[i].couleur == ' ') || (paquet->jeu[i].valeur[0] == '*' && paquet->jeu[i].couleur == ' ')){
            nb_bout += 1;
        }
    }
    
    //on definit le nombre de points a atteindre
    switch (nb_bout)
    {
    case 1:
        nb_points = 51;
        break;
    case 2:
        nb_points = 41;
        break;
    case 3:
        nb_points = 36;
        break;
    
    default:
        nb_points = 56;
        break;
    }
    return calculer_points(paquet) - nb_points;
}

float score_final(struct paquet *paquet, char choix_contrat, bool preneur){
    if (score(paquet) >= 0){
        if (preneur) return (25 + score(paquet)) * contrat(choix_contrat)*3;
        else return -(25 + score(paquet)) * contrat(choix_contrat);
    }
    else {
        if (preneur) return - (25 + score(paquet)) * contrat(choix_contrat)*3;
        else return (25 + score(paquet)) * contrat(choix_contrat);
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
    //on creait une carte precedente
    struct carte cartePrecedente;
    init_carte(&cartePrecedente);
    cartePrecedente.couleur = 'D';
    cartePrecedente.valeur[0] = '3';
    cartePrecedente.valeur[1] = '\0';
    cartePrecedente.point = 0.5;
    //on choisit une carte actuelle
    struct carte carteActuelle;
    init_carte(&carteActuelle);
    carteActuelle.couleur = 'T';
    carteActuelle.valeur[0] = '5';
    carteActuelle.valeur[1] = '\0';
    carteActuelle.point = 0.5;
   
   //on cree une carte atout 
    struct carte carteAtout;
    init_carte(&carteAtout);
    carteAtout.couleur = 'C';
    carteAtout.valeur[0] = '5';
    carteAtout.valeur[1] = '\0';
    carteAtout.point = 0.5;
    
   //on cree un jeu de 2 cartes 
    struct paquet jeu2;
    creer_paquet(&jeu2);
    jeu2.jeu[0] = carteAtout;
    jeu2.jeu[1] = carteActuelle;
    jeu2.nb_cartes = 2;
    //on teste si on accepte la carte
    if (accepter_carte(&cartePrecedente, &carteAtout, &jeu2)){
        printf("On accepte la carte\n");
    } else {
        printf("On refuse la carte\n");
    }

    printf ("Points: %f\n", calculer_points(&jeu2));
    return 0;
}