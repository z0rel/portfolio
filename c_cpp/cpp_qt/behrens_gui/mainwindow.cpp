#include "pybind11/pybind11.h"
#include <pybind11/embed.h> // everything needed for embedding

namespace py = pybind11;
using namespace py::literals;

#include <list>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtWidgets/QFileDialog>


py::scoped_interpreter guard{}; // start the interpreter and keep it alive
py::module behrens_converter = py::module::import("behrens_converter");
py::object constants = behrens_converter.attr("constants");
py::object localization = behrens_converter.attr("localization");
py::object D = constants.attr("configure_defaults")();
py::object set_value = constants.attr("set_value");


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    localization.attr("locale_set_up")();

    // py::scoped_interpreter guard{}; // start the interpreter and keep it alive

    // QObject::connect(ui->start_conversion, SIGNAL(QAbstractButton::clicked(bool)), this, SLOT(start_conversion(bool)));

    QObject::connect(ui->dir_src, SIGNAL(QLineEdit::textChanged()), this, SLOT(MainWindow::changed_src()));
    QObject::connect(ui->dir_src_change, SIGNAL(QPushButton::clicked()), this, SLOT(MainWindow::changed_src_click()));

    QObject::connect(ui->dir_dst, SIGNAL(QLineEdit::textChanged()), this, SLOT(MainWindow::changed_dst()));
    QObject::connect(ui->dir_dst_change, SIGNAL(QPushButton::clicked()), this, SLOT(MainWindow::changed_dst_click()));

    QObject::connect(ui->coeff_ef, SIGNAL(QDoubleSpinBox::valueChanged(double val)), this, SLOT(MainWindow::changed_ef(double val)));
    QObject::connect(ui->coeff_e, SIGNAL(QDoubleSpinBox::valueChanged(double val)), this, SLOT(MainWindow::changed_e(double val)));
    QObject::connect(ui->coeff_fast_M25, SIGNAL(QDoubleSpinBox::valueChanged(double val)), this, SLOT(MainWindow::changed_f_m25(double val)));
    QObject::connect(ui->coeff_G023_M22, SIGNAL(QDoubleSpinBox::valueChanged(double val)), this, SLOT(MainWindow::changed_m22_angle(double val)));

    QObject::connect(ui->show_m25_warning, SIGNAL(QCheckBox::stateChanged()), this, SLOT(MainWindow::changed_m22_angle_warning()));
    QObject::connect(ui->show_verbose, SIGNAL(QCheckBox::stateChanged()), this, SLOT(MainWindow::changed_verbose()));
    QObject::connect(ui->show_verbose_m25, SIGNAL(QCheckBox::stateChanged()), this, SLOT(MainWindow::changed_verbose_g02_m25()));

    ui->dir_dst->setText(QString::fromStdString(D.attr("DST_DIRECTORY").cast<std::string>()));
    ui->dir_src->setText(QString::fromStdString(D.attr("SRC_DIRECTORY").cast<std::string>()));
    ui->coeff_ef->setValue(D.attr("CORRECTION_COEFFICIENT").cast<double>());
    ui->coeff_e->setValue(D.attr("STEPS_BY_MINUTE").cast<double>());

    ui->coeff_fast_M25->setValue(D.attr("F_M25_H").cast<double>());
    ui->coeff_G023_M22->setValue(D.attr("ANGLE_TRESHOLD").cast<double>());

    ui->show_m25_warning->setChecked(D.attr("PRINT_ANGLE_WARNS").cast<int>());
    ui->show_verbose->setChecked(D.attr("VERBOSE_LOG").cast<int>());
    ui->show_verbose_m25->setChecked(D.attr("VERBOSE_G02_G03_M25").cast<int>());
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_start_conversion_clicked()
{
    py::object ctx = behrens_converter.attr("get_gui_context")();
    py::object convert_file = behrens_converter.attr("convert_file");
    py::object fileslist = ctx.attr("fileslist");
    size_t sz = fileslist.attr("__len__")().cast<size_t>();

    size_t prev = 0;
    for (size_t i = 0; i < sz; ++i) {
        py::object fname = fileslist.attr("__getitem__")(i);
        size_t next = (i * 100) / sz;
        if (next != prev) {
            this->ui->conversion_progress->setValue(next);
            prev = next;
        }
        convert_file(ctx, fname);
        std::string text = ctx.attr("text").cast<std::string>();
        this->ui->log_text_edit->setHtml(QString::fromStdString(text));
    }
    this->ui->conversion_progress->setValue(100);
}


void MainWindow::on_dir_src_textChanged(const QString &arg1)
{
    py::str val = arg1.toStdString();
    set_value("SRC_DIRECTORY", val);
}

void MainWindow::on_dir_src_change_clicked()
{
    QString sdir = QFileDialog::getExistingDirectory(this,
                tr("Open source catalouge"), ".", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    set_value("SRC_DIRECTORY", sdir.toStdString());
    this->ui->dir_src->setText(sdir);
}

void MainWindow::on_dir_dst_textChanged(const QString &arg1)
{
    py::str val = arg1.toStdString();
    set_value("DST_DIRECTORY", val);
}

void MainWindow::on_dir_dst_change_clicked()
{
    QString sdir = QFileDialog::getExistingDirectory(this,
                tr("Open destination catalouge"), ".", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    set_value("DST_DIRECTORY", sdir.toStdString());
    this->ui->dir_dst->setText(sdir);
}

void MainWindow::on_coeff_ef_valueChanged(double value)
{
    set_value("CORRECTION_COEFFICIENT", value);
}

void MainWindow::on_coeff_e_valueChanged(double value)
{
    set_value("STEPS_BY_MINUTE", value);
}

void MainWindow::on_coeff_fast_M25_valueChanged(double value)
{
    set_value("F_M25_H", value);
}

void MainWindow::on_show_m25_warning_stateChanged(int arg1)
{
    set_value("PRINT_ANGLE_WARNS", bool(arg1));
}

void MainWindow::on_coeff_G023_M22_valueChanged(double value)
{
    set_value("ANGLE_TRESHOLD", value);
}

void MainWindow::on_show_verbose_stateChanged(int value)
{
    set_value("VERBOSE_LOG", bool(value));
}

void MainWindow::on_show_verbose_m25_stateChanged(int value)
{
    set_value("VERBOSE_G02_G03_M25", value);
}

