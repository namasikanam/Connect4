#include <iostream>
#include <ctime>
#include <algorithm>
#include <cstdlib>
#include <map>
#include <vector>
#include "Point.h"
#include "Strategy.h"
#include <conio.h>
#include <atlstr.h>
#include <cstdio>

using namespace std;

double c = sqrt(2) / 2; // UCT算法中的待调参数，理论最优解是sqrt(2)
double threshold_of_uct = 1;

// 调试用
FILE* ferr = fopen("error.txt", "w");

// 一些全局变量
int m, n;
int nox, noy;

typedef unsigned long long ull;

struct state{
	int owner; // 在这个节点谁是先手，1是用户，2是程序（我是程序）。
	int board[12][12];
	int top[12];
	double win; // 平局的话，win += 0.5
	int number; // 访问次数
	int decisive; // -1表示这不是一个确定局面，0表示输了，1表示平局，2表示赢了。这是在下棋之前的考量。
	int child[12];
}tree[1000005];
int top_of_tree;

inline void print_tree(int node, int rest_depth) {
	fprintf(ferr, "win / number = %.1f / %d\n", tree[node].win, tree[node].number);
	fprintf(ferr, "decisive = %d\n", tree[node].decisive);
	for (int x = 0; x < m; ++x)
		for (int y = 0; y < n; ++y)
			fprintf(ferr, "%d%c", tree[node].board[x][y], " \n"[y == n - 1]);
	for (int y = n; y--;)
		if (tree[node].child[y] && rest_depth > 0)
			print_tree(tree[node].child[y], rest_depth - 1);
}

inline int checkLeaf(int node, int x, int y) { // 事实上，赢是不必判的，我们判断是它有没有输。
	static int dx[] = { 1, 0, 0, 1, 1}, dy[] = { 0, 1, -1, 1, -1};
	for (int i = 5; i--;) {
		int counter = 1;
		for (int tx = x + dx[i], ty = y + dy[i]; tx >= 0 && tx < m && ty >= 0 && ty < n && tree[node].board[tx][ty] == tree[node].board[x][y]; tx += dx[i], ty += dy[i]) ++counter;
		for (int tx = x - dx[i], ty = y - dy[i]; tx >= 0 && tx < m && ty >= 0 && ty < n && tree[node].board[tx][ty] == tree[node].board[x][y]; tx -= dx[i], ty -= dy[i]) ++counter;
		if (counter >= 4) {
			//fprintf(ferr, "you'll lose at direction (%d, %d)\n", dx[i], dy[i]);
			return 0;
		}
	}

	int count = 0;
	for (int x = m; x--;)
		for (int y = n; y--;)
			if ((count += tree[node].board[x][y] == 0) > 1)
				return -1;
	return 1;
}

