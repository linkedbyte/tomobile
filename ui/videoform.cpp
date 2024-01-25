#include "VideoForm.h"
#include "ui_VideoForm.h"
#include <QDesktopWidget>
#include <QFileInfo>
#include <QLabel>
#include <QMessageBox>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>
#include <QShortcut>
#include <QStyle>
#include <QStyleOption>
#include <QTimer>
#include <QWindow>
#include <QLabel>
#include <QShortcut>
#include <QClipboard>
#include <QPushButton>
#include <QStyleOption>
#include <QPainter>
#include "videotitlebar.h"

#define TITLE_BAR_HEIGHT 36
#define BOTTOM_BAR_HEIGHT 50
#define LEFT_MARGIN 0
#define TOP_MARGIN 0
#define RIGHT_MARGIN 0
#define BOTTOM_MARGIN 0

VideoForm::VideoForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoForm)
{
    ui->setupUi(this);
    videoWidget = new VideoWidget(this);
    videoWidget->setObjectName("VideoForm");
    hasframe = false;
    controller = NULL;
    lastFrame = NULL;

    setMouseTracking(true);
    this->setAcceptDrops(true);

    initShortcut();

#ifndef Q_OS_OSX
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
#endif

    titleBar = new VideoTitleBar(this);
    connect(titleBar,&VideoTitleBar::barAction,this,[=](VideoTitleBarAction action){
        switch (action) {
        case VideoTitleBarAction::CLOSE:
            this->close();
            break;
        case VideoTitleBarAction::MIN:
            this->showMinimized();
            break;
        case VideoTitleBarAction::MAX:
            this->showMaximized();
            break;
        case VideoTitleBarAction::FULLSCREEN:
            this->switchFullScreen();
            break;
        default:
            break;
        }

    });
    bottomBar = new VideoBottomBar(this);
    connect(bottomBar,&VideoBottomBar::barAction,this,[=](VideoBottomBarAction action){
        if(!controller)
            return;
        switch(action)
        {
        case VideoBottomBarAction::HOME:
            this->controller->actionHome();
            break;
        case VideoBottomBarAction::BACK:
            this->controller->actionBack();
            break;
        case VideoBottomBarAction::MENU:
            this->controller->actionMenu();
            break;
        default:
            break;
        }
    });
}
void VideoForm::showFPS(bool flag)
{
    if (!fpsLabel) {
        return;
    }
    fpsLabel->setVisible(flag);
}
void VideoForm::updateFPS(uint fps)
{
    if (!fpsLabel) {
        return;
    }
    fpsLabel->setText(QString("FPS:%1").arg(fps));
}

void VideoForm::closeEvent(QCloseEvent *event)
{
    emit signal_close();
    event->accept();
}

VideoForm::~VideoForm()
{
    delete ui;
}

void VideoForm::setTitle(const std::string &title)
{
    this->titleBar->setDeviceName(title);
}
void VideoForm::setVisible(bool visible)
{
    QWidget::setVisible(visible);
}
void VideoForm::updateFrame()
{
    AVFrame *frame = av_frame_alloc();
    this->frameBuffer->consume(frame);
    QSize newSize(frame->width,frame->height);
    updateShowSize(newSize);
    videoWidget->updateTextures(frame->data[0],frame->data[1],frame->data[2],
                                frame->linesize[0],frame->linesize[1],frame->linesize[2]);
    av_frame_free(&frame);
}

void VideoForm::setController(ControllerThread *handle)
{
    if(!handle)
        return;
    this->controller = handle;
}

void VideoForm::setFrameBuffer(FrameBuffer *fb)
{
    frameBuffer = fb;
}

