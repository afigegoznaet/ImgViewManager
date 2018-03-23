#include "imgthumbnaildelegate.h"
#include "imglistview.h"

ImgThumbnailDelegate::ImgThumbnailDelegate(QObject* parent)
	: QItemDelegate(parent), canDraw(true){

	flags = Qt::AlignHCenter | Qt::AlignBottom;
}


void ImgThumbnailDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
							  const QModelIndex &index) const{

	QStyleOptionViewItem newOpt(option);
	newOpt.text = option.text.split('/').last();
    QItemDelegate::paint(painter, newOpt, index);
}

