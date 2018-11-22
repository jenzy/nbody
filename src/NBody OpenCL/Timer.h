#pragma once
#include <ctime>
#include <stack>

class Timer {
public:
    Timer(): m_average(0), m_N(0) {}
    ~Timer() {}

private:
    clock_t m_simple_tic = -1;
    std::stack<clock_t> m_tics;
    double m_average;
    long m_N;

public:
    void TicSimple() {
        m_simple_tic = clock( );
    }

    double TocSimple() const {
        if( m_simple_tic == -1 ) return 0;
        return (double) (clock( ) - m_simple_tic) / CLOCKS_PER_SEC;
    }

    void Tic() {
        m_tics.push( clock( ) );
    }

    double Toc() {
        const double dt = (double) (clock( ) - m_tics.top( )) / CLOCKS_PER_SEC;
        m_tics.pop( );
        m_average += dt;
        m_N++;
        return dt;
    }

    double GetAverage() const {
        return m_average / m_N;
    }
};

