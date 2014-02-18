#pragma once
#include <ctime>
#include <stack>

class Timer {
public:
	Timer() {}
	~Timer() {}
private:
	clock_t m_simple_tic = -1;
	std::stack<clock_t> m_tics;
	double m_average;
	long m_N;
public:
	inline void TicSimple() {
		m_simple_tic = clock( );
	}
	inline double TocSimple() {
		if( m_simple_tic == -1 ) return 0;
		return (double) (clock( ) - m_simple_tic) / CLOCKS_PER_SEC;
	}
	inline void Tic() {
		m_tics.push( clock( ) );
	}
	inline double Toc() {
		double dt = (double) (clock( ) - m_tics.top( )) / CLOCKS_PER_SEC;
		m_tics.pop( );
		m_average += dt;
		m_N++;
		return dt;
	}
	double GetAverage() {
		return m_average / m_N;
	}
};

