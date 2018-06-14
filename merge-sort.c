// V. Freitas [2018] @ ECL-UFSC
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <omp.h>
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

/*
 * Orderly merges two int arrays (numbers[begin..middle] and numbers[middle..end]) into one (sorted).
 * \retval: merged array -> sorted
 */
// int* numbers é um ponteiro que aponta para numeros de um array
// int begin é onde começa o array
// int middle é a metade do array
// int end é o final do array
// values é o array que é usado
void mergeEmArraysDiferentes(int* numbersA, int beginA, int middleA, int endA, int * sortedA, int* numbersB, int beginB, int middleB, int endB, int* sortedB) {
	int i, j, r, s;
	i = beginA; j = middleA;
	r = beginB; s = middleB;

	sortedA = (int*)malloc(endA*sizeof(int));
	sortedB = (int*)malloc(endB*sizeof(int));

	//* implementação para resultados em array separados
	#pragma omp parallel for
	debug("Merging array A  Begin: %d, Middle: %d, End: %d\n", beginA, middleA, endA);
	for (int k = beginA; k < endA; ++k) {
		debug("LHS[%d]: %d, RHS[%d]: %d\n", i, numbers[i], j, numbers[j]);
		if (i < middle && (j >= end || numbers[i] < numbers[j])) {
			sortedA[k] = numbers[i];
			i++;
		} else {
			sortedA[k] = numbers[j];
			j++;
		}
	}
	#pragma omp parallel for
	debug("Merging array B  Begin: %d, Middle: %d, End: %d\n", beginB, middleB, endB);
	for (int l = beginB; l < endB; ++l) {
		debug("LHS[%d]: %d, RHS[%d]: %d\n", r, numbers[r], j, numbers[s]);
		if (r < middle && (s >= end || numbers[r] < numbers[s])) {
			sortedB[l] = numbers[r];
			r++;
		} else {
			sortedB[l] = numbers[s];
			s++;
		}
	}
}

void merge(int* numbersA, int beginA, int middleA, int endA, int * sorted, int* numbersB , int beginB, int middleB, int endB) {
	int i, j, iA, iB, iS; // iA, iB e iS são as posições atuais dos arrays A, B e Sorted
	i = beginA; j = (beginA + endB)/2; /*i é o inicio do array A (primeira posição do array sorted)
	 e j é o meio do array sorted (meio entre o inicio de a e o fim de b)*/
	middleS = (beginA + endB)/2;
	iA = iB = iS = 0;
	endS = endB + endA; // tamanho total do array de 
	sorted = (int*)malloc(endS*sizeof(int)); // aloca o array sorted na memoria


	while ((iA < endA) && (iB < endB)) {
		if (First[iA] <= Second[iB]) {
			sortedd[iS] = First[iA];
			iS++; iA++;
		} 
		else {
			sorted[iS] = Second[iB];
			iS++; iB++;
		}
	} // array sorted com todos os elementos de A e B

	debug("Merging. Begin: %d, Middle: %d, End: %d\n", beginA, j, endB);
	for (int k = beginA; k < endB; ++k) { // ordena todos os elementos do sorted
		debug("LHS[%d]: %d, RHS[%d]: %d\n", i, numbers[i], j, numbers[j]);
		if (i < middle && (j >= end || numbers[i] < numbers[j])) {
			sorted[k] = numbers[i];
			i++;
		} else {
			sorted[k] = numbers[j];
			j++;
		}
	}
}


/*
 * Merge sort recursive step
 */
void recursive_merge_sort(int* tmp, int begin, int end, int* numbers) {
	// dividir o temp em duas partes (o temp recebido é o numbers original)
	// number é iniciado como um array vazio

	if (end - begin < 2)
		return;
	else {
		int middle = (begin + end)/2;
		recursive_merge_sort(numbers, begin, middle, tmp);
		recursive_merge_sort(numbers, middle, end, tmp);
		merge(tmp, begin, middle, end, numbers);
	}
}

// First Merge Sort call
void merge_sort(int * numbersA, int sizeA, int * numbersB, int sizeB, int * tmp) {
	recursive_merge_sort(numbers, 0, size, tmp); // chama a função recusva passando o numbers como um array vazio e o temp como o numbers
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

	// Basic MERGE unit test
	if (DEBUG > 1) { // debug ???
		int * a = (int*)malloc(8*sizeof(int));
		a[0] = 1; a[1] = 3; a[2] = 4; a[3] = 7;
		a[4] = 0; a[5] = 2; a[6] = 5; a[7] = 6;

		int * b = (int*)malloc(8*sizeof(int));
		b[0] = 1; b[1] = 3; b[2] = 4; b[3] = 7;
		b[4] = 0; b[5] = 2; b[6] = 5; b[7] = 6;
		int * values = (int*)malloc(8*sizeof(int));
		merge(a, 0, 4, 8, values); // mudar para 2 arrays
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
		merge_sort(a, 8, b); // mudar para 2 arrays
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

		merge_sort(a, 9, b); // mudar para 2 arrays
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

	srand(seed);
	sortable = malloc(arr_size*sizeof(int));
	tmp 	 = malloc(arr_size*sizeof(int));
	
	populate_array(sortable, arr_size, max_val);
	tmp = memcpy(tmp, sortable, arr_size*sizeof(int));

	print_array(sortable, arr_size);
	merge_sort(sortable, arr_size, tmp);
	print_array(sortable, arr_size);

	
	free(sortable);
	free(tmp);
	return 0;
}
