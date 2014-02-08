#include "abstract_graph_view.h"

#include <QWheelEvent>
#include <QDebug>

#include "graph_view.h"
#include "graph_overlay.h"

#include <assert.h>

void AbstractGraphView::mouseMoveEvent(QMouseEvent *e) {
	if (viewport()) {
		GraphRanges ranges = getRanges();
		QDateTime time = ranges.getXTime(e->x());
		float price = ranges.getYPrice(e->y());
		emit dataPointHovered(time, price);
		e->accept();
	}
}

void AbstractGraphView::wheelEvent(QWheelEvent *e) {
	if (viewport()) {
		// if (e->modifiers() & Qt::ControlModifier) {
		emit dataPointZoomed(getRanges().getXTime(e->x()), e->delta());
		e->accept();
	}
}

void AbstractGraphView::resizeEvent(QResizeEvent*) {
	emit resized();
}

AbstractGraphView::AbstractGraphView() {
	scene = new QGraphicsScene;
	setScene(scene);
	scene->setParent(this);

	setMouseTracking(true);

	connect(this, SIGNAL(resized()), this, SLOT(notifyOverlaysRangesChanged()));
	connect(this, SIGNAL(resized()), this, SLOT(redraw()));
}

void AbstractGraphView::assignViewport(GraphViewport* viewport) {
	assert(viewport);
	qDebug() << "Assigning new viewport:" << viewport;

	for (GraphOverlay* g: overlays) delete g;
	overlays.clear(); // TODO: destroy...

	internalizeViewport(viewport);
	addOverlays();

	notifyOverlaysProjectionChanged();
	notifyOverlaysRangesChanged();
	redraw(); // Because of newly created viewport.
}

void AbstractGraphView::notifyOverlaysProjectionChanged() {
	if (viewport()) {
		qDebug() << "notifying overlays of projection change";
		OHLCProvider* projection = viewport()->getSourceProjection();

		for (GraphOverlay* overlay: overlays) {
			overlay->projectionChanged(projection);
		}
	}
}

void AbstractGraphView::notifyOverlaysRangesChanged() {
	if (viewport()) {
		qDebug() << "notifying overlays of ranges change";
		GraphRanges ranges = getRanges();

		for (GraphOverlay* overlay: overlays) {
			overlay->rangesChanged(ranges);
		}
	}
}

GraphRanges AbstractGraphView::getRanges() {
	assert(viewport());
	GraphRanges ranges = viewport()->getRanges();
	ranges.width = width();
	ranges.height = height();
	return ranges;
}

void AbstractGraphView::redraw() {
	if (!viewport()) {
		qDebug() << "redrawing with NULL viewport, drawing nothing.";
		return;
	}

	qDebug() << "Redrawing. Viewport from:" << viewport()->getViewBegin() << "to:" << viewport()->getViewEnd();
	scene->clear();

	qDebug() << "== DRAWING OVERLAYS ==";
	for (GraphOverlay* overlay: overlays) {
		overlay->insertIntoScene(scene);
		qDebug() << "===";
	}
	qDebug() << "== OVERLAYS FINISHED ==";
}

QGraphicsScene* AbstractGraphView::getScene() {
	return scene;
}