__kernel void FloydWarshall(

    int elements,
    __global float* input,
    __global float* output) {

    int row = get_global_id(0);

    int col = get_global_id(1);

    int i=0;

    for (i=0; i<elements; i++) {
        float compare = input[row*elements+i] + input[i*elements+col];

        if (input[row*elements+col] > compare) {
            input[row*elements+col] = compare;
        }

        //Sync threads
        barrier(CLK_GLOBAL_MEM_FENCE);
    }

}

