#ifndef MOTOBARCODE_H
#define MOTOBARCODE_H

#include <QObject>
#include <QDebug>
#include <QLibrary>
#include <QtEndian>
#include <QCoreApplication>
#include "barcode_reader_interface.h"
#include <dbt.h>

//#include <setupapi.h>

enum SnapiParamIds
{
    ImageFileType = 304,
    VideoViewfinder = 324
};
class SnapiParam
{
public:
    SnapiParamIds id;
    short value;
};
class motoBARcode : public barcode_reader_interface//, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    explicit motoBARcode()
    {
        QString p = QCoreApplication::applicationDirPath () + "/SNAPI.dll";
        lib.setFileName(p);
        qDebug() << p << " " << lib.load();

        resolve_dll();

    }
    ~motoBARcode()
    {
        //        if (NotificationHandle != nullptr)
        //           UnregisterDeviceNotification(NotificationHandle);
    }
    void NotifyRegister(HWND hwnd)
    {
        //Подписываемся на события
        DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;
        ZeroMemory( &NotificationFilter, sizeof(NotificationFilter) );   //???
        NotificationFilter.dbcc_size = sizeof(NotificationFilter);
        NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
        //       for (int i = 0; i < J2543_interfaces.size(); i++)
        {
            //   NotificationFilter.dbcc_classguid = {0xa5dcbf10, 0x6530, 0x11d2, {0x90, 0x1f, 0x0, 0xc0, 0x4f, 0xb9, 0x51, 0xed}} ; // подпишемся на все наши интерфейсы
            //           NotificationFilter.dbcc_classguid = J2543_interfaces.at(i);
            NotificationFilter.dbcc_name[0] = '\0';
            NotificationHandle = RegisterDeviceNotification( hwnd,
                                                             &NotificationFilter,
                                                             //DEVICE_NOTIFY_ALL_INTERFACE_CLASSES
                                                             DEVICE_NOTIFY_WINDOW_HANDLE
                                                             );
            if ( NotificationHandle == nullptr )
            {
                qDebug() << " event not register!!";
            }
            else
                qDebug() << " event registered!!";
        }
    }
    void ini(HWND hwnd) Q_DECL_OVERRIDE
    {
        qDebug() << "SNAPI_Init: " << (bool*)(SNAPI_Init( hwnd,  DeviceHandles, &NumDevices)+0) << "  NumDevices: " << NumDevices << "  DeviceHandle[0]: " << DeviceHandles[0];

        handle = DeviceHandles[0];
    }
    void ini(uint16_t VID = 0x05E0,
                  uint16_t PID = 0x1900,
                  int iface = 0, int config = 1,
             int alt_config = 0, char EP_INTR = 0x81) Q_DECL_OVERRIDE{}
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result ) Q_DECL_OVERRIDE
    {
        Q_UNUSED( result )
        Q_UNUSED( eventType )

        MSG* pWindowsMessage = static_cast<MSG*>(message);
        void* wParam = (void*)pWindowsMessage->wParam;
        void* lParam = (void*)pWindowsMessage->lParam;

        //qDebug() << pWindowsMessage->message;
        //           PDEV_BROADCAST_DEVICEINTERFACE pDevInf = (PDEV_BROADCAST_DEVICEINTERFACE)lParam;
        if (pWindowsMessage->message < 0x8000)
        {
            return false;
        }
        qDebug() << QString::number(pWindowsMessage->message, 16);
        switch (pWindowsMessage->message)
        {
        case 0x8001:{
            int n = qFromLittleEndian<qint32>(wParam);

            QByteArray bc((char*)dllBuffer, n);
            qDebug() << "barcode code type: " << bc << " n: "  << n;
            emit readyRead_barcode(bc);
            SoundBeeper(26);
            break;}
        case 0x8002:
            //     Image(this, ImageDataFactory.CreateImageData(dllBuffer, hiword));
            break;
        case 0x8003:
        {
            qDebug() << "get video: " ;
            //            Video(this, ImageDataFactory.CreateImageData(dllBuffer, hiword));
            break;
        }
        case 0x8008:{
            int n = (int)&pWindowsMessage->wParam;
            QByteArray debug((char*)versionBuffer, 60);
            qDebug() << "Version: " << debug;
            //        byte[] array4 = new byte[hiword];
            //          Array.Copy(versionBuffer, 0, array4, 0, hiword);
            //          Version(this, new VersionData(array4, hiword));
            break;}
        case 0x8009:
        {
            QByteArray debug((char*)paramBuffer, (int)pWindowsMessage->wParam);
            qDebug() << "ParamData: " << paramBuffer;
            //            Params(this, new ParamData(array2, hiword));
            break;
        }
        case 0x800A:
            //if (Manager.ScannerMsg != null)
            //   onScannerMsg(lParam, pWindowsMessage->message, (int)(pWindowsMessage->wParam + 4), (int)pWindowsMessage->wParam);
            if ( (int)pWindowsMessage->wParam != 0)
            {
                QByteArray debug((char*)capBuffer, (int)pWindowsMessage->wParam);
                qDebug() << "capBuffer: " << debug;
                //             Capabilities(this, new CapabilitiesData(array, hiword));
            }
            break;
        case 0x800E:
        case 0x800F:
            //if (Manager.ScannerMsg != null)
        {
            onScannerMsg((int)(int)pWindowsMessage->lParam, pWindowsMessage->message, (qint32)pWindowsMessage->wParam, (qint32)pWindowsMessage->wParam);
        }
            break;
        case 0x800C:
        {
            qDebug() << "Attach - detach?";
            int num = (qint32)(pWindowsMessage->wParam + 4);
            //            if (num == 0)
            //            {
            //                if (!SnapiScanner.AttachmentBug)
            //                {
            //                    break;
            //                }
            //                if ((int)pWindowsMessage->lParam == 0)
            //                {
            //                    if (priorHandle != 0)
            //                    {
            //                        pWindowsMessage->lParam = (IntPtr)priorHandle;
            //                        SnapiScanner snapiScanner = new SnapiScanner((int)pWindowsMessage->lParam, scannerId++);
            //                        knownScanners[numScanners++] = snapiScanner;
            //                        if (Manager.Attachedmgr != null)
            //                        {
            //                            Manager.Attachedmgr(snapiScanner);
            //                        }
            //                    }
            //                }
            //                else
            //                {
            //                    priorHandle = (int)pWindowsMessage->lParam;
            //                }
            //                break;
            //          }

            //            int num2 = 0;
            //            SnapiScanner[] array = new SnapiScanner[10];
            //            while (knownScanners[num2] != null)
            //            {
            //                if ((int)pWindowsMessage->lParam == knownScanners[num2].handle)
            //                {
            //                    Array.Copy(knownScanners, array, 10L);
            //                    SnapiScanner scanner = knownScanners[num2];
            //                    int i;
            //                    for (i = num2; (long)i < 9L; i++)
            //                    {
            //                        knownScanners[i] = array[i + 1];
            //                    }
            //                    knownScanners[i] = null;
            //                    numScanners--;
            //                    if (Manager.Detachedmgr != null)
            //                    {
            //                        Manager.Detachedmgr(scanner);
            //                    }
            //                    break;
            //                }
            //                num2++;
            //                if ((long)num2 == 10)
            //                {
            //                    break;
            //                }
            //            }

            //            if (Manager.ScannerMsg != null)
            //            {
            onScannerMsg((int)(int)pWindowsMessage->lParam, pWindowsMessage->message, num, (qint32)pWindowsMessage->wParam);
            //Manager.ScannerMsg((int)pWindowsMessage->lParam, pWindowsMessage->message, num, Marshal.ReadInt32(pWindowsMessage->wParam, 0));
            //            }
            break;
        }
        case 0x8004:
        case 0x8005:
        case 0x8006:
        case 0x8007:
        case 0x800B:
        case 0x800D:
            break;
        }
        return false;
    }

