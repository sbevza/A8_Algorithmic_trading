#ifndef SRC_MAINWINDOW_H_
#define SRC_MAINWINDOW_H_

#include <QMainWindow>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include "../controllers/algo_trading_controller.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void on_LoadDataCsv_clicked();
    void on_clean_button_clicked();
    void on_PlotCubicSpline_clicked();
    void on_PlotNewtonPolynomial_clicked();

    void on_GetValue_clicked();


    void on_showPoints_clicked();

private:
    Ui::MainWindow *ui;
    s21::AlgoTradingController *controller_;


    QString createGraphLabel(const QString& type, int degree, int pointCount);
    void updateUiState();
    void setupDateTimeEditLimits();
    bool showPoints_ = false;
    QVector<double> generateX(double xStart, double xEnd, int numPoints);
    void plotInterpolatedGraph(const QVector<double>& x, const QVector<double>& y, const QString& label);
    void plotInterpolatedFunction(
        const QString& type,
        int degree,
        std::function<double(const QDateTime&)> valueFunc,
        int pointCount
        );

};

#endif // SRC_MAINWINDOW_H_
