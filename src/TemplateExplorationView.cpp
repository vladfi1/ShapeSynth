/****************************************************************************
 **
 ** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
 ** Contact: http://www.qt-project.org/legal
 **
 ** This file is part of the demonstration applications of the Qt Toolkit.
 **
 ** $QT_BEGIN_LICENSE:LGPL$
 ** Commercial License Usage
 ** Licensees holding valid commercial Qt licenses may use this file in
 ** accordance with the commercial license agreement provided with the
 ** Software or, alternatively, in accordance with the terms contained in
 ** a written agreement between you and Digia.  For licensing terms and
 ** conditions see http://qt.digia.com/licensing.  For further information
 ** use the contact form at http://qt.digia.com/contact-us.
 **
 ** GNU Lesser General Public License Usage
 ** Alternatively, this file may be used under the terms of the GNU Lesser
 ** General Public License version 2.1 as published by the Free Software
 ** Foundation and appearing in the file LICENSE.LGPL included in the
 ** packaging of this file.  Please review the following information to
 ** ensure the GNU Lesser General Public License version 2.1 requirements
 ** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 **
 ** In addition, as a special exception, Digia gives you certain additional
 ** rights.  These rights are described in the Digia Qt LGPL Exception
 ** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
 **
 ** GNU General Public License Usage
 ** Alternatively, this file may be used under the terms of the GNU
 ** General Public License version 3.0 as published by the Free Software
 ** Foundation and appearing in the file LICENSE.GPL included in the
 ** packaging of this file.  Please review the following information to
 ** ensure the GNU General Public License version 3.0 requirements will be
 ** met: http://www.gnu.org/copyleft/gpl.html.
 **
 **
 ** $QT_END_LICENSE$
 **
 ****************************************************************************/


// Modified by Melinos Averkiou

// Copyright (c) 2013-2014 Melinos Averkiou <m.averkiou@cs.ucl.ac.uk>

#include "TemplateExplorationView.h"

#include <QtGui>
#ifndef QT_NO_OPENGL
#include <QtOpenGL>
#endif

#include <qmath.h>

void GraphicsView::saveGeneralSettings(QTextStream& _out)
{
    _out << "Clicked: \n";
    
    for (int i=0; i<clickedPoints_.size(); ++i)
    {
        _out << clickedPoints_[i].x() << " , " << clickedPoints_[i].y() << "\n";
    }
    
    _out << "Hovered: \n";
    
    for (int i=0; i<hoveredPoints_.size(); ++i)
    {
        _out << hoveredPoints_[i].x() << " , " << hoveredPoints_[i].y() << "\n";
    }
}

void GraphicsView::wheelEvent(QWheelEvent *e)
{
    if (e->modifiers() & Qt::ControlModifier) {
        if (e->delta() > 0)
            view->zoomIn(6);
        else
            view->zoomOut(6);
        e->accept();
       
        QGraphicsScene* sc = scene();

        if (sc)
        {
            QRectF scr = sc->sceneRect();
           
            QRectF scItemr = sc->itemsBoundingRect();
            
            sc->setSceneRect(scItemr);
        }
        
    }
    else
    {
        QGraphicsView::wheelEvent(e);
    }
    
}

void GraphicsView::resizeEvent ( QResizeEvent * event )
{


}

void GraphicsView::mousePressEvent(QMouseEvent *event)
{

    
}

void GraphicsView::mouseReleaseEvent ( QMouseEvent * event )
{
    if (dragMode() == QGraphicsView::ScrollHandDrag || event->button() != Qt::LeftButton)
    {
        QGraphicsView::mouseReleaseEvent(event);
        return;
    }
    
    QPoint mousePos = event->pos();
    QPointF mousePosScene = mapToScene(mousePos);
    
    mousePosScene.setY(-mousePosScene.y());
    
    TIMELOG->append(QString("%1 : clicked_red_point switching to synthesis").arg((qlonglong)QDateTime::currentMSecsSinceEpoch()));
    
    emit selectedPointChanged(mousePosScene.x(), mousePosScene.y());
    emit interactionModeChanged(QString("Interaction Mode: Synthesis"));
    
    clickedPoints_.push_back(mousePosScene);
}

void GraphicsView::mouseMoveEvent ( QMouseEvent * event )
{
    setFocus();
    
    if (hoverMode)
    {
        QPoint mousePos = event->pos();
        QPointF mousePosScene = mapToScene(mousePos);
        
        mousePosScene.setY(-mousePosScene.y());
        
        emit hoveredPointChanged(mousePosScene.x(), mousePosScene.y());
        
        hoveredPoints_.push_back(mousePosScene);
    }
    
    QGraphicsView::mouseMoveEvent(event);
    
}

void GraphicsView::keyReleaseEvent (QKeyEvent * event)
{
    if (event->key() == Qt::Key_H)
    {
        hoverMode = !hoverMode;
        
        if (hoverMode)
        {
            TIMELOG->append(QString("%1 : pressed_H switching to exploration").arg((qlonglong)QDateTime::currentMSecsSinceEpoch()));
            emit interactionModeChanged(QString("Interaction Mode: Exploration"));
            emit nIndependentPartsChanged(QString("No of independent parts: 0"));
            emit nnUsedChanged(QString("No of Neighbours used: 0"));
        }
        else
        {
            TIMELOG->append(QString("%1 : pressed_H exploration_done").arg((qlonglong)QDateTime::currentMSecsSinceEpoch()));
        }
    }
    QGraphicsView::keyReleaseEvent(event);
}

