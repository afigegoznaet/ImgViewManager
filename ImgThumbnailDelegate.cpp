#include "ImgThumbnailDelegate.hpp"
#include "ImgListView.hpp"
#include "MainWindow.hpp"
ImgThumbnailDelegate::ImgThumbnailDelegate(QObject* parent) : QItemDelegate(parent),
	canDraw(true), selectionBrush(QColor(204,232,255)), hoverBrush(QColor(229,243,255)){

	flags = Qt::AlignHCenter | Qt::AlignBottom;
	auto parentWindow = qobject_cast<MainWindow*>(parent);
	auto parentObject = parent;
	while(nullptr == parentWindow){
		parentWindow = qobject_cast<MainWindow*>(parentObject);
		parentObject = parentObject->parent();
	}
	previewLabel = new QLabel(parentWindow);
	previewLabel->setStyleSheet("background:transparent; border: solid 10px grey;  background-color: rgba(255, 0, 0,127);");
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
			paintPreview();
			previewLabel->setVisible(true);
		}
		if(option.state & QStyle::State_Selected){
			painter->setRenderHint(QPainter::HighQualityAntialiasing);
			painter->fillRect(option.rect, selectionBrush);
			modOption.palette.setColor(QPalette::HighlightedText, option.palette.text().color());
			modOption.palette.setColor(QPalette::Highlight, selectionBrush.color());
			previewLabel->setVisible(false);
		}
	}

	QItemDelegate::paint(painter, modOption,index);
}

void ImgThumbnailDelegate::paintPreview() const{

	adjustSize();
	QPixmap pix(500,500);
	QPainter painter(&pix);


	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
	//painter.drawPixmap(hDelta, vDelta, img.width(), img.height(), QPixmap::fromImage(img));
	painter.save();
	painter.restore();

	painter.save();
	painter.restore();
	previewLabel->setPixmap(pix);
}

void ImgThumbnailDelegate::adjustSize() const{
	auto parentWindow = qobject_cast<MainWindow*>(parent());
	auto parentObject = parent();
	while(nullptr == parentWindow){
		parentWindow = qobject_cast<MainWindow*>(parentObject);
		parentObject = parentObject->parent();
	}

	auto tmpSize = parentWindow->getTreeWidgetSize();
	auto tmpPos = parentWindow->getTreeWidgetPos();
	if(imgSize != tmpSize){
		imgSize = tmpSize;
		previewLabel->setFixedWidth(imgSize.width());
		previewLabel->setFixedHeight(imgSize.height());
	}
	//previewLabel->move(previewLabel->mapToParent(QPoint(0,0)));
	if( imgPos != tmpPos){
		imgPos = tmpPos;
		previewLabel->move( imgPos);
	}
}
