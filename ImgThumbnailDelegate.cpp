#include "ImgThumbnailDelegate.hpp"
#include "ImgListView.hpp"
#include "MainWindow.hpp"
#include <QPainter>
#include <QImageReader>
#include <QLabel>

ImgThumbnailDelegate::ImgThumbnailDelegate(QObject *parent)
	: QItemDelegate(parent), canDraw(false), enablePreview(false),
	  selectionBrush(QColor(204, 232, 255)), hoverBrush(QColor(229, 243, 255)) {

	flags = Qt::AlignHCenter | Qt::AlignBottom;
	auto parentWindow = qobject_cast<MainWindow *>(parent);
	auto parentObject = parent;
	while (nullptr == parentWindow) {
		parentWindow = qobject_cast<MainWindow *>(parentObject);
		parentObject = parentObject->parent();
	}
	previewLabel = new QLabel(parentWindow);
	previewLabel->setStyleSheet(
		"background:transparent; border: solid 10px grey;  background-color: rgba(229,243,255,127);");
	previewLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	// qDebug() << "Preview enabled: " << enablePreview;
}

void ImgThumbnailDelegate::drawDisplay(QPainter *				   painter,
									   const QStyleOptionViewItem &option,
									   const QRect &			   rect,
									   const QString &			   text) const {
	QString shortText = text.split('/').last();
	shortText.truncate(shortText.indexOf('.'));
	QRect newRect(rect);
	if (newRect.height() < painter->fontMetrics().height())
		newRect.setTop(rect.bottom() - painter->fontMetrics().height());

	QItemDelegate::drawDisplay(painter, option, newRect, shortText);
}

void ImgThumbnailDelegate::paint(QPainter *					 painter,
								 const QStyleOptionViewItem &option,
								 const QModelIndex &		 index) const {

	QStyleOptionViewItem modOption(option);

	if (index.isValid()) {
		if (option.state & QStyle::State_MouseOver) {
			painter->setRenderHint(QPainter::HighQualityAntialiasing);
			painter->fillRect(option.rect, hoverBrush);
			paintPreview(index);
			if (enablePreview)
				previewLabel->setVisible(true);
		}
		if (option.state & QStyle::State_Selected) {
			painter->setRenderHint(QPainter::HighQualityAntialiasing);
			painter->fillRect(option.rect, selectionBrush);
			modOption.palette.setColor(QPalette::HighlightedText,
									   option.palette.text().color());
			modOption.palette.setColor(QPalette::Highlight,
									   selectionBrush.color());
		}
	}

	QItemDelegate::paint(painter, modOption, index);
}

void ImgThumbnailDelegate::paintPreview(const QModelIndex &index) const {
	auto  view = qobject_cast<ImgListView *>(parent());
	auto  currentFileName = view->getFileName(index);
	auto &bigImgCache = view->getBigImgCache();
	if (bigImgCache.contains(currentFileName)) {
		auto pix = bigImgCache[currentFileName];
		previewLabel->setPixmap(pix);
		return;
	}

	// qDebug()<<index;
	// qDebug()<<currentFileName;
	auto scaledPix = drawScaledPixmap(currentFileName);
	bigImgCache[currentFileName] = scaledPix;
	previewLabel->setPixmap(std::move(scaledPix));
}

void ImgThumbnailDelegate::adjustSize() {

	auto parentWindow = qobject_cast<MainWindow *>(parent());
	auto parentObject = parent();
	while (nullptr == parentWindow) {
		parentWindow = qobject_cast<MainWindow *>(parentObject);
		parentObject = parentObject->parent();
	}

	auto tmpPos = parentWindow->getTreeWidgetPos();
	auto newSize = parentWindow->getTreeWidgetSize();

	if (imgSize != newSize) {
		imgSize = newSize;
		previewLabel->setFixedWidth(imgSize.width());
		previewLabel->setFixedHeight(imgSize.height());
	}
	// previewLabel->move(previewLabel->mapToParent(QPoint(0,0)));
	if (imgPos != tmpPos) {
		imgPos = tmpPos;
		previewLabel->move(imgPos);
		previewLabel->hide();
	}
}

void ImgThumbnailDelegate::hidePreview() const {
	previewLabel->setVisible(false);
}

QPixmap ImgThumbnailDelegate::drawScaledPixmap(QString fileName) const {
	QImageReader reader(fileName);
	if (!reader.canRead()) {
		// qDebug() << "can't Read";
		reader.setFileName(":/Images/bad_img.png");
	}
	// reader.setScaledSize(imgSize);
	reader.setAutoTransform(true);
	// reader.setQuality(15);
	auto	 img = reader.read();
	QPixmap	 pix(img.size());
	QPainter painter(&pix);

	if (!img.isNull()) {
		painter.setRenderHints(QPainter::Antialiasing
							   | QPainter::SmoothPixmapTransform);
		painter.drawPixmap(0, 0, QPixmap::fromImage(img));
		painter.save();
		painter.restore();
		// return pix;
		return pix.scaled(
			imgSize.width(), imgSize.height(), Qt::KeepAspectRatio,
			hiQPreview ? Qt::SmoothTransformation : Qt::FastTransformation);
	}
	return {};
}
