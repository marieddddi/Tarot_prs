#include "fonctions.h"  


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
    char* valeurs[14] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "v", "C", "D", "R"};  // Valeurs sous forme de chaînes de caractères
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

int valeur_en_int(const char *valeur) {
    // Convertit une chaîne de caractères en entier
    return atoi(valeur);
}

bool accepter_carte(struct carte *carteLaplusForte, struct carte *carteActuelle, struct paquet *paquet, char couleurJouee){
    //si precedent nul accepte
    if (carteLaplusForte->couleur == 0){
        printf ("Carte precedente nulle\n");
        return true;
    }
    if (strcmp(carteActuelle->valeur, "*")==0) return true;
    //si couleur identique et que ce n'est pas un atout on accepte
    if (! est_atout(carteActuelle) && est_meme_couleur(carteActuelle, couleurJouee)){
        return true;
    }

    //si on a pas de carte de la meme couleur et que c'est un atout
    if (!est_atout(carteLaplusForte) && !possede_couleur(paquet, couleurJouee) && est_atout(carteActuelle)){
        return true;
    }

    //si on a pas de carte de la meme couleur et qu'on a pas d'atout
    if (! possede_couleur(paquet, carteLaplusForte->couleur) && ! possede_couleur(paquet, ' ')){
        return true;
    }

    // si on a un atout et que la carte actuelle est un atout, on regarde bien que la carte actuelle est plus forte
    if (est_atout(carteLaplusForte) && est_atout(carteActuelle)){
        //on affiche la carte precedente et la carte actuelle
        int valeurActuelle = valeur_en_int(carteActuelle->valeur);
        int valeurForte = valeur_en_int(carteLaplusForte->valeur);
        if (valeurActuelle > valeurForte){
            return true;
        }
        //si on peut mettre qu'un atout plus faible
        for (int i = 0; i < paquet->nb_cartes; i++){
            if (est_atout(&paquet->jeu[i]) && valeur_en_int(paquet->jeu[i].valeur) > valeurForte){
                return false;
            }
        }
        return true;
    }
    printf ("Vous ne pouvez pas jouer cette carte\n");
    return false;
}

//fonction pour savoir qui a la plus forte carte
int qui_a_la_plus_forte_carte(struct carte *carteLaPlusForte, struct carte *carteActuelle, char couleurJouee) {
    //si precedent nul accepte
    if (carteLaPlusForte->couleur == 0) {
        return 1;
    }
    if (strcmp(carteActuelle->valeur,"*")==0){
        return 0;
    }
    //si c'est la bonne couleur, on regarde qui a la plus forte carte (selon la valeur)
    if (carteActuelle->couleur== couleurJouee &&
        atoi(carteActuelle->valeur) > atoi(carteLaPlusForte->valeur) &&
        carteLaPlusForte->couleur==couleurJouee) {
        return 1;
    }
    if (carteActuelle->couleur==couleurJouee && carteActuelle->valeur < carteLaPlusForte->valeur && carteLaPlusForte->couleur==couleurJouee){
        return 0;
    }
    //si on joue un atout, on gagne
    if (est_atout(carteActuelle) && !est_atout(carteLaPlusForte)) {
        return 1;
    }/*
    if (est_atout(carteLaPlusForte) && !est_atout(carteActuelle)) {
        printf ("atout\n");
        return 0;
    }
    if (!est_atout (carteActuelle) && carteActuelle->couleur != couleurJouee){
        printf ("la ?\n");
        return 0;
    }*/
    return 0;
}


float calculer_points(struct paquet *paquet){
    float points = 0;
    for (int i = 0; i < paquet->nb_cartes; i++){
        points += paquet->jeu[i].point;
    }
    return points;
}

//definir le plus grand contrat: si que 0 on refait un tour !
float contrat (char *choix_contrat){
    if (strcmp(choix_contrat, "Passe") == 0){
        return 0;
    }
    else if (strcmp(choix_contrat, "Petite") == 0){
        return 1;
    }
    else if (strcmp(choix_contrat, "Garde") == 0){
        return 2;
    }
    return 3; //3 = erreur 
}