int select(int node) { // 返回值表示这次是成功了还是失败了，成功失败是对于当前先手而言的。
	//if (node) {
		//fprintf(ferr, "-> select(%d)\n", node);
		//fprintf(ferr, "But top_of_tree = %d\n", top_of_tree);
	//}

	tree[node].number += 1;

	if (tree[node].decisive != -1) { // 叶子节点无法再扩展子节点。
		tree[node].win += tree[node].decisive / 2.0;

		// _cprintf("<- select(%d)\n", node);
		/*fprintf(ferr, "Go to a decisive node %d\n", node);*/

		return tree[node].decisive;
	}

	// 扩展子节点

	int maxy = -1;

	// 寻找制胜位
	tree[top_of_tree].owner = 3 - tree[node].owner;
	memcpy(tree[top_of_tree].board, tree[node].board, sizeof(int[12][12]));
	for (int y = n; y--;)
		if (tree[node].top[y] > 0) {
			tree[top_of_tree].board[tree[node].top[y] - 1][y] = tree[node].owner;
			if (checkLeaf(top_of_tree, tree[node].top[y] - 1, y) == 0) {
				maxy = y;
				break;
			}
			tree[top_of_tree].board[tree[node].top[y] - 1][y] = 0;
		}

	// 看一下对方有没有制胜位
	if (maxy == -1) {
		bool exist = 0;
		tree[top_of_tree].owner = tree[node].owner;
		for (int y = n; y--;)
			if (tree[node].top[y] > 0) {
				tree[top_of_tree].board[tree[node].top[y] - 1][y] = 3 - tree[node].owner;
				if (checkLeaf(top_of_tree, tree[node].top[y] - 1, y) == 0)
					if (exist) maxy = -1;
					else maxy = y, exist = 1;
				tree[top_of_tree].board[tree[node].top[y] - 1][y] = 0;
			}
		if (exist && maxy == -1) {
			tree[node].decisive = 0;
			return 0;
		}
	}
	
	// UCT
	if (maxy == -1) {
		double uct = -1;
		static int p[12] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
		random_shuffle(p, p + n);
		for (int i = n; i--;)
			if (tree[node].top[p[i]] > 0) {
				int ch;
				if ((ch = tree[node].child[p[i]]) && tree[ch].number >= threshold_of_uct) {
					double tmp;
					if ((tmp = 1 - tree[ch].win / tree[ch].number + c * sqrt(log(tree[node].number) / tree[ch].number)) > uct) uct = tmp, maxy = p[i];
				}
				else {
					maxy = p[i];
					break;
				}

				//_cprintf("tree[%d].child[%d] = %d\n", node, y, ch);
			}
	}

	//_cprintf("UCT: %d(node) -> %d(maxy), child = %d\n", node, maxy, tree[node].child[maxy]);

	if (tree[node].child[maxy]) { // 找到了一个还没有被expanse的节点，递归select
		int answer = 2 - select(tree[node].child[maxy]);
		tree[node].win += answer / 2.0;

		// _cprintf("<- select(%d)\n", node);

		return answer;
	}
	else { // 否则的话，做simulation
		// 新建一个节点
		tree[top_of_tree].owner = 3 - tree[node].owner;
		memcpy(tree[top_of_tree].board, tree[node].board, sizeof(tree[node].board));
		tree[top_of_tree].board[tree[node].top[maxy] - 1][maxy] = tree[node].owner;
		memcpy(tree[top_of_tree].top, tree[node].top, sizeof(tree[node].top));
		tree[top_of_tree].decisive = checkLeaf(top_of_tree, tree[top_of_tree].top[maxy] - 1, maxy);
		--tree[top_of_tree].top[maxy];
		tree[top_of_tree].top[maxy] -= nox == tree[top_of_tree].top[maxy] - 1 && noy == maxy;
		tree[top_of_tree].win = 0;
		tree[top_of_tree].number = 1;
		memset(tree[top_of_tree].child, 0, sizeof(tree[top_of_tree].child));
		
		double tmp_win;

		if (tree[top_of_tree].decisive == -1) { // simulate
			state& simunode = tree[top_of_tree + 1];
			simunode.owner = tree[top_of_tree].owner;
			memcpy(simunode.board, tree[top_of_tree].board, sizeof(int[12][12]));
			memcpy(simunode.top, tree[top_of_tree].top, sizeof(int[12]));

			int leaf_info;
			do {
				int number_of_child = 0;
				for (int y = n; y--;)
					number_of_child += simunode.top[y] > 0;
				int chosen_child = rand() % number_of_child;
				for (int y = n; y--;)
					if (simunode.top[y] > 0 && chosen_child-- == 0) {
						simunode.board[--simunode.top[y]][y] = simunode.owner;
						simunode.owner = 3 - simunode.owner;

						leaf_info = checkLeaf(top_of_tree + 1, tree[top_of_tree].top[y], y);

						simunode.top[y] -= nox == simunode.top[y] - 1 && noy == y;
						break;
					}
			} while (leaf_info == -1);

			tmp_win = leaf_info / 2.0;
			if (simunode.owner != tree[top_of_tree].owner) tmp_win = 1 - tmp_win;
		}
		else tmp_win = tree[top_of_tree].decisive / 2.0;
		tree[top_of_tree].win += tmp_win;
		tree[node].win += 1 - tmp_win;

		tree[node].child[maxy] = top_of_tree++;

		/*printf("Add a new node -> %d\n", top_of_tree);*/

		// _cprintf("<- select(%d)\n", node);

		
		/*fprintf(ferr, "New node %d:\n", top_of_tree - 1);
		for (int x = 0; x < m; ++x)
			for (int y = 0; y < n; ++y)
				fprintf(ferr, "%d%c", tree[top_of_tree-1].board[x][y], " \n"[y == n - 1]);
		fprintf(ferr, "decisive = %d\n", tree[top_of_tree-1].decisive);*/

		return (int)(2 * (1 - tmp_win));
	}
}

