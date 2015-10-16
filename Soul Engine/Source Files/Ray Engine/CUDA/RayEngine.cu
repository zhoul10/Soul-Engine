#include "Ray Engine\RayEngine.cuh"

uint raySeedGl=0;


inline __device__ int getGlobalIdx_1D_1D()
{
	return blockIdx.x *blockDim.x + threadIdx.x;
}

__global__ void EngineExecute(uint n, RayJob* jobs, uint raySeed){

	uint index = getGlobalIdx_1D_1D();

	if (index < n){

	thrust::default_random_engine rng(randHash(raySeed) * randHash(index));
	thrust::uniform_real_distribution<float> uniformDistribution(0.0f, 1.0f);


		RayJob* job = jobs;
		uint startIndex = 0;

		while (jobs->nextRay != NULL && !(index < startIndex + jobs->rayAmount)){
			startIndex += job->rayAmount*job->samples;
			job = job->nextRay;
		}

		uint localIndex = index - startIndex;

		Ray ray;
		job->camera->SetupRay(localIndex, ray, rng, uniformDistribution);

		//uint x = localIndex / job->camera->resolution.x;
		//uint y = localIndex % job->camera->resolution.y;

		//calculate something


			
			/*float jitterValueX = uniformDistribution(rng);
			if (jitterValueX>0.6f){
				job->resultsT[localIndex] = make_float4(0.0f, 0.0f, 0.0f, 1.0f);
			}
			else{*/
				job->resultsT[localIndex] = make_float4(1.0f, 1.0f, 1.0f, 1.0f);
			//}
	}
}

__host__ void ProcessJobs(RayJob* jobs){
	raySeedGl++;

	if (jobs!=NULL){
	uint n = 0;

	RayJob* temp = jobs;
	n += temp->rayAmount;
	while (temp->nextRay != NULL){
		temp = temp->nextRay;
		n += temp->rayAmount*temp->samples;
	}

	if (n!=0){

		int blockSize;   // The launch configurator returned block size 
		int minGridSize; // The minimum grid size needed to achieve the 
		// maximum occupancy for a full device launch 
		int gridSize;    // The actual grid size needed, based on input size 

		cudaOccupancyMaxPotentialBlockSize(&minGridSize, &blockSize,
			EngineExecute, 0, 0);
		// Round up according to array size 
		gridSize = (n + blockSize - 1) / blockSize;


		//execute engine


		cudaEvent_t start, stop; 
		float time; 
		cudaEventCreate(&start); 
		cudaEventCreate(&stop); 
		cudaEventRecord(start, 0);

		EngineExecute << <gridSize, blockSize >> >(n, jobs, raySeedGl);

		cudaEventRecord(stop, 0); 
		cudaEventSynchronize(stop); 
		cudaEventElapsedTime(&time, start, stop); 
		cudaEventDestroy(start); 
		cudaEventDestroy(stop);

		std::cout << "RayEngine Execution: " << time << "ms"<< std::endl;

		CudaCheck(cudaDeviceSynchronize());
	}
	}


}