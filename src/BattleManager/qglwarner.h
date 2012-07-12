#ifndef QGLWARNER_H
#define QGLWARNER_H

#include <QGLWidget>
#include <QDeclarativeView>

/** Necessary because Qt crashes if a loader element containing a ShaderEffectItem is
  still active when the declarative view is deleted, so we need to be warned when the
  gl is hidden, so that we can unload the shader before the widget is destroyed
  */
class QGLWarner: public QGLWidget {
    Q_OBJECT
public:
    QGLWarner(QWidget *parent=NULL);
    ~QGLWarner();

    void setVisible(bool visible);
    void show();
    void hide();

    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);
public slots:
    void deleteLater();
    void realDeleteLater();
signals:
    void disappearing();
    void appearing();
private:
};

#endif // QGLWARNER_H
