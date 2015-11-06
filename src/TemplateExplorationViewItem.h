//
//  TemplateExplorationViewItem.h
//
// Copyright (c) 2013-2014 Melinos Averkiou <m.averkiou@cs.ucl.ac.uk>

#ifndef TEMPLATE_EXPLORATION_VIEW_ITEM_H
#define TEMPLATE_EXPLORATION_VIEW_ITEM_H

#include <QtGui/QColor>
#include <QtGui/QGraphicsItem>
#include <QGraphicsView>
#include <QObject>
#include "MatchT.h"

class TemplateExplorationViewItem : public QGraphicsItem
{
    
public:
    typedef MatchT<TriangleMesh> Match;
    
    TemplateExplorationViewItem(Match* _match, int _index);
    
    QRectF boundingRect() const;
    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget);
  
    void setRepresentative( bool _isRepresentative)
    {
        isRepresentative_ = _isRepresentative;
        this->setZValue(1000000);
    }
    
    int index()
    {
        return index_;
    }
    
    void colorOverride(const QColor& _color)
    {
        colorOverride_ = _color;
    }
    
    Match* match()
    {
        return match_;
    }
    
protected:
    
    void hoverEnterEvent ( QGraphicsSceneHoverEvent * event );
    void hoverLeaveEvent ( QGraphicsSceneHoverEvent * event );

private:
    
    Match* match_;
    float scalex=1, scaley=1;
    bool isHovered = false;
    bool isRepresentative_ = false;
    int index_ = -std::numeric_limits<int>::max();
    QColor colorOverride_;
    QPixmap icon_;
    bool pixmapLoaded_ = false;
};

#endif
