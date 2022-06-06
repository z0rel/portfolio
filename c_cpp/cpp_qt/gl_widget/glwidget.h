#ifndef GLWIDGET_H_
#define GLWIDGET_H_

#include <global.h>

#include <QGLWidget>

/// @defgroup gls_incl Определения GLC
/// @{
/// Фабрика
#include <GLC_Factory>
/// Освещение
#include <GLC_Light>
/// Окно просмотра с камерой по умолчанию
#include <GLC_Viewport>
/// Мир, которым управляет GLC_object
#include <GLC_World>
/// Контроллер перемещения используемый для изменения точки обзора
#include <GLC_MoverController>
/// @}

#include <GLC_Axis>

class GLWidget : public QGLWidget
{
public:
	GLWidget(QWidget *p_parent);
	~GLWidget();

	// Установить перспективу "Вид спереди";
	void setFrontPerspective();
private:
	void initializeGL();
	void paintGL();
	void resizeGL(int width, int height);
	// Создание объекта GLC_Object для отображения
	void CreateScene();

	/// События мыши
	void mousePressEvent(QMouseEvent * e);
	void mouseMoveEvent(QMouseEvent * e);
	void mouseReleaseEvent(QMouseEvent * e);
private:
	/// @defgroup glc_obj Объекты GLC
	/// @{
	GLC_Light m_Light;
	GLC_World m_World;
	GLC_Viewport m_GlView;
	GLC_MoverController m_MoverController;
	GLC_Point3d eye_start;


	/// @}
};
#endif // GLWIDGET_H_
