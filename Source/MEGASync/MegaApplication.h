#ifndef MEGAAPPLICATION_H
#define MEGAAPPLICATION_H

#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QDir>
#include <QLocalServer>
#include <QLocalSocket>
#include <QDataStream>
#include <QQueue>

#include "gui/InfoDialog.h"
#include "gui/SetupWizard.h"
#include "gui/SettingsDialog.h"
#include "gui/UploadToMegaDialog.h"
#include "gui/MultiQFileDialog.h"
#include "gui/PasteMegaLinksDialog.h"
#include "control/Preferences.h"
#include "control/HTTPServer.h"
#include "control/MegaUploader.h"
#include "control/UpdateTask.h"
#include "sdk/megaapi.h"
#include "sdk/qt/QTMegaListener.h"

Q_DECLARE_METATYPE(QQueue<QString>)

class Notificator;

class MegaApplication : public QApplication, public MegaListener
{
    Q_OBJECT

public:
    explicit MegaApplication(int &argc, char **argv);
    ~MegaApplication();

    void initialize();

    static const int VERSION_CODE;
    static const QString VERSION_STRING;
    static const QString TRANSLATION_FOLDER;
    static const QString TRANSLATION_PREFIX;
    static QString applicationFilePath();
    static QString applicationDirPath();
    static QString applicationDataPath();
    void changeLanguage(QString languageCode);
    void updateTrayIcon();

    virtual void onRequestStart(MegaApi* api, MegaRequest *request);
    virtual void onRequestFinish(MegaApi* api, MegaRequest *request, MegaError* e);
    virtual void onRequestTemporaryError(MegaApi *api, MegaRequest *request, MegaError* e);
	virtual void onTransferStart(MegaApi *api, MegaTransfer *transfer);
	virtual void onTransferFinish(MegaApi* api, MegaTransfer *transfer, MegaError* e);
    virtual void onTransferUpdate(MegaApi *api, MegaTransfer *transfer);
    virtual void onTransferTemporaryError(MegaApi *api, MegaTransfer *transfer, MegaError* e);
    virtual void onUsersUpdate(MegaApi* api, UserList *users);
    virtual void onNodesUpdate(MegaApi* api, NodeList *nodes);
    virtual void onReloadNeeded(MegaApi* api);
    virtual void onSyncStateChanged(MegaApi *api);

	/*
    virtual void onSyncStateChanged(Sync*, syncstate);
    virtual void onSyncRemoteCopy(Sync*, const char*);
    virtual void onSyncGet(Sync*, const char*);
    virtual void onSyncPut(Sync*, const char*);
	*/

    MegaApi *getMegaApi() { return megaApi; }

    void unlink();
    void showInfoMessage(QString message, QString title = tr("MEGAsync"));
    void showWarningMessage(QString message, QString title = tr("MEGAsync"));
    void showErrorMessage(QString message, QString title = tr("MEGAsync"));
    void showNotificationMessage(QString message, QString title = tr("MEGAsync"));
    void setUploadLimit(int limit);
    void startUpdateTask();
    void stopUpdateTask();
    void applyProxySettings();
    void showUpdatedMessage();
    void updateUserStats();
    void addRecentFile(QString fileName, long long fileHandle, QString localPath = QString());
    void checkForUpdates();
    void showTrayMenu(QPoint *point = NULL);

signals:
    void startUpdaterThread();
    void tryUpdate();
    void installUpdate();

public slots:
    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void onMessageClicked();
    void start();
    void openSettings();
    void changeProxy();
    void pauseSync();
    void resumeSync();
	void importLinks();
    void uploadActionClicked();
    void copyFileLink(mega::handle fileHandle);
    void shellUpload(QQueue<QString> newUploadQueue);
    void shellExport(QQueue<QString> newExportQueue);
	void showUploadDialog();
	void onLinkImportFinished();
    void onRequestLinksFinished();
    void onUpdateCompleted();
    void onUpdateAvailable(bool requested);
    void onInstallingUpdate(bool requested);
    void onUpdateNotFound(bool requested);
    void onUpdateError();
    void rebootApplication(bool update = true);
    void exitApplication();
    void pauseTransfers(bool pause);
    void aboutDialog();
    void refreshTrayIcon();
    void cleanAll();
    void onDupplicateLink(QString link, QString name, long long handle);
    void onDupplicateUpload(QString localPath, QString name, long long handle);
    void onInstallUpdateClicked();
    void showInfoDialog();

protected:
    void createTrayIcon();
    bool showTrayIconAlwaysNEW();
    void loggedIn();
    void startSyncs();
	void stopSyncs();
    void processUploadQueue(mega::handle nodeHandle);

    QSystemTrayIcon *trayIcon;
    QMenu *initialMenu;
    QMenu *trayMenu;
    QAction *exitAction;
    QAction *settingsAction;
    //QAction *pauseAction;
    //QAction *resumeAction;
	QAction *importLinksAction;
    QAction *uploadAction;
    QAction *aboutAction;
    QAction *changeProxyAction;
    QAction *initialExitAction;
    QAction *updateAction;

	SetupWizard *setupWizard;
    SettingsDialog *settingsDialog;
    InfoDialog *infoDialog;
    Preferences *preferences;
    MegaApi *megaApi;
    HTTPServer *httpServer;
    UploadToMegaDialog *uploadFolderSelector;
    MultiQFileDialog *multiUploadFileDialog;
	QQueue<QString> uploadQueue;
	long long totalDownloadSize, totalUploadSize;
	long long totalDownloadedSize, totalUploadedSize;
	long long uploadSpeed, downloadSpeed;
    long long lastStartedDownload;
    long long lastStartedUpload;
    int exportOps;
    mega::syncstate_t syncState;
	QTMegaListener *delegateListener;
	QMap<int, QString> uploadLocalPaths;
    MegaUploader *uploader;
    QTimer *refreshTimer;
    QTimer *infoDialogTimer;
    QTranslator *translator;
    PasteMegaLinksDialog *pasteMegaLinksDialog;
    QMessageBox *exitDialog;
    QString lastTrayMessage;

    static QString appPath;
    static QString appDirPath;
    static QString dataPath;

    QThread *updateThread;
    UpdateTask *updateTask;
    Notificator *notificator;

    bool reboot;
    bool syncActive;
    bool paused;
    bool indexing;
    bool waiting;
    bool updated;
    bool updateBlocked;
    long long lastExit;
    bool finished;
    bool updateAvailable;
};

#endif // MEGAAPPLICATION_H