void VideoForm::dropEvent(QDropEvent *event)
{
    const QMimeData *qmd = event->mimeData();
    QList<QUrl> urls = qmd->urls();

    for (const QUrl &url : urls) {
        QString file = url.toLocalFile();
        QFileInfo fileInfo(file);

        if (!fileInfo.exists()) {
            //file does not exist
            continue;
        }

        if (fileInfo.isFile() && fileInfo.suffix() == "apk") {
            //emit device->installApkRequest(file);
            continue;
        }
       // emit device->pushFileRequest(file, Config::getInstance().getPushFilePath() + fileInfo.fileName());
    }

}
void VideoForm::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}
QRect VideoForm::getScreenRect()
{
    QRect screenRect;
    QWidget *win = window();
    if (!win) {
        return screenRect;
    }

    QWindow *winHandle = win->windowHandle();
    QScreen *screen = QGuiApplication::primaryScreen();
    if (winHandle) {
        screen = winHandle->screen();
    }
    if (!screen) {
        return screenRect;
    }

    screenRect = screen->availableGeometry();
    return screenRect;
}
void VideoForm::updateShowSize(const QSize &size)
{
    if (frameSize != size) {
        frameSize = size;
        this->videoWidget->setFrameSize(size);

        widthHeightRatio = 1.0f * size.width() / size.height();

        bool vertical = widthHeightRatio < 1.0f ? true : false;
        QSize showSize = size;
        QRect screenRect = getScreenRect();
        if (screenRect.isEmpty()) {
            qWarning() << "getScreenRect is empty";
            return;
        }
        if (vertical) {
            showSize.setHeight(qMin(size.height(), screenRect.height() - 160));
            showSize.setWidth(showSize.height() * widthHeightRatio);
        } else {
            showSize.setWidth(qMin(size.width(), screenRect.width() / 2));
            showSize.setHeight(showSize.width() / widthHeightRatio);
        }

        if (showSize != this->size()) {
            resize(showSize.width(),showSize.height()+TITLE_BAR_HEIGHT+BOTTOM_BAR_HEIGHT);

            moveCenter();
        }
    }
}
void VideoForm::moveCenter()
{
    QRect screenRect = getScreenRect();
    if (screenRect.isEmpty()) {
        qWarning() << "getScreenRect is empty";
    }
    else{
        move(screenRect.center() - QRect(0, 0, size().width(), size().height()).center());
    }
}
void VideoForm::mousePressEvent(QMouseEvent *event)
{
    if(videoWidget->geometry().contains(event->pos()))
    {
        if(!controller)
            return;

        if (event->button() == Qt::MiddleButton)
        {
            controller->actionHome();
        }
        else if(event->button() == Qt::XButton1)
        {
            controller->actionAppSwitch();
        }
        else if(event->button() == Qt::XButton2)
        {
            /*if (event->clicks() < 2) {
            expand_notification_panel(controller);
        } else {
            expand_settings_panel(controller);
        }*/
        }
        else if(event->button() == Qt::RightButton)
        {
            controller->pressBackOrTurnScreenOn();
        }
        else
        {
            QPoint pos = videoWidget->mapFromParent(event->pos());
            event->setLocalPos(pos);
            controller->mouseEvent(event,videoWidget->getFrameSize(),videoWidget->size());
        }
    }
    else
    {
        if(event->button() == Qt::LeftButton)
        {
            dragPosition = event->globalPos() - frameGeometry().topLeft();
            event->accept();
        }
    }
}

void VideoForm::switchFullScreen()
{
    if(isFullScreen())
    {
        showNormal();
        QSize size = this->size();
        this->titleBar->setVisible(true);
        this->bottomBar->setVisible(true);
        this->resize(size.width(),size.height());
    }
    else
    {
        QScreen *screen = QGuiApplication::primaryScreen();
        QSize screenSize = screen->size();
        showFullScreen();
        this->titleBar->setVisible(false);
        this->bottomBar->setVisible(false);
        int w,h,x,y;
        float whratio = 1.0f * frameSize.width()/frameSize.height();
        if(whratio<1.0f)//垂直
        {
            h = screenSize.height();
            w = screenSize.height()*whratio;
            x = (screenSize.width()-w)/2;
            y = 0;
            this->videoWidget->resize(w,h);
            this->videoWidget->move(x,y);
        }
        else
        {
            w = screenSize.width();
            h = screenSize.width()*whratio;
            y = (screenSize.height()-h)/2;
            x = 0;
            this->videoWidget->resize(w,h);
            this->videoWidget->move(x,y);
        }
    }
}

void VideoForm::resize(int w, int h)
{
    this->titleBar->setGeometry(0,0,w,TITLE_BAR_HEIGHT);
    this->videoWidget->setGeometry(LEFT_MARGIN,this->titleBar->height(),w,h-TITLE_BAR_HEIGHT-BOTTOM_BAR_HEIGHT);
    this->bottomBar->setGeometry(0,videoWidget->y()+videoWidget->height(),w,BOTTOM_BAR_HEIGHT);
    QWidget::resize(w,h);
}

