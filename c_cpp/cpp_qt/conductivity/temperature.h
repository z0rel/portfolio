#ifndef MAKEOFMATRCLASS_H
#define MAKEOFMATRCLASS_H


#include <QFile>
#include <QString>
#include <QTextStream>

#include "global.h"


class Temperature
{
public:
	Temperature(TintMatr A1, TintMatr A2, TintMatr A3, int dim3, double ThirdKindCoeff[7][5],
				bool to_file);
	virtual ~Temperature();

    /// размерность бруса
    int K1;
    int N1;
    int M1;

	void MakeTetraiders(int dim1, TintMatr A1, TintMatr &TetrMatr);
	void buildMatrK_andVectF(TintMatr A2, TintMatr A3, int dim3, TrealMatr &K, TVector &F,
							 int dim1, int dim2, TintMatr TetrMatr);
	void TemperatureToFile(TVector g);

	void HaletskiiMethod(TrealMatr K, int dim2, TVector F, TVector &g);

private:
	int dim3;
    TintMatr A1;
    TintMatr A2;
    TintMatr A3;
    TVector F;
    TrealMatr K;

    int Tetr[8][6];
    int index1;
    int index2;
    int dim2_Nuzel;
    int dim1_Nkub;

    double h;
    double q;
    double T0;
	double ThirdKindCoeff[7][5];

	bool output_to_file;

	void setA123(TintMatr A1, TintMatr A2, TintMatr A3);

	void freeObj();

    long double b(int index);
    long double c(int index);
    long double d(int index);
	long double det (int index);

	void FindBound(int &ii, int &jj, int &kk);
	bool firstKind(int s);
	void correctMatrix(double hh, double qq, double TTo, int ii, int jj, int kk);

	int L(TrealMatr K, int dim2);
	int i0(int i, int lenta);
	int i_n(int i, int n, int lenta);
};

#endif // MAKEOFMATRCLASS_H