public slots:
    void init() Q_DECL_OVERRIDE
    {
        Claim();
        TransmitVersion();
        //  SnapShot();
        //PullTrigger();
        SoundBeeper(26);
        qDebug() << "";
    }
private:
    int DeviceHandles[10];
    int NumDevices;
    HDEVNOTIFY NotificationHandle;

    int handle;
    bool isAttached = false;
    bool isValid = true;
    void* dllBuffer;
    void* versionBuffer;
    void* paramBuffer;
    void* capBuffer;
    bool paramPersist;

    void onScannerMsg(int handle, int msg, int loword, int hiword)
    {
        if (this->handle != handle)
        {
            return;
        }
        switch (msg)
        {
        case 0x800F:
            qDebug() << "FirmwareUpdate 0x800F: " ;
            //            FirmwareUpdate(this, new FirmwareUpdateData(false, hiword));
            break;
        case 0x800E:
            qDebug() << "FirmwareUpdate 0x800E: " ;
            //            FirmwareUpdate(this, new FirmwareUpdateData(true, hiword));
            break;

        case 32780: qDebug() << "detach???";
            if (loword == 1)
            {
                //             Manager.ScannerMsg -= onScannerMsg;
                //             SnapiScanner.Disconnect(this);
                Release();
                isValid = false;
            }
            break;
        case 32772:
        case 32773:
        case 32774:
        case 32775:
        case 32779:
        case 32781:
            break;
        }
    }

    bool Claim()
    {
        if (isAttached)
        {
            return true;
        }
        if (!isValid)
        {
            return false;
        }
        if (SNAPI_Connect(handle) == 0)
        {
            isAttached = true;
            //            Monitor.Enter(this);
            dllBuffer = malloc(1312000);
            versionBuffer = malloc(1312000);
            paramBuffer = malloc(1312000);
            capBuffer = malloc(1312000);

            SNAPI_SetDecodeBuffer(handle, dllBuffer, 1312000);
            SNAPI_SetImageBuffer(handle, dllBuffer, 1312000);
            SNAPI_SetVideoBuffer(handle, dllBuffer, 1312000);
            SNAPI_SetVersionBuffer(handle, versionBuffer, 1312000);
            SNAPI_SetCapabilitiesBuffer(handle, capBuffer, 1312000);
            SNAPI_SetParameterBuffer(handle, paramBuffer, 1312000);
            SNAPI_SetParamPersistance(handle, paramPersist ? 1 : 0);
            //            Monitor.Exit(this);
        }
        return isAttached;
    }
    void Release()
    {
        if (isAttached)
        {
            //            Monitor.Enter(this);
            SNAPI_Disconnect(handle);
            isAttached = false;
            free(dllBuffer);
            free(versionBuffer);
            free(paramBuffer);
            free(capBuffer);
            //            Monitor.Exit(this);
        }
    }
    bool SetParam(SnapiParam param)
    {
        if (!isAttached || !isValid)
        {
            return false;
        }

        short params[2] { static_cast<short>(param.id), param.value };
        return SNAPI_SetParameters(params, 2, handle) == 0;
    }
    void OnAttachmentDevice(//SnapiScanner s
                            )
    {
        //        prevHandleDis = 0;
        //        if (s.handle != prevHandleCon)
        //        {
        //            prevHandleCon = s.handle;
        //            if (SnapiScanner.Attached != null)
        //            {
        //                SnapiScanner.Attached(s);
        //            }
        //        }
    }
    void OnDetachmentDevice(//SnapiScanner s
                            )
    {
        //        prevHandleCon = 0;
        //        if (s.handle != prevHandleDis)
        //        {
        //            prevHandleDis = s.handle;
        //            if (SnapiScanner.Disconnect != null)
        //            {
        //                SnapiScanner.Disconnect(s);
        //            }
        //        }
    }
    int SnapShot()
    {
        if (!isAttached || !isValid)
        {
            return false;
        }
        return SNAPI_SnapShot(handle);
    }
    int AbortFirmwareUpdate()
    {
        if (!isAttached || !isValid)
        {
            return false;
        }
        return SNAPI_AbortFirmwareUpdate(handle);
    }
    int UpdateFirmware(QByteArray filePath)
    {
        if (!isAttached || !isValid)
        {
            return false;
        }
        return SNAPI_UpdateFirmware(filePath.data(), handle);
    }
    int RequestScannerCapabilities()
    {
        if (!isAttached || !isValid)
            return false;
        return SNAPI_RequestScannerCapabilities(handle);
    }
    int TransmitVersion()
    {
        if (!isAttached || !isValid)
        {
            return false;
        }
        return SNAPI_TransmitVersion(handle);
    }
    int RequestParameters(short ParamId[2])
    {
        if (!isAttached || !isValid)
        {
            return false;
        }
        return SNAPI_RequestParameters(ParamId, 2, handle);
    }
    int SoundBeeper(byte nBeepCode)
    {
        if (!isAttached || !isValid)
        {
            return false;
        }
        return SNAPI_SoundBeeper(handle, nBeepCode);
    }
    int LedOff(byte nLEDSelection)
    {
        if (!isAttached || !isValid)
        {
            return false;
        }
        return SNAPI_LedOff(handle, nLEDSelection);
    }
    int LedOn(byte nLEDSelection)
    {
        if (!isAttached || !isValid)
        {
            return false;
        }
        return SNAPI_LedOn(handle, nLEDSelection);
    }
    int ReleaseTrigger()
    {
        if (!isAttached || !isValid)
        {
            return false;
        }
        return SNAPI_ReleaseTrigger(handle);
    }
    int PullTrigger()
    {
        if (!isAttached || !isValid)
            return false;
        return SNAPI_PullTrigger(handle);
    }
    int AimOn()
    {
        if (!isAttached || !isValid)
            return false;
        return SNAPI_AimOn(handle);
    }
    int AimOff()
    {
        if (!isAttached || !isValid)
            return false;
        return SNAPI_AimOff(handle);
    }
    int FlushMacroPdf()
    {
        if (!isAttached || !isValid)
            return false;
        return SNAPI_FlushMacroPdf(handle);
    }
    int AbortMacroPdf()
    {
        if (!isAttached || !isValid)
            return false;
        return SNAPI_AbortMacroPdf(handle);
    }
    int SetParameterDefaults()
    {
        if (!isAttached || !isValid)
            return false;
        return SNAPI_SetParameterDefaults(handle);
    }
    int TransmitVideo()
    {
        if (!isAttached || !isValid)
            return false;
        return SNAPI_TransmitVideo(handle);
    }

    void resolve_dll()
    {
        SNAPI_Init = (_SNAPI_Init) lib.resolve("SNAPI_Init");
        SNAPI_Connect = (_SNAPI_Connect) lib.resolve("SNAPI_Connect");
        SNAPI_Disconnect = (_SNAPI_Disconnect) lib.resolve("SNAPI_Disconnect");
        SNAPI_SetDecodeBuffer = (_SNAPI_SetDecodeBuffer) lib.resolve("SNAPI_SetDecodeBuffer");
        SNAPI_SetImageBuffer = (_SNAPI_SetImageBuffer) lib.resolve("SNAPI_SetImageBuffer");
        SNAPI_SetVideoBuffer = (_SNAPI_SetVideoBuffer) lib.resolve("SNAPI_SetVideoBuffer");
        SNAPI_SetVersionBuffer = (_SNAPI_SetVersionBuffer) lib.resolve("SNAPI_SetVersionBuffer");
        SNAPI_SetCapabilitiesBuffer = (_SNAPI_SetCapabilitiesBuffer) lib.resolve("SNAPI_SetCapabilitiesBuffer");
        SNAPI_SetParameterBuffer = (_SNAPI_SetParameterBuffer) lib.resolve("SNAPI_SetParameterBuffer");
        SNAPI_SetParamPersistance = (_SNAPI_SetParamPersistance) lib.resolve("SNAPI_SetParamPersistance");
        SNAPI_SetParameters = (_SNAPI_SetParameters) lib.resolve("SNAPI_SetParameters");
        SNAPI_SnapShot = (_SNAPI_SnapShot) lib.resolve("SNAPI_SnapShot");
        SNAPI_AbortFirmwareUpdate = (_SNAPI_AbortFirmwareUpdate) lib.resolve("SNAPI_AbortFirmwareUpdate");
        SNAPI_UpdateFirmware = (_SNAPI_UpdateFirmware) lib.resolve("SNAPI_UpdateFirmware");
        SNAPI_RequestScannerCapabilities = (_SNAPI_RequestScannerCapabilities) lib.resolve("SNAPI_RequestScannerCapabilities");
        SNAPI_TransmitVersion = (_SNAPI_TransmitVersion) lib.resolve("SNAPI_TransmitVersion");
        SNAPI_RequestParameters = (_SNAPI_RequestParameters) lib.resolve("SNAPI_RequestParameters");
        SNAPI_SoundBeeper = (_SNAPI_SoundBeeper) lib.resolve("SNAPI_SoundBeeper");
        SNAPI_LedOff = (_SNAPI_LedOff) lib.resolve("SNAPI_LedOff");
        SNAPI_LedOn = (_SNAPI_LedOn) lib.resolve("SNAPI_LedOn");
        SNAPI_ReleaseTrigger = (_SNAPI_ReleaseTrigger) lib.resolve("SNAPI_ReleaseTrigger");
        SNAPI_PullTrigger = (_SNAPI_PullTrigger) lib.resolve("SNAPI_PullTrigger");
        SNAPI_AimOn = (_SNAPI_AimOn) lib.resolve("SNAPI_AimOn");
        SNAPI_AimOff = (_SNAPI_AimOff) lib.resolve("SNAPI_AimOff");
        SNAPI_FlushMacroPdf = (_SNAPI_FlushMacroPdf) lib.resolve("SNAPI_FlushMacroPdf");
        SNAPI_AbortMacroPdf = (_SNAPI_AbortMacroPdf) lib.resolve("SNAPI_AbortMacroPdf");
        SNAPI_SetParameterDefaults = (_SNAPI_SetParameterDefaults) lib.resolve("SNAPI_SetParameterDefaults");
        SNAPI_TransmitVideo = (_SNAPI_TransmitVideo) lib.resolve("SNAPI_TransmitVideo");
    }
    QLibrary lib;
    typedef long int (__stdcall *_SNAPI_Init)(void*, int[], int*);
    _SNAPI_Init SNAPI_Init;
    typedef long int (__stdcall *_SNAPI_Connect)(int);
    _SNAPI_Connect SNAPI_Connect;
    typedef long int (__stdcall *_SNAPI_Disconnect)(int);
    _SNAPI_Disconnect SNAPI_Disconnect;
    typedef long int (__stdcall *_SNAPI_SetDecodeBuffer)(int, void*, int);
    _SNAPI_SetDecodeBuffer SNAPI_SetDecodeBuffer;
    typedef long int (__stdcall *_SNAPI_SetImageBuffer)(int, void*, int);
    _SNAPI_SetImageBuffer SNAPI_SetImageBuffer;
    typedef long int (__stdcall *_SNAPI_SetVideoBuffer)(int, void*, int);
    _SNAPI_SetVideoBuffer SNAPI_SetVideoBuffer;
    typedef long int (__stdcall *_SNAPI_SetVersionBuffer)(int, void*, int);
    _SNAPI_SetVersionBuffer SNAPI_SetVersionBuffer;
    typedef long int (__stdcall *_SNAPI_SetCapabilitiesBuffer)(int, void*, int);
    _SNAPI_SetCapabilitiesBuffer SNAPI_SetCapabilitiesBuffer;
    typedef long int (__stdcall *_SNAPI_SetParameterBuffer)(int, void*, int);
    _SNAPI_SetParameterBuffer SNAPI_SetParameterBuffer;
    typedef long int (__stdcall *_SNAPI_SetParamPersistance)(int, int);
    _SNAPI_SetParamPersistance SNAPI_SetParamPersistance;
    typedef long int (__stdcall *_SNAPI_SetParameters)(void*, int, int);
    _SNAPI_SetParameters SNAPI_SetParameters;
    typedef long int (__stdcall *_SNAPI_SnapShot)(int);
    _SNAPI_SnapShot SNAPI_SnapShot;
    typedef long int (__stdcall *_SNAPI_AbortFirmwareUpdate)(int);
    _SNAPI_AbortFirmwareUpdate SNAPI_AbortFirmwareUpdate;
    typedef long int (__stdcall *_SNAPI_UpdateFirmware)(void*, int);
    _SNAPI_UpdateFirmware SNAPI_UpdateFirmware;
    typedef long int (__stdcall *_SNAPI_RequestScannerCapabilities)(int);
    _SNAPI_RequestScannerCapabilities SNAPI_RequestScannerCapabilities;
    typedef long int (__stdcall *_SNAPI_TransmitVersion)( int);
    _SNAPI_TransmitVersion SNAPI_TransmitVersion;
    typedef long int (__stdcall *_SNAPI_RequestParameters)(short [], int, int);
    _SNAPI_RequestParameters SNAPI_RequestParameters;
    typedef long int (__stdcall *_SNAPI_SoundBeeper)(int, byte);
    _SNAPI_SoundBeeper SNAPI_SoundBeeper;
    typedef long int (__stdcall *_SNAPI_LedOff)(int, byte);
    _SNAPI_LedOff SNAPI_LedOff;
    typedef long int (__stdcall *_SNAPI_LedOn)(int, byte);
    _SNAPI_LedOn SNAPI_LedOn;
    typedef long int (__stdcall *_SNAPI_ReleaseTrigger)( int);
    _SNAPI_ReleaseTrigger SNAPI_ReleaseTrigger;
    typedef long int (__stdcall *_SNAPI_PullTrigger)( int);
    _SNAPI_PullTrigger SNAPI_PullTrigger;
    typedef long int (__stdcall *_SNAPI_AimOn)(int);
    _SNAPI_AimOn SNAPI_AimOn;
    typedef long int (__stdcall *_SNAPI_AimOff)(int);
    _SNAPI_AimOff SNAPI_AimOff;
    typedef long int (__stdcall *_SNAPI_FlushMacroPdf)(int);
    _SNAPI_FlushMacroPdf SNAPI_FlushMacroPdf;
    typedef long int (__stdcall *_SNAPI_AbortMacroPdf)(int);
    _SNAPI_AbortMacroPdf SNAPI_AbortMacroPdf;
    typedef long int (__stdcall *_SNAPI_SetParameterDefaults)(int);
    _SNAPI_SetParameterDefaults SNAPI_SetParameterDefaults;
    typedef long int (__stdcall *_SNAPI_TransmitVideo)(int);
    _SNAPI_TransmitVideo SNAPI_TransmitVideo;
signals:
    void readyRead_barcode(QByteArray);
    void init_completed();
    void log(QString);
};

#endif // MOTOBARCODE_H
