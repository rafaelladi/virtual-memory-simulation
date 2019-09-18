#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

int tampag = 1024, tamfis = 3072, paginas, desloc, numquad, pagefault=0;

typedef struct pg {
  int ind, br, cont; //INDICE, BIT DE REFERENCIA, CONTADOR
  struct pg *fis; //PONTEIRO QUE GUARDA ENDERECO DA MEMORIA PRINCIPAL
} pagina;

typedef struct p {
  char nome[3]; //NOME DO PROCESSO
  int tam, numpag; //TAMANHO E NUMERO DE PAGINAS DO PROCESSO
  pagina *tab; //TABELA DE PAGINAS DO PROCESSO
} processo;

typedef struct n { //LISTA DE PROCESSOS
  processo proc;
  struct n *prox;
} listaEncadeada;

listaEncadeada *lista; //LISTA DE PROCESSOS
pagina **mp; //REPRESENTA A MEMORIA PRINCIPAL

void clrscr();
void inicializa_mp();
void inicializa_lista();
processo cria_processo(char n[3], int t, char u[2]);
listaEncadeada * cria_nodo(processo pro);
void insere_lista(listaEncadeada *novo);
void inicializa_tab(processo p);
void def_end();
pagina * procura_processo(char n[], int *tam, int *np);
int conv(int vet[], int n);
int verifica_endereco(int end[], int n, int np, int *ind);
void imprime_tabela(pagina *tab, int np);
int lru_puro(pagina *tab, int ind, int np);
int lru_aproximado(pagina *tab, int ind, int np);
int segunda_chance(pagina *tab, int ind, int np);
void le_arq(int op);
void reseta_processos();
int menu();

void clrscr() { //LIMPA A TELA
  printf("\33[H\33[2J");
}

void inicializa_mp() { //ALOCA ESPACO PARA A MEMORIA FISICO
  int a;
  mp = (pagina **) malloc(numquad * sizeof(pagina *));
  for(a=0;a<numquad;a++) {
    mp[a] = NULL;
  }
}

void inicializa_lista() { //INICIALIZA LISTA DE PROCESSOS
  lista = NULL;
}

processo cria_processo(char n[3], int t, char u[2]) { //CRIA UM NOVO PROCESSO
  processo novo;
  strcpy(novo.nome, n);
  if(!strcmp(u, "MB")) { //CONVERTE MB PARA KB
    novo.tam = t * 1000;
  } else {
    novo.tam = t;
  }
  return novo; //RETORNO O PROCESSO CRIADO
}

listaEncadeada * cria_nodo(processo pro) { //ALOCA ESPACO PARA UM NOVO NODO NA LISTA
  int a;
  listaEncadeada *novo;
  novo = (listaEncadeada *) malloc(sizeof(listaEncadeada));
  novo->proc = pro;
  return novo;
}

void insere_lista(listaEncadeada *novo) { //INSERE UM NOVO NODO NA LISTA
  listaEncadeada *aux;
  novo->prox = NULL;
  if(lista == NULL) { //PARA LISTA VAZIA
    lista = novo;
  } else {
    aux = lista;
    while(aux->prox != NULL) { //INSERE NOVO PROCESSO NO FINAL DA LISTA
      system("read -p \"novo processo na lista\" saindo");
      aux = aux->prox;
    }
    aux->prox = novo;
  }
}

void inicializa_tab(processo p) { //INICIALIZA TABELA DE PAGINAS
  int a;
  for(a=0;a<p.numpag;a++) {
    p.tab[a].ind = a;
    p.tab[a].br = 0;
    p.tab[a].fis = NULL;
    p.tab[a].cont = 0;
  }
}

void def_end() { //DEFINE BITS DE DESLOCAMENTO
  int a = tampag, i, cont=0;
  for(i=0;a>0;i++) { //CONTA TAMANHO DO NUMERO EM BINARIO
    a/=2;
  }
  cont = pow(2, i);
  if(cont > tampag) //SEMPRE VAI ENTRAR
    i--; //DIMINUI UM BIT
  desloc = i;
  printf("%d bits para deslocamento\n", desloc);
}

