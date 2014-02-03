#pragma once
#include <ctime>
#include <stack>

class Timer {
public:
	Timer();
	~Timer();
private:
	clock_t m_simple_tic;
	std::stack<clock_t> m_tics;
public:
	void TicSimple();
	double TocSimple();
	void Tic( );
	double Toc( );
};

