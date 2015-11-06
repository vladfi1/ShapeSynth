//
//  TemplateExplorationViewItem.cpp
//
// Copyright (c) 2013-2014 Melinos Averkiou <m.averkiou@cs.ucl.ac.uk>

#include "TemplateExplorationViewItem.h"

#include <QtGui>

TemplateExplorationViewItem::TemplateExplorationViewItem(Match* _match, int _index)
{
    match_ = _match;
    index_ = _index;
    
    colorOverride_.setRgb(0, 0, 0);
    
    if (index_==-1)
    {
        this->setZValue(1000000);
    }
    
    setAcceptsHoverEvents(true);
}

QRectF TemplateExplorationViewItem::boundingRect() const
{
    float scx = 1.0 / scene()->views()[0]->transform().m11();
    
    
    float scy = 1.0 / scene()->views()[0]->transform().m22();

    
    if (isRepresentative_ || index_==-1)
    {
        return QRectF(-6*scx,-6*scy,12*scx,12*scy);
    }
    else
    {
        if (isHovered)
        {
            return QRectF(-6*scx,-6*scy,12*scx,12*scy);
        }
        else
        {
            return QRectF(-3*scx,-3*scy,6*scx,6*scy);
        }
    }
}

QPainterPath TemplateExplorationViewItem::shape() const
{
    QPainterPath path;
    
    if (isRepresentative_ || index_ == -1)
    {
        path.addRect((1.0/scalex) *-6, (1.0/scaley) *-6, (1.0/scalex) *12, (1.0/scaley) *12);
    }
    else
    {
        if (isHovered)
        {
            path.addEllipse(QPointF(0.0, 0.0), (1.0/scalex) *6.0 , (1.0/scaley) *6.0 );
        }
        else
        {
            path.addEllipse(QPointF(0.0, 0.0),(1.0/scalex) * 3.0 , (1.0/scaley) *3.0 );
        }
    }
    //path.addEllipse(0, 0, 6, 6);

    return path;
}

void TemplateExplorationViewItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    
    int label = match_->label();
    
    if (label<0 || label > 99)
    {
        label = 0;
    }
    
    QColor fillColor;
    
    if (colorOverride_.red()!=0 && colorOverride_.green()!=0 && colorOverride_.blue()!=0)
    {
        // override the color, for showing representatives in the case we are only showing one cluser
        fillColor = colorOverride_;
    }
    else if (index_ == -1)
    {
        // Red color, for showing the coordinates of the template
        fillColor = QColor(255,0,0);
    }
    else
    {
        // Otherwise color based on the label of the match
        fillColor = tewColors[label];
    }
    
    if (option->state & QStyle::State_Selected)
        fillColor = fillColor.dark(150);
        
    if (option->state & QStyle::State_MouseOver)
        fillColor = fillColor.light(125);
    
    painter->save();
    
    QBrush b = painter->brush();
    
    QTransform tr = painter->worldTransform();
    
    scalex = tr.m11();
    scaley = tr.m22();
    
    painter->scale(1.0 / scalex, 1.0 / scaley);
    painter->setBrush(fillColor);

    if (index_ == -1)
    {
        painter->setBrush(QColor(255, 0, 0));
        painter->drawRect(-6, -6, 12, 12);
    }
    else if(index_ < -1)
    {
        painter->drawRect(-6, -6, 12, 12);
    }
    else if(isRepresentative_)
    {
        painter->drawRect(-6, -6, 12, 12);
    }
    else
    {
        if (option->state & QStyle::State_MouseOver)
        {
            painter->drawEllipse(QPointF(0.0, 0.0), 6.0 , 6.0 );
        }
        else
        {
            painter->drawEllipse(QPointF(0.0, 0.0), 3.0 , 3.0 );
        }
    }
    
    // For drawing the icon of the match when hovering over it
    if (option->state & QStyle::State_MouseOver && index_!=-1)
    {
        // Only load the icon once (could load it in the constructor for all the items, but could possibly take too long, so load it when needed and save it)
        if (!pixmapLoaded_)
        {
            QStringList path = match_->filename().split("/");
            QString name = path.last();
            path.removeLast();
            path.removeLast();
            path.push_back("img_models");
            path.push_back(name.split(".").first().append(".jpg"));
            
            icon_ = QPixmap(path.join("/"));
            icon_ = icon_.scaledToHeight(EXPLORATION_VIEW_ICON_HEIGHT);
            pixmapLoaded_ = true;
        }
            painter->drawPixmap(QPointF(0,0),icon_);
    }
    
    // Restore painter settings
    painter->setBrush(b);
    
    painter->restore();
}


void TemplateExplorationViewItem::hoverEnterEvent ( QGraphicsSceneHoverEvent * event )
{
    this->setZValue(1000000000);
    isHovered = true;
    update();
}

void TemplateExplorationViewItem::hoverLeaveEvent ( QGraphicsSceneHoverEvent * event )
{
    if (!isRepresentative_)
    {
        this->setZValue(0);
    }
    isHovered = false;
    update();
}
