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


//structure d'une carte a supp car dans tarot.h

struct carte{
    char couleur;
    char valeur[3];
    float point;
};

//strucutre d'un paquet 
 struct paquet{
    struct carte jeu[78];
    int nb_cartes;
};

void init_carte(struct carte *carte);
void creer_jeu(struct carte jeu[78]);
void creer_paquet(struct paquet *p);
bool est_atout(struct carte *carte);
bool est_meme_couleur(struct carte *carte, char couleur);
bool possede_couleur(struct paquet *p, char couleur);
bool accepter_carte(struct carte *cartePrecedente, struct carte *carteActuelle, struct paquet *paquet);
float calculer_points(struct paquet *paquet);
float contrat(char *choix_contrat);
float score(struct paquet *paquet);
float score_final(struct paquet *paquet, char choix_contrat, bool preneur);
void distribuer_cartes(struct paquet *jeu, struct paquet *j1, struct paquet *j2, struct paquet *j3, struct paquet *j4, struct paquet *chien);
