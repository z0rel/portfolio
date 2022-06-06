#ifndef MAKETETRAIDERS_H
#define MAKETETRAIDERS_H


#include <QFile>
#include <QString>
#include <QTextStream>
#include <QRegularExpression>

#include "global.h"


class MakeTetraidersObj
{
public:
	MakeTetraidersObj(double ThirdKindCoeff[7][5], bool to_file);

	void MakeCube(int N, int M, int K,
				  TintMatr &A1, TintMatr &A2, TintMatr &A3,
				  int &dim1, int &dim2, int &dim3);
	void MakeTetraiders();

private:
	double ThirdKindCoeff[7][5];

	bool output_to_file;

	int bounded(int x, int y, int z, int n);

	void corner(int i, int j, int l, TintMatr A2, TintMatr A3, int &por, int N, int M, int K);
	void xparallelside(int i, int l, TintMatr A2, TintMatr A3, int &por, int N, int M, int K);
	void yparallelside(int j, int l, TintMatr A2, TintMatr A3, int &por, int N, int M, int K);


	/// N, M, K - размерности разбиений куба по осям
	/// K - также определяет количество слоев
    void CreateKub(int N, int M, int K, TintMatr &A1, TintMatr &A2, TintMatr &A3, int &dim1, int &dim2, int &dim3);
};


#endif // MAKETETRAIDERS_H
