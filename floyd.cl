__kernel void FloydWarshall(int k, int elements, __global float* input, __global float* output) {
    int row = get_global_id(0);
    int col = get_global_id(1);

    float compare = input[row * elements + k] + input[k * elements + col];

    if (input[row * elements + col] > compare) {
        output[row * elements + col] = compare;
    } else {
        output[row * elements + col] = input[row * elements + col];
    }

    //barrier(CLK_GLOBAL_MEM_FENCE);

    input[row * elements + col] = output[row * elements + col];

};