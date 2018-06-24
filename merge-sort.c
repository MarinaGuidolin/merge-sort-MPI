// O COPYRIGHT DO VINICIUS FREITAS
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

int* ordenado, enviado;
int mpiSize, mpiRank;

int * merge(int *numbersA, int sizeA, int *numbersB, int sizeB, int *sorted) {
	int posicaoA = 0;
	int posicaoB = 0;
	int posicaoSorted = 0;
	int i; //posicao
	
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


void recursive_merge_sort_seq(int *tmp, int begin, int end, int * sorted) {
	int mid = (begin+end)/2;
	int sizeA = mid - begin + 1;
	int sizeB = end - mid;

	if (end == begin) {
		return;
	} else {
		recursive_merge_sort_seq(tmp, begin, mid, sorted); // primeira metade (maior se for impar)
		recursive_merge_sort_seq(tmp, mid+1, end, sorted); // segunda metade 
		merge(tmp + begin, sizeA, tmp + mid + 1, sizeB, sorted);
	}
}

void recursive_merge_sort_parallel(int *tmp, int begin, int end, int * sorted, int nivel, int rankAnterior, int maxRank) {
	MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank);
	int proxProcesso = mpiRank + pow(2, nivel);
	int tagrecebido, sizeRecebido;
	MPI_Recv(&tagrecebido, 1, MPI_INT, rankAnterior, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

	if (tagrecebido == 0){
		int* recebido;
		MPI_Recv(&beginRecebido, 1, MPI_INT, rankAnterior, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv(&sizeRecebido, 1, MPI_INT, rankAnterior, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		recebido = (int*)malloc(sizeRecebido*sizeof(int));
		MPI_Recv(&recebido, sizeRecebido, MPI_INT, rankAnterior, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		print_array(recebido, sizeRecebido);
		tagrecebido = 1;
		tmp = recebido;
		end = (int*)sizeRecebido - 1;
		begin = (int*)beginRecebido;
		free(recebido);
	}

	if (proxProcesso > maxRank) {
		recursive_merge_sort_seq(tmp, begin, end, sorted);
	}
	
	int mid = (begin+end)/2;
	int sizeA = mid - begin + 1;
	int sizeB = end - mid; // tamanho do array a ser enviado (2a metade)

	if (end == begin) {
		return;
	} else {
		enviado = (int*)malloc(mid*sizeof(int));
		enviado = tmp + mid + 1;
		int tagenviado = 0;
		int midEnvia = mid + 1;
		MPI_Send(&midEnvia, 1, MPI_INT, proxProcesso, MPI_ANY_TAG, MPI_COMM_WORLD);
		MPI_Send(&sizeB, 1, MPI_INT, proxProcesso, MPI_ANY_TAG, MPI_COMM_WORLD);
		MPI_Send(&tagenviado, 1, MPI_INT, proxProcesso, MPI_ANY_TAG, MPI_COMM_WORLD);
		MPI_Send(&enviado, sizeB, MPI_INT, proxProcesso, MPI_ANY_TAG, MPI_COMM_WORLD);
		recursive_merge_sort_parallel(tmp, begin, mid, sorted, nivel++, mpiRank, maxRank); // primeira metade (maior se for impar) - nao é um processo
		merge(tmp + begin, sizeA, tmp + mid + 1, sizeB, sorted);
		free(enviado);
	}
}

// First Merge Sort call
void merge_sort(int* numbers, int size, int *tmp, int rank, int maxRank) {
	MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank);
	if (mpiRank = 0) {
		recursive_merge_sort_parallel(numbers, 0, size-1, tmp, 0, mpiRank, maxRank); // chama a função recusva passando o numbers como um array vazio e o temp como o numbers
	}
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
	int arr_size;

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
	
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &mpiSize);

	populate_array(sortable, arr_size, max_val);
	tmp = memcpy(tmp, sortable, arr_size*sizeof(int));
	printf("array original\n");
	print_array(tmp, arr_size);
	ordenado = malloc(arr_size*sizeof(int));

	int maxRank = mpiSize - 1;
	
	merge_sort(sortable, arr_size, tmp, mpiRank, maxRank);
	
	printf("FINAL sort\n");
	print_array(ordenado, arr_size);

	MPI_Finalize();
	free(sortable);
	free(tmp);
	return 0;
}
