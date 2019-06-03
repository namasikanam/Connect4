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

double c = sqrt(2) / 2; // UCT�㷨�еĴ����������������Ž���sqrt(2)
double threshold_of_uct = 1;

// ������
FILE* ferr = fopen("error.txt", "w");

// һЩȫ�ֱ���
int m, n;
int nox, noy;

typedef unsigned long long ull;

struct state{
	int owner; // ������ڵ�˭�����֣�1���û���2�ǳ������ǳ��򣩡�
	int board[12][12];
	int top[12];
	double win; // ƽ�ֵĻ���win += 0.5
	int number; // ���ʴ���
	int decisive; // -1��ʾ�ⲻ��һ��ȷ�����棬0��ʾ���ˣ�1��ʾƽ�֣�2��ʾӮ�ˡ�����������֮ǰ�Ŀ�����
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

inline int checkLeaf(int node, int x, int y) { // ��ʵ�ϣ�Ӯ�ǲ����еģ������ж�������û���䡣
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

int select(int node) { // ����ֵ��ʾ����ǳɹ��˻���ʧ���ˣ��ɹ�ʧ���Ƕ��ڵ�ǰ���ֶ��Եġ�
	//if (node) {
		//fprintf(ferr, "-> select(%d)\n", node);
		//fprintf(ferr, "But top_of_tree = %d\n", top_of_tree);
	//}

	tree[node].number += 1;

	if (tree[node].decisive != -1) { // Ҷ�ӽڵ��޷�����չ�ӽڵ㡣
		tree[node].win += tree[node].decisive / 2.0;

		// _cprintf("<- select(%d)\n", node);
		/*fprintf(ferr, "Go to a decisive node %d\n", node);*/

		return tree[node].decisive;
	}

	// ��չ�ӽڵ�

	int maxy = -1;

	// Ѱ����ʤλ
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

	// ��һ�¶Է���û����ʤλ
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

	if (tree[node].child[maxy]) { // �ҵ���һ����û�б�expanse�Ľڵ㣬�ݹ�select
		int answer = 2 - select(tree[node].child[maxy]);
		tree[node].win += answer / 2.0;

		// _cprintf("<- select(%d)\n", node);

		return answer;
	}
	else { // ����Ļ�����simulation
		// �½�һ���ڵ�
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
	���Ժ����ӿ�,�ú������Կ�ƽ̨����,ÿ�δ��뵱ǰ״̬,Ҫ�����������ӵ�,�����ӵ������һ��������Ϸ��������ӵ�,��Ȼ�Կ�ƽ̨��ֱ����Ϊ��ĳ�������
	
	input:
		Ϊ�˷�ֹ�ԶԿ�ƽ̨ά����������ɸ��ģ����д���Ĳ�����Ϊconst����
		M, N : ���̴�С M - ���� N - ���� ����0��ʼ�ƣ� ���Ͻ�Ϊ����ԭ�㣬����x��ǣ�����y���
		top : ��ǰ����ÿһ���ж���ʵ��λ��. e.g. ��i��Ϊ��,��_top[i] == M, ��i������,��_top[i] == 0
		_board : ���̵�һά�����ʾ, Ϊ�˷���ʹ�ã��ڸú����տ�ʼ���������Ѿ�����ת��Ϊ�˶�ά����board
				��ֻ��ֱ��ʹ��board���ɣ����Ͻ�Ϊ����ԭ�㣬�����[0][0]��ʼ��(����[1][1])
				board[x][y]��ʾ��x�С���y�еĵ�(��0��ʼ��)
				board[x][y] == 0/1/2 �ֱ��Ӧ(x,y)�� ������/���û�����/�г������,�������ӵ㴦��ֵҲΪ0
		lastX, lastY : �Է���һ�����ӵ�λ��, ����ܲ���Ҫ�ò�����Ҳ������Ҫ�Ĳ������ǶԷ�һ����
				����λ�ã���ʱ��������Լ��ĳ����м�¼�Է������ಽ������λ�ã�����ȫȡ�������Լ��Ĳ���
		noX, noY : �����ϵĲ������ӵ�(ע:��ʵ���������top�Ѿ����㴦���˲������ӵ㣬Ҳ����˵���ĳһ��
				������ӵ�����ǡ�ǲ������ӵ㣬��ôUI�����еĴ�����Ѿ������е�topֵ�ֽ�����һ�μ�һ������
				��������Ĵ�����Ҳ���Ը�����ʹ��noX��noY��������������ȫ��Ϊtop������ǵ�ǰÿ�еĶ�������,
				��Ȼ�������ʹ��lastX,lastY�������п��ܾ�Ҫͬʱ����noX��noY��)
		���ϲ���ʵ���ϰ����˵�ǰ״̬(M N _top _board)�Լ���ʷ��Ϣ(lastX lastY),��Ҫ���ľ�������Щ��Ϣ�¸������������ǵ����ӵ�
	output:
		������ӵ�Point
*/
extern "C" __declspec(dllexport) Point* getPoint(const int M, const int N, const int* top, const int* _board, 
	const int lastX, const int lastY, const int noX, const int noY){
	/*
		��Ҫ������δ���
	*/
	int x = -1, y = -1;//���ս�������ӵ�浽x,y��
	int** board = new int*[M];
	for(int i = 0; i < M; i++){
		board[i] = new int[N];
		for(int j = 0; j < N; j++){
			board[i][j] = _board[i * N + j];
		}
	}
	
	/*
		�������Լ��Ĳ������������ӵ�,Ҳ���Ǹ�����Ĳ�����ɶ�x,y�ĸ�ֵ
		�ò��ֶԲ���ʹ��û�����ƣ�Ϊ�˷���ʵ�֣�����Զ����Լ��µ��ࡢ.h�ļ���.cpp�ļ�
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

	// ��ʼ��һ�����ڵ����
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
		��Ҫ������δ���
	*/
	clearArray(M, N, board);
	return new Point(x, y);
}


/*
	getPoint�������ص�Pointָ�����ڱ�dllģ���������ģ�Ϊ��������Ѵ���Ӧ���ⲿ���ñ�dll�е�
	�������ͷſռ䣬����Ӧ�����ⲿֱ��delete
	��û�п�����Ϊʲô�����������������Ѵ����أ���
*/
extern "C" __declspec(dllexport) void clearPoint(Point* p){
	delete p;
	return;
}

/*
	���top��board����
*/
void clearArray(int M, int N, int** board){
	for(int i = 0; i < M; i++){
		delete[] board[i];
	}
	delete[] board;
}


/*
	������Լ��ĸ�������������������Լ����ࡢ����������µ�.h .cpp�ļ�������ʵ������뷨
*/
