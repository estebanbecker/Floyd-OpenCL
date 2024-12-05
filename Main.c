#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <CL/cl.h>
#include <time.h>

// Fonction pour initialiser la matrice d'adjacence
float* initialiserGraphe(int n) {
    // Allouer de la mémoire pour la matrice d'adjacence
    float* matrice = (float*)malloc(n * n * sizeof(float));

    // Initialiser la matrice d'adjacence
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i == j) {
                // L'arc de i à i vaut 0
                matrice[i * n + j] = 0.0f; 
            } else if (j == (i + 1) % n) {
                // Arc de i à i+1 vaut 2, et arc de n-1 à 0 vaut 5
                matrice[i * n + j] = (i == n - 1) ? 5.0f : 2.0f;
            } else {
                // Tous les autres arcs valent 5n
                matrice[i * n + j] = 5.0f * n; 
            }
        }
    }

    return matrice;
}

// Fonction pour libérer la mémoire allouée pour la matrice d'adjacence
void libererGraphe(float* matrice) {
    free(matrice);
}

// Fonction pour afficher la matrice d'adjacence
void afficherGraphe(float* matrice, int n) {
    printf("Matrice d'adjacence:\n");
    printf("    ");
    for (int j = 0; j < (n > 20 ? 10 : n); j++) {
        printf("%6d ", j);
    }
    if (n > 20) {
        printf(" ... ");
        for (int j = n - 10; j < n; j++) {
            printf("%6d ", j);
        }
    }
    printf("\n");

    for (int i = 0; i < (n > 20 ? 10 : n); i++) {
        printf("%4d ", i);
        for (int j = 0; j < (n > 20 ? 10 : n); j++) {
            printf("%6.2f ", matrice[i * n + j]);
        }
        if (n > 20) {
            printf(" ... ");
            for (int j = n - 10; j < n; j++) {
                printf("%6.2f ", matrice[i * n + j]);
            }
        }
        printf("\n");
    }

    if (n > 20) {
        printf(" ... \n");
        for (int i = n - 10; i < n; i++) {
            printf("%4d ", i);
            for (int j = 0; j < 10; j++) {
                printf("%6.2f ", matrice[i * n + j]);
            }
            printf(" ... ");
            for (int j = n - 10; j < n; j++) {
                printf("%6.2f ", matrice[i * n + j]);
            }
            printf("\n");
        }
    }
}

// Lecture du fichier source
char* load_program_source(const char *filename) {
    FILE *fp;
    char *source;
    int sz = 0;
    struct stat status;
    fp = fopen(filename, "r");
    if (fp == 0) {
        // Handle error
    }
    if (stat(filename, &status) == 0)
        sz = (int) status.st_size;
    source = (char *) malloc(sz + 1);
    fread(source, sz, 1, fp);
    source[sz] = '\0';
    return source;
}

