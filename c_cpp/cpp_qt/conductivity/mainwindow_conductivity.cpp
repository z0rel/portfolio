#include "mainwindow_conductivity.h"
#include "ui_mainwindow_conductivity.h"

#include <QMessageBox>


MainWindowConductivity::MainWindowConductivity(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindowConductivity)
{
	ui->setupUi(this);

	p_GLWidget= new GLWidget(this);
	ui->verticalLayout_Logic->addWidget(p_GLWidget);

	ui->verticalLayout_Logic->setStretch(0,1);
	ui->verticalLayout_Logic->setStretch(1,10);

	plot = new SurfacePlot(this);

	ui->verticalLayout_surface->addWidget( plot, 0, 0 );
	ui->verticalLayout_surface->setStretch(0,1);
	ui->verticalLayout_surface->setStretch(1,1);

	ui->verticalLayout_Logic->setStretch(0,1);
	ui->verticalLayout_Logic->setStretch(1,10);

	plot->setTitle(QString::fromWCharArray(L"Брус"));
	plot->setZoom(0.8);
	plot->setRotation(30,0,-15);

	plot->setShift(-2, 0, -2);

	plot->setCoordinateStyle(BOX);

	bar = (Bar*)plot->setPlotStyle(Bar());

	hat = new Hat(*plot);

	hat->setMesh(23,21);
	hat->setDomain(-1.0,2.0,-1.0,2.0);

	hat->create();

	plot->illuminate(10);

	for (unsigned i=0; i!=plot->coordinates()->axes.size(); ++i)
	{
		plot->coordinates()->axes[i].setMajors(10);
		plot->coordinates()->axes[i].setMinors(-1);
	}

	plot->setMeshLineWidth(1);
	plot->coordinates()->setGridLinesColor(RGBA(0,0,0.5));
	plot->coordinates()->setLineWidth(1);
	plot->coordinates()->setNumberColor(RGBA(0,0,0));
	plot->coordinates()->setNumberFont("Courier",12);
	plot->setTitleFont("Courier",12);
	plot->coordinates()->setLabelFont("Courier",14, QFont::Bold);
	plot->coordinates()->axes[X1].setLabelString("X");
	plot->coordinates()->axes[Y1].setLabelString("Y");
	plot->coordinates()->axes[Z1].setLabelString("Z");
	plot->coordinates()->axes[X2].setLabelString("X");
	plot->coordinates()->axes[Y2].setLabelString("Y");
	plot->coordinates()->axes[Z2].setLabelString("Z");
	plot->coordinates()->axes[X3].setLabelString("X");
	plot->coordinates()->axes[Y3].setLabelString("Y");
	plot->coordinates()->axes[Z3].setLabelString("Z");
	plot->coordinates()->axes[X4].setLabelString("X");
	plot->coordinates()->axes[Y4].setLabelString("Y");
	plot->coordinates()->axes[Z4].setLabelString("Z");
	plot->coordinates()->setLineSmooth(true);

	plot->updateData();
	plot->updateGL();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_LINE_SMOOTH);

	connect(ui->pushButton_ViewTop, SIGNAL(clicked()), this, SLOT(setFrontPerspective()));
	connect(ui->spinBox_Y, SIGNAL(valueChanged(int)), this, SLOT(changeK(int)));

	connect(ui->spinBox_Y, SIGNAL(valueChanged(int)), this, SLOT(setIfchange()));
	connect(ui->spinBox_X, SIGNAL(valueChanged(int)), this, SLOT(setIfchange()));

	connect_GU();

	connect(ui->spinBox_Side, SIGNAL(valueChanged(int)), this, SLOT(restoreSide(int)));
	connect(ui->pushButton_DrawLay, SIGNAL(clicked()), this, SLOT(drawLay()));

	ifchange = true;

    for (int i = 0; i <= 6; i++)
    {
        for (int j = 0; j <= 4; j++)
        {
			ThirdKindCoeff[i][j] = 0;
        }
    }

	saveLay();
}


