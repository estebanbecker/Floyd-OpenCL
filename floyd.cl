__kernel void FloydWarshall( int elements, __global float* input, __global float* output) {

    int row = get_global_id(0);

    int col = get_global_id(1);

    int i=0;

    output[row*elements+col] = input[row*elements+col];

    barrier(CLK_GLOBAL_MEM_FENCE);

    for (i=0; i<elements; i++) {
        float compare = output[row*elements+i] + output[i*elements+col];

        if (output[row*elements+col] > compare) {
            output[row*elements+col] = compare;
        }

        //Sync threads
        barrier(CLK_GLOBAL_MEM_FENCE);
    }

};