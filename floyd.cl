__kernel void FloydWarshall( int elements, __global float* input, __global float* output) {

    int row = get_global_id(0);

    int col = get_global_id(1);

    int i=0;

    barrier(CLK_GLOBAL_MEM_FENCE);

    for (i=0; i<elements; i++) {
        float compare = input[row*elements+i] + input[i*elements+col];

        if (input[row*elements+col] > compare) {
            output[row*elements+col] = compare;
        } else {
            output[row*elements+col] = input[row*elements+col];
        }

        //Sync threads
        barrier(CLK_GLOBAL_MEM_FENCE);

        input[row*elements+col] = output[row*elements+col];

        //Sync threads
        barrier(CLK_GLOBAL_MEM_FENCE);
    };

};