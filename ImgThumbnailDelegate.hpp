#ifndef IMGTHUMBNAILDELEGATE_H
#define IMGTHUMBNAILDELEGATE_H
#include <QItemDelegate>

class QLabel;
class QAbstractItemModel;

class ImgThumbnailDelegate : public QItemDelegate {
	Q_OBJECT
public:
	explicit ImgThumbnailDelegate(QObject *parent);
	void  setModel(QAbstractItemModel *_model) { model = _model; }
	void  stopDrawing() { canDraw = false; }
	void  resumeDrawing() { canDraw = true; }
	void  setGridSize(QSize _gridSize) { gridSize = _gridSize; }
	QSize sizeHint(const QStyleOptionViewItem &,
				   const QModelIndex &) const override {
		return gridSize;
	}
	void					hidePreview() const;
	const QSize &			getPreviewSize() const { return imgSize; }
	const std::atomic_bool &getHiQPreview() const { return hiQPreview; }
	[[nodiscard]] QPixmap	drawScaledPixmap(QString fileName) const;

protected:
	void drawDisplay(QPainter *painter, const QStyleOptionViewItem &option,
					 const QRect &rect, const QString &text) const override;
	void paint(QPainter *painter, const QStyleOptionViewItem &option,
			   const QModelIndex &index) const override;
signals:

public slots:
	void showPreview(bool flag) { enablePreview = flag; }
	void enableHiQPreview(bool flag) { hiQPreview = flag; }
	void adjustSize();

private:
	QLabel *			previewLabel;
	QPoint				imgPos;
	QSize				imgSize;
	QAbstractItemModel *model{};
	int					flags;
	std::atomic_bool	canDraw;
	std::atomic_bool	enablePreview;
	std::atomic_bool	hiQPreview{};
	QSize				gridSize;
	const QBrush		selectionBrush;
	const QBrush		hoverBrush;

	void paintPreview(const QModelIndex &index) const;
};

#endif // IMGTHUMBNAILDELEGATE_H
