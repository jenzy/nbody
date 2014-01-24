#include "Timer.h"

Timer::Timer() {}


Timer::~Timer() {}


void Timer::Tic() {
	m_tic = clock();
}


double Timer::Toc() {
	return (double) (clock() - m_tic) / CLOCKS_PER_SEC;
}
