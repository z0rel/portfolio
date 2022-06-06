#ifndef GLBRUS_H
#define GLBRUS_H

#include <QGLWidget>

//////////////////////////// GLC specific///////////////////////////////////////
// The factory
#include <GLC_Factory>
// Light
#include <GLC_Light>
// The Viewport with a default camera
#include <GLC_Viewport>
// The world which manage product structure
#include <GLC_World>
// The Mover controller is used to change the point of view
#include <GLC_MoverController>
//////////////////////////End GLC specific/////////////////////////////////////

class GlBrus : public QGLWidget
{
public:
	GlBrus(QWidget *p_parent);
	~GlBrus();

private:
	void initializeGL();
	void paintGL();
	void resizeGL(int width, int height);
	// Create GLC_Object to display
	void CreateScene();

	//Mouse events
	void mousePressEvent(QMouseEvent * e);
	void mouseMoveEvent(QMouseEvent * e);
	void mouseReleaseEvent(QMouseEvent * e);
private:
//////////////////////////// GLC specific///////////////////////////////////////
	GLC_Light m_Light;
	QColor m_DefaultColor;
	GLC_World m_World;
	GLC_Viewport m_GlView;
	GLC_MoverController m_MoverController;
//////////////////////////End GLC specific/////////////////////////////////////
};

#endif // GLBRUS_H