pagina * procura_processo(char n[], int *tam, int *np) { //PROCURA PROCESSO POR NOME E RETORNA A TABELA DE PAGINAS
  listaEncadeada *aux;
  if(lista == NULL) { //PARA LISTA VAZIA
    printf("Lista vazia!\n");
    return NULL;
  }
  aux = lista;
  while(aux != NULL) { //PROCURA ATE O ULTIMO PROCESSO
    if(!strcmp(aux->proc.nome, n)) {
      *tam = aux->proc.tam; //SALVA TAMANHO DO PROCESSO EM TAM PARA SER USADO DEPOIS
      *np = aux->proc.numpag; //SALVA NUMERO DE PAGINAS DO PROCESSO EM NP PARA SER USADO DEPOIS
      return aux->proc.tab; //RETORNO A TABELA DE PAGINAS DO PROCESSO
    }
    aux = aux->prox;
  }
}

int conv(int vet[], int n) { //TRANSFORMA UM VETOR DE NUMEROS (EX:[1][0][1][1]) EM UM INTEIRO
  int i, res=0;
  for(i=n;i>=0;i--) { //N = TAMANHO DO NUMERO (VETOR)
    res+=vet[i]*pow(10, i);
  }
  return res; //RETORNA O INTEIRO OBTIDO
}

int verifica_endereco(int end[], int n, int np, int *ind) { //VERIFICA SE ENDERECO REQUERIDO E VALIDO
  int a, b=0, d[100], bin, dec=0, x, base=1;
  for(a=0;a<100;a++) { //INICIALIZA VETOR DE INTEIRO
    d[a]=0;
  }
  for(a=desloc-1;a>=0;a--) { //SALVA DO BIT DE DESLOCAMENTO ATE O PRIMEIRO BIT NO VETOR D
    d[a]=end[a];
  }
  bin = conv(d, desloc); //CONVERTE O VETOR D PARA UM NUMERO INTEIRO (BINARIO)
  a = bin;
  while(a > 0) { //TRANSFORMA  BINARIO EM DECIMAL
    x = a % 10;
    dec += x * base;
    a /= 10 ;
    base *= 2;
  }
  if(dec > tampag) //SE DECIMAL (DESLOCAMENTO) FOR MAIOR QUE O TAMANHO DA PAGINA O ENDERECO REQUERIDO E INVALIDO
    return 0;
  else {
    for(a=0;a<100;a++) { //INICIALIZA VETOR DE INTEIRO
      d[a]=0;
    }
    b=n;
    for(a=n-desloc;a>=0;a--) { //SALVA DO BIT DE INDICE ATE BIT DE DESLOCAMENTO NO VETOR D
      d[a]=end[b--];
    }
    bin = conv(d, (n-desloc)); //CONVERTE O VETOR D PARA UM NUMERO INTEIRO (BINARIO)
    a = bin;
    dec=0;
    base=1;
    while(a > 0) { //TRANSFORMA  BINARIO EM DECIMAL
      x = a % 10;
      dec += x * base;
      a /= 10 ;
      base *= 2;
    }
    if(dec>=np) { //SE DECIMAL FOR MAIOR QUE O NUMERO DE PAGINAS O ENDERECO REQUERIDO E INVALIDO
      return 0;
    }
    *ind = dec; //SALVA INDICE EM IND PARA SER USADO DEPOIS
  }
}

int lru_puro(pagina *tab, int ind, int np) { //ALGORITMO LRU PURO COM CONTADOR
  int a=0, b, sub, maior = 0, maiorcont, nemmp=0, x;
  float perc;
  while((mp[a] != NULL) && (a < numquad)) { //ACRESCENTA UM NO CONTADOR DE TODAS AS PAGINAS EM MEMORIA
    mp[a]->cont++;
    a++;
  }
  for(a=0;a<np;a++) { //PROCURA A PAGINA NA TABELA
    if(tab[a].ind == ind) { //ACHA PAGINA
      tab[a].cont = 0; //RESETA CONTADOR
      if(tab[a].fis != NULL) { //PARA PAGINA JA EM MEMORIA
        for(x=0;x<np;x++) { //CALCULA PERCENTUAL DO PROCESSO EM MEMORIA
          if(tab[x].fis != NULL)
            nemmp++;
        }
        perc = 100*nemmp/np;
        printf("Pagina ja em memoria\n%.2f%% do processo em memoria\n", perc);
        imprime_tabela(tab, np);
        system("read -p \"\n\" saindo");
        tab[a].cont = 0;
        return 1;
      } else {
        for(b=0;b<numquad;b++) { //PROCURA ESPACO LIVRE NA MP
          if(mp[b] == NULL) { //PARA ESPACO LIVRE
            pagefault++;
            mp[b] = &tab[a];
            tab[a].fis = mp[b];
            tab[a].cont = 0;
            for(x=0;x<np;x++) { //CALCULA PERCENTUAL DO PROCESSO EM MEMORIA
              if(tab[x].fis != NULL)
                nemmp++;
            }
            perc = 100*nemmp/np;
            printf("Page Fault\n%.2f%% do processo em memoria\n", perc);
            imprime_tabela(tab, np);
            system("read -p \"\n\" saindo");
            return 1;
          }
        }
        maiorcont = mp[0]->cont;
        for(b=1;b<numquad;b++) { //MP CHEIA, PROCURA PAGINA PARA SUBSTITUIR(CONTADOR MAIOR)
          if(maiorcont < mp[b]->cont) {
            maiorcont = mp[b]->cont;
            maior = b;
          }
        }
        pagefault++;
        mp[maior]->fis = NULL; //MUDA TABELA PARA INDICAR QUE PAGINA NAO ESTA MAIS EM MEMORIA
        mp[maior] = &tab[a]; //SUBSTITUI A PAGINA
        tab[a].fis = mp[maior];
        tab[a].cont = 0;
        for(x=0;x<np;x++) { //CALCULA PERCENTUAL DO PROCESSO EM MEMORIA
          if(tab[x].fis != NULL)
            nemmp++;
        }
        perc = 100*nemmp/np;
        printf("Memoria cheia, substitui paginas\nPage Fault\n%.2f%% do processo em memoria\n", perc);
        imprime_tabela(tab, np);
        system("read -p \"\n\" saindo");
        return 1;
      }
    }
  }
}

