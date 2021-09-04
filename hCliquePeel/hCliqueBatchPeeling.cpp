#include <stdlib.h>
#include <cstdio>
#include <stdbool.h>
#include <string.h>
#include <string>
#include <vector>
#include <utility>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <time.h>
#include <cstdarg>
#include "../header/heapLLU.h"
#include "GraphNew.hpp"
#include "../header/tool.hpp"
using namespace std;

#define debug 0

void initColStarDegree(Graph & g, double** dp, int h, int colorNum, int* color, int** CC)
{
	double* NotColor0 = new double[h]();
	double* MustColor0 = new double[h]();
	for (int i = 0; i < g.n; i++)
	{
		dp[i] = new double[h];
		int colorNum_i = 0;
		//int* C = new int[colorNum]();
		CC[i] = new int[colorNum]();
		for (int j = g.cd[i]; j < g.cd[i + 1]; j++)
		{
			int nbr = g.adj[j];
			CC[i][color[nbr]]++;
			colorNum_i = max(colorNum_i, color[nbr]);
		}

		//int* tol = new int[k + 1];

		NotColor0[0] = 1;
		for (int c = 1; c <= colorNum_i; c++)
		{
			for (int j = h - 1; j > 0; j--)
				NotColor0[j] = NotColor0[j - 1] * CC[i][c] + NotColor0[j];
		}

		for (int j = 1; j < h; j++)
		{
			MustColor0[j] = NotColor0[j - 1] * CC[i][0];
			dp[i][j] = MustColor0[j] + NotColor0[j];
		}
		fill(NotColor0, NotColor0 + h, 0.0);
		fill(MustColor0, MustColor0 + h, 0.0);
	}
	delete[] NotColor0;
	delete[] MustColor0;
}

void ColorfulStarCoreDecomp(Graph& g, double** dp, int h, int* color, int** CC, double* ColofulStarCoreNum)
{
	double* tmpDP = new double[g.n];
	for (int i = 0; i < g.n; i++) tmpDP[i] = dp[i][h - 1];
	bheapLLU<double>* heap = mkheapLLU<double>(g.n, tmpDP);

	double maxStarDegree = -1;
	for (int i = 0; i < g.n; i++)
	{
		maxStarDegree = max(maxStarDegree, dp[i][h - 1]);
	}
	printf("maxStarDegree = %lf\n", maxStarDegree);

	int leftN = g.n, leftM = g.e;

	double* NotColor = new double[h]();
	double* MustColor = new double[h]();

	double starCoreNum = 0;
	int times = 0, maxN = 0, maxM = 0;
	keyvalueLLU<double> kv;

	while (leftN > 0)
	{
		times++;
		double Min = 1e300;
		kv = popminLLU<double>(heap);

		//printf("id = %d value = %lf\n", kv.key, kv.value);

		if (kv.value > starCoreNum)
		{
			starCoreNum = kv.value;
			maxN = leftN;
			maxM = leftM;
		}

		if (times % 50000 == 0)
			printf("times = %d left nodes = %d tolMax = %lf maxN = %d maxM = %d density = %lf\n", times, leftN, starCoreNum, maxN, maxM, 1.0 * maxM / maxN);

		//////
		leftN--;
		int i = kv.key;
		leftM -= g.deg[i];
		ColofulStarCoreNum[i] = starCoreNum;

		for (int j = g.cd[i]; j < g.cd[i] + g.deg[i]; j++)
		{
			int nbr = g.adj[j];

			for (int p = g.cd[nbr]; p < g.cd[nbr] + g.deg[nbr]; p++)
			{
				int hnbr = g.adj[p];
				if (hnbr == i)
				{
					swap(g.adj[p], g.adj[g.cd[nbr] + g.deg[nbr] - 1]);
					g.deg[nbr]--;
					break;
				}
			}

			CC[nbr][color[i]]--;

			NotColor[0] = 1;
			for (int p = 1; p < h; p++)
			{
				MustColor[p] = NotColor[p - 1] * (CC[nbr][color[i]] + 1);
				NotColor[p] = dp[nbr][p] - MustColor[p];
			}

			for (int p = 1; p < h; p++)
			{
				MustColor[p] = NotColor[p - 1] * CC[nbr][color[i]];
				dp[nbr][p] = NotColor[p] + MustColor[p];
			}
			updateLLU<double>(heap, nbr, dp[nbr][h - 1]);
		}
	}

	/////
	printf("End: times = %d left nodes = %d tolMax = %lf maxN = %d maxM = %d density = %lf\n", times, leftN, starCoreNum, maxN, maxM, 1.0 * maxM / maxN);

	delete[] NotColor;
	delete[] MustColor;
}

