#include "iconfont.h"

IconFont *IconFont::_instance = 0;
IconFont::IconFont()
{
    int fontId = QFontDatabase::addApplicationFont(":/res/font/fontawesome-webfont.ttf");
    QString fontName = QFontDatabase::applicationFontFamilies(fontId).at(0);
    m_font = QFont(fontName);
}

IconFont *IconFont::instance()
{
    static QMutex mutex;
    if (!_instance) {
        QMutexLocker locker(&mutex);
        if (!_instance) {
            _instance = new IconFont();
        }
    }
    return _instance;
}

void IconFont::setFont(QLabel *label, QChar c, int fontSize)
{
    m_font.setPointSize(fontSize);
    label->setFont(m_font);
    label->setText(c);
}

void IconFont::setFont(QPushButton *button, QChar c, int fontSize)
{
    m_font.setPointSize(fontSize);
    button->setFont(m_font);
    button->setText(c);
}