View::View(const QString &name, QWidget *parent)
: QFrame(parent)
{
    setFrameStyle(Sunken | StyledPanel);
    graphicsView = new GraphicsView(this);
    graphicsView->setRenderHint(QPainter::Antialiasing, true);
    //graphicsView->setDragMode(QGraphicsView::RubberBandDrag);
    graphicsView->setOptimizationFlags(QGraphicsView::DontSavePainterState);
    graphicsView->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    
    graphicsView->setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
    
    int size = style()->pixelMetric(QStyle::PM_ToolBarIconSize);
    QSize iconSize(size, size);
    
    QToolButton *zoomInIcon = new QToolButton;
    zoomInIcon->setAutoRepeat(true);
    zoomInIcon->setAutoRepeatInterval(33);
    zoomInIcon->setAutoRepeatDelay(0);
    zoomInIcon->setIcon(QPixmap(":/zoomin.png"));
    zoomInIcon->setIconSize(iconSize);
    QToolButton *zoomOutIcon = new QToolButton;
    zoomOutIcon->setAutoRepeat(true);
    zoomOutIcon->setAutoRepeatInterval(33);
    zoomOutIcon->setAutoRepeatDelay(0);
    zoomOutIcon->setIcon(QPixmap(":/zoomout.png"));
    zoomOutIcon->setIconSize(iconSize);
    zoomSlider = new QSlider;
    zoomSlider->setMinimum(0);
    zoomSlider->setMaximum(1000);
    zoomSlider->setValue(0);
    zoomSlider->setTickPosition(QSlider::TicksRight);
    
    // Zoom slider layout
    QVBoxLayout *zoomSliderLayout = new QVBoxLayout;
    //zoomSliderLayout->addWidget(zoomInIcon);
    zoomSliderLayout->addWidget(zoomSlider);
    //zoomSliderLayout->addWidget(zoomOutIcon);
    
    QToolButton *rotateLeftIcon = new QToolButton;
    rotateLeftIcon->setIcon(QPixmap(":/rotateleft.png"));
    rotateLeftIcon->setIconSize(iconSize);
    QToolButton *rotateRightIcon = new QToolButton;
    rotateRightIcon->setIcon(QPixmap(":/rotateright.png"));
    rotateRightIcon->setIconSize(iconSize);
    rotateSlider = new QSlider;
    rotateSlider->setOrientation(Qt::Horizontal);
    rotateSlider->setMinimum(-360);
    rotateSlider->setMaximum(360);
    rotateSlider->setValue(0);
    rotateSlider->setTickPosition(QSlider::TicksBelow);
    
    // Rotate slider layout
    QHBoxLayout *rotateSliderLayout = new QHBoxLayout;
    rotateSliderLayout->addWidget(rotateLeftIcon);
    rotateSliderLayout->addWidget(rotateSlider);
    rotateSliderLayout->addWidget(rotateRightIcon);
    
    resetButton = new QToolButton;
    resetButton->setText(tr("0"));
    resetButton->setEnabled(false);
    
    // Label layout
    QHBoxLayout *labelLayout = new QHBoxLayout;
    label = new QLabel(name);
    label2 = new QLabel(tr("Pointer Mode"));
    selectModeButton = new QToolButton;
    selectModeButton->setText(tr("Select"));
    selectModeButton->setCheckable(true);
    selectModeButton->setChecked(true);
    dragModeButton = new QToolButton;
    dragModeButton->setText(tr("Drag"));
    dragModeButton->setCheckable(true);
    dragModeButton->setChecked(false);
    antialiasButton = new QToolButton;
    antialiasButton->setText(tr("Antialiasing"));
    antialiasButton->setCheckable(true);
    antialiasButton->setChecked(false);
    openGlButton = new QToolButton;
    openGlButton->setText(tr("OpenGL"));
    openGlButton->setCheckable(true);
#ifndef QT_NO_OPENGL
    openGlButton->setEnabled(QGLFormat::hasOpenGL());
#else
    openGlButton->setEnabled(false);
#endif
    printButton = new QToolButton;
    printButton->setIcon(QIcon(QPixmap(":/fileprint.png")));
    
    QButtonGroup *pointerModeGroup = new QButtonGroup;
    pointerModeGroup->setExclusive(true);
    pointerModeGroup->addButton(selectModeButton);
    pointerModeGroup->addButton(dragModeButton);
    
    labelLayout->addWidget(label);
    labelLayout->addStretch();
    
    QGridLayout *topLayout = new QGridLayout;
    topLayout->addLayout(labelLayout, 0, 0);
    topLayout->addWidget(graphicsView, 1, 0);
    topLayout->addLayout(zoomSliderLayout, 1, 1);
    
    setLayout(topLayout);
    
    connect(resetButton, SIGNAL(clicked()), this, SLOT(resetView()));
    connect(zoomSlider, SIGNAL(valueChanged(int)), this, SLOT(setupMatrix()));
    connect(rotateSlider, SIGNAL(valueChanged(int)), this, SLOT(setupMatrix()));
    connect(graphicsView->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(setResetButtonEnabled()));
    connect(graphicsView->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(setResetButtonEnabled()));
    connect(selectModeButton, SIGNAL(toggled(bool)), this, SLOT(togglePointerMode()));
    connect(dragModeButton, SIGNAL(toggled(bool)), this, SLOT(togglePointerMode()));
    connect(antialiasButton, SIGNAL(toggled(bool)), this, SLOT(toggleAntialiasing()));
    connect(openGlButton, SIGNAL(toggled(bool)), this, SLOT(toggleOpenGL()));
    connect(rotateLeftIcon, SIGNAL(clicked()), this, SLOT(rotateLeft()));
    connect(rotateRightIcon, SIGNAL(clicked()), this, SLOT(rotateRight()));
    connect(zoomInIcon, SIGNAL(clicked()), this, SLOT(zoomIn()));
    connect(zoomOutIcon, SIGNAL(clicked()), this, SLOT(zoomOut()));
    connect(printButton, SIGNAL(clicked()), this, SLOT(print()));
    
    
    setupMatrix();
   
}

