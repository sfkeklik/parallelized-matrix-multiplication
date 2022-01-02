// If you dont want closing console immediately, Compile with CTRL-F5 

#include "pch.h"
#include <pthread.h>
#include <time.h>
#include <iostream>

using namespace std;

#define NUM_THREADS 4

template <typename T>
struct thread_data {
	T* matrix1;
	T* matrix2;
	int start;
	int finish;
	int width;
	T* resultMatrix;
};

template <typename T>
void* parallelizedMatrixMultiplication(void* threadArg);

		
int main()
{


	//Serial Multipication Func

	int n = 5000;
	cout << "Matrix Size : " <<  n << "x" << n << endl;

	cout << "\n******************** Serial Multipication ********************\n" << endl;

	float* matrix1 = new float[n*n];
	float* matrix2 = new float[n*n];
	float* resultMatrix = new float[n*n];



	for (int row = 0; row < n; row++)
		for (int col = 0; col < n; col++) {
			int pos = row * n + col;
			matrix1[pos] = 1.0f;
			matrix2[pos] = 1.0f;

		}

	clock_t tStart = clock();




	for (int row = 0; row < n; row++)
		for (int col = 0; col < n; col++) {
			int matrix1Pos = row * n + col;
			int matrix2Pos = col * n + row;
			resultMatrix[matrix1Pos] = matrix1[matrix1Pos] * matrix2[matrix2Pos];
		}
	
	printf("\nSerial Multiplication Time taken: %.4fs\n", (double)(clock() - tStart) / CLOCKS_PER_SEC);


	//Parallelized Multipication Func
	pthread_t threads[NUM_THREADS];
	struct thread_data<float> td[NUM_THREADS];
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	int rc;
	void* status;

	clock_t tStart2 = clock();

	//Block Data Sharing
	cout << "\n******************** Block Data Sharing ********************\n" << endl;
	for (int i = 0; i < NUM_THREADS; i++) {
		cout << "main() : creating thread, " << i << endl;
		td[i].matrix1 = matrix1;
		td[i].matrix2 = matrix2;
		td[i].resultMatrix = resultMatrix;
		td[i].width = n;

		//Block Data Sharing
		td[i].start = (n / (float) NUM_THREADS)*i;
		td[i].finish = (n / (float) NUM_THREADS)*(i + 1);

		// Initialize and set thread joinable
		pthread_create(&threads[i], &attr, parallelizedMatrixMultiplication<float>, (void *)&td[i]);
		cout << "main() : created thread, " << i << endl;
	}
	// free attribute and wait for the other threads
	pthread_attr_destroy(&attr);
	for (int i = 0; i < NUM_THREADS; i++) {
		pthread_join(threads[i], &status);

		cout << "Main: completed thread id :" << i;
		cout << "  exiting with status :" << status << endl;
	}
	printf("\nParallelized Multiplication with Block Data Sharing Time taken: %.4fs\n", (double)(clock() - tStart2) / CLOCKS_PER_SEC);


	//Sequential Data Sharing
	cout << "\n******************** Sequential Data Sharing ********************\n" << endl;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	clock_t tStart3 = clock();

	int sequentialDivide = 4;
	for (int i = 0; i < sequentialDivide; i++) {
		for (int j = 0; j < NUM_THREADS; j++) {
			cout << "main() : creating thread, " << j << endl;
			td[j].matrix1 = matrix1;
			td[j].matrix2 = matrix2;
			td[j].resultMatrix = resultMatrix;
			td[j].width = n;	

			//Sequential Data Sharing
			td[j].start = (n / (float)(NUM_THREADS * sequentialDivide))*j + (n / (float)sequentialDivide) * i;
			td[j].finish = (n / (float)(NUM_THREADS * sequentialDivide))*(j + 1) + (n / (float)sequentialDivide) * i;

			//wait for the j'th thread
			if (i > 0) {
				pthread_join(threads[j], &status);
				cout << "Main: completed thread id :" << j;
				cout << "  exiting with status :" << status << endl;
			}
			// Initialize threads
			pthread_create(&threads[j],&attr, parallelizedMatrixMultiplication<float>, (void *)&td[j]);
			cout << "main() : created thread, " << j << endl;
		}
	}
	// free attribute
	pthread_attr_destroy(&attr);

	printf("\nParallelized Multiplication with Sequential Data Sharing Time taken: %.4fs\n", (double)(clock() - tStart3) / CLOCKS_PER_SEC);








	delete[] matrix1;
	delete[] matrix2;
	delete[] resultMatrix;




	return 0;
}

template <typename T>
void* parallelizedMatrixMultiplication(void* threadArg) {
	struct thread_data<T> *my_data;
	my_data = (struct thread_data<T> *) threadArg;
	void* status=NULL;
	for (int row = my_data->start; row < my_data->finish; row++)
		for (int col = my_data->start; col < my_data->finish; col++) {
			int resultPos = row * my_data->width + col;
			my_data->resultMatrix[resultPos] = 0;
			for (int k = 0; k < my_data->width; k++) {
				int matrix1Pos = row * my_data->width + k;
				int matrix2Pos = k * my_data->width + col;
				my_data->resultMatrix[resultPos] += my_data->matrix1[matrix1Pos] * my_data->matrix2[matrix2Pos];
			}
		}


	pthread_exit(NULL);
	return NULL;
}


