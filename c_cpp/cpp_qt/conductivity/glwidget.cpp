#include <GLC_UserInput>

#include <QtDebug>
#include <QFile>

#include "glwidget.h"


// Для решения проблемы VSYNC под Mac OS X
#if defined(Q_OS_MAC)
#include <OpenGL.h>
#endif


GLWidget::GLWidget(QWidget *p_parent)
    : QGLWidget(p_parent)
    , m_Light()
    , m_GlView(this)
    , m_MoverController()
{
	/// ---- Определения GLC ----
	m_Light.setPosition(1.0, 1.0, 1.0);

	QColor repColor;
	repColor.setRgbF(1.0, 0.11372, 0.11372, 1.0);
	m_MoverController= GLC_Factory::instance()->createDefaultMoverController(repColor, &m_GlView);

	m_GlView.cameraHandle()->setDefaultUpVector(glc::Z_AXIS);
	m_GlView.cameraHandle()->setTopView();

    /// Создание объектов для отображения
    CreateScene();
	/// ---- Конец определений GLC ----
}


GLWidget::~GLWidget() {}


/// Установить перспективу "Вид спереди";
void GLWidget::setFrontPerspective()
{
	m_GlView.cameraHandle()->setEyeCam(eye_start);
	m_GlView.cameraHandle()->setTargetCam(GLC_Point3d(0.0, 0.0, 0.0));
	m_GlView.cameraHandle()->setTopView();
	updateGL();
}


void GLWidget::initializeGL()
{
	// Для решения проблемы VSYNC под Mac OS X
	#if defined(Q_OS_MAC)
	const GLint swapInterval = 1;
	CGLSetParameter(CGLGetCurrentContext(), kCGLCPSwapInterval, &swapInterval);
	#endif

    /// Инициализация OpenGL
    m_GlView.initGl();

	/// Для создания текстуры должен присутствовать контекст рендеринга.
	m_GlView.reframe(m_World.boundingBox());

	eye_start = m_GlView.cameraHandle()->eye();
}


void GLWidget::paintGL()
{
    /// Очистка экрана
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /// Загрузка матрицы идентичности
    glLoadIdentity();

	/// ---- Определения GLC ----
    /// Вычисление глубины камеры представления.
    m_GlView.setDistMinAndMax(m_World.boundingBox());

    /// Определение света.
    m_Light.enable();
	m_Light.glExecute();

	m_GlView.glExecuteCam();                /// Определение матрицы представления.
	m_World.render(0, glc::ShadingFlag);    /// Отображение коллекции GLC_Object
	m_MoverController.drawActiveMoverRep(); /// Отображение информации UI (орбитальный круг)
	/// ---- Конец определений GLC ----
}


void GLWidget::resizeGL(int width, int height)
{
	m_GlView.setWinGLSize(width, height);	/// Вычисление пропорций окна
}


/// Создание объекта GLC_Object для отображения
void GLWidget::CreateScene()
{
	m_GlView.loadBackGroundImage(":resources/pluto.png"); /// Загрузка фонового изображения.
    /// Загрузка фигуры с ее материалами
	QFileInfo  matr_inf(":resources/cube.mtl");
	QFile  fileEx07(":resources/cube.obj");
    QStringList lst(matr_inf.absoluteFilePath());
	m_World= GLC_Factory::instance()->createWorldFromFile(fileEx07, &lst);
}


void GLWidget::mousePressEvent(QMouseEvent *e)
{
	if (m_MoverController.hasActiveMover()) return;
	switch (e->button())
	{
	case (Qt::RightButton):
		m_MoverController.setActiveMover(GLC_MoverController::TrackBall, GLC_UserInput(e->x(), e->y()));
		updateGL();
		break;
	case (Qt::LeftButton):
		m_MoverController.setActiveMover(GLC_MoverController::Pan, GLC_UserInput(e->x(), e->y()));
		updateGL();
		break;
	case (Qt::MidButton):
		m_MoverController.setActiveMover(GLC_MoverController::Zoom, GLC_UserInput(e->x(), e->y()));
		updateGL();
		break;
	default:
		break;
	}
}


void GLWidget::mouseMoveEvent(QMouseEvent * e)
{
    if (!m_MoverController.hasActiveMover())
    {
        return;
    }
	m_MoverController.move(GLC_UserInput(e->x(), e->y()));
	m_GlView.setDistMinAndMax(m_World.boundingBox());
	updateGL();
}


void GLWidget::mouseReleaseEvent(QMouseEvent*)
{
    if (!m_MoverController.hasActiveMover())
    {
        return;
    }
	m_MoverController.setNoMover();
	updateGL();
}
