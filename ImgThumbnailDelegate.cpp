#include "ImgThumbnailDelegate.hpp"
#include "ImgListView.hpp"

ImgThumbnailDelegate::ImgThumbnailDelegate(QObject* parent)
	: QItemDelegate(parent), canDraw(true){

	flags = Qt::AlignHCenter | Qt::AlignBottom;
}



void ImgThumbnailDelegate::drawDisplay(QPainter *painter, const QStyleOptionViewItem &option,
				 const QRect &rect, const QString &text) const{
	QString shortText = text.split('/').last();
	shortText.truncate(shortText.indexOf('.'));
	//shortText.indexOf('.');
//	qDebug()<<painter->font();
//	qDebug()<<painter->fontInfo().pixelSize();
//	qDebug()<<painter->fontMetrics().height();
//	qDebug()<<rect;
//	qDebug()<<rect.width();
//	qDebug()<<rect.height();
	QRect newRect(rect);
	if(newRect.height() < painter->fontMetrics().height() )
		newRect.setTop(rect.bottom() -painter->fontMetrics().height());
//	qDebug()<<newRect;
//	qDebug()<<newRect.width();
//	qDebug()<<newRect.height();
	QItemDelegate::drawDisplay(painter, option, newRect, shortText);
}