void VideoForm::showMaximized()
{
    if(isMaximized())
    {
        showNormal();
        QSize size = this->size();
        this->resize(size.width(),size.height());
    }
    else
    {
        QWidget::showMaximized();
        QSize screenSize = this->size();
        int w,h,x,y;
        float whRatio = 1.0f * frameSize.width()/frameSize.height();
        if(whRatio<1.0f)//垂直
        {
            h = screenSize.height()-TITLE_BAR_HEIGHT-BOTTOM_BAR_HEIGHT;
            w = h*whRatio;
            x = (screenSize.width()-w)/2;

            this->titleBar->setGeometry(x,0,w,TITLE_BAR_HEIGHT);
            this->videoWidget->resize(w,h);
            this->videoWidget->move(x,TITLE_BAR_HEIGHT);
            y = videoWidget->y()+videoWidget->height();

            this->bottomBar->setGeometry(x,y,w,BOTTOM_BAR_HEIGHT);
        }
        else
        {
            w = screenSize.width();
            h = screenSize.width()*whRatio;
            y = (screenSize.height()-h)/2;
            x = 0;
            this->videoWidget->resize(w,h);
            this->videoWidget->move(x,y);
        }
    }

}
void VideoForm::initShortcut()
{
    QShortcut *shortcut = nullptr;

    shortcut = new QShortcut(QKeySequence("Ctrl+f"), this);
    shortcut->setAutoRepeat(false);
    connect(shortcut, &QShortcut::activated, this, [this]() {
        switchFullScreen();
    });

    shortcut = new QShortcut(QKeySequence("Ctrl+h"), this);
    shortcut->setAutoRepeat(false);
    connect(shortcut, &QShortcut::activated, this, [this]() {
        if(controller)
            controller->actionHome();
    });


    shortcut = new QShortcut(QKeySequence("Ctrl+s"), this);
    shortcut->setAutoRepeat(false);
    connect(shortcut, &QShortcut::activated, this, [this]() {
        controller->actionAppSwitch();
    });

    shortcut = new QShortcut(QKeySequence("Ctrl+m"), this);
    shortcut->setAutoRepeat(false);
    connect(shortcut, &QShortcut::activated, this, [this]() {
        controller->actionMenu();
    });

    shortcut = new QShortcut(QKeySequence("Ctrl+up"), this);
    shortcut->setAutoRepeat(false);
    connect(shortcut, &QShortcut::activated, this, [this]() {
        controller->actionVolumeUp();
    });

    shortcut = new QShortcut(QKeySequence("Ctrl+down"), this);
    shortcut->setAutoRepeat(false);
    connect(shortcut, &QShortcut::activated, this, [this]() {
        controller->actionVolumeDown();
    });

    shortcut = new QShortcut(QKeySequence("Ctrl+p"), this);
    shortcut->setAutoRepeat(false);
    connect(shortcut, &QShortcut::activated, this, [this]() {
        controller->actionPower();
    });

    shortcut = new QShortcut(QKeySequence("Ctrl+o"), this);
    shortcut->setAutoRepeat(false);
    connect(shortcut, &QShortcut::activated, this, [this]() {
        controller->setScreenPowerMode(!screenMode);
    });

    shortcut = new QShortcut(QKeySequence("Ctrl+n"), this);
    shortcut->setAutoRepeat(false);
    connect(shortcut, &QShortcut::activated, this, [this]() {

        controller->expandNotificationPanel();
    });

    shortcut = new QShortcut(QKeySequence("Ctrl+Shift+n"), this);
    shortcut->setAutoRepeat(false);
    connect(shortcut, &QShortcut::activated, this, [this]() {

        controller->collapsePanels();
    });


    shortcut = new QShortcut(QKeySequence("Ctrl+r"), this);
    shortcut->setAutoRepeat(false);
    connect(shortcut, &QShortcut::activated, this, [this]() {

        controller->rotateDevice();
    });


    shortcut = new QShortcut(QKeySequence("Ctrl+c"), this);
    shortcut->setAutoRepeat(false);
    connect(shortcut, &QShortcut::activated, this, [this]() {

        controller->getDeviceClipboard(COPY_KEY_COPY);
    });

    shortcut = new QShortcut(QKeySequence("Ctrl+v"), this);
    shortcut->setAutoRepeat(false);
    connect(shortcut, &QShortcut::activated, this, [this]() {
        QClipboard *board = QApplication::clipboard();
        QString text = board->text();

        controller->setDeviceClipboard(text.toStdString().c_str());
    });

    shortcut = new QShortcut(QKeySequence("Ctrl+x"), this);
    shortcut->setAutoRepeat(false);
    connect(shortcut, &QShortcut::activated, this, [this]() {

        controller->getDeviceClipboard(COPY_KEY_CUT);
    });

}
void VideoForm::mouseReleaseEvent(QMouseEvent *event)
{
    if (videoWidget->geometry().contains(event->pos()))
    {
        if(!controller)
            return;
        controller->mouseEvent(event,videoWidget->getFrameSize(),videoWidget->size());
    }
    else{
        dragPosition = QPoint(0, 0);
    }
}
void VideoForm::mouseMoveEvent(QMouseEvent *event)
{
    if (videoWidget->geometry().contains(event->pos()))
    {
        if(event->buttons() & Qt::LeftButton){
            if(controller)
                controller->mouseMoveEvent(event,videoWidget->getFrameSize(),videoWidget->size());
        }
    }
    else
    {
        if(!dragPosition.isNull())
        {
            if (event->buttons() & Qt::LeftButton) {
                move(event->globalPos() - dragPosition);
                event->accept();
            }
        }
    }

}
void VideoForm::mouseDoubleClickEvent(QMouseEvent *event)
{

}
void VideoForm::wheelEvent(QWheelEvent *event)
{
    if (videoWidget->geometry().contains(event->position().toPoint()))
    {
        if(controller)
            controller->mouseWhellEvent(event,videoWidget->getFrameSize(),videoWidget->size());
    }
}
void VideoForm::keyPressEvent(QKeyEvent *event)
{

    if(event->key() == Qt::Key_Escape && !event->isAutoRepeat()){
        //switchFullScreen();
    }
    //this->isShortcut(event);
}
void VideoForm::keyReleaseEvent(QKeyEvent *event)
{

}

