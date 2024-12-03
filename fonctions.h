//structure d'une carte a supp car dans tarot.h

struct carte{
    char couleur;
    char valeur[3];
};

//strucutre d'un paquet 
 struct paquet{
    struct carte jeu[78];
    int nb_cartes;
};