
cc=g++
cflags= -Wall `sdl2-config --cflags --libs`
OpenCL= -lOpenCL -I/opt/AMDAPPSDK-3.0-0-Beta/include -L/opt/AMDAPPSDK-3.0-0-Beta/lib/x86_64/

all: cpu_st cpu_mt gpu_buf gpu_img
cpu_st: cpu_st.cpp shared_all.hpp
	$(cc) -o $@ $< $(cflags)
cpu_mt: cpu_mt.cpp shared_all.hpp
	$(cc) -o $@ $< $(cflags)
gpu_buf: gpu_buf.cpp shared_all.hpp
	$(cc) -o $@ $< $(cflags) $(OpenCL)
gpu_img: gpu_img.cpp shared_all.hpp
	$(cc) -o $@ $< $(cflags) $(OpenCL)

clean:
	rm  cpu_st cpu_mt gpu_buf gpu_img
