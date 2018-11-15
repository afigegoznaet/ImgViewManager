#ifndef THUMBNAILSSORTER_HPP
#define THUMBNAILSSORTER_HPP
#include <QSortFilterProxyModel>
#include <QCollator>

class ThumbnailsSorter : public QSortFilterProxyModel {
	Q_OBJECT
public:
	explicit ThumbnailsSorter(QObject *parent = Q_NULLPTR);
	void sort(int column = 0,
			  Qt::SortOrder order = Qt::AscendingOrder) override;

public slots:
	void sortByPath(bool flag);

protected:
	bool lessThan(const QModelIndex &left,
				  const QModelIndex &right) const override;

private:
	bool fullPathSorting = true;
	QCollator comparator;
};

#endif // THUMBNAILSSORTER_HPP
