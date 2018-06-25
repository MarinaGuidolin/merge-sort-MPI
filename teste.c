// V. Freitas [2018] @ ECL-UFSC
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <mpi.h>
//#include <mpi.h>

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
int rank, processos, maxProcessos;
int tag = 0;
int *ordenado;
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

int * merge(int *numbersA, int sizeA, int *numbersB, int sizeB) {
	int posicaoA = 0, posicaoB = 0, posicaoSorted = 0, i; //posicao
	
	int sizeSorted = sizeA+sizeB;
	//print_array(numbersA, sizeA);
	//print_array(numbersB,sizeB);

	while ((posicaoA < sizeA) && (posicaoB < sizeB)) {
		if (numbersA[posicaoA] <= numbersB[posicaoB]) {
			ordenado[posicaoSorted] = numbersA[posicaoA];
			posicaoSorted++; posicaoA++;
		} 
		else {
			ordenado[posicaoSorted] = numbersB[posicaoB];
			posicaoSorted++; posicaoB++;
		}
	}

	if (posicaoA >= sizeA){
		for (i = posicaoSorted; i < sizeSorted; i++, posicaoB++){
			ordenado[i] = numbersB[posicaoB];
		}
	}
	else if (posicaoB >= sizeB){
		for (i = posicaoSorted; i < sizeSorted; i++, posicaoA++){
			ordenado[i] = numbersA[posicaoA];
		}
	}

	for (i = 0; i < sizeA; i++){
		numbersA[i] = ordenado[i];
	}
	for (i = 0; i < sizeB; i++){
		numbersB[i] = ordenado[sizeA+i];
	}
	return ordenado;
}
void recursive_merge_sort_seq(int *tmp, int begin, int end) {
	int mid = (begin+end)/2;
	int sizeA = mid - begin + 1;
	int sizeB = end - mid;

	if (end == begin) {
		return;
	} else {
		recursive_merge_sort_seq(tmp, begin, mid); // primeira metade (maior se for impar)
		recursive_merge_sort_seq(tmp, mid+1, end); // segunda metade 
		merge(tmp + begin, sizeA, tmp + mid + 1, sizeB);
	}
}

void recursive_merge_sort(int *tmp, int begin, int end)
{

	int size;
	int mid = (begin+end)/2; // indice final da primeira metade
	int sizeA = mid - begin + 1; // tamanho da primeira metade
	int sizeB = end - mid; // tamanho da segunda metade
	
	int *enviado = tmp + mid + 1; // segunda metade, que deve ser enviada
	printf("array enviado - segunda metade do recebido\n");
	print_array(enviado, sizeB);
	int proxProcesso = rank + pow(2, tag);

	if (proxProcesso > maxProcessos) {
		return;	
	}

 	if(rank == 0){// se o processo for zero, ele enviará a segunda metade	
		if (end == begin) { // se nao tiver tamanho o suficiente, retorna para o merge
			return;
		}
		tag++; // aumenta a tag pois ele é um processo que já enviou uma vez
		MPI_Send(&tag, 1, MPI_INT, proxProcesso, 0, MPI_COMM_WORLD); //envia a tag 1 para o proximo processo
		MPI_Send(&sizeB, 1, MPI_INT, proxProcesso, 0, MPI_COMM_WORLD); // tamanho do enviado
		MPI_Send(enviado, sizeB, MPI_INT, proxProcesso, 0, MPI_COMM_WORLD); // envia a segunda metade
		printf("array de envio enviado do processo %d para o processo %d:\n", rank, proxProcesso);
		print_array(enviado, sizeB);
		//free(enviado);

		//MPI_Recv(arrayEntrada, size, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		//recursive_merge_sort(tmp, begin, mid); // primeira metade (maior se for impar)
		//recursive_merge_sort(tmp, mid+1, end); // segunda metade 
		//merge(tmp + begin, sizeA, enviado, sizeB);
	}
	if(tag == 0){ // se a tag for zero, significa que o processo ainda não recebeu -  nenum processo que ja recebeu recebe novamente
		 MPI_Recv(&tag, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // recebe uma tag != 0
		 MPI_Recv(&size, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // size só sera definido quando recebido
		 printf("processo %d recebeu o a tag %d\n", rank, tag);
		 int *recebido = malloc(size*sizeof(int));
		 MPI_Recv(recebido, size, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		 printf("o array recebido é:\n");
		 print_array(recebido, size);
		 printf("o novo array tmp é  \n");
		 tmp = recebido;
		 print_array(tmp, size);
		 //free(recebido);
	}
    enviado = tmp + mid + 1; // novo array enviado
    printf("segundo enviado\n");
    print_array(enviado, size);
    end = size -1;
    mid = (begin+end)/2; // indice final da primeira metade
	sizeA = mid - begin + 1; // tamanho da primeira metade
	sizeB = end - mid; // tamanho da segunda metade
	
    if (end == begin)
    {
        return;
    }
	tag++; // aumenta a tag pois ele é um processo que já enviou uma vez
	MPI_Send(&tag, 1, MPI_INT, proxProcesso, 0, MPI_COMM_WORLD); //envia a tag 1 para o proximo processo
	MPI_Send(&sizeB, 1, MPI_INT, proxProcesso, 0, MPI_COMM_WORLD); // tamanho do enviado
	MPI_Send(enviado, sizeB, MPI_INT, proxProcesso, 0, MPI_COMM_WORLD); // envia a segunda metade
	printf("array de envio enviado do processo %d para o processo %d:\n", rank, proxProcesso);
	print_array(enviado, sizeB);
	//free(enviado);

	//MPI_Recv(arrayEntrada, size, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	//recursive_merge_sort(tmp, begin, mid); // primeira metade (maior se for impar)
	//recursive_merge_sort(tmp, mid+1, end); // segunda metade 
	//merge(tmp + begin, sizeA, enviado, sizeB);
}


// First Merge Sort call
void merge_sort(int * numbers, int size, int * tmp) {
	recursive_merge_sort(numbers, 0, size-1); // chama a função recusva passando o numbers como um array vazio e o temp como o numbers
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
	int seed, max_val;
	int * sortable;
	int * tmp;
	size_t arr_size;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &processos);
	maxProcessos = processos + 1;
	if (rank == 0){
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
	}

	
	//free(sortable);
	//free(tmp);
	MPI_Finalize();
	return 0;
}