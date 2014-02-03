#include "Timer.h"

Timer::Timer() {}


Timer::~Timer() {}


void Timer::TicSimple( ) {
	m_simple_tic = clock();
}


double Timer::TocSimple( ) {
	return (double) (clock() - m_simple_tic) / CLOCKS_PER_SEC;
}

void Timer::Tic() {
	m_tics.push( clock() );
}

double Timer::Toc() {
	double dt = (double) (clock( ) - m_tics.top()) / CLOCKS_PER_SEC;
	m_tics.pop();
	return dt;
}