int lru_aproximado(pagina *tab, int ind, int np) { //ALGORITMO LRU APROXIMADO USANDO BIT DE REFERENCIA
  int a, b, c, x, nemmp=0;
  float perc;
  for(a=0;a<np;a++) { //PROCURA PAGINA NA TABELA
    if(tab[a].ind == ind) { //ACHA PAGINA
      tab[a].br = 1; //SETA BIT DE REFERENCIA PARA 1
      if(tab[a].fis != NULL) { //PARA PAGINA JA EM MEMORIA
        for(x=0;x<np;x++) { //CALCULA PERCENTUAL DO PROCESSO EM MEMORIA
          if(tab[x].fis != NULL)
            nemmp++;
        }
        perc = 100*nemmp/np;
        printf("Pagina ja em memoria\n%.2f%% do processo em memoria\n", perc);
        imprime_tabela(tab, np);
        system("read -p \"\n\" saindo");
        tab[a].br = 1;
        return 1;
      } else {
        for(b=0;b<numquad;b++) { //PROCURA ESPACO LIVRE NA MP
          if(mp[b] == NULL) { //PARA ESPACO LIVRE
            pagefault++;
            mp[b] = &tab[a];
            tab[a].fis = mp[b];
            tab[a].br = 1;
            for(x=0;x<np;x++) { //CALCULA PERCENTUAL DO PROCESSO EM MEMORIA
              if(tab[x].fis != NULL)
                nemmp++;
            }
            perc = 100*nemmp/np;
            printf("Page Fault\n%.2f%% do processo em memoria\n", perc);
            imprime_tabela(tab, np);
            system("read -p \"\n\" saindo");
            return 1;
          }
        } //MP CHEIA, PROCURA PAGINA PARA SUBSTITUIR
        pagefault++;
        for(c=0;c<numquad*2;c++) { //REPETE DUAS VEZES CASO TODAS AS PAGINAS ESTEJAM COM BR = 1
          for(b=0;b<numquad;b++) {
            if(mp[b]->br == 1) { //SE BIT DE REFERENCIA E 1 SETAR PARA 0
              mp[b]->br = 0;
            } else { //SE BR = 0 SUBSTITUI PAGINA
              mp[b]->fis = NULL; //MUDA TABELA PARA INDICAR QUE PAGINA NAO ESTA MAIS EM MEMORIA
              mp[b] = &tab[a];
              tab[a].fis = mp[b];
              tab[a].br = 1;
              for(x=0;x<np;x++) { //CALCULA PERCENTUAL DO PROCESSO EM MEMORIA
                if(tab[x].fis != NULL)
                  nemmp++;
              }
              perc = 100*nemmp/np;
              printf("Memoria cheia, substitui paginas\nPage Fault\n%.2f%% do processo em memoria\n", perc);
              imprime_tabela(tab, np);
              system("read -p \"\n\" saindo");
              return 1;
            }
          }
        }
      }
    }
  }
}

