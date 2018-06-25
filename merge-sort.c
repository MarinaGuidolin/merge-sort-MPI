// V. Freitas [2018] @ ECL-UFSC
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <mpi.h>

/*** 
 * Todas as Macros pré-definidas devem ser recebidas como parâmetros de
 * execução da sua implementação paralela!! 
 ***/
#ifndef DEBUG
#define DEBUG 0
#endif

#ifndef NELEMENTS
#define NELEMENTS 100
#endif

#ifndef MAXVAL
#define MAXVAL 255
#endif // MAX_VAL

#define TAG_TAMANHO 1111
#define TAG_METADE 612
#define TAG_NIVEL 7
#define TAG_PROCESSO_ANTERIOR 44

int processosCriados = 0;
int *ordenado;
int nivel= 0;
int rank, processos, maxProcessos;
/*
 * More info on: http://en.cppreference.com/w/c/language/variadic
 */
void debug(const char* msg, ...) { // ?? o que faz, o que precisa depois dos ...
	if (DEBUG > 2) {
		va_list args;
		va_start(args, msg); 
		vprintf(msg, args); 
		va_end(args);
	}
}

int * merge(int *numbersA, int sizeA, int *numbersB, int sizeB, int *sorted) {
	int posicaoA = 0, posicaoB = 0, posicaoSorted = 0, i; //posicao
	
	int sizeSorted = sizeA+sizeB;
	print_array(numbersA, sizeA);
	print_array(numbersB,sizeB);

	sorted = (int *)malloc(sizeSorted*sizeof(int));
	while ((posicaoA < sizeA) && (posicaoB < sizeB)) {
		if (numbersA[posicaoA] <= numbersB[posicaoB]) {
			sorted[posicaoSorted] = numbersA[posicaoA];
			posicaoSorted++; posicaoA++;
		} 
		else {
			sorted[posicaoSorted] = numbersB[posicaoB];
			posicaoSorted++; posicaoB++;
		}
	}

	if (posicaoA >= sizeA){
		for (i = posicaoSorted; i < sizeSorted; i++, posicaoB++){
			sorted[i] = numbersB[posicaoB];
		}
	}
	else if (posicaoB >= sizeB){
		for (i = posicaoSorted; i < sizeSorted; i++, posicaoA++){
			sorted[i] = numbersA[posicaoA];
		}
	}

	for (i = 0; i < sizeA; i++){
		numbersA[i] = sorted[i];
	}
	for (i = 0; i < sizeB; i++){
		numbersB[i] = sorted[sizeA+i];
	}
	ordenado = sorted;
	return sorted;
}


void recursive_merge_sort_p(int *tmp, int begin, int end, int * sorted){
	int proxProcesso = rank + pow(2, nivel); // se ele nao recebeu nada, usa o nivel e o rank atual
	int tag = 0;
	if(proxProcesso > maxProcessos){
		return;
	}//ou recursive_merge_sort(tmp, begin, end, sorted);
	if (end == begin) {
		return;
	} else {
			int mid = (begin+end)/2; // ultimo indice da primeira metade
			int sizeA = mid - begin + 1; // tamanho da primeira metade
			int sizeB = end - mid; // tamanho da segunda metade
			int* enviado = tmp + mid + 1; // segunda metade a ser enviada

			int proximoNivel = nivel + 1;
			int processoAnterior = rank;
			int proxProcesso = rank + pow(2, nivel);
			
			MPI_Send(&rank, 1, MPI_INT, proxProcesso, TAG_PROCESSO_ANTERIOR, MPI_COMM_WORLD);
			MPI_Send(&sizeB, 1, MPI_INT, proxProcesso, TAG_TAMANHO, MPI_COMM_WORLD);
			MPI_Send(&enviado, sizeB, MPI_INT, proxProcesso, TAG_METADE, MPI_COMM_WORLD); // envia a segunda metade do array principal
			nivel++;
			MPI_Send(&proximoNivel, 1, MPI_INT, proxProcesso, TAG_NIVEL , MPI_COMM_WORLD); // envia o nivel
			tag = 1; // tag do processo que enviou será 1
			//recursive_merge_sort_p(tmp, begin, mid, sorted); // primeira metade (maior se for impar)
			recursive_merge_sort_p(tmp, mid+1, end, sorted); // segunda metade 

		/*if(tag == 0) { // se ele ainda nao recebeu de nenhum processo
			int *recebido = malloc(sizeB*sizeof(int));
			MPI_Recv(&nivel, 1, MPI_INT, rank, MPI_ANY_TAG, TAG_NIVEL, MPI_STATUS_IGNORE); // Recebe o array ordenado
			MPI_Recv(&recebido, sizeB, MPI_INT, rank, TAG_METADE, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
			processosCriados++;
			tmp = recebido;
			free(recebido);
		}*/
		merge(tmp + begin, sizeA, tmp + mid + 1, sizeB, sorted);
	} 
}

