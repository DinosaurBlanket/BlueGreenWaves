This repo is mainly intended as an example of how to use OpenCL with SDL.
There are four programs which all do the same thing, 
open a window and write pixel values.
"cpu_st"  runs single-threaded on the CPU.
"cpu_mt"  runs multi-threaded on the CPU.
"gpu_buf" runs on the GPU, writing to a buffer returned by "clCreateBuffer".
"gpu_img" runs on the GPU, writing to an image returned by "clCreateImage".