int segunda_chance(pagina *tab, int ind, int np) {
  int a, b, c, d, x, nemmp=0;
  float perc;
  pagina *aux;
  for(a=0;a<np;a++) { //PROCURA PAGINA NA TABELA
    if(tab[a].ind == ind) { //ACHA PAGINA
      tab[a].br = 1; //SETA BIT DE REFERENCIA PARA 1
      if(tab[a].fis != NULL) { //VERIFICA SE PAGINA JA ESTA EM MEMORIA
        tab[a].br = 1;
        for(b=0;b<numquad;b++) { //SETA BR PRA 1 E PASSA PARA O FINAL DA FILA
          if(mp[b] == &tab[a]) {
            for(c=b;c<numquad-1;c++) {
              mp[c] = mp[c+1];
            }
            mp[c] = &tab[a];
            tab[a].fis = mp[c];
            for(x=0;x<np;x++) { //CALCULA PERCENTUAL DO PROCESSO EM MEMORIA
              if(tab[x].fis != NULL)
                nemmp++;
            }
            perc = 100*nemmp/np;
            printf("Pagina ja em memoria\n%.2f%% do processo em memoria\n", perc);
            imprime_tabela(tab, np);
            system("read -p \"\n\" saindo");
            return 1;
          }
        }
        return 1;
      } else {
        for(b=0;b<numquad;b++) { //PROCURA ESPACO LIVRE NA MP
          if(mp[b] == NULL) { //PARA ESPACO LIVRE
            pagefault++;
            mp[b] = &tab[a];
            tab[a].fis = mp[b];
            tab[a].br = 1;
            for(x=0;x<np;x++) { //CALCULA PERCENTUAL DO PROCESSO EM MEMORIA
              if(tab[x].fis != NULL)
                nemmp++;
            }
            perc = 100*nemmp/np;
            printf("Page Fault\n%.2f%% do processo em memoria\n", perc);
            imprime_tabela(tab, np);
            system("read -p \"\n\" saindo");
            return 1;
          }
        } //MP CHEIA, PROCURA PAGINA PARA SUBSTITUIR
        pagefault++;
        for(b=0;b<numquad*2;b++) {
          if(mp[0]->br == 1) { //BR = 1 -> BR = 0 E PASSA PARA O FINAL
            mp[0]->br = 0;
            aux = mp[0];
            for(c=0;c<numquad-1;c++) {
              mp[c] = mp[c+1];
            }
            mp[c] = aux;
          } else {
            mp[0]->fis = NULL; //ATUALIZA TABELA DE PAGINAS
            for(c=0;c<numquad-1;c++) {
              mp[c] = mp[c+1];
            }
            mp[c] = &tab[a];
            tab[a].fis = mp[c];
            for(x=0;x<np;x++) { //CALCULA PERCENTUAL DO PROCESSO EM MEMORIA
              if(tab[x].fis != NULL)
                nemmp++;
            }
            perc = 100*nemmp/np;
            printf("Memoria cheia, substitui paginas\nPage Fault\n%.2f%% do processo em memoria\n", perc);
            imprime_tabela(tab, np);
            system("read -p \"\n\" saindo");
            return 1;
          }
        }
      }
    }
  }
}

