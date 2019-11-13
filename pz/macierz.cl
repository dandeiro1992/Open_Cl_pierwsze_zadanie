__kernel void mnozenie(__global float* A,__global float* C)
{
	int i=get_global_id(0);
	C[i]=1/(A[i]*A[i]);
}