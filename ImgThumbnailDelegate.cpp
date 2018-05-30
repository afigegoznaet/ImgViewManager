#include "ImgThumbnailDelegate.hpp"
#include "ImgListView.hpp"

ImgThumbnailDelegate::ImgThumbnailDelegate(QObject* parent) : QItemDelegate(parent),
	canDraw(true), selectionBrush(QColor(204,232,255)), hoverBrush(QColor(229,243,255)){

	flags = Qt::AlignHCenter | Qt::AlignBottom;
}



void ImgThumbnailDelegate::drawDisplay(QPainter *painter, const QStyleOptionViewItem &option,
				 const QRect &rect, const QString &text) const{
	QString shortText = text.split('/').last();
	shortText.truncate(shortText.indexOf('.'));
	QRect newRect(rect);
	if(newRect.height() < painter->fontMetrics().height() )
		newRect.setTop(rect.bottom() -painter->fontMetrics().height());

	QItemDelegate::drawDisplay(painter, option, newRect, shortText);
}

void ImgThumbnailDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
							  const QModelIndex &index) const{

	QStyleOptionViewItem modOption(option);

	if(index.isValid()){
		if(option.state & QStyle::State_MouseOver){
			painter->setRenderHint(QPainter::HighQualityAntialiasing);
			painter->fillRect(option.rect, hoverBrush);
		}
		if(option.state & QStyle::State_Selected){
			painter->setRenderHint(QPainter::HighQualityAntialiasing);
			painter->fillRect(option.rect, selectionBrush);
			modOption.palette.setColor(QPalette::HighlightedText, option.palette.text().color());
			modOption.palette.setColor(QPalette::Highlight, selectionBrush.color());
		}
	}

	QItemDelegate::paint(painter, modOption,index);
}

