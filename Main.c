#include <stdio.h>
#include <stdlib.h>

// Fonction pour initialiser la matrice d'adjacence
int** initialiserGraphe(int n) {
    // Allouer de la mémoire pour la matrice d'adjacence
    int** matrice = (int**)malloc(n * sizeof(int*));
    for (int i = 0; i < n; i++) {
        matrice[i] = (int*)malloc(n * sizeof(int));
    }

    // Initialiser la matrice d'adjacence
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i == j) {
                matrice[i][j] = 0;
            } else if (j == (i + 1) % n) {
                matrice[i][j] = (i == n - 1) ? 5 : 2;
            } else {
                matrice[i][j] = 5 * n;
            }
        }
    }

    return matrice;
}

// Fonction pour libérer la mémoire allouée pour la matrice d'adjacence
void libererGraphe(int** matrice, int n) {
    for (int i = 0; i < n; i++) {
        free(matrice[i]);
    }
    free(matrice);
}

// Fonction pour afficher la matrice d'adjacence
void afficherGraphe(int** matrice, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("%d ", matrice[i][j]);
        }
        printf("\n");
    }
}

int main() {
    int n = 5; // Exemple avec 5 noeuds
    int** graphe = initialiserGraphe(n);

    // Afficher la matrice d'adjacence
    afficherGraphe(graphe, n);

    // Libérer la mémoire allouée
    libererGraphe(graphe, n);

    return 0;
}