void deleteNodes(Graph* g, int* delArray, int size)
{

	if (size == 1)
	{
		int u = delArray[0];
		for (int j = g->cd[u]; j < g->cd[u] + g->deg[u]; j++)
		{
			int v = g->adj[j];
			for (int k = g->cd[v]; k < g->cd[v] + g->deg[v]; k++)
			{
				int w = g->adj[k];
				if (w == u)
				{
					g->adj[k] = g->adj[g->cd[v] + g->deg[v] - 1];
					g->adj[g->cd[v] + g->deg[v] - 1] = w;
					g->deg[v]--;
					break;
				}
			}
		}
		//g->deg[u] = 0;
		return;
	}
}

int main(int argc, char** argv)
{
	auto t0 = getTime();

	Graph g;
	int h = atoi(argv[1]);
	cout << "Reading edgelist from file " << argv[2] << endl;
	g.readedgelist(argv[2]);
	cout << "Reading edgelist finished!" << endl;
	g.mkGraph();
	cout << "mkGraph finished!" << endl;
	auto t1 = getTime();


	int* subss = new int[g.n];
	for (int i = 0; i < g.n; i++) subss[i] = i;
	g.clique = new Clique(g.n, g.e, 7);

	h = 3;

	long long tol = 0;
	long long* cnt = new long long[g.n]();
	g.kClique(h, &tol, cnt);

	printf("Toltal cliques: %lld\n", tol);

	bheapLLU<long long>* cHeap = mkheapLLU<long long>(g.n, cnt);

	keyvalueLLU<long long> kv;

	long long curCliqueCore = 0, leftClique = tol;
	int leftN = g.n, leftM = g.e;
	int maxCliDenN = 0;	//the number of nodes in the subgraph achiving the largest hclique density;
	int maxCliDenM = 0;	//the number of edges in the subgraph achiving the largest hclique density;

	int maxCliCoreDenN = 0;	//the number of nodes in the subgraph achiving the largest hclique density;
	int maxCliCoreDenM = 0;	//the number of edges in the subgraph achiving the largest hclique density;
	double cliqueDensity = 0.0, curCliqueDensity, cliqueCoreDen = 0.0;

	int* nbrArr = new int[g.maxDeg], nbrNum;
	long long* nbrCnt = new long long[g.n](), nbrTol;
	long long maxCliDeg = 0;

	int coreTocore = 0, cntctc = 0;

	int* delBatch = new int[g.n](), delSize = 0;
	int curCliqueCoreNum = -1;
	bool* delLabel = new bool[g.n]();
	//peeling ordering infects maxCliqueDensity.
	while (leftN > 0)
	{
		if (topLLU<long long>(cHeap).value > curCliqueCoreNum)
		{

		}
		kv = popminLLU<long long>(cHeap);
		delBatch[delSize++] = kv.key;


		while (topLLU<long long>(cHeap).value <= curCliqueCoreNum)
		{
			kv = popminLLU<long long>(cHeap);
			delBatch[delSize++] = kv.key;
			delLabel[kv.key] = true;
		}

		for (int i = 0; i < delSize; i++)
		{
			deleteNodes(&g, &delBatch[i], 1);
		}
		
		nbrNum = 0;
		for (int i = 0; i < delSize; i++)
		{
			int u = delBatch[i];

			for (int j = g.cd[u]; j < g.cd[u] + g.deg[u]; j++)
			{
				int v = g.adj[j];
				if(delLabel[v] == false) 
					nbrArr[nbrNum++] = v;
			}
			nbrTol = 0;
			g.kCliqueNew(h - 1, &nbrTol, nbrCnt, nbrArr, nbrNum);

			leftClique -= nbrTol;



			for (int i = 0; i < nbrNum; i++)
			{
				int nbrId = nbrArr[i];
				cnt[nbrId] -= nbrCnt[nbrId];
				updateLLU(cHeap, nbrId, cnt[nbrId]);
				//printf("nbrId = %d, nbrCnt = %lld\n",nbrId, nbrCnt[nbrId]);
			}
			//printf("------------\n");


			for (int i = 0; i < nbrNum; i++)
				nbrCnt[nbrArr[i]] = 0;

			deleteNodes(&g, &delId, 1);
			leftN--;


		}

		





		long long cliqueDeg = kv.value;
		curCliqueDensity = 1.0 * leftClique / leftN;

		if (maxCliDeg < cliqueDeg)
		{
			cliqueCoreDen = curCliqueDensity;
			maxCliCoreDenN = leftN;
			maxCliCoreDenM = leftM;



			maxCliDeg = cliqueDeg;
			cntctc += coreTocore;
			printf("maxCliDeg = %lld -> %lld, coreTocore = %d, \t leftN = %d, \t tolcn = %d\n", maxCliDeg - 1, maxCliDeg, coreTocore, leftN, cntctc);

			coreTocore = 0;
		}
		coreTocore++;

		maxCliDeg = max(maxCliDeg, cliqueDeg);

		//printf("id = %d, cliqueDeg = %lld, leftClique = %lld, leftN = %d\n", kv.key, kv.value, leftClique, leftN);



		//if (g.n - leftN < 100)
			//printf("-------------------------maxCliDeg = %lld, delId = %d, leftN = %d, leftClique: %lld\n", maxCliDeg, kv.key, leftN, leftClique);


		if (cliqueDensity < curCliqueDensity)
		{
			cliqueDensity = curCliqueDensity;
			maxCliDenN = leftN;
			maxCliDenM = leftM;
			//cliqueDensity = 1.0 * leftClique / leftN;
		}

		int delId = kv.key;
		nbrNum = 0;
		for (int i = g.cd[delId]; i < g.cd[delId] + g.deg[delId]; i++)
		{
			int adj = g.adj[i];
			nbrArr[nbrNum++] = adj;
		}
		nbrTol = 0;
		g.kCliqueNew(h - 1, &nbrTol, nbrCnt, nbrArr, nbrNum);


		leftClique -= nbrTol;



		for (int i = 0; i < nbrNum; i++)
		{
			int nbrId = nbrArr[i];
			cnt[nbrId] -= nbrCnt[nbrId];
			updateLLU(cHeap, nbrId, cnt[nbrId]);
			//printf("nbrId = %d, nbrCnt = %lld\n",nbrId, nbrCnt[nbrId]);
		}
		//printf("------------\n");


		for (int i = 0; i < nbrNum; i++)
			nbrCnt[nbrArr[i]] = 0;

		deleteNodes(&g, &delId, 1);
		leftN--;
	}

	cntctc += coreTocore;
	printf("after maxCliDeg = %lld, coreTocore = %d, tolcn = %d\n", maxCliDeg, coreTocore, cntctc);

	printf("maxCliDenN = %d, cliqueDensity = %lf\n", maxCliDenN, cliqueDensity);
	printf("maxCliCoreDenN = %d, cliqueCoreDensity = %lf\n", maxCliCoreDenN, cliqueCoreDen);


	auto tClique = getTime();

	printf("- Overall time = %lfs\n", ((double)timeGap(t1, tClique)) / 1e6);

	return 0;



	int delSet[] = { 0,8,9,1,2,3,4,5,6,7 };
	for (int i = 0; i < 10; i++)
	{

	}




	h = 3;
	g.kClique(h, &tol, cnt);
	printf("%d-clique: %lld\n", h, tol);


	memset(cnt, 0, sizeof(long long) * g.n); tol = 0;
	h = 3;
	g.kClique(h, &tol, cnt);
	printf("%d-clique: %lld\n", h, tol);

	memset(cnt, 0, sizeof(long long) * g.n); tol = 0;
	h = 3;
	g.kCliqueNew(h, &tol, cnt, subss, g.n);
	printf("New %d-clique: %lld\n", h, tol);


	memset(cnt, 0, sizeof(long long) * g.n); tol = 0;
	h = 4;
	g.kClique(h, &tol, cnt);
	printf("%d-clique: %lld\n", h, tol);

	memset(cnt, 0, sizeof(long long) * g.n); tol = 0;
	h = 4;
	g.kCliqueNew(h, &tol, cnt, subss, g.n);
	printf("New %d-clique: %lld\n", h, tol);

	memset(cnt, 0, sizeof(long long) * g.n); tol = 0;
	h = 5;
	g.kClique(h, &tol, cnt);
	printf("%d-clique: %lld\n", h, tol);


	memset(cnt, 0, sizeof(long long) * g.n); tol = 0;
	h = 5;
	g.kCliqueNew(h, &tol, cnt, subss, g.n);
	printf("New %d-clique: %lld\n", h, tol);

	memset(cnt, 0, sizeof(long long) * g.n); tol = 0;
	h = 4;
	g.kClique(h, &tol, cnt);
	printf("%d-clique: %lld\n", h, tol);

	memset(cnt, 0, sizeof(long long) * g.n); tol = 0;
	h = 4;
	g.kCliqueNew(h, &tol, cnt, subss, g.n);
	printf("New %d-clique: %lld\n", h, tol);

	memset(cnt, 0, sizeof(long long) * g.n); tol = 0;
	h = 3;
	g.kClique(h, &tol, cnt);
	printf("%d-clique: %lld\n", h, tol);

	memset(cnt, 0, sizeof(long long) * g.n); tol = 0;
	h = 3;
	g.kCliqueNew(h, &tol, cnt, subss, g.n);
	printf("New %d-clique: %lld\n", h, tol);

	//bheapLLU<long long> *cheap = mkheapLLU<long long>(g.n, cnt);

	//int leftN = g.n;
	//keyvalueLLU<long long> kv;

	//double hCliqueDensity = tol/g.n;

	//long long leftCliquNum = tol;

	//while (leftN > 0)
	//{
	//	kv = popminLLU<long long>(cheap);
	//	leftN--;
	//	int i = kv.key;
	//	leftCliquNum -= cnt[i];



	//	for (int j = g.cd[i]; j < g.cd[i] + g.deg[i]; j++)
	//	{
	//		int nbr = g.adj[j];

	//		for (int p = g.cd[nbr]; p < g.cd[nbr] + g.deg[nbr]; p++)
	//		{
	//			int hnbr = g.adj[p];
	//			if (hnbr == i)
	//			{
	//				swap(g.adj[p], g.adj[g.cd[nbr] + g.deg[nbr] - 1]);
	//				g.deg[nbr]--;
	//				break;
	//			}
	//		}

	//		CC[nbr][color[i]]--;

	//		NotColor[0] = 1;
	//		for (int p = 1; p < h; p++)
	//		{
	//			MustColor[p] = NotColor[p - 1] * (CC[nbr][color[i]] + 1);
	//			NotColor[p] = dp[nbr][p] - MustColor[p];
	//		}

	//		for (int p = 1; p < h; p++)
	//		{
	//			MustColor[p] = NotColor[p - 1] * CC[nbr][color[i]];
	//			dp[nbr][p] = NotColor[p] + MustColor[p];
	//		}
	//		updateLLU<double>(heap, nbr, dp[nbr][h - 1]);
	//	}


	//	
	//}





	//return 0;

	int* color = new int[g.n];
	int colorNum = g.color(color);
	printf("colorNum = %d\n", colorNum);

	if (debug)
	{
		//int setColor[] = { 0,3,0,1,2,0,4,3,1,0 };
		//color = setColor;
		for (int i = 0; i < g.n; i++)
		{
			printf("id = %d color = %d\n", i, color[i]);
		}
	}

	double** dp = new double* [g.n];
	int** CC = new int* [g.n];
	initColStarDegree(g, dp, h, colorNum, color, CC);

	auto t2 = getTime();


	if (debug)
	{
		for (int i = 0; i < g.n; i++)
		{
			printf("id = %d starDegree = %lf\n", i, dp[i][h - 1]);
		}
	}

	double* ColofulStarCoreNum = new double[g.n];
	ColorfulStarCoreDecomp(g, dp, h, color, CC, ColofulStarCoreNum);


	if (debug)
	{
		for (int i = 0; i < g.n; i++)
		{
			printf("id = %d starDegree = %lf\n", i, ColofulStarCoreNum[i]);
		}
	}

	auto t3 = getTime();
	printf("- Overall time = %lfs\n", ((double)timeGap(t2, t3)) / 1e6);

	printf("The End\n");

	return 0;

}