/*
	策略函数接口,该函数被对抗平台调用,每次传入当前状态,要求输出你的落子点,该落子点必须是一个符合游戏规则的落子点,不然对抗平台会直接认为你的程序有误
	
	input:
		为了防止对对抗平台维护的数据造成更改，所有传入的参数均为const属性
		M, N : 棋盘大小 M - 行数 N - 列数 均从0开始计， 左上角为坐标原点，行用x标记，列用y标记
		top : 当前棋盘每一列列顶的实际位置. e.g. 第i列为空,则_top[i] == M, 第i列已满,则_top[i] == 0
		_board : 棋盘的一维数组表示, 为了方便使用，在该函数刚开始处，我们已经将其转化为了二维数组board
				你只需直接使用board即可，左上角为坐标原点，数组从[0][0]开始计(不是[1][1])
				board[x][y]表示第x行、第y列的点(从0开始计)
				board[x][y] == 0/1/2 分别对应(x,y)处 无落子/有用户的子/有程序的子,不可落子点处的值也为0
		lastX, lastY : 对方上一次落子的位置, 你可能不需要该参数，也可能需要的不仅仅是对方一步的
				落子位置，这时你可以在自己的程序中记录对方连续多步的落子位置，这完全取决于你自己的策略
		noX, noY : 棋盘上的不可落子点(注:其实这里给出的top已经替你处理了不可落子点，也就是说如果某一步
				所落的子的上面恰是不可落子点，那么UI工程中的代码就已经将该列的top值又进行了一次减一操作，
				所以在你的代码中也可以根本不使用noX和noY这两个参数，完全认为top数组就是当前每列的顶部即可,
				当然如果你想使用lastX,lastY参数，有可能就要同时考虑noX和noY了)
		以上参数实际上包含了当前状态(M N _top _board)以及历史信息(lastX lastY),你要做的就是在这些信息下给出尽可能明智的落子点
	output:
		你的落子点Point
*/
extern "C" __declspec(dllexport) Point* getPoint(const int M, const int N, const int* top, const int* _board, 
	const int lastX, const int lastY, const int noX, const int noY){
	/*
		不要更改这段代码
	*/
	int x = -1, y = -1;//最终将你的落子点存到x,y中
	int** board = new int*[M];
	for(int i = 0; i < M; i++){
		board[i] = new int[N];
		for(int j = 0; j < N; j++){
			board[i][j] = _board[i * N + j];
		}
	}
	
	/*
		根据你自己的策略来返回落子点,也就是根据你的策略完成对x,y的赋值
		该部分对参数使用没有限制，为了方便实现，你可以定义自己新的类、.h文件、.cpp文件
	*/
	
	AllocConsole();

	//fprintf(ferr, "=============Begin============\n");

	m = M, n = N;
	nox = noX, noy = noY;
	top_of_tree = 1;

	/*for (int x = 0; x < m; ++x)
		for (int y = 0; y < n; ++y)
			_cprintf("%d%c", board[x][y], " \n"[y == n - 1]);
	_cprintf("======================\n");*/

	// 初始化一个根节点出来
	memset(tree[0].board, 0, sizeof(int[12][12]));
	for (int x = 0; x < m; ++x)
		for (int y = 0; y < n; ++y)
			tree[0].board[x][y] = board[x][y];
	memset(tree[0].top, 0, sizeof(int[12]));
	for (int y = n; y--;) tree[0].top[y] = top[y];
	tree[0].win = tree[0].number = 0;
	tree[0].owner = 2;
	tree[0].decisive = -1;
	memset(tree[0].child, 0, sizeof(int[12]));
	
	double start_clock = (double)clock() / CLOCKS_PER_SEC;

	int tmp_count = 0;
	while ((double)clock() / CLOCKS_PER_SEC - start_clock < 2.5) {
		//fprintf(ferr, "----------MCTS----------\n");

		select(0);
		/*if (++tmp_count % 100000 == 0) {
			_cprintf("Finish %d selections\n", tmp_count);
			_cprintf("top_of_tree = %d\n", top_of_tree);
		}*/
	}
	/*_cprintf("tmp_count = %d\n", tmp_count);*/

	 /*for (int i = 10000; i--;) {
		 fprintf(ferr, "----------MCTS----------\n");
		 select(0);
	 }*/

	//print_tree(0, 2);

	/*_cprintf("tree[0].number = %d\n", tree[0].number);
	_cprintf("top_of_tree = %d\n", top_of_tree);*/

	int max_number = 0;
	for (int j = n; j--;)
		if (tree[0].child[j] && tree[tree[0].child[j]].number > max_number)
			max_number = tree[tree[0].child[j]].number, y = j;
	if (tree[0].decisive == 0)
		for (int j = n; j--;)
			if (tree[0].top[j] > 0) {
				y = j;
				break;
			}
	x = top[y] - 1;

	//fprintf(ferr, "============END============\n");

	/*
		不要更改这段代码
	*/
	clearArray(M, N, board);
	return new Point(x, y);
}


/*
	getPoint函数返回的Point指针是在本dll模块中声明的，为避免产生堆错误，应在外部调用本dll中的
	函数来释放空间，而不应该在外部直接delete
	（没有看懂，为什么这样做会避免产生“堆错误”呢？）
*/
extern "C" __declspec(dllexport) void clearPoint(Point* p){
	delete p;
	return;
}

/*
	清除top和board数组
*/
void clearArray(int M, int N, int** board){
	for(int i = 0; i < M; i++){
		delete[] board[i];
	}
	delete[] board;
}


/*
	添加你自己的辅助函数，你可以声明自己的类、函数，添加新的.h .cpp文件来辅助实现你的想法
*/
