[1mdiff --git a/mainwindow.cpp b/mainwindow.cpp[m
[1mindex 7118b60..de5a5f9 100644[m
[1m--- a/mainwindow.cpp[m
[1m+++ b/mainwindow.cpp[m
[36m@@ -154,7 +154,7 @@[m [mvoid MainWindow::mouseMoveEvent(QMouseEvent* event)[m
 void MainWindow::mousePressEvent(QMouseEvent* event)[m
 {[m
     //globalPos()鼠标当前位置           pos()主窗口当前位置[m
[31m-    mPos = event->globalPos() - this->pos();[m
[32m+[m[32m    mPos = event->globalPos() - this->pos();    //鼠标按下时，相对于窗口左上角的偏移量[m
 }[m
 [m
 void MainWindow::slot_exitapp()[m
