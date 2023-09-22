va: host.c
	@mkdir -p bin
	gcc --std=c99 -lm -fopenmp -O3 host.c -o bin/host ../../lib/processing/ProcessingHelperHost.c ../../lib/communication/CommHelper.c  ../../lib/communication/CommOps.c ../../lib/management/SmallTableInit.c ../../lib/management/Management.c ../../lib/processing/map/Map.c ../../lib/processing/zip/Zip.c `dpu-pkg-config --cflags --libs dpu`