void MainWindowConductivity::connect_GU()
{
    connect(ui->doubleSpinBox_const,     SIGNAL(valueChanged(double)), this, SLOT(saveLay()));
    connect(ui->radioButton_const,       SIGNAL(clicked()),            this, SLOT(saveLay()));
    connect(ui->radioButton_x,           SIGNAL(clicked()),            this, SLOT(saveLay()));
    connect(ui->radioButton_y,           SIGNAL(clicked()),            this, SLOT(saveLay()));
    connect(ui->radioButton_z,           SIGNAL(clicked()),            this, SLOT(saveLay()));
    connect(ui->radioButton_x_z,         SIGNAL(clicked()),            this, SLOT(saveLay()));
    connect(ui->radioButton_x_y_z,       SIGNAL(clicked()),            this, SLOT(saveLay()));
    connect(ui->radioButton_x3_y3_z3,    SIGNAL(clicked()),            this, SLOT(saveLay()));
    connect(ui->radioButton_x2_y2_z2_20, SIGNAL(clicked()),            this, SLOT(saveLay()));
    connect(ui->doubleSpinBox_h,         SIGNAL(valueChanged(double)), this, SLOT(saveLay()));
    connect(ui->doubleSpinBox_Too,       SIGNAL(valueChanged(double)), this, SLOT(saveLay()));
    connect(ui->doubleSpinBox_q,         SIGNAL(valueChanged(double)), this, SLOT(saveLay()));
}


void MainWindowConductivity::disconnect_GU()
{
    disconnect(ui->doubleSpinBox_const,     SIGNAL(valueChanged(double)), this, SLOT(saveLay()));
    disconnect(ui->radioButton_x,           SIGNAL(clicked()),            this, SLOT(saveLay()));
    disconnect(ui->radioButton_y,           SIGNAL(clicked()),            this, SLOT(saveLay()));
    disconnect(ui->radioButton_z,           SIGNAL(clicked()),            this, SLOT(saveLay()));
    disconnect(ui->radioButton_x_z,         SIGNAL(clicked()),            this, SLOT(saveLay()));
    disconnect(ui->radioButton_x_y_z,       SIGNAL(clicked()),            this, SLOT(saveLay()));
    disconnect(ui->radioButton_x3_y3_z3,    SIGNAL(clicked()),            this, SLOT(saveLay()));
    disconnect(ui->radioButton_x2_y2_z2_20, SIGNAL(clicked()),            this, SLOT(saveLay()));
    disconnect(ui->doubleSpinBox_h,         SIGNAL(valueChanged(double)), this, SLOT(saveLay()));
    disconnect(ui->doubleSpinBox_Too,       SIGNAL(valueChanged(double)), this, SLOT(saveLay()));
    disconnect(ui->doubleSpinBox_q,         SIGNAL(valueChanged(double)), this, SLOT(saveLay()));
}


MainWindowConductivity::~MainWindowConductivity()
{
	delete ui;
}


/// Установить "Вид спереди" для 3d-куба
void MainWindowConductivity::setFrontPerspective()
{
    p_GLWidget->setFrontPerspective();
    plot->setRotation(30,0,-15);
    plot->setShift(-2, 0, -2);
}


/// Сохранение заданных граничных условий для слоя
void MainWindowConductivity::saveLay()
{
	int side = ui->spinBox_Side->value();
	/// Если выбрано граничное условие 3-го рода

    if (ui->tabWidget->currentIndex() == 1)
	{
        if ((!ui->doubleSpinBox_h->value() || !ui->doubleSpinBox_q->value())
			 && ((ui->doubleSpinBox_h->value() != ui->doubleSpinBox_q->value())
                 || (!ui->doubleSpinBox_h->value() && !ui->doubleSpinBox_q->value())))
        {
			ThirdKindCoeff[side][1] = side;
			ThirdKindCoeff[side][2] = ui->doubleSpinBox_h->value();
			ThirdKindCoeff[side][3] = ui->doubleSpinBox_Too->value();
			ThirdKindCoeff[side][4] = ui->doubleSpinBox_q->value();
		}
        else
        {
            QMessageBox *msg = new QMessageBox(
                            QMessageBox::Critical,
                            QString::fromWCharArray(L"Ошибка"),
                            QString::fromWCharArray(L"Или h, или q должно быть нулевым!")
                        );
			msg->setAttribute(Qt::WA_DeleteOnClose);
			msg->show();

			ui->doubleSpinBox_h->setFocus();
		}
	}
	/// Если выбрано граничное условие 1-го рода
	else
	{
		ThirdKindCoeff[side][1] = 0; // если = 0 то Г.У - константное
		ThirdKindCoeff[side][3] = 0;
        if (ui->radioButton_const->isChecked())
        {
			ThirdKindCoeff[side][2] = const_func;
			ThirdKindCoeff[side][3] = ui->doubleSpinBox_const->value();
		}
        else if (ui->radioButton_x_y_z->isChecked())
        {
			ThirdKindCoeff[side][2] = x_y_z_func;
        }
        else if (ui->radioButton_x->isChecked())
        {
			ThirdKindCoeff[side][2] = x_func;
        }
        else if (ui->radioButton_y->isChecked())
        {
			ThirdKindCoeff[side][2] = y_func;
        }
        else if (ui->radioButton_z->isChecked())
        {
			ThirdKindCoeff[side][2] = z_func;
        }
        else if (ui->radioButton_x_z->isChecked())
        {
			ThirdKindCoeff[side][2] = x_z_func;
        }
        else if (ui->radioButton_x3_y3_z3->isChecked())
        {
			ThirdKindCoeff[side][2] = x3_y3_z3_func;
        }
        else if (ui->radioButton_x2_y2_z2_20->isChecked())
        {
			ThirdKindCoeff[side][2] = x2_y2_z2_20_func;
        }
	}
	setIfchange();
}


