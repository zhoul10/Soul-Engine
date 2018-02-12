#pragma once

#include "Multithreading\Scheduler.h"
#include "Compute\DeviceRasterBuffer.h"
#include "Compute\CUDA\CUDABuffer.h"
#include "Raster Engine\RasterBackend.h"
#include "Raster Engine\OpenGL\OpenGLBuffer.h"

#include "Compute\ComputeDevice.h"
#include "Utility/CUDA/CudaHelper.cuh"

#include <cuda_runtime_api.h>
#include <cuda_gl_interop.h>


template <class T>
class CUDARasterBuffer : public CUDABuffer<T>, public DeviceRasterBuffer<T> {

public:

	//Types

	typedef T                                     value_type;
	typedef uint		                          size_type;


	//Construction and Destruction 
	
	CUDARasterBuffer(const ComputeDevice&);

	~CUDARasterBuffer();


	//Data Migration
	
	void MapResources() override;

	void UnmapResources() override;

	void BindData(uint) override;

	void Resize(uint) override;

	void Move(const ComputeDevice&) override;

private:

	Buffer* rasterBuffer = nullptr;

	struct cudaGraphicsResource* cudaBuffer;

};

template <class T>
CUDARasterBuffer<T>::CUDARasterBuffer(const ComputeDevice& device):
	DeviceRasterBuffer(device),
	CUDABuffer(device),
	DeviceBuffer(device),
	cudaBuffer(nullptr)
{

}

template <class T>
CUDARasterBuffer<T>::~CUDARasterBuffer()
{

};

template <class T>
void CUDARasterBuffer<T>::MapResources() 
{

	if (RasterBackend::backend == OpenGL) {

		CUDARasterBuffer* buff = this;
		Scheduler::AddTask(LAUNCH_IMMEDIATE, FIBER_HIGH, true, [&buff]() {
			RasterBackend::MakeContextCurrent();

			CudaCheck(cudaGraphicsMapResources(1, &buff->cudaBuffer, nullptr));

			size_t num_bytes;
			T* ptr = nullptr;
			CudaCheck(cudaGraphicsResourceGetMappedPointer((void **)&ptr, &num_bytes,
				buff->cudaBuffer));

			buff->CUDABuffer::buffer = ptr;
		});


		Scheduler::Block();

	}
	else {
		//TODO
		//Vulkan Stuff


	}
}

template <class T>
void CUDARasterBuffer<T>::UnmapResources() {

	if (RasterBackend::backend == OpenGL) {

		CUDARasterBuffer* buff = this;

		Scheduler::AddTask(LAUNCH_IMMEDIATE, FIBER_HIGH, true, [&buff]() {
			RasterBackend::MakeContextCurrent();
			CudaCheck(cudaGraphicsUnmapResources(1, &buff->cudaBuffer, nullptr));
		});
		Scheduler::Block();

		CUDABuffer<T>::buffer = nullptr;
	}
	else {
		//TODO
		//Vulkan Stuff


	}
}

template <class T>
void CUDARasterBuffer<T>::BindData(uint pos)
{


	if (RasterBackend::backend == OpenGL) {

		Scheduler::AddTask(LAUNCH_IMMEDIATE, FIBER_HIGH, true, [this, &pos]() {
			RasterBackend::MakeContextCurrent();
			OpenGLBuffer* oglBuffer = static_cast<OpenGLBuffer*>(rasterBuffer);

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, pos, oglBuffer->GetBufferID());


		});
		Scheduler::Block();

	}
	else {
		//TODO
		//Vulkan Stuff


	}


}

template <class T>
void CUDARasterBuffer<T>::Resize(uint newSize) 
{

	if (newSize > 0) {

		/*const auto device = DeviceBuffer<T>::residentDevice.GetOrder();
		CudaCheck(cudaSetDevice(device));*/
		rasterBuffer = RasterBackend::CreateBuffer(newSize * sizeof(T));

		if (RasterBackend::backend == OpenGL) {

			Scheduler::AddTask(LAUNCH_IMMEDIATE, FIBER_HIGH, true, [this]() {
				RasterBackend::MakeContextCurrent();

				OpenGLBuffer* oglBuffer = static_cast<OpenGLBuffer*>(rasterBuffer);

				CudaCheck(cudaGraphicsGLRegisterBuffer(&cudaBuffer
					, oglBuffer->GetBufferID()
					, cudaGraphicsRegisterFlagsWriteDiscard));

			});
			Scheduler::Block();

		}
		else {
			//TODO
			//Vulkan Stuff


		}
	}
}

template <class T>
void CUDARasterBuffer<T>::Move(const ComputeDevice&)
{
	
}