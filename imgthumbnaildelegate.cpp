#include "imgthumbnaildelegate.h"
#include "imglistview.h"

ImgThumbnailDelegate::ImgThumbnailDelegate(QObject* parent)
	: QItemDelegate(parent), canDraw(true){

	flags = Qt::AlignHCenter | Qt::AlignBottom;
}



void ImgThumbnailDelegate::drawDisplay(QPainter *painter, const QStyleOptionViewItem &option,
                 const QRect &rect, const QString &text) const{
    QString shortText = text.split('/').last();
    shortText.truncate(shortText.indexOf('.'));
    //shortText.indexOf('.');
    QItemDelegate::drawDisplay(painter, option, rect, shortText);
}