void recursive_merge_sort(int *tmp, int begin, int end, int * sorted)
{
	int mid = (begin+end)/2;
	int sizeA = mid - begin + 1;
	int sizeB = end - mid;

	if (end == begin) {
		return;
	} else {
		recursive_merge_sort(tmp, begin, mid, sorted); // primeira metade (maior se for impar)
		recursive_merge_sort(tmp, mid+1, end, sorted); // segunda metade 
		merge(tmp + begin, sizeA, tmp + mid + 1, sizeB, sorted);

	}
}
// First Merge Sort call
void merge_sort(int * numbers, int size, int * tmp) {
	recursive_merge_sort_p(numbers, 0, size-1, tmp); // chama a função recusva passando o numbers como um array vazio e o temp como o numbers
}

void print_array(int* array, int size) {
	printf("Printing Array:\n");
	for (int i = 0; i < size; ++i) {
		printf("%d. ", array[i]);
	}
	printf("\n");
}

void populate_array(int* array, int size, int max) {
	int m = max+1;
	for (int i = 0; i < size; ++i) {
		array[i] = rand()%m;
	}
}
int main (int argc, char ** argv) {
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &processos);
	maxProcessos = processos + 1;	
	int seed, max_val;
	int * sortable;
	int * tmp;
	size_t arr_size;
	if (rank == 0){
	// Basic MERGE unit test
		if (DEBUG > 1) {
			int * a = (int*)malloc(8*sizeof(int));
			a[0] = 1; a[1] = 3; a[2] = 4; a[3] = 7;
			a[4] = 0; a[5] = 2; a[6] = 5; a[7] = 6;
			int * b = (int*)malloc(8*sizeof(int));
			b[0] =20 ; b[1] = 11; b[2] = 12; b[3] = 10;
			b[4] = 9 ; b[5] = 8; b[6] = 6; b[7] = 7;


			int * values = (int*)malloc(8*sizeof(int));
			merge(a, 8, b, 8, values);
			free (a);
			print_array(values, 8);
			free(values);
			return 2;
		}

	// Basic MERGE-SORT unit test
		if (DEBUG > 0) {
			int * a = (int*)malloc(8*sizeof(int)); 
			int * b = (int*)malloc(8*sizeof(int));
			a[0] = 7; a[1] = 6; a[2] = 5; a[3] = 4;
			a[4] = 3; a[5] = 2; a[6] = 1; a[7] = 0;

			b = memcpy(b, a, 8*sizeof(int));
			merge_sort(a, 8, b); 
			print_array(b, 8);

			free(a);
			free(b);

			a = (int*)malloc(9*sizeof(int));
			b = (int*)malloc(9*sizeof(int));
			a[0] = 3; a[1] = 2; a[2] = 1; 
			a[3] = 10; a[4] = 11; a[5] = 12; 
			a[6] = 0; a[7] = 1; a[8] = 1;

			b = memcpy(b, a, 9*sizeof(int));
			print_array(b, 9);

			merge_sort(a, 9, b); 
			print_array(b, 9);

			free(a);
			free(b);
			printf("\n");
			return 1;
		}

		switch (argc) {
			case 1:
			seed = time(NULL);
			arr_size = NELEMENTS;
			max_val = MAXVAL;
			break;
			case 2:
			seed = atoi(argv[1]);
			arr_size = NELEMENTS;
			max_val = MAXVAL;
			break;
			case 3:
			seed = atoi(argv[1]);
			arr_size = atoi(argv[2]);
			max_val = MAXVAL;
			break;
			case 4:
			seed = atoi(argv[1]);
			arr_size = atoi(argv[2]);
			max_val = atoi(argv[3]);
			break;
			default:
			printf("Too many arguments\n");
			break;	

		}

		int proxProcesso = rank + pow(2, nivel); // definido o processo ápós o zero = 1
		srand(seed);
		sortable = malloc(arr_size*sizeof(int));
		tmp 	 = malloc(arr_size*sizeof(int));

		populate_array(sortable, arr_size, max_val);
		tmp = memcpy(tmp, sortable, arr_size*sizeof(int));
		printf("array original\n");
		print_array(tmp, arr_size);
		ordenado = malloc(arr_size*sizeof(int));
		merge_sort(sortable, arr_size, tmp);
		printf("FINAL sort\n");
		print_array(ordenado, arr_size);
	} else {
		int processoAnterior = pow(2, nivel) - rank; // esperando do processo anterior
		int sizeB, nivel; // tamanho da segunda metade recebida
		MPI_Recv(&sizeB, 1, MPI_INT, processoAnterior, TAG_TAMANHO, MPI_COMM_WORLD, MPI_STATUS_IGNORE);  //recebe do processo anterior
		MPI_Recv(&nivel, 1, MPI_INT, processoAnterior, TAG_NIVEL, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		if(processosCriados > maxProcessos){ // nao pode mais criar processos novos, entao ele so recebe e envia o array ja 
			int *recebido2 = malloc(sizeB*sizeof(int));
			MPI_Recv(&recebido2, sizeB, MPI_INT, processoAnterior, TAG_METADE, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // recebe uma segunda metade com tmanho sizeB
			int *sorted = malloc(sizeB*sizeof(int)); 
			recursive_merge_sort_p(recebido2, 0, sizeB, sorted); // executa o merge paralelo com uma segunda metade recebida
			free(recebido2);
			free(sorted);
		}
	}	
	free(sortable);
	free(tmp);
	return 0;
}