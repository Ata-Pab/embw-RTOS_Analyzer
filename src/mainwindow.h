#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "rtos/rtosevent.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class UdpReceiver;
class LogPanel;
class TracerPanel;
class QLabel;
class QLineEdit;
class QSpinBox;
class QPushButton;
class QScrollArea;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onConnectClicked();
    void onDisconnectClicked();
    void onClearClicked();
    void onEventReceived(const RtosEvent &event);
    void onError(const QString &msg);

private:
    void buildUi();
    void setConnected(bool connected);

    Ui::MainWindow *ui;

    UdpReceiver *m_receiver{nullptr};
    LogPanel *m_logPanel{nullptr};
    TracerPanel *m_tracerPanel{nullptr};

    QLineEdit *m_ipEdit{nullptr};
    QSpinBox *m_portSpin{nullptr};
    QPushButton *m_connectBtn{nullptr};
    QPushButton *m_disconnectBtn{nullptr};
    QLabel *m_statusDot{nullptr};
    QLabel *m_statusLabel{nullptr};
    QLabel *m_eventCountLabel{nullptr};
    int m_eventCount{0};
};

#endif // MAINWINDOW_H
