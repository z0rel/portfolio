#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    Ui::MainWindow *ui;

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_start_conversion_clicked();
    void on_dir_src_textChanged(const QString &arg1);
    void on_dir_src_change_clicked();
    void on_dir_dst_textChanged(const QString &arg1);
    void on_dir_dst_change_clicked();
    void on_coeff_ef_valueChanged(double arg1);
    void on_coeff_e_valueChanged(double arg1);
    void on_coeff_fast_M25_valueChanged(double arg1);
    void on_show_m25_warning_stateChanged(int arg1);
    void on_coeff_G023_M22_valueChanged(double arg1);
    void on_show_verbose_stateChanged(int arg1);
    void on_show_verbose_m25_stateChanged(int arg1);
};

#endif // MAINWINDOW_H
