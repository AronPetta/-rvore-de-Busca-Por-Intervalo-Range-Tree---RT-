#include "AVL.h"
#include <cstdio>
#include <random>
#include <vector>

using namespace tcii;

void print(const int& i) {
  printf("%d ", i);
}

template <typename C>
void tryInsert(avl::Tree<int, C>& t, int i) {
  auto [it, success] = t.insert(i);
  printf("Inserindo %d: %s\n", i, success ? "Sucesso" : "Falhou (Já existe)");
}

template <typename T>
using C = std::greater<T>;

int main() {
  std::random_device rd;
  std::mt19937 gen(rd());
  
  std::uniform_int_distribution<> dis(1, 100);

  std::vector<int> tamanhos_testes = {5, 10, 20}; // Vezes que a árvore será testada com diferentes tamanhos de entrada.

  int inserir_num = 777; // Numero qualquer para inserir na arvore
  for (int tamanho : tamanhos_testes) {
    avl::Tree<int, C<int>> t;
    
    printf("\n==================================\n");
    printf("          CRIANDO ÁRVORE\n");
    printf("==================================\n");

    // Gerando e inserindo pontos aleatórios
    while (t.size() < static_cast<size_t>(tamanho)) {
      int numero_aleatorio = dis(gen);
      t.insert(numero_aleatorio); 
    }

    printf("Árvore criada! Tamanho: %d | Altura: %d\n", t.size(), t.height());
    
    puts("\nPontos Da Arvore:");
    t.iterate(print);
    puts(""); 

    puts("\nInserindo ponto qualquer:");
    tryInsert(t, inserir_num);

    puts("\nBusca do elemento inserido:");
    if (auto it = t.find(inserir_num); it != t.end()) {
      printf("Elemento %d encontrado!\n", inserir_num);
    } else {
      printf("Elemento %d não está na árvore.", inserir_num);
    }
  }

  return 0; 
}
