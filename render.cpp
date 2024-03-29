#include "render.h"
#include <QPainter>
#include <QImage>
#include <QLabel>
#include <QTimer>
#include "MeshObject.h"
#include "context.h"
#include "Util.h"
#include "QDateTime"

struct Render::Impl
{
	clock_t		pre = 0;
	QLabel* lable;
	QTimer timer;
};

Render::Render(QWidget* parent) : QWidget(parent), impl(new Impl)
{
	impl->lable = new QLabel(this);
	impl->lable->setPalette(QPalette(Qt::red));
	impl->lable->setGeometry(20, 20, 60, 30);
	impl->lable->setStyleSheet("color:red;");

	connect(&impl->timer, SIGNAL(timeout()), this, SLOT(update()));
	impl->timer.start(10);

	QPixmap pixmap(QString::fromStdString(g::getResDir()) + "timg.jpg");
	g::image = pixmap.toImage();
}

Render::~Render()
{
	delete impl;
}

void Render::paintEvent(QPaintEvent*)
{
	//--------------------prepare-----------------------------
	float t = clock() - impl->pre;
	impl->lable->setText(QString::number(1000.0 / t, 'f', 2) + " FPS");
	impl->pre = clock();

	QRgb* pixels = new QRgb[width() * height()];
	for (int x = 0; x < width(); ++x)
	{
		for (int y = 0; y < height(); ++y)
		{
			g::context.colorBuffer_[y][x] = g::context.clearColor_;
			g::context.depthBuffer_[y][x] = FLT_MAX;
		}
	}

	//--------------------draw-----------------------------
	for (MeshObject* mo : MeshObject::pool())
		mo->draw();

	//-------------------------------------------------
	QPainter painter(this);
	QImage   image(( uchar* )pixels, width(), height(), QImage::Format_ARGB32);
	for (int x = 0; x < width(); ++x)
	{
		for (int y = 0; y < height(); ++y)
		{
			Vec3 c					 = g::context.colorBuffer_[y][x] * 255;
			pixels[x * height() + y] = c.to_color();
		}
	}
	painter.drawImage(0, 0, image);

	delete[] pixels;
}

void Render::resizeEvent(QResizeEvent* event)
{
	g::context.width_ = event->size().width(), g::context.height_ = event->size().height();
	const float width = g::context.width_, height = g::context.height_;

	g::context.projectionMatrix_.makePerspective(90, width / height, .1, 10000);
	//Y向上
	g::context.viewMatrix_.lookAt(Vec3(), Vec3(0, 0, -1), Vec3(0, 1, 0));

	Vec3Array c(width, Vec3());
	g::context.colorBuffer_ = vector<Vec3Array>((int)height, c);

	FloatArray f(width, FLT_MAX);
	g::context.depthBuffer_ = vector<FloatArray>((int)height, f);
}

void Render::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Space)
	{
		g::rotation_axis++;

		for (MeshObject* mo : MeshObject::pool())
		{
			mo->matrix_ = mo->orgin_matrix_;
		}			
	}
	else if (event->key() == Qt::Key_S)
	{
		g::enable_rotation = !g::enable_rotation;
	}
	else if (event->key() == Qt::Key_A)
	{
		for (MeshObject* mo : MeshObject::pool())
		{
			mo->matrix_ = mo->matrix_ * Matrix::translate(0, 0, 1);
		}
	}
	else if (event->key() == Qt::Key_D)
	{
		for (MeshObject* mo : MeshObject::pool())
		{
			mo->matrix_ = mo->matrix_ * Matrix::translate(0, 0, -1);
		}
	}
}
