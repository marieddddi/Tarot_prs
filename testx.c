#include <ncurses.h>
#include <unistd.h>

#define MESSAGE "=== Mon Message Permanent ==="

int main() {
    // Initialisation de ncurses
    initscr();
    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE);  // Mode non bloquant pour getch()
    keypad(stdscr, TRUE);

    int ch;

    while (1) {
        clear();  // Efface l'écran avant de redessiner
        mvprintw(0, (COLS - sizeof(MESSAGE)) / 2, MESSAGE); // Affiche en haut centré
        refresh();  // Rafraîchit l'affichage

        ch = getch();
        if (ch == 'q') break; // Quitte si 'q' est pressé

        usleep(100000); // Pause pour éviter de surcharger le CPU
    }

    // Fin de ncurses
    endwin();
    return 0;
}