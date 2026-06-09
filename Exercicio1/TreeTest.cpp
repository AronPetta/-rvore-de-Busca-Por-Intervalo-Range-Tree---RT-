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

  int inserir_num = 90; // Numero qualquer para inserir na arvore
  int busca_num = 90; // Numero qualquer para buscar na arvore
  int remocao = 85; // Numero qualquer para tentar remover da arvore

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
    
    puts("\nPontos Da Arvore:");
    t.iterate(print);
    puts("");
    puts(""); 

    printf("-> ");
    tryInsert(t, inserir_num);
    puts(""); 

    printf("-> Buscando o elemento %d na árvore:\n", busca_num);
    if (auto it = t.find(busca_num); it != t.end()) {
      printf("Elemento encontrado!\n");
    } else {
      printf("Elemento não está na árvore.");
    }
    
  printf("\n-> Tentando remover o elemento %d da arvore:\n", remocao);
  if (t.erase(remocao))
  {
    printf("Sucesso: O elemento %d foi removido!\n", remocao);
  }
  else
  {
    printf("Falha: O elemento %d nao foi encontrado para remocao.\n", remocao);
  }
  }
  return 0; 
}