// Multiplication matricielle
int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s <taille du graphe>\n", argv[0]);
        return -1;
    }

    int elements = atoi(argv[1]);
    if (elements <= 0) {
        printf("La taille du graphe doit être un entier positif.\n");
        return -1;
    }

    char* programSource = load_program_source("floyd.cl");

    // This code executes on the OpenCL host

    // Host data
    float *A = NULL;  // Input array
    float *C = NULL;  // Output array

    // Compute the size of the data
    size_t datasize = sizeof(float) * elements * elements;

    // Allocate space for input/output data
    A = initialiserGraphe(elements);
    afficherGraphe(A, elements);
    C = (float*)malloc(datasize);
    cl_int status;

    //-----------------------------------------------------
    // STEP 1: Discover and initialize the platforms
    //-----------------------------------------------------

    cl_uint numPlatforms = 0;
    cl_platform_id *platforms = NULL;

    // Calcul du nombre de plateformes
    status = clGetPlatformIDs(0, NULL, &numPlatforms);
    printf("Number of platforms = %d\n", numPlatforms);

    // Allocation de l'espace
    platforms = (cl_platform_id*)malloc(numPlatforms * sizeof(cl_platform_id));

    // Trouver les plateformes
    status = clGetPlatformIDs(numPlatforms, platforms, NULL);

    char Name[1000];
    clGetPlatformInfo(platforms[0], CL_PLATFORM_NAME, sizeof(Name), Name, NULL);
    printf("Name of platform : %s\n", Name);
    fflush(stdout);

    //-----------------------------------------------------
    // STEP 2: Discover and initialize the devices
    //-----------------------------------------------------

    cl_uint numDevices = 0;
    cl_device_id *devices = NULL;

    // Calcul du nombre de périphériques
    status = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, 0, NULL, &numDevices);
    printf("Number of devices = %d\n", (int)numDevices);

    // Allocation de l'espace
    devices = (cl_device_id*)malloc(numDevices * sizeof(cl_device_id));

    // Trouver les périphériques
    status = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, numDevices, devices, NULL);

    for (int i = 0; i < numDevices; i++) {
        clGetDeviceInfo(devices[i], CL_DEVICE_NAME, sizeof(Name), Name, NULL);
        printf("Name of device %d: %s\n\n", i, Name);
    }

    //-----------------------------------------------------
    // STEP 3: Create a context
    //-----------------------------------------------------
    printf("Création du contexte\n");
    fflush(stdout);

    cl_context context = NULL;
    context = clCreateContext(NULL, numDevices, devices, NULL, NULL, &status);

    //-----------------------------------------------------
    // STEP 4: Create a command queue
    //-----------------------------------------------------
    printf("Création de la file d'attente\n");
    fflush(stdout);

    cl_command_queue cmdQueue;
    cmdQueue = clCreateCommandQueue(context, devices[0], 0, &status);

    if (status != CL_SUCCESS) {
        printf("Erreur lors de la création de la file d'attente: %d\n", status);
        return -1;
    }

    //-----------------------------------------------------
    // STEP 5: Create device buffers
    //-----------------------------------------------------
    printf("Création des buffers\n");
    fflush(stdout);

    cl_mem bufferA = clCreateBuffer(context, CL_MEM_READ_WRITE, datasize, NULL, &status);
    cl_mem bufferC = clCreateBuffer(context, CL_MEM_READ_WRITE, datasize, NULL, &status);

    if (status != CL_SUCCESS) {
        printf("Erreur lors de la création des buffers: %d\n", status);
        return -1;
    }

    //-----------------------------------------------------
    // STEP 6: Write host data to device buffers
    //-----------------------------------------------------
    printf("Ecriture dans les buffers\n");
    fflush(stdout);

    status = clEnqueueWriteBuffer(cmdQueue, bufferA, CL_TRUE, 0, datasize, A, 0, NULL, NULL);

    if (status != CL_SUCCESS) {
        printf("Erreur lors de l'écriture dans les buffers: %d\n", status);
        return -1;
    }

    //-----------------------------------------------------
    // STEP 7: Create and compile the program
    //-----------------------------------------------------
    printf("CreateProgramWithSource\n");
    fflush(stdout);

    cl_program program = clCreateProgramWithSource(context, 1, (const char**)&programSource, NULL, &status);

    printf("Compilation\n");
    fflush(stdout);
    // Compilation du programme
    status = clBuildProgram(program, numDevices, devices, NULL, NULL, NULL);

    if (status != CL_SUCCESS) {
        printf("ERREUR A LA COMPILATION: %d\n", status);

        // Récupérer la taille du journal de compilation
        size_t log_size;
        clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

        // Allouer de la mémoire pour le journal de compilation
        char *log = (char *)malloc(log_size);

        // Récupérer le journal de compilation
        clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

        // Afficher le journal de compilation
        printf("%s\n", log);
        free(log);
        return -1;
    }

    //-----------------------------------------------------
    // STEP 8: Create the kernel
    //-----------------------------------------------------
    cl_kernel kernel = NULL;
    printf("Création du kernel\n");
    fflush(stdout);
    kernel = clCreateKernel(program, "FloydWarshall", &status);

    if (status != CL_SUCCESS) {
        printf("Erreur lors de la création du kernel: %d\n", status);
        return -1;
    }

    //-----------------------------------------------------
    // STEP 9: Set the kernel arguments
    //-----------------------------------------------------
    printf("Passage des paramètres\n");
    fflush(stdout);

    status = clSetKernelArg(kernel, 1, sizeof(int), (void*)&elements);
    status |= clSetKernelArg(kernel, 2, sizeof(cl_mem), (void*)&bufferA);
    status |= clSetKernelArg(kernel, 3, sizeof(cl_mem), (void*)&bufferC);

    if (status != CL_SUCCESS) {
        printf("Erreur lors du passage des paramètres: %d\n", status);
        return -1;
    }

    //-----------------------------------------------------
    // STEP 10: Configure the work-item structure
    //-----------------------------------------------------
    size_t globalWorkSize[2] = { elements, elements };

    //-----------------------------------------------------
    // STEP 11: Enqueue the kernel for execution
    //-----------------------------------------------------
    printf("Debut des appels\n");

    // Start timer
    clock_t start = clock();

    for (int k = 0; k < elements; k++) {
        // Set the current iteration as a kernel argument
        status = clSetKernelArg(kernel, 0, sizeof(int), (void*)&k);

        if (status != CL_SUCCESS) {
            printf("Erreur lors du passage des paramètres: %d\n", status);
            return -1;
        }

        // Enqueue the kernel
        status = clEnqueueNDRangeKernel(cmdQueue, kernel, 2, NULL, globalWorkSize, NULL, 0, NULL, NULL);

        if (status != CL_SUCCESS) {
            printf("Erreur lors de l'exécution du kernel: %d\n", status);
            return -1;
        }

        // Wait for the kernel to finish
        clFinish(cmdQueue);


    }

    //-----------------------------------------------------
    // STEP 12: Read the output buffer back to the host
    //-----------------------------------------------------
    clEnqueueReadBuffer(cmdQueue, bufferA, CL_TRUE, 0, datasize, C, 0, NULL, NULL);

    printf("Matrice Input:\n");
    afficherGraphe(A, elements);

    printf("Matrice Output:\n");
    afficherGraphe(C, elements);

    // Stop timer
    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Temps d'exécution du noyau: %f secondes\n", time_spent);


    //-----------------------------------------------------
    // STEP 13: Release OpenCL resources
    //-----------------------------------------------------
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(cmdQueue);
    clReleaseMemObject(bufferA);
    clReleaseMemObject(bufferC);
    clReleaseContext(context);

    // Free host resources
    libererGraphe(A);
    free(C);
    free(platforms);
    free(devices);
    return 0;
}
