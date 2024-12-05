#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <CL/cl.h>


// Fonction pour initialiser la matrice d'adjacence
float* initialiserGraphe(int n) {
    // Allouer de la mémoire pour la matrice d'adjacence
    float* matrice = (float*)malloc(n * n * sizeof(float));

    // Initialiser la matrice d'adjacence
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i == j) {
                matrice[i * n + j] = 0.0f; // L'arc de i à i vaut 0
            } else if (j == (i + 1) % n) {
                matrice[i * n + j] = (i == n - 1) ? 5.0f : 2.0f; // Arc de i à i+1 vaut 2, et arc de n-1 à 0 vaut 5
            } else {
                matrice[i * n + j] = 5.0f * n; // Tous les autres arcs valent 5n
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
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("%.2f ", matrice[i * n + j]);
        }
        printf("\n");
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
int main() {
    char* programSource = load_program_source("floyd.cl");

    // This code executes on the OpenCL host

    // Host data
    float *A = NULL;  // Input array
    float *C = NULL;  // Output array

    // Elements in each row and column
    const int elements = 5;

    // Compute the size of the data
    size_t datasize = sizeof(float)*elements*elements;

    // Allocate space for input/output data
    A = initialiserGraphe(datasize);
    C = (float*)malloc(datasize);
    cl_int status;

    //-----------------------------------------------------
    // STEP 1: Discover and initialize the platforms
    //-----------------------------------------------------

    cl_uint numPlatforms = 0;
    cl_platform_id *platforms = NULL;

    // 
    // Calcul du nombre de plateformes
    //
    status = clGetPlatformIDs(0, NULL, &numPlatforms);
    printf("Number of platforms = %d\n", numPlatforms);

    // Allocation de l'espace
    platforms =
    (cl_platform_id*)malloc(
                            numPlatforms*sizeof(cl_platform_id));

    // Trouver les plateformes
    status = clGetPlatformIDs(numPlatforms, platforms,
                              NULL);

    char Name[1000];
    clGetPlatformInfo(platforms[0], CL_PLATFORM_NAME, sizeof(Name), Name, NULL);
    printf("Name of platform : %s\n", Name);
    fflush(stdout);

    //-----------------------------------------------------
    // STEP 2: Discover and initialize the devices
    //-----------------------------------------------------

    cl_uint numDevices = 0;
    cl_device_id *devices = NULL;

    // 
    // calcul du nombre de périphériques
    //
    status = clGetDeviceIDs(
                            platforms[0],
                            CL_DEVICE_TYPE_ALL,
                            0,
                            NULL,
                            &numDevices);

    printf("Number of devices = %d\n", (int)numDevices);

    // Allocation de l'espace
    devices =
    (cl_device_id*)malloc(
                          numDevices*sizeof(cl_device_id));

    // Trouver les périphériques
    status = clGetDeviceIDs(
                            platforms[0],
                            CL_DEVICE_TYPE_ALL,
                            numDevices,
                            devices,
                            NULL);

    for (int i=0; i<numDevices; i++){
        clGetDeviceInfo(devices[i], CL_DEVICE_NAME, sizeof(Name), Name, NULL);
        printf("Name of device %d: %s\n\n", i, Name);
    }

    //-----------------------------------------------------
    // STEP 3: Create a context
    //-----------------------------------------------------
    printf("Création du contexte\n");
    fflush(stdout);    

    cl_context context = NULL;

    context = clCreateContext(
                              NULL,
                              numDevices,
                              devices,
                              NULL,
                              NULL,
                              &status);

    //-----------------------------------------------------
    // STEP 4: Create a command queue
    //-----------------------------------------------------

    printf("Création de la file d'attente\n");
    fflush(stdout);
    cl_command_queue cmdQueue;

    cmdQueue = clCreateCommandQueue(
                                    context,
                                    devices[0],
                                    0,
                                    &status);

    //-----------------------------------------------------
    // STEP 5: Create device buffers
    //-----------------------------------------------------

    printf("Création des buffers\n");
    fflush(stdout);

    cl_mem bufferA;
    cl_mem bufferC; 

    bufferA = clCreateBuffer(
                             context,
                             CL_MEM_READ_WRITE,
                             datasize,
                             NULL,
                             &status);


    bufferC = clCreateBuffer(
                             context,
                             CL_MEM_WRITE_ONLY,
                             datasize,
                             NULL,
                             &status);

    //-----------------------------------------------------
    // STEP 6: Write host data to device buffers
    //-----------------------------------------------------

    printf("Ecriture dans les buffers\n");
    fflush(stdout);

    status = clEnqueueWriteBuffer(
                                  cmdQueue,
                                  bufferA,
                                  CL_TRUE,
                                  0,
                                  datasize,
                                  A,
                                  0,
                                  NULL,
                                  NULL);

    //-----------------------------------------------------
    // STEP 7: Create and compile the program
    //-----------------------------------------------------

    printf("CreateProgramWithSource\n");
    fflush(stdout);

    cl_program program = clCreateProgramWithSource(
                                                   context,
                                                   1,
                                                   (const char**)&programSource,
                                                   NULL,
                                                   &status);

    //printf("%s\n",programSource);

    printf("Compilation\n");
    fflush(stdout);
    status = clBuildProgram(
                            program,
                            numDevices,
                            devices,
                            NULL,
                            NULL,
                            NULL);

    if (status) printf("ERREUR A LA COMPILATION: %d\n", status);

    //-----------------------------------------------------
    // STEP 8: Create the kernel
    //-----------------------------------------------------

    cl_kernel kernel = NULL;

    printf("Création du kernel\n");
    fflush(stdout);
    kernel = clCreateKernel(program, "simpleMultiply", &status);

    //-----------------------------------------------------
    // STEP 9: Set the kernel arguments
    //-----------------------------------------------------

    // Associate the input and output buffers with the
    // kernel
    // using clSetKernelArg()

    printf("Passage des paramètres\n");
    fflush(stdout);

    status  = clSetKernelArg(
                             kernel,
                             0,
                             sizeof(cl_int),
                             (void*) &elements);

    status  = clSetKernelArg(
                             kernel,
                             1,
                             sizeof(cl_mem),
                             (void*) &bufferA);

    status = clSetKernelArg(
                             kernel,
                             2,
                             sizeof(cl_mem),
                             (void*) &bufferC);


    //-----------------------------------------------------
    // STEP 10: Configure the work-item structure
    //-----------------------------------------------------

    // Define an index space (global work size) of work
    // items for
    // execution. A workgroup size (local work size) is not
    // required,
    // but can be used.

    size_t MaxGroup;
    clGetDeviceInfo(devices[0],CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &MaxGroup, NULL);

    clGetDeviceInfo(devices[0],CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &MaxGroup, NULL);

    printf("CL_DEVICE_MAX_WORK_GROUP_SIZE = %d\n", (int) MaxGroup);

    size_t MaxItems[3];
    clGetDeviceInfo(devices[0],CL_DEVICE_MAX_WORK_ITEM_SIZES, 3*sizeof(size_t), MaxItems, NULL);
    printf("CL_DEVICE_MAX_WORK_ITEM_SIZES = (%d, %d, %d)\n", (int) MaxItems[0], (int)MaxItems[1], (int)MaxItems[2]);

    size_t globalWorkSize[2]={elements, elements};
    size_t localWorkSize[3]={20,20};
    // There are 'elements' work-items

    //-----------------------------------------------------
    // STEP 11: Enqueue the kernel for execution
    //-----------------------------------------------------

    // Execute the kernel by using
    // clEnqueueNDRangeKernel().
    // 'globalWorkSize' is the 1D dimension of the
    // work-items

    printf("Debut des appels\n");
    status = clEnqueueNDRangeKernel(
                                    cmdQueue,
                                    kernel,
                                    2,
                                    NULL,
                                    globalWorkSize,
                                    NULL,
                                    //localWorkSize,
                                    0,
                                    NULL,
                                    NULL);

    printf("Fin premier appel: status=%d\n", status);
    clFinish(cmdQueue);  // Pas nécessaire car la pile a été créée "In-order"

    //-----------------------------------------------------
    // STEP 12: Read the output buffer back to the host
    //-----------------------------------------------------

    // 
    //Lecture de la matrice C
    //
    clEnqueueReadBuffer(
                        cmdQueue,
                        bufferC,
                        CL_TRUE,
                        0,
                        datasize,
                        C,
                        0,
                        NULL,
                        NULL);

    afficherGraphe(A, elements);
    afficherGraphe(C, elements);



    //-----------------------------------------------------
    // STEP 13: Release OpenCL resources
    //-----------------------------------------------------

    // Free OpenCL resources
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
