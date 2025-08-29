QT       += core gui 

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

CONFIG += c++20

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cc \
    views/mainwindow.cc \
    controllers/algo_trading_controller.cc \
    qcustomplot/qcustomplot.cc \
    models/csv_parser.cc \
    models/trade_data.cc \
    models/cubic_spline_interpolator.cc \
    models/newton_interpolator.cc \
    models/least_squares_approximator.cc


HEADERS += \
    views/mainwindow.h \
    controllers/algo_trading_controller.h \
    qcustomplot/qcustomplot.h \
    models/csv_parser.h \
    models/trade_data.h \
    models/cubic_spline_interpolator.h \
    models/spline_point.h \
    models/newton_interpolator.h \
    models/least_squares_approximator.h


FORMS += \
    views/mainwindow.ui


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources/resources.qrc

