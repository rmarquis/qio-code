static const char* radixSortSimpleKernelsDX11 = \
	"typedef uint u32;\n"
	"\n"
	"#define GET_GROUP_IDX groupIdx.x\n"
	"#define GET_LOCAL_IDX localIdx.x\n"
	"#define GET_GLOBAL_IDX globalIdx.x\n"
	"#define GROUP_LDS_BARRIER GroupMemoryBarrierWithGroupSync()\n"
	"#define DEFAULT_ARGS uint3 globalIdx : SV_DispatchThreadID, uint3 localIdx : SV_GroupThreadID, uint3 groupIdx : SV_GroupID\n"
	"#define AtomInc(x) InterlockedAdd(x, 1)\n"
	"#define AtomInc1(x, out) InterlockedAdd(x, 1, out)\n"
	"\n"
	"//	takahiro end\n"
	"#define WG_SIZE 128\n"
	"#define NUM_PER_WI 4\n"
	"\n"
	"#define GET_GROUP_SIZE WG_SIZE\n"
	"\n"
	"typedef struct\n"
	"{\n"
	"	u32 m_key;\n"
	"	u32 m_value;\n"
	"}SortData;\n"
	"\n"
	"cbuffer SortCB : register( b0 )\n"
	"{\n"
	"	u32 m_startBit;\n"
	"	u32 m_numGroups;\n"
	"	u32 m_padding[2];\n"
	"};\n"
	"\n"
	"StructuredBuffer<SortData> sortData : register( t0 );\n"
	"RWStructuredBuffer<u32> ldsHistogramOut : register( u0 );\n"
	"\n"
	"groupshared u32 ldsHistogram[16][256];\n"
	"\n"
	"[numthreads(WG_SIZE, 1, 1)]\n"
	"void LocalCountKernel( DEFAULT_ARGS )\n"
	"{\n"
	"	int lIdx = GET_LOCAL_IDX;\n"
	"	int gIdx = GET_GLOBAL_IDX;\n"
	"\n"
	"	for(int i=0; i<16; i++)\n"
	"	{\n"
	"		ldsHistogram[i][lIdx] = 0.f;\n"
	"		ldsHistogram[i][lIdx+128] = 0.f;\n"
	"	}\n"
	"\n"
	"	GROUP_LDS_BARRIER;\n"
	"\n"
	"	SortData datas[NUM_PER_WI];\n"
	"	datas[0] = sortData[gIdx*NUM_PER_WI+0];\n"
	"	datas[1] = sortData[gIdx*NUM_PER_WI+1];\n"
	"	datas[2] = sortData[gIdx*NUM_PER_WI+2];\n"
	"	datas[3] = sortData[gIdx*NUM_PER_WI+3];\n"
	"\n"
	"	datas[0].m_key = (datas[0].m_key >> m_startBit) & 0xff;\n"
	"	datas[1].m_key = (datas[1].m_key >> m_startBit) & 0xff;\n"
	"	datas[2].m_key = (datas[2].m_key >> m_startBit) & 0xff;\n"
	"	datas[3].m_key = (datas[3].m_key >> m_startBit) & 0xff;\n"
	"\n"
	"	int tableIdx = lIdx%16;\n"
	"\n"
	"	AtomInc(ldsHistogram[tableIdx][datas[0].m_key]);\n"
	"	AtomInc(ldsHistogram[tableIdx][datas[1].m_key]);\n"
	"	AtomInc(ldsHistogram[tableIdx][datas[2].m_key]);\n"
	"	AtomInc(ldsHistogram[tableIdx][datas[3].m_key]);\n"
	"\n"
	"	GROUP_LDS_BARRIER;\n"
	"\n"
	"	u32 sum0, sum1;\n"
	"	sum0 = sum1 = 0;\n"
	"	for(int i=0; i<16; i++)\n"
	"	{\n"
	"		sum0 += ldsHistogram[i][lIdx];\n"
	"		sum1 += ldsHistogram[i][lIdx+128];\n"
	"	}\n"
	"\n"
	"	ldsHistogramOut[lIdx*m_numGroups+GET_GROUP_IDX] = sum0;\n"
	"	ldsHistogramOut[(lIdx+128)*m_numGroups+GET_GROUP_IDX] = sum1;\n"
	"}\n"
	"\n"
	"\n"
	"RWStructuredBuffer<SortData> sortDataOut : register( u0 );\n"
	"RWStructuredBuffer<u32> scannedHistogram : register( u1 );\n"
	"\n"
	"groupshared u32 ldsCurrentLocation[256];\n"
	"\n"
	"[numthreads(WG_SIZE, 1, 1)]\n"
	"void ScatterKernel( DEFAULT_ARGS )\n"
	"{\n"
	"	int lIdx = GET_LOCAL_IDX;\n"
	"	int gIdx = GET_GLOBAL_IDX;\n"
	"\n"
	"	{\n"
	"		ldsCurrentLocation[lIdx] = scannedHistogram[lIdx*m_numGroups+GET_GROUP_IDX];\n"
	"		ldsCurrentLocation[lIdx+128] = scannedHistogram[(lIdx+128)*m_numGroups+GET_GROUP_IDX];\n"
	"	}\n"
	"\n"
	"	GROUP_LDS_BARRIER;\n"
	"\n"
	"	SortData datas[NUM_PER_WI];\n"
	"	int keys[NUM_PER_WI];\n"
	"	datas[0] = sortData[gIdx*NUM_PER_WI+0];\n"
	"	datas[1] = sortData[gIdx*NUM_PER_WI+1];\n"
	"	datas[2] = sortData[gIdx*NUM_PER_WI+2];\n"
	"	datas[3] = sortData[gIdx*NUM_PER_WI+3];\n"
	"\n"
	"	keys[0] = (datas[0].m_key >> m_startBit) & 0xff;\n"
	"	keys[1] = (datas[1].m_key >> m_startBit) & 0xff;\n"
	"	keys[2] = (datas[2].m_key >> m_startBit) & 0xff;\n"
	"	keys[3] = (datas[3].m_key >> m_startBit) & 0xff;\n"
	"\n"
	"	int dst[NUM_PER_WI];\n"
	"	for(int i=0; i<WG_SIZE; i++)\n"
	"//	for(int i=0; i<m_padding[0]; i++)	//	to reduce compile time\n"
	"	{\n"
	"		if( i==lIdx )\n"
	"		{\n"
	"			AtomInc1(ldsCurrentLocation[keys[0]], dst[0]);\n"
	"			AtomInc1(ldsCurrentLocation[keys[1]], dst[1]);\n"
	"			AtomInc1(ldsCurrentLocation[keys[2]], dst[2]);\n"
	"			AtomInc1(ldsCurrentLocation[keys[3]], dst[3]);\n"
	"		}\n"
	"		GROUP_LDS_BARRIER;\n"
	"	}\n"
	"	sortDataOut[dst[0]] = datas[0];\n"
	"	sortDataOut[dst[1]] = datas[1];\n"
	"	sortDataOut[dst[2]] = datas[2];\n"
	"	sortDataOut[dst[3]] = datas[3];\n"
	"}\n"
	"";