QGraphicsView *View::view() const
{
    return static_cast<QGraphicsView *>(graphicsView);
}

void View::resetView()
{
    zoomSlider->setValue(500);
    rotateSlider->setValue(0);
    setupMatrix();
    graphicsView->ensureVisible(QRectF(0, 0, 0, 0));
    
    resetButton->setEnabled(false);
}

void View::fitScene()
{
    QGraphicsScene* scene = graphicsView->scene();
    
    if (scene)
    {
        
        while (true)
        {
            QRectF sceneRect = scene->itemsBoundingRect();
            
            std::cout << "Scene rect  " << sceneRect.topLeft().x() << " " << sceneRect.topLeft().y() << " " << sceneRect.bottomRight().x() << " " << sceneRect.bottomRight().y() << std::endl;
            
            if (sceneRect.width()<=0 || sceneRect.height()<=0)
            {
                return;
            }
            
            scene->setSceneRect(sceneRect);
            
            float scalex = graphicsView->transform().m11();
            float scaley = graphicsView->transform().m22();
            //float scale = scalex > scaley ? scalex : scaley;
            
            int viewportw = graphicsView->viewport()->width();
            int viewporth = graphicsView->viewport()->height();
            
            std::cout << "Scale: " << scalex << " " << scaley << std::endl;
            
            if(abs(scalex * sceneRect.width() - viewportw) < 20 || abs(scaley * sceneRect.height() - viewporth) < 20)
            {
                float scale = scalex > scaley ? scalex: scaley;
                
                scaleRatio_ = scalex / scaley;
                
                int slider = 50.0 * (log(scale) / log(2.0)) + 0;
                
                zoomSlider->blockSignals(true);
                zoomSlider->setValue(slider);
                zoomSlider->blockSignals(false);
                break;
            }
            graphicsView->fitInView(sceneRect);
            
        }
        

    }
}

void View::setResetButtonEnabled()
{
    resetButton->setEnabled(true);
}

void View::setupMatrix()
{
    qreal scale = qPow(qreal(2), (zoomSlider->value() - 0) / qreal(50));
    
    QMatrix matrix;
    matrix.scale(scaleRatio_ *scale,  scale);
    matrix.rotate(rotateSlider->value());
    
    graphicsView->setMatrix(matrix);
    setResetButtonEnabled();
}

void View::togglePointerMode()
{
    graphicsView->setDragMode(selectModeButton->isChecked()
                              ? QGraphicsView::RubberBandDrag
                              : QGraphicsView::ScrollHandDrag);
    graphicsView->setInteractive(selectModeButton->isChecked());
}

void View::toggleOpenGL()
{
#ifndef QT_NO_OPENGL
    graphicsView->setViewport(openGlButton->isChecked() ? new QGLWidget(QGLFormat(QGL::SampleBuffers)) : new QWidget);
#endif
}

void View::toggleAntialiasing()
{
    graphicsView->setRenderHint(QPainter::Antialiasing, antialiasButton->isChecked());
}

void View::print()
{
#ifndef QT_NO_PRINTER
    QPrinter printer;
    QPrintDialog dialog(&printer, this);
    if (dialog.exec() == QDialog::Accepted) {
        QPainter painter(&printer);
        graphicsView->render(&painter);
    }
#endif
}

void View::zoomIn(int level)
{
    zoomSlider->setValue(zoomSlider->value() + level);
}

void View::zoomOut(int level)
{
    zoomSlider->setValue(zoomSlider->value() - level);
}

void View::rotateLeft()
{
    rotateSlider->setValue(rotateSlider->value() - 10);
}

void View::rotateRight()
{
    rotateSlider->setValue(rotateSlider->value() + 10);
}

void View::resizeEvent ( QResizeEvent * event )
{
    //fitScene();
}

