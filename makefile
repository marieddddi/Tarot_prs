CC = gcc
CFLAGS = -Wall -g

# Détecte tous les fichiers .c commençant par "exo" et contenant un chiffre.
SRCS = $(wildcard *.c)
# Remplace l'extension .c par .o pour obtenir les fichiers objets correspondants.
OBJS = $(SRCS:.c=.o)
# Remplace l'extension .c par rien pour obtenir les noms des exécutables.
EXES = $(SRCS:.c=)

all: $(EXES)

# Règle générique pour créer chaque exécutable à partir de son fichier objet.
%: %.o
	$(CC) $(CFLAGS) -o $@ $^

# Règle générique pour créer chaque fichier objet à partir de son fichier source .c.
%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) $(EXES)