
void initOpenCL(
	cl_device_id     *devices, 
	const cl_uint     maxDevices, 
	cl_context       &context,
	cl_command_queue &commandQueue,
	cl_int           &status
) {
	cl_uint platformCount;
	cl_platform_id platform = NULL;
	const int maxPlatforms = 8;
	cl_platform_id platforms[maxPlatforms];
	status = clGetPlatformIDs(maxPlatforms, platforms, &platformCount);
	if (status != CL_SUCCESS) {
		cout << "failed: clGetPlatformIDs" << endl;
		return;
	}
	if (platformCount < 1) {
		cout << "failed to find any OpenCL platforms" << endl;
		return;
	}
	platform = platforms[0];
	cl_uint deviceCount = 0;
	status = clGetDeviceIDs(
		platform, CL_DEVICE_TYPE_GPU, maxDevices, devices, &deviceCount
	);
	if (status != CL_SUCCESS) {
		cout << "failed: clGetDeviceIDs" << endl;
		return;
	}
	if (!deviceCount) {
		status = clGetDeviceIDs(
			platform, CL_DEVICE_TYPE_CPU, maxDevices, devices, &deviceCount
		);
		if (!deviceCount) {
			cout << "failed to find any GPU or CPU devices" << endl;
			return;
		}
		cout << "no GPU devices found, using CPU instead" << endl;
	}	
	context = clCreateContext(NULL,1, devices, NULL,NULL, &status);
	if (status != CL_SUCCESS) {
		cout << "failed: clCreateContext" << endl;
		return;
	}
	commandQueue = clCreateCommandQueueWithProperties(
		context, devices[0], 0, &status
	);
	if (status != CL_SUCCESS) {
		cout << "failed: clCreateCommandQueueWithProperties" << endl;
		return;
	}
}