//dscore que l'on a fait, on doit voir si on a des bouts dans notre jeu -> a utiliser a la fin de la partie car on peut recuperer des bouts (1)
float score(struct paquet *paquet){
    int nb_bout = 0;
    float nb_points = 0;
    for (int i = 0; i < paquet->nb_cartes; i++){
        if ((strcmp(paquet->jeu[i].valeur,"1")==0 && paquet->jeu[i].couleur==' ') || (strcmp(paquet->jeu[i].valeur,"21")==0 && paquet->jeu[i].couleur==' ') || (strcmp(paquet->jeu[i].valeur,"*")==0 && paquet->jeu[i].couleur==' ')){
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

float score_final(struct paquet *paquet, char *choix_contrat, bool preneur){
    if (score(paquet) >= 0){
        if (preneur) return (25 + score(paquet)) * contrat(choix_contrat)*3;
        else return -(25 + score(paquet)) * contrat(choix_contrat);
    }
    else {
        if (preneur) return - (25 + score(paquet)) * contrat(choix_contrat)*3;
        else return (25 + score(paquet)) * contrat(choix_contrat);
    }
}

// Fonction qui permet de distribuer les cartes
void distribuer_cartes(struct paquet *jeu, struct paquet *j1, struct paquet *j2, struct paquet *j3, struct paquet *j4, struct paquet *chien) {
    // Mélange du jeu
    srand(time(NULL));  // Initialisation du générateur de nombres aléatoires
    for (int i = 0; i < jeu->nb_cartes; i++) {
        int j = rand() % jeu->nb_cartes;  // Sélection aléatoire d'une carte
        struct carte temp = jeu->jeu[i];  // Echange des cartes
        jeu->jeu[i] = jeu->jeu[j];
        jeu->jeu[j] = temp;
    }

    // Initialisation des paquets des joueurs et du chien
    j1->nb_cartes = 0;
    j2->nb_cartes = 0;
    j3->nb_cartes = 0;
    j4->nb_cartes = 0;
    chien->nb_cartes = 0;

    // Distribution des cartes
    for (int i = 0; i < jeu->nb_cartes; i++) {
        if (i < 18) {  // 18 cartes pour le joueur 1
            j1->jeu[j1->nb_cartes++] = jeu->jeu[i];
        }
        else if (i < 36) {  // 18 cartes pour le joueur 2
            j2->jeu[j2->nb_cartes++] = jeu->jeu[i];
        }
        else if (i < 54) {  // 18 cartes pour le joueur 3
            j3->jeu[j3->nb_cartes++] = jeu->jeu[i];
        }
        else if (i < 72) {  // 18 cartes pour le joueur 4
            j4->jeu[j4->nb_cartes++] = jeu->jeu[i];
        }
        else {  // Les cartes restantes pour le chien
            chien->jeu[chien->nb_cartes++] = jeu->jeu[i];
        }
    }
}

void afficher_paquet(struct paquet *paquet) {
    for (int i = 0; i < paquet->nb_cartes; i++) {
        printf("%c %s \n", paquet->jeu[i].couleur, paquet->jeu[i].valeur);
    }
}



/*
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

    //test distribution des cartes
    struct paquet j1, j2, j3, j4, chien;
    creer_paquet(&j1);
    creer_paquet(&j2);
    creer_paquet(&j3);
    creer_paquet(&j4);
    creer_paquet(&chien);
    distribuer_cartes(&p, &j1, &j2, &j3, &j4, &chien);
    printf ("Joueur 1\n");
    for (int i = 0; i < j1.nb_cartes; i++){
        printf("Carte %d: %c %s %f\n", i, j1.jeu[i].couleur, j1.jeu[i].valeur, j1.jeu[i].point);
    }
    printf ("Joueur 2\n");
    for (int i = 0; i < j2.nb_cartes; i++){
        printf("Carte %d: %c %s %f\n", i, j2.jeu[i].couleur, j2.jeu[i].valeur, j2.jeu[i].point);
    }
    printf ("Joueur 3\n");
    for (int i = 0; i < j3.nb_cartes; i++){
        printf("Carte %d: %c %s %f\n", i, j3.jeu[i].couleur, j3.jeu[i].valeur, j3.jeu[i].point);
    }
    printf ("Joueur 4\n");
    for (int i = 0; i < j4.nb_cartes; i++){
        printf("Carte %d: %c %s %f\n", i, j4.jeu[i].couleur, j4.jeu[i].valeur, j4.jeu[i].point);
    }
    printf ("Chien\n");
    for (int i = 0; i < chien.nb_cartes; i++){
        printf("Carte %d: %c %s %f\n", i, chien.jeu[i].couleur, chien.jeu[i].valeur, chien.jeu[i].point);
    }



    return 0;
}*/