void MainWindowConductivity::restoreSide(int side)
{
	disconnect_GU();
	ui->doubleSpinBox_const->setValue(0);
	ui->doubleSpinBox_h->setValue(0);
	ui->doubleSpinBox_Too->setValue(0);
	ui->doubleSpinBox_q->setValue(0);

	if (ThirdKindCoeff[side][1] <= 0)
	{
		ui->tabWidget->setCurrentIndex(0);

		switch((int)ThirdKindCoeff[side][2])
		{
			case const_func:
				ui->radioButton_const->setChecked(true);
				ui->doubleSpinBox_const->setValue(ThirdKindCoeff[side][3]);
				break;
            case x_y_z_func:
                ui->radioButton_x_y_z->setChecked(true);
                break;
            case x_func:
                ui->radioButton_x->setChecked(true);
                break;
            case y_func:
                ui->radioButton_y->setChecked(true);
                break;
            case z_func:
                ui->radioButton_z->setChecked(true);
                break;
            case x_z_func:
                ui->radioButton_x_z->setChecked(true);
                break;
            case x3_y3_z3_func:
                ui->radioButton_x3_y3_z3->setChecked(true);
                break;
            case x2_y2_z2_20_func:
                ui->radioButton_x2_y2_z2_20->setChecked(true);
                break;
		}
	}
	else
	{
		ui->tabWidget->setCurrentIndex(1);
		ui->doubleSpinBox_h->setValue(ThirdKindCoeff[side][2]);
		ui->doubleSpinBox_Too->setValue(ThirdKindCoeff[side][3]);
		ui->doubleSpinBox_q->setValue(ThirdKindCoeff[side][4]);
	}
	connect_GU();
}


void MainWindowConductivity::changeK(int Y)
{
    ui->label_lay_NumLayVal->setText(QString("0-") + QString::number(Y));
    ui->spinBox_lay->setMaximum(Y);
    ui->spinBox_lay->setValue(Y);
}


void MainWindowConductivity::setIfchange()
{
	ifchange = true;
}


QColor MainWindowConductivity::value_to_color(long double value)
{
	QColor res;
	res.setRgbF(1.0 - (max_solve - value) / offset, 0.0, 1.0 - (value - min_solve) / offset, 1);
	return res;
}


bool MainWindowConductivity::rect_in_lay_y(const Bar::Rectangle &rect, int y)
{
    for (int i = 0; i < 4; i++)
    {
        if (rect.points[i].y_lay != y)
        {
            return false;
        }
    }
    return true;
}


bool MainWindowConductivity::rect_in_lay_x(const Bar::Rectangle &rect, int x)
{
    for (int i = 0; i < 4; i++)
    {
        if (rect.points[i].x_lay != x)
        {
            return false;
        }
    }
    return true;
}


bool MainWindowConductivity::rect_in_lay_z(const Bar::Rectangle &rect, int z)
{
    for (int i = 0; i < 4; i++)
    {
        if (rect.points[i].z_lay != z)
        {
            return false;
        }
    }
	return true;
}


