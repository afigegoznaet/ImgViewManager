#include "ThumbnailsSorter.hpp"
#include <QDebug>

ThumbnailsSorter::ThumbnailsSorter(QObject *parent)
	: QSortFilterProxyModel(parent) {

	comparator.setNumericMode(true);
}


void ThumbnailsSorter::sort(int column, Qt::SortOrder order) {
	// qDebug()<<"sorting!";
	beginResetModel();
	QSortFilterProxyModel::sort(column, order);
	endResetModel();
}


void ThumbnailsSorter::sortByPath(bool flag) {
	fullPathSorting = flag;
	// qDebug()<<"sorting flag:"<<flag;
	sort();
}

bool ThumbnailsSorter::lessThan(const QModelIndex &left,
								const QModelIndex &right) const {
	if (fullPathSorting)
		return QSortFilterProxyModel::lessThan(left, right);

	QString leftFile =
		sourceModel()->data(left, Qt::DisplayRole).toString().section('/', -1);
	QString rightFile =
		sourceModel()->data(right, Qt::DisplayRole).toString().section('/', -1);

	return comparator.compare(leftFile, rightFile) < 0;
	// return !QSortFilterProxyModel::lessThan(left, right);
}
