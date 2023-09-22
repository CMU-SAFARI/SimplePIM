log: host.c
	@mkdir -p bin
	gcc --std=c99 -ldl -lm -fopenmp -O3 host.c -o bin/host ../../lib/processing/ProcessingHelperHost.c ../../lib/communication/CommHelper.c ../../lib/communication/CommOps.c ../../lib/management/SmallTableInit.c ../../lib/management/Management.c ../../lib/processing/gen_red/GenRed.c `dpu-pkg-config --cflags --libs dpu`