enum Qt::Key ShortcutKeys[] = {
    Qt::Key_C,
    Qt::Key_F,
    Qt::Key_H,
    Qt::Key_F,
    Qt::Key_S,
    Qt::Key_M,
    Qt::Key_Up,
    Qt::Key_Down,
    Qt::Key_P,
    Qt::Key_O,
    Qt::Key_N,
    Qt::Key_R,
    Qt::Key_V,
    Qt::Key_X
};
bool VideoForm::isShortcut(QKeyEvent *event)
{
    if(event->modifiers() ==(Qt::ControlModifier | Qt::ShiftModifier))
    {
        return true;
    }
    if(event->modifiers() == Qt::CTRL)
    {
        for(int i = 0;i<sizeof(ShortcutKeys)/sizeof(Qt::Key);i++)
        {
            if(event->key() == ShortcutKeys[i])
                return true;
        }
        return false;
    }
    return false;
}

void VideoForm::resizeEvent(QResizeEvent *event)
{
    QSize s = event->size();
    QSize old = event->oldSize();
    QWidget::resizeEvent(event);
}

void VideoForm::changeEvent(QEvent *event)
{
    //if(event->type() == QEvent::ActivationChange)
    //if(event->type() == QEvent::WindowStateChange)
    //{
        //bool l = this->isActiveWindow();
    //    if(this->isActiveWindow())
    //    {
    //        showToolsForm(true);
    //        m_toolsForm.showNormal();
    //    }
    //}
    if(this->windowState() == Qt::WindowMaximized)
    {
        //int i = 0;
    }
    QWidget::changeEvent(event);
//    if(event->type() != QEvent::WindowStateChange)
//    {
//        event->accept();

//    }
//    else if(event->type() == )
//    if(this->windowState() == Qt::WindowMaximized)
//    {
//        QScreen *screen = QGuiApplication::primaryScreen();
//        QSize screenSize = screen->availableGeometry().size();
//        int w,h,x,y;
//        float whratio = 1.0f * m_frameSize.width()/m_frameSize.height();
//        if(whratio < 1.0f)//垂直
//        {
//            h = screenSize.height();
//            w = screenSize.height()*whratio;
//            x = (screenSize.width()-w)/2;
//            y = 0;
//            this->p_video->resize(w,h);
//            this->p_video->move(x,y);
//        }
//        else
//        {
//            w = screenSize.width();
//            h = screenSize.width()*whratio;
//            y = (screenSize.height()-h)/2;
//            x = 0;
//            this->p_video->resize(w,h);
//            this->p_video->move(x,y);
//        }
//    }
//    else if(this->windowState() == Qt::WindowMinimized)
//    {
//        m_toolsForm.showNormal();
//    }
    //    else{}
}

void VideoForm::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget,&opt,&painter,this);
}