void MainWindowConductivity::drawLay()
{
    TintMatr A1;
    TintMatr A2;
    TintMatr A3;
    TintMatr TetrMatr;
    int dim1;
    int dim2;
    int dim3;

	MakeTetraidersObj makeTetraiders(ThirdKindCoeff, ui->checkBox_to_file->isChecked());
	makeTetraiders.MakeCube(ui->spinBox_X->value(),
							ui->spinBox_Z->value(),
							ui->spinBox_Y->value(),
							A1, A2, A3, dim1, dim2, dim3);

	Temperature temperature(A1, A2, A3, dim3, ThirdKindCoeff, ui->checkBox_to_file->isChecked());
	temperature.M1 = ui->spinBox_Z->value();
	temperature.N1 = ui->spinBox_X->value();
	temperature.K1 = ui->spinBox_Y->value();

	temperature.MakeTetraiders(dim1, A1, TetrMatr);

    // спец. матрица K
    TrealMatr K;

    // вектор правой части
    TVector F;

	temperature.buildMatrK_andVectF(A2, A3, dim3, K, F, dim1, dim2, TetrMatr);

    /// вектор - решение
    TVector g;
	temperature.HaletskiiMethod(K,dim2,F,g);

    bar->rectangles.clear();


	Bar::Point3 p[8];

	max_solve = -LDBL_MAX;
	min_solve = LDBL_MAX;

    double max_x = -DBL_MAX;
    double max_y = -DBL_MAX;
    double max_z = -DBL_MAX;
    double min_x = DBL_MAX;
    double min_y = DBL_MAX;
    double min_z = DBL_MAX;

	for (int i = 1; i <= dim2; i++)
	{
        if (g[i] < min_solve) {
			min_solve = g[i];
        }
        if (g[i] > max_solve) {
			max_solve = g[i];
        }
	}

	mid_solve = min_solve / 2.0 + max_solve / 2.0;

	offset = max_solve - min_solve;

	Bar::Rectangle rects[6];

	QVector<int> indexes;
    double t_x;
    double t_y;
    double t_z;

	for (int i = 1; i <= dim1; i++)
	{
		indexes.clear();
		for (int j = 0; j < 8; j++)
		{
			int k = A1[i][j + 1];
			p[j] = Bar::Point3(A2[k][2], A2[k][3], A2[k][4], value_to_color(g[k]), A2[k][2], A2[k][3], A2[k][4]);
		}

        rects[0] = Bar::Rectangle(p[0], p[1], p[2], p[3]);
        rects[1] = Bar::Rectangle(p[4], p[5], p[6], p[7]);
        rects[2] = Bar::Rectangle(p[1], p[2], p[5], p[6]);
        rects[3] = Bar::Rectangle(p[0], p[3], p[4], p[7]);
        rects[4] = Bar::Rectangle(p[2], p[3], p[4], p[5]);
        rects[5] = Bar::Rectangle(p[0], p[1], p[6], p[7]);

        if (ui->radioButton_soilid->isChecked() && ui->checkBox_hollow->isChecked())
        {
            for (int j = 0; j < 6; j++)
            {
                if (rect_in_lay_y(rects[j], 0)
                        || rect_in_lay_x(rects[j], 0)
                        || rect_in_lay_z(rects[j], 0)
                        || rect_in_lay_y(rects[j], ui->spinBox_X->value())
                        || rect_in_lay_x(rects[j], ui->spinBox_Z->value())
                        || rect_in_lay_z(rects[j], ui->spinBox_lay->value()))
                {
                    indexes << j;
                }
            }
        }
        else if (ui->radioButton_soilid->isChecked())
        {
            for (int j = 0; j < 6; j++)
            {
                indexes << j;
             }
        }
        else if (ui->radioButton_split_y->isChecked())
        {
            indexes << 0 << 1;
        }
        else if (ui->radioButton_split_z->isChecked())
        {
            indexes << 2 << 3;
        }
        else if (ui->radioButton_split_x->isChecked())
        {
            indexes << 4 << 5;
        }

        for (int j = 0; j < indexes.count(); j++)
        {
            for (int k = 0; k < 4; k++)
            {
                t_x = rects[indexes.at(j)].points[k].y * ui->doubleSpinBox_hX->value();
                t_y = rects[indexes.at(j)].points[k].z * ui->doubleSpinBox_hY->value();
                t_z = rects[indexes.at(j)].points[k].x * ui->doubleSpinBox_hZ->value();
                rects[indexes.at(j)].points[k].x = t_x;
                rects[indexes.at(j)].points[k].y = t_y;
                rects[indexes.at(j)].points[k].z = t_z;
                if (max_x < t_x)
                {
                    max_x = t_x;
                }
                if (max_y < t_y)
                {
                    max_y = t_y;
                }
                if (max_z < t_z)
                {
                    max_z = t_z;
                }
                if (min_x > t_x)
                {
                    min_x = t_x;
                }
                if (min_y > t_y)
                {
                    min_y = t_y;
                }
                if (min_z > t_z)
                {
                    min_z = t_z;
                }
            }
            bar->rectangles << rects[indexes.at(j)];
        }
	}

	double o = 0.5;
	plot->coordinates()->axes[X1].setPosition(Triple(min_x - o, min_y - o, min_z - o),
											  Triple(max_x + o, min_y - o, min_z - o));
	plot->coordinates()->axes[X2].setPosition(Triple(min_x - o, min_y - o, max_z + o),
											  Triple(max_x + o, min_y - o, max_z + o));
	plot->coordinates()->axes[X3].setPosition(Triple(min_x - o, max_y + o, max_z + o),
											  Triple(max_x + o, max_y + o, max_z + o));
	plot->coordinates()->axes[X4].setPosition(Triple(min_x - o, max_y + o, min_z - o),
											  Triple(max_x + o, max_y + o, min_z - o));

	plot->coordinates()->axes[Y1].setPosition(Triple(min_x - o, min_y - o, min_z - o),
											  Triple(min_x - o, max_y + o, min_z - o));
	plot->coordinates()->axes[Y2].setPosition(Triple(max_x + o, min_y - o, min_z - o),
											  Triple(max_x + o, max_y + o, min_z - o));
	plot->coordinates()->axes[Y3].setPosition(Triple(max_x + o, min_y - o, max_z + o),
											  Triple(max_x + o, max_y + o, max_z + o));
	plot->coordinates()->axes[Y4].setPosition(Triple(min_x - o, min_y - o, max_z + o),
											  Triple(min_x - o, max_y + o, max_z + o));

	plot->coordinates()->axes[Z1].setPosition(Triple(min_x - o, max_y + o, min_z - o),
											  Triple(min_x - o, max_y + o, max_z + o));
	plot->coordinates()->axes[Z2].setPosition(Triple(min_x - o, min_y - o, min_z - o),
											  Triple(min_x - o, min_y - o, max_z + o));
	plot->coordinates()->axes[Z3].setPosition(Triple(max_x + o, min_y - o, min_z - o),
											  Triple(max_x + o, min_y - o, max_z + o));
	plot->coordinates()->axes[Z4].setPosition(Triple(max_x + o, max_y + o, min_z - o),
											  Triple(max_x + o, max_y + o, max_z + o));

	plot->coordinates()->axes[X1].setLimits(min_x - 1, max_x + 1);
	plot->coordinates()->axes[X2].setLimits(min_x - 1, max_x + 1);
	plot->coordinates()->axes[X3].setLimits(min_x - 1, max_x + 1);
	plot->coordinates()->axes[X4].setLimits(min_x - 1, max_x + 1);
	plot->coordinates()->axes[Y1].setLimits(min_y - 1, max_y + 1);
	plot->coordinates()->axes[Y2].setLimits(min_y - 1, max_y + 1);
	plot->coordinates()->axes[Y3].setLimits(min_y - 1, max_y + 1);
	plot->coordinates()->axes[Y4].setLimits(min_y - 1, max_y + 1);
	plot->coordinates()->axes[Z1].setLimits(min_z - 1, max_z + 1);
	plot->coordinates()->axes[Z2].setLimits(min_z - 1, max_z + 1);
	plot->coordinates()->axes[Z3].setLimits(min_z - 1, max_z + 1);
	plot->coordinates()->axes[Z4].setLimits(min_z - 1, max_z + 1);

	plot->coordinates()->setAutoScale(true);
	plot->coordinates()->setStandardScale();
	plot->coordinates()->setGridLines(false, false, Qwt3D::BACK);

	ColorLegend * legend = plot->legend();

	legend->setLimits(min_solve, max_solve);
	legend->setAutoScale(true);
	legend->setTitleFont("Courier", 12);
	legend->drawNumbers(true);

	plot->showColorLegend(true);

    if (ui->checkBox_grid->isChecked())
    {
		bar->is_line = true;
    }
	else
    {
		bar->is_line = false;
    }

	plot->update();
	plot->updateData();
	plot->updateGeometry();
	plot->updateGL();

	free_matr<int>(TetrMatr, dim1 * 5);
	free_matr<int>(A1, dim1 + 1);
	free_matr<int>(A2, dim2 + 1);
	free_matr<int>(A3, dim3 + 1);
	free_matr<long double>(F);
	free_matr<long double>(g);
	free_matr<long double>(K,  dim2 + 1);
}
