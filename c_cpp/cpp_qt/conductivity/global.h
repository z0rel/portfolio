#ifndef GLOBAL_H
#define GLOBAL_H


#include <stddef.h>
#include <math.h>
#include <QFile>
#include <QTextStream>


typedef int**         TintMatr;
typedef long double** TrealMatr;
typedef long double*  TVector;


/// Функции для обратной совместимости с соответствущими на Delphi


template <typename T> void setlength(T** &arr, int dim1, int dim2) {
	arr = new T*[dim1];
	for (int i = 0; i < dim1; i++) 	{
		arr[i] = new T[dim2];
		memset(arr[i], 0, sizeof(T)*dim2);
	}
}


template <typename T> void setlength(T* &arr, int dim1) {
	arr = new T[dim1];
	memset(arr, 0, sizeof(T) * dim1);
}


template <typename T> void free_matr(T** &arr, int dim1) {
	if (arr == NULL) return;
	for (int i = 0; i < dim1; i++)
		delete[] arr[i];
	delete[] arr;
	arr = NULL;
}


template <typename T> void free_matr(T* &arr) {
	if (arr == NULL) return;
	delete[] arr;
	arr = NULL;
}


template <typename T>
void outMatrToFile(const char * name, T** A,
				   int start_n1, int end_n1,
				   int start_n2, int end_n2)
{
	QFile file(name);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text) )
    {
        return;
    }
	QTextStream file_stream(&file);
	file_stream << end_n1 - start_n1 + 1 << " " << end_n2 - start_n2 + 1 << "\n";
	for (int i = start_n1; i <= end_n1; i++)
	{
        for (int j = start_n2; j <= end_n2; j++)
        {
			file_stream << (double)A[i][j] << " ";
        }
		file_stream << "\n";
	}
	file_stream.flush(); file.flush(); file.close();
}


template <typename T>
void outVectToFile(const char * name, T* A, int start_n1, int end_n1)
{
	QFile file(name);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return;
    }
	QTextStream file_stream(&file);

	file_stream << end_n1 - start_n1 + 1 << " " << "\n";
    for (int i = start_n1; i < end_n1; i++)
    {
        file_stream << (double)A[i] << "\n";
    }
    file_stream.flush();
    file.flush();
    file.close();
}


enum TypeFirstKind
{
	const_func       = 0,
	x_y_z_func       = 1,
	x_func           = 2,
	y_func           = 3,
	z_func           = 4,
	x_z_func         = 5,
	x3_y3_z3_func    = 7,
	x2_y2_z2_20_func = 8
};

#endif // GLOBAL_H
