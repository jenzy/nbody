#pragma once
#include <ctime>

class Timer {
public:
	Timer();
	~Timer();
private:
	clock_t m_tic;
public:
	void Tic();
	double Toc();
};

