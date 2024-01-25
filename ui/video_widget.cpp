#include <QCoreApplication>
#include <QOpenGLTexture>
#include <QSurfaceFormat>

#include "video_widget.h"
static const GLfloat coordinate[] = {
    -1.0f,
    -1.0f,
    0.0f,
    1.0f,
    -1.0f,
    0.0f,
    -1.0f,
    1.0f,
    0.0f,
    1.0f,
    1.0f,
    0.0f,
    0.0f,
    1.0f,
    1.0f,
    1.0f,
    0.0f,
    0.0f,
    1.0f,
    0.0f
};

static const QString vertShader = R"(
    attribute vec3 vertexIn;    // xyz顶点坐标
    attribute vec2 textureIn;   // xy纹理坐标
    varying vec2 textureOut;    // 传递给片段着色器的纹理坐标
    void main(void)
    {
        gl_Position = vec4(vertexIn, 1.0);  // 1.0表示vertexIn是一个顶点位置
        textureOut = textureIn; // 纹理坐标直接传递给片段着色器
    }
)";

// 片段着色器
static QString fragShader = R"(
    varying vec2 textureOut;        // 由顶点着色器传递过来的纹理坐标
    uniform sampler2D textureY;     // uniform 纹理单元，利用纹理单元可以使用多个纹理
    uniform sampler2D textureU;     // sampler2D是2D采样器
    uniform sampler2D textureV;     // 声明yuv三个纹理单元
    void main(void)
    {
        vec3 yuv;
        vec3 rgb;

        // SDL2 BT709_SHADER_CONSTANTS
        // https://github.com/spurious/SDL-mirror/blob/4ddd4c445aa059bb127e101b74a8c5b59257fbe2/src/render/opengl/SDL_shaders_gl.c#L102
        const vec3 Rcoeff = vec3(1.1644,  0.000,  1.7927);
        const vec3 Gcoeff = vec3(1.1644, -0.2132, -0.5329);
        const vec3 Bcoeff = vec3(1.1644,  2.1124,  0.000);

        // 根据指定的纹理textureY和坐标textureOut来采样
        yuv.x = texture2D(textureY, textureOut).r;
        yuv.y = texture2D(textureU, textureOut).r - 0.5;
        yuv.z = texture2D(textureV, textureOut).r - 0.5;

        // 采样完转为rgb
        // 减少一些亮度
        yuv.x = yuv.x - 0.0625;
        rgb.r = dot(yuv, Rcoeff);
        rgb.g = dot(yuv, Gcoeff);
        rgb.b = dot(yuv, Bcoeff);
        // 输出颜色值
        gl_FragColor = vec4(rgb, 1.0);
    }
)";

VideoWidget::VideoWidget(QWidget *parent) : QOpenGLWidget(parent)
{
    frameSize = {-1,-1};
    needUpdate = false;
    textureInited = false;
}

VideoWidget::~VideoWidget()
{
    makeCurrent();
    vOpenGLBuffer.destroy();
    deInitTextures();
    doneCurrent();
}

QSize VideoWidget::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize VideoWidget::sizeHint() const
{
    return size();
}

void VideoWidget::setFrameSize(const QSize &size)
{
    if (this->frameSize != size) {
        this->frameSize = size;
        needUpdate = true;
        // inittexture immediately
        repaint();
    }
}

const QSize &VideoWidget::getFrameSize()
{
    return frameSize;
}

void VideoWidget::updateTextures(quint8 *dataY, quint8 *dataU, quint8 *dataV, quint32 linesizeY, quint32 linesizeU, quint32 linesizeV)
{
    if (textureInited) {
        updateTexture(texture[0], 0, dataY, linesizeY);
        updateTexture(texture[1], 1, dataU, linesizeU);
        updateTexture(texture[2], 2, dataV, linesizeV);
        update();
    }
}

void VideoWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glDisable(GL_DEPTH_TEST);

    // 顶点缓冲对象初始化
    vOpenGLBuffer.create();
    vOpenGLBuffer.bind();
    vOpenGLBuffer.allocate(coordinate, sizeof(coordinate));
    initShader();
    // 设置背景清理色为黑色
    glClearColor(0.0, 0.0, 0.0, 0.0);
    // 清理颜色背景
    glClear(GL_COLOR_BUFFER_BIT);

}

void VideoWidget::paintGL()
{
    if (needUpdate) {
        deInitTextures();
        initTextures();
        needUpdate = false;
    }

    if (textureInited) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture[0]);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture[1]);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, texture[2]);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
}

void VideoWidget::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);
    repaint();
}

void VideoWidget::initShader()
{
    // opengles的float、int等要手动指定精度
    if (QCoreApplication::testAttribute(Qt::AA_UseOpenGLES)) {
        fragShader.prepend(R"(
                             precision mediump int;
                             precision mediump float;
                             )");
    }
    shaderProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, vertShader);
    shaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, fragShader);
    shaderProgram.link();
    shaderProgram.bind();

    // 指定顶点坐标在vbo中的访问方式
    // 参数解释：顶点坐标在shader中的参数名称，顶点坐标为float，起始偏移为0，顶点坐标类型为vec3，步幅为3个float
    shaderProgram.setAttributeBuffer("vertexIn", GL_FLOAT, 0, 3, 3 * sizeof(float));
    // 启用顶点属性
    shaderProgram.enableAttributeArray("vertexIn");

    // 指定纹理坐标在vbo中的访问方式
    // 参数解释：纹理坐标在shader中的参数名称，纹理坐标为float，起始偏移为12个float（跳过前面存储的12个顶点坐标），纹理坐标类型为vec2，步幅为2个float
    shaderProgram.setAttributeBuffer("textureIn", GL_FLOAT, 12 * sizeof(float), 2, 2 * sizeof(float));
    shaderProgram.enableAttributeArray("textureIn");

    // 关联片段着色器中的纹理单元和opengl中的纹理单元（opengl一般提供16个纹理单元）
    shaderProgram.setUniformValue("textureY", 0);
    shaderProgram.setUniformValue("textureU", 1);
    shaderProgram.setUniformValue("textureV", 2);
}

void VideoWidget::initTextures()
{
    // 创建纹理
    glGenTextures(1, &texture[0]);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    // 设置纹理缩放时的策略
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // 设置st方向上纹理超出坐标时的显示策略
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, frameSize.width(), frameSize.height(), 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, nullptr);

    glGenTextures(1, &texture[1]);
    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, frameSize.width() / 2, frameSize.height() / 2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, nullptr);

    glGenTextures(1, &texture[2]);
    glBindTexture(GL_TEXTURE_2D, texture[2]);
    // 设置纹理缩放时的策略
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // 设置st方向上纹理超出坐标时的显示策略
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, frameSize.width() / 2, frameSize.height() / 2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, nullptr);

    textureInited = true;
}

void VideoWidget::deInitTextures()
{
    if (QOpenGLFunctions::isInitialized(QOpenGLFunctions::d_ptr)) {
        glDeleteTextures(3, texture);
    }

    memset(texture, 0, sizeof(texture));
    textureInited = false;
}

void VideoWidget::updateTexture(GLuint texture, quint32 textureType, quint8 *pixels, quint32 stride)
{
    if (!pixels)
        return;

    QSize size = 0 == textureType ? frameSize : frameSize / 2;

    makeCurrent();
    glBindTexture(GL_TEXTURE_2D, texture);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, static_cast<GLint>(stride));
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size.width(), size.height(), GL_LUMINANCE, GL_UNSIGNED_BYTE, pixels);
    doneCurrent();
}
