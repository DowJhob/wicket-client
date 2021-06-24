#ifndef BARCODE_READER_INTERFACE_H
#define BARCODE_READER_INTERFACE_H

#include <QObject>
#include <QAbstractNativeEventFilter>
#include <windows.h>

class barcode_reader_interface : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    explicit barcode_reader_interface()
        //: VID( VID), PID( PID), EP_INTR(EP_INTR), iface ( iface), config (config), alt_config ( alt_config)
    {}

    virtual void ini(HWND hwnd) = 0;
    virtual void ini(uint16_t VID = 0x05E0,
              uint16_t PID = 0x1900,
              int iface = 0, int config = 1,
              int alt_config = 0, char EP_INTR = 0x81) = 0;

//explicit barcode_reader_interface();
  //  virtual bool nativeEventFilter(const QByteArray &eventType, void *message, long *result ) Q_DECL_OVERRIDE;
public slots:
    virtual void init() = 0;

signals:
    void readyRead_barcode(QByteArray);
    void init_completed();
    void log(QString);

};

#endif // BARCODE_READER_INTERFACE_H
