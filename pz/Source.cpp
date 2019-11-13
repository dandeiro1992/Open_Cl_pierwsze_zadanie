// Przyk³adowy program w open Cl do szukania platform i liczenia liczby Pi
#ifdef _WIN32
#define _CRT_SECURE_NO_DEPRECATE
#endif
#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>
#include<iostream>
#define LICZBA_ITERACJI (20000000)
#define MAX_SOURCE_SIZE (0x100000)

using namespace std;

int main(int argc, char ** argv)
{
	cl_int ret;
	cl_int error;
	cl_uint platformCount;
	cl_uint *deviceCount;
	cl_uint *deviceCount2;
	cl_platform_id *list_of_platforms;
	cl_device_id *list_of_devices;
	cl_device_id *list_of_devices2;
	cl_uint pointer;
	cl_uint *wydajnosc;
	cl_uint *wydajnosc2;
	size_t liczba;
	cl_uint units;
	cl_uint frequency;
	bool GPU = true; //true jesli jest gpu false jesli nie jest
	char wyraz[128];

	//Dane
	cl_context context = NULL;
	cl_command_queue command_queue = NULL;
	//cl_mem memobj = NULL;
	cl_program program = NULL;
	cl_kernel kernel = NULL;

	int i;
	float *A;
	float *C;
	float suma;
	float liczba_pi;

	A = (float *)malloc(LICZBA_ITERACJI * sizeof(float));
	C = (float *)malloc(LICZBA_ITERACJI * sizeof(float));
	cl_mem Amobj = NULL;
	cl_mem Cmobj = NULL;

	//koniec danych

	error = clGetPlatformIDs(0, NULL, &platformCount);
	if (error != CL_SUCCESS)
	{
		cout << "nie moglem uzyskac informacji o platformach" << endl;
	}

	list_of_platforms = new cl_platform_id[platformCount];
	deviceCount = new cl_uint[platformCount];
	deviceCount2 = new cl_uint[platformCount];
	error = clGetPlatformIDs(platformCount, list_of_platforms,NULL);

	wydajnosc = new cl_uint[platformCount];
	wydajnosc2 = new cl_uint[platformCount];
	/////////////// najpierw wybieram platforme z GPU
	for (cl_uint i = 0; i < platformCount; i++)
	{
		wydajnosc[i] = 0;
		clGetPlatformInfo(list_of_platforms[i], CL_PLATFORM_NAME, sizeof(wyraz), wyraz, NULL);
		cout << "nazwa platformy to: " << wyraz << endl;
		error = clGetDeviceIDs(list_of_platforms[i], CL_DEVICE_TYPE_GPU, 0, NULL, &deviceCount[i]);
		//cout << error << "error dla device" << endl;
		cout << deviceCount[i] << "device count dla " << i << " platformy" << endl;
		list_of_devices = new cl_device_id[deviceCount[i]];
		error = clGetDeviceIDs(list_of_platforms[i], CL_DEVICE_TYPE_GPU, deviceCount[i], list_of_devices, NULL);
		for (cl_uint j = 0; j < deviceCount[i]; j++)
		{
			clGetDeviceInfo(list_of_devices[j], CL_DEVICE_NAME, sizeof(wyraz), wyraz, NULL);
			cout << "Device name: " << wyraz << endl;
			clGetDeviceInfo(list_of_devices[j], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(liczba), &liczba, NULL);
			cout << "liczba work group: " << liczba << endl;
			clGetDeviceInfo(list_of_devices[j], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(units), &units, NULL);
			cout << "units: " << units << endl;
			clGetDeviceInfo(list_of_devices[j], CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(frequency), &frequency, NULL);
			cout << "frequency: " << frequency << endl;
			wydajnosc[i] = wydajnosc[i] + ((cl_uint)liczba)*units*frequency;
		}
		cout << "ogolna wydajnosc dla " << i << " tej platformy wynosi    " << wydajnosc[i] << endl;
	}

	cout << "przechodzimy do CPU   " << endl << endl << endl;
	for (cl_uint i = 0; i < platformCount; i++)
	{
		wydajnosc2[i] = 0;
		clGetPlatformInfo(list_of_platforms[i], CL_PLATFORM_NAME, sizeof(wyraz), wyraz, NULL);
		cout << "nazwa platformy to: " << wyraz << endl;
		error = clGetDeviceIDs(list_of_platforms[i], CL_DEVICE_TYPE_CPU, 0, NULL, &deviceCount2[i]);
		//cout << error << "error dla device" << endl;
		cout << deviceCount2[i] << "device count dla " << i << " platformy" << endl;
		list_of_devices2 = new cl_device_id[deviceCount2[i]];
		error = clGetDeviceIDs(list_of_platforms[i], CL_DEVICE_TYPE_CPU, deviceCount2[i], list_of_devices2, NULL);
		for (cl_uint j = 0; j < deviceCount2[i]; j++)
		{
			clGetDeviceInfo(list_of_devices2[j], CL_DEVICE_NAME, sizeof(wyraz), wyraz, NULL);
			cout <<"Device name: " <<wyraz << endl;
			clGetDeviceInfo(list_of_devices2[j], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(liczba), &liczba, NULL);
			cout << "liczba work group: " << liczba<< endl;
			clGetDeviceInfo(list_of_devices2[j], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(units), &units, NULL);
			cout << "units: " << units << endl;
			clGetDeviceInfo(list_of_devices2[j], CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(frequency), &frequency, NULL);
			cout << "frequency: " << frequency << endl;
			wydajnosc2[i] = wydajnosc2[i] + ((cl_uint)liczba)*units*frequency;
		}
		cout << "ogolna wydajnosc2 dla " << i << " tej platformy wynosi    " << wydajnosc2[i] << endl;
	}
	// zrobimy program ktory w pierwszym kroku bierze karte graficzna jesli nie to z pozostalych liczy najlepsze:
	//sprawdzamy wydajnosci;
	cout << "wydajnosc  " << wydajnosc[0] << endl;
	cout << "wydajnosc  " << wydajnosc[1] << endl;
	cout << "wydajnosc  " << wydajnosc2[0] << endl;
	cout << "wydajnosc  " << wydajnosc2[1] << endl;
	cout << "wybieramy najlepsze parametry dla grafiki a pozniej stworzymy program dla tej platformy w ktorej byla grafika:" << endl;
	pointer = 0;
	for (cl_uint i = 0; i < platformCount; i++)
	{
		if (wydajnosc[i] >= wydajnosc[pointer])
			pointer = i;
	}
	if (wydajnosc[pointer] == 0)
	{
		GPU = false;
		pointer = 0;
		//szukamy najlepszego z CPU
		for (cl_uint i = 0; i < platformCount; i++)
		{
			if (wydajnosc2[i] >= wydajnosc2[pointer])
				pointer = i;
		}
	}

	//cout << "pointer   " << pointer << endl;

	if (GPU)
	{
		// tworzymy w koncu liste urzadzen do platformy wybranej powy¿ej 
	cout << " tworzymy w koncu liste urzadzen do platformy wybranej powy¿ej " << endl;
	error = clGetDeviceIDs(list_of_platforms[pointer], CL_DEVICE_TYPE_GPU, 0, NULL, &deviceCount2[pointer]);
	cout << deviceCount2[pointer] << "device count dla " << pointer << " platformy" << endl;
	list_of_devices2 = new cl_device_id[deviceCount2[pointer]];
	cout << "hej" << deviceCount2[pointer] << endl;
	error = clGetDeviceIDs(list_of_platforms[pointer], CL_DEVICE_TYPE_GPU, deviceCount2[pointer], list_of_devices2, NULL);
	cout << "hej" << deviceCount2[pointer] << endl;
	for (cl_uint j = 0; j < deviceCount2[pointer]; j++)
	{
		wydajnosc2[pointer] = 0;
		clGetDeviceInfo(list_of_devices2[j], CL_DEVICE_NAME, sizeof(wyraz), wyraz, NULL);
		cout << "Device name: " << wyraz << endl;
		clGetDeviceInfo(list_of_devices2[j], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(liczba), &liczba, NULL);
		cout << "liczba work group: " << liczba << endl;
		clGetDeviceInfo(list_of_devices2[j], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(units), &units, NULL);
		cout << "units: " << units << endl;
		clGetDeviceInfo(list_of_devices2[j], CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(frequency), &frequency, NULL);
		cout << "frequency: " << frequency << endl;
		wydajnosc2[pointer] = wydajnosc2[pointer] + ((cl_uint)liczba)*units*frequency;
	}
	cout << "ogolna wydajnosc2 dla " << pointer << " tej platformy wynosi    " << wydajnosc2[pointer] << endl;
	}
	else
	{
		// tworzymy w koncu liste urzadzen do platformy wybranej powy¿ej 
		cout << " tworzymy w koncu liste urzadzen do platformy wybranej powy¿ej " << endl;
		error = clGetDeviceIDs(list_of_platforms[pointer], CL_DEVICE_TYPE_CPU, 0, NULL, &deviceCount2[pointer]);
		cout << deviceCount2[pointer] << "device count dla " << pointer << " platformy" << endl;
		list_of_devices2 = new cl_device_id[deviceCount2[pointer]];
		cout << "hej" << deviceCount2[pointer] << endl;
		error = clGetDeviceIDs(list_of_platforms[pointer], CL_DEVICE_TYPE_CPU, deviceCount2[pointer], list_of_devices2, NULL);
		cout << "hej" << deviceCount2[pointer] << endl;
		for (cl_uint j = 0; j < deviceCount2[pointer]; j++)
		{
			wydajnosc2[pointer] = 0;
			clGetDeviceInfo(list_of_devices2[j], CL_DEVICE_NAME, sizeof(wyraz), wyraz, NULL);
			cout << "Device name: " << wyraz << endl;
			clGetDeviceInfo(list_of_devices2[j], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(liczba), &liczba, NULL);
			cout << "liczba work group: " << liczba << endl;
			clGetDeviceInfo(list_of_devices2[j], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(units), &units, NULL);
			cout << "units: " << units << endl;
			clGetDeviceInfo(list_of_devices2[j], CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(frequency), &frequency, NULL);
			cout << "frequency: " << frequency << endl;
			wydajnosc2[pointer] = wydajnosc2[pointer] + ((cl_uint)liczba)*units*frequency;
		}
		cout << "ogolna wydajnosc2 dla " << pointer << " tej platformy wynosi    " << wydajnosc2[pointer] << endl;
	}	
	
	/*odczytywanie lancucha znakow */
	FILE *fp;
	const char fileName[] = "./macierz.cl";
	size_t source_size;
	char *source_str;

	/* Load kernel source file */
	fp = fopen(fileName, "rb");
	if (!fp) {
		fprintf(stderr, "Failed to load kernel.\n");
		exit(1);
	}
	source_str = (char *)malloc(MAX_SOURCE_SIZE);
	source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
	fclose(fp);

	/* Initialize input data */
	for (i = 0; i <LICZBA_ITERACJI; i++)
	{
		A[i] = (float)(i + 1);
	}
	
	/* Create OpenCL context */
	context = clCreateContext(NULL, deviceCount2[pointer], list_of_devices2, NULL, NULL, &ret);
	//cout << ret << endl;
	
	/* Create Command Queue */
	command_queue = clCreateCommandQueue(context, list_of_devices2[0], 0, &ret);

	/* Create Buffer Object */
	Amobj = clCreateBuffer(context, CL_MEM_READ_WRITE, LICZBA_ITERACJI * sizeof(float), NULL, &ret);
	Cmobj = clCreateBuffer(context, CL_MEM_READ_WRITE, LICZBA_ITERACJI * sizeof(float), NULL, &ret);

	/* Copy input data to the memory buffer */
	ret = clEnqueueWriteBuffer(command_queue, Amobj, CL_TRUE, 0, LICZBA_ITERACJI * sizeof(float), A, 0, NULL, NULL);

	/* Create kernel program from source file*/
	program = clCreateProgramWithSource(context, 1, (const char **)&source_str, (const size_t *)&source_size, &ret);
	ret = clBuildProgram(program, 1, list_of_devices2, NULL, NULL, NULL);

	/* Create data parallel OpenCL kernel */
	kernel = clCreateKernel(program, "mnozenie", &ret);

	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&Amobj);
	ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&Cmobj);

	size_t global_item_size = LICZBA_ITERACJI;
	size_t local_item_size = 10;

	/* Execute OpenCL kernel as data parallel */
	ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL,
		&global_item_size, &local_item_size, 0, NULL, NULL);

	/* Transfer result to host */
	ret = clEnqueueReadBuffer(command_queue, Cmobj, CL_TRUE, 0, LICZBA_ITERACJI * sizeof(float), C, 0, NULL, NULL);

	/* Display Results */
	/*for (i = 0; i < LICZBA_ITERACJI; i++)
	{
		printf("%f ", C[i]);
	}
	printf("\n");
	/*for (i = 0; i < LICZBA_ITERACJI; i++)
	{
	printf("%7.2f ", A[i]);
	}*/
	suma = 0;

	for (int i = 0; i < LICZBA_ITERACJI; i++)
	{
		suma = suma + C[i];
	}

	//obliczam liczbê Pi
	liczba_pi = sqrt(suma * 6);

	printf("liczba Pi wynosi ,%f", liczba_pi);

	/* Finalization */
	ret = clFlush(command_queue);
	ret = clFinish(command_queue);
	ret = clReleaseKernel(kernel);
	ret = clReleaseProgram(program);
	//ret = clReleaseMemObject(memobj);
	ret = clReleaseCommandQueue(command_queue);
	ret = clReleaseContext(context);

	free(source_str);
	free(A);
	free(C);

	getchar();
	return 0;
}