void le_arq(int op) { //LE O ARQUIVO TEXTO COM PROCESSOS E CHAMADAS
  FILE *arq;
  int b=0, t, cont, tamanho, numpag=0, hex, i, teste, proctam, procnp, end[100], indice;
  processo pr;
  pagina *tabela;
  char a[100], u[3], n[3], tam[11], endereco[100];
  char c;
  arq = fopen("arq1.txt", "r");
  while(fgets(a, 100, arq) != NULL) { //LE LINHA POR LINHA
    a[strcspn(a, "\n")] = '\0'; //TROCA O ENTER (\n) POR \0
    b=0;
    while(a[b]!=' ') { //GUARDA NOME DO PROCESSO
      n[b] = a[b];
      b++;
    }
    n[b] = '\0';
    b++;
    if(a[b]!='C') { //PARA LEITURA OU ESCRITA (R/W)
      b+=2; //"PULA" R OU W
      cont=0;
      while(a[b]!='\0') { //GUARDA ENDERECO COMO STRING
        endereco[cont] = a[b];
        cont++;
        b++;
      }
      endereco[cont] = '\0';
      hex = strtol(endereco, NULL, 16); //CONVERTE STRING DE ENDERECO PARA HEX
      for(i=0;hex>0;i++) { //CONVERTE HEX PARA BINARIO (VETOR DE INTEIRO)
        end[i]=hex%2;
        hex/=2;
      }
      --i;
      tabela = procura_processo(n, &proctam, &procnp); //PROCURA PROCESSO REQUERIDO, GUARDA TAMANHO E NUMERO DE PAGINAS
      printf("\n%s\n", a);
      if(!verifica_endereco(end, i, procnp, &indice)) { //PARA ENDERECO INVALIDO
        system("read -p \"Falha de Requisicao\" saindo");
      } else { //ENTRA NO ALGORITMO ESCOLHIDO(LRU PURO, LRU APROXIMADO OU SEGUNDA CHANCE)
        if(op == 1)
          lru_puro(tabela, indice, procnp);
        else if(op == 2)
          lru_aproximado(tabela, indice, procnp);
        else if(op == 3)
          segunda_chance(tabela, indice, procnp);
      }
    } else { //CRIACAO DE UM NOVO PROCESSO
      b+=2; //"PULA" C
      cont=0;
      while(a[b]!=' ') { //GUARDA TAMANHO EM STRING
        tam[cont] = a[b];
        cont++;
        b++;
      }
      tam[cont] = '\0';
      t = atoi(tam); //CONVERTE TAMANHO PARA INTEIRO
      b++;
      u[0] = a[b]; //GUARDA UNIDADE DE TAMANHO
      b++;
      u[1] = a[b];
      u[2] = '\0';
      pr = cria_processo(n, t, u); //CRIA NOVO PROCESSO COM DADOS OBTIDOS (NOME, TAMANHO, UNIDADE)
      tamanho = pr.tam;
      numpag = 0;
      while(tamanho > 0) { //CALCULA NUMERO DE PAGINAS DO PROCESSO
        tamanho -= tampag; //VE QUANTAS PAGINAS "CABEM" DENTRO DO TAMANHO DO PROCESSO
        numpag++;
      }
      pr.numpag = numpag;
      printf("%s Numero de paginas = %d\n", a,  numpag);
      system("read -p \"Novo processo criado\" saindo");
      pr.tab = (pagina *) malloc(numpag * sizeof(pagina)); //ALOCA ESPACO PARA TABELA DE PAGINAS DO PROCESSO
      inicializa_tab(pr);
      insere_lista(cria_nodo(pr));
    }
    printf("\n");
  }
  fclose(arq);
}

void imprime_tabela(pagina *tab, int np) { //IMPRIME TABELA DE PAGINAS
  int a;
  printf("ind | fis\n"); //MOSTRA INDICE DA TABELA E ENDERECO FISICO CASO ESTEJA ALOCADA
  for(a=0;a<np;a++) {
    printf("%d | ", tab[a].ind);
    if(tab[a].fis == NULL)
      printf("NULL\n");
    else
      printf("%p\n", tab[a].fis);
  }
}

void reseta_processos() { //RESETA NUMERO DE PAGE FAULTS E LISTA
  pagefault = 0;
  lista = NULL;
}

int menu() { //MENU PARA ESCOLHA DE ALORITMO
  int a;
  printf("1 - LRU Puro\n2 - LRU Aproximado\n3 - Segunda Chance\n0 - Sair\n");
  scanf("%d", &a);
  switch(a) {
    case 1:
      clrscr();
      reseta_processos();
      inicializa_mp();
      le_arq(1);
      printf("LRU puro: %d page faults\n", pagefault);
      system("read -p \"\n\" saindo");
      clrscr();
      return 1;

    case 2:
      clrscr();
      reseta_processos();
      inicializa_mp();
      le_arq(2);
      printf("LRU aproximado: %d page faults\n", pagefault);
      system("read -p \"\n\" saindo");
      clrscr();
      return 1;

    case 3:
      clrscr();
      reseta_processos();
      inicializa_mp();
      le_arq(3);
      printf("Segunda chance: %d page faults\n", pagefault);
      system("read -p \"\n\" saindo");
      clrscr();
      return 1;

    case 0:
      return 0;

    default:
      return 1;
  }
}

void main(int argc, char *argv[]) { //PRIMEIRO PARAM -> TAMANHO DA PAGINA, SEGUNDO PARAM -> TAMANHO MEMORIA FISICA
  int a=1;
  tampag = atoi(argv[1]);
  tamfis = atoi(argv[2]);
  if(tampag > tamfis || tamfis % tampag != 0) { //PARA TAMANHO DE PAGINA MAIOR QUE TAMANHO DA MEMORIA OU NUMEROS NAO MULTIPLOS
    printf("Valores de tamanho de pagina e memoria invalidos\n");
    exit(0);
  }
  clrscr();
  numquad = tamfis/tampag; //DEFINE NUMERO DE QUADROS
  while(a) {
    clrscr();
    printf("A memoria foi dividida em %d quadros\n", numquad);
    def_end();
    a = menu();
  }
}
