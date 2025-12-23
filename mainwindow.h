#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlTableModel>
#include <QModelIndex>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Login
    void on_btnLogin_clicked();

    // Navigation
    void on_btnManageStudent_clicked();
    void on_btnManageCourses_clicked();
    void on_btnManageEnrollment_clicked();
    void on_btnLogout_clicked();

    // Table row click handlers
    void onStudentRowClicked(const QModelIndex &index);
    void onCourseRowClicked(const QModelIndex &index);
    void onEnrollmentRowClicked(const QModelIndex &index);

    void on_btnAddStudents_clicked();

    void on_btnUpdateStudents_clicked();

    void on_btnDeleteStudents_clicked();

    void on_btnClearStudents_clicked();

    void on_btnAddCourses_clicked();

    void on_btnUpdateCourses_clicked();

    void on_btnDeleteCourses_clicked();

    void on_btnClearCourses_clicked();

    void on_btnAddEnrollment_clicked();

    void on_btnUpdateEnrollment_clicked();

    void on_btnDeleteEnrollment_clicked();

    void on_btnClearEnrollment_clicked();

    void on_btnBackFromStudents_clicked();

    void on_btnBackFromCourses_clicked();

    void on_btnBackFromEnrollment_clicked();

private:
    Ui::MainWindow *ui;

    // Database connection
    QSqlDatabase db;

    // Table models
    QSqlTableModel *modelStudents;
    QSqlTableModel *modelCourses;
    QSqlTableModel *modelEnrollment;

    // Combo-loading functions
    void loadDepartments();
    void loadLecturers();
    void loadStudentsForEnrollment();
    void loadCoursesForEnrollment();
    void loadSemesters();

    QString currentUserRole;

};

#endif // MAINWINDOW_H
