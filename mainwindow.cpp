#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
, ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qDebug() << "Available SQL Drivers:" << QSqlDatabase::drivers();

    // ---------------------------
    // DATABASE CONNECTION
    // ---------------------------
    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setPort(3306);
    db.setDatabaseName("UniversityDB");
    db.setUserName("root");
    db.setPassword("Elkanahlewis03$");

    if (db.open()) {
        qDebug() << "MySQL Connection: SUCCESS";

        // ----------------------------------------------------
        // INITIALIZE TABLE MODELS
        // ----------------------------------------------------
        modelStudents = new QSqlTableModel(this, db);
        modelStudents->setTable("Students");
        modelStudents->select();
        ui->tableView->setModel(modelStudents);

        modelCourses = new QSqlTableModel(this, db);
        modelCourses->setTable("Courses");
        modelCourses->select();
        ui->tblCourses->setModel(modelCourses);

        modelEnrollment = new QSqlTableModel(this, db);
        modelEnrollment->setTable("Enrollment");
        modelEnrollment->select();
        ui->tblEnrollment->setModel(modelEnrollment);

        // =============================
        // TABLE ROW CLICK SIGNALS
        // =============================
        connect(ui->tableView, &QTableView::clicked,
                this, &MainWindow::onStudentRowClicked);

        connect(ui->tblCourses, &QTableView::clicked,
                this, &MainWindow::onCourseRowClicked);

        connect(ui->tblEnrollment, &QTableView::clicked,
                this, &MainWindow::onEnrollmentRowClicked);

    } else {
        qDebug() << "MySQL Connection ERROR:" << db.lastError().text();
    }

    // Start application on login page
    ui->stackedWidget->setCurrentWidget(ui->loginPage);
}

MainWindow::~MainWindow()
{
    delete ui;
}

//////////////////////////////////////////////////////////////
// LOGIN
//////////////////////////////////////////////////////////////

void MainWindow::on_btnLogin_clicked()
{
    QString username = ui->txtUsername->text();
    QString password = ui->txtPassword->text();
    QString role = ui->cmbRole->currentText();

    if (username.isEmpty() || password.isEmpty()) {
        ui->lblStatusLogin->setText("Please fill all fields.");
        return;
    }

    QSqlQuery query;
    query.prepare("SELECT Role FROM UserAccounts WHERE Username = :u AND Password = :p");
    query.bindValue(":u", username);
    query.bindValue(":p", password);

    if (!query.exec()) {
        ui->lblStatusLogin->setText("Database Error.");
        return;
    }

    if (query.next()) {
        QString dbRole = query.value(0).toString();

        if (role == dbRole) {

            // SAVE THE ROLE OF THE LOGGED-IN USER
            currentUserRole = dbRole;

            ui->lblStatusLogin->setText("Login Successful!");
            ui->lblStatusLogin->setStyleSheet("color: lightgreen;");
            ui->stackedWidget->setCurrentIndex(1); // Main menu
        } else {
            ui->lblStatusLogin->setText("Role mismatch!");
        }

    } else {
        ui->lblStatusLogin->setText("Invalid credentials.");
    }
}

//////////////////////////////////////////////////////////////
// MAIN MENU NAVIGATION
//////////////////////////////////////////////////////////////

void MainWindow::on_btnManageStudent_clicked()
{
    if (currentUserRole == "Lecturer" || currentUserRole == "Student") {
        QMessageBox::warning(this, "Access Denied",
                             "You do not have permission to manage students.");
        return;
    }

    loadDepartments();
    modelStudents->select();
    ui->stackedWidget->setCurrentWidget(ui->pageStudents);
}

void MainWindow::on_btnManageCourses_clicked()
{
    if (currentUserRole == "Student") {
        QMessageBox::warning(this, "Access Denied",
                             "Students cannot access course management.");
        return;
    }

    loadDepartments();
    loadLecturers();
    modelCourses->select();
    ui->stackedWidget->setCurrentWidget(ui->pageCourses);

    // If the user is a Lecturer, lock the CRUD buttons
    bool canEdit = (currentUserRole == "Admin" || currentUserRole == "Staff");

    ui->btnAddCourses->setEnabled(canEdit);
    ui->btnUpdateCourses->setEnabled(canEdit);
    ui->btnDeleteCourses->setEnabled(canEdit);
}

void MainWindow::on_btnManageEnrollment_clicked()
{
    if (currentUserRole == "Student") {
        QMessageBox::warning(this, "Access Denied",
                             "Students cannot manage enrollment.");
        return;
    }

    loadStudentsForEnrollment();
    loadCoursesForEnrollment();
    loadSemesters();
    modelEnrollment->select();
    ui->stackedWidget->setCurrentWidget(ui->pageEnrollment);

    bool canEdit = (currentUserRole == "Admin" || currentUserRole == "Staff");

    ui->btnAddEnrollment->setEnabled(canEdit);
    ui->btnUpdateEnrollment->setEnabled(canEdit);
    ui->btnDeleteEnrollment->setEnabled(canEdit);
}

void MainWindow::on_btnLogout_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->loginPage);
    ui->txtUsername->clear();
    ui->txtPassword->clear();
    ui->lblStatusLogin->clear();
}

//////////////////////////////////////////////////////////////
// TABLE ROW CLICK HANDLERS
//////////////////////////////////////////////////////////////

void MainWindow::onStudentRowClicked(const QModelIndex &index)
{
    int row = index.row();

    ui->txtStudentId->setText(modelStudents->index(row, 0).data().toString());
    ui->txtFirstName->setText(modelStudents->index(row, 1).data().toString());
    ui->txtLastName->setText(modelStudents->index(row, 2).data().toString());
    ui->txtEmail->setText(modelStudents->index(row, 3).data().toString());
    ui->txtPhone->setText(modelStudents->index(row, 4).data().toString());
    ui->txtAddress->setText(modelStudents->index(row, 5).data().toString());

    QDate d = QDate::fromString(modelStudents->index(row, 6).data().toString(), "yyyy-MM-dd");
    ui->dtAdmissionDate->setDate(d);

    int deptId = modelStudents->index(row, 7).data().toInt();
    ui->cmbDepartment->setCurrentIndex(ui->cmbDepartment->findData(deptId));
}

void MainWindow::onCourseRowClicked(const QModelIndex &index)
{
    int row = index.row();

    ui->txtCourseId->setText(modelCourses->index(row, 0).data().toString());
    ui->txtCourseName->setText(modelCourses->index(row, 1).data().toString());
    ui->txtCourseCode->setText(modelCourses->index(row, 2).data().toString());
    ui->spnCreditHours->setValue(modelCourses->index(row, 3).data().toInt());

    int deptId = modelCourses->index(row, 4).data().toInt();
    ui->cmbDepartmentCourses->setCurrentIndex(ui->cmbDepartmentCourses->findData(deptId));

    int lecId = modelCourses->index(row, 5).data().toInt();
    ui->cmbLecturerCourses->setCurrentIndex(ui->cmbLecturerCourses->findData(lecId));
}

void MainWindow::onEnrollmentRowClicked(const QModelIndex &index)
{
    int row = index.row();

    ui->txtEnrollmentId->setText(modelEnrollment->index(row, 0).data().toString());

    int studentId = modelEnrollment->index(row, 1).data().toInt();
    ui->cmbStudent->setCurrentIndex(ui->cmbStudent->findData(studentId));

    int courseId = modelEnrollment->index(row, 2).data().toInt();
    ui->cmbCourse->setCurrentIndex(ui->cmbCourse->findData(courseId));

    int semesterId = modelEnrollment->index(row, 3).data().toInt();
    ui->cmbSemester->setCurrentIndex(ui->cmbSemester->findData(semesterId));

    QDate date = QDate::fromString(modelEnrollment->index(row, 4).data().toString(), "yyyy-MM-dd");
    ui->dtEnrollmentDate->setDate(date);

    ui->txtGrade->setText(modelEnrollment->index(row, 5).data().toString());
}

//////////////////////////////////////////////////////////////
// LOAD FUNCTIONS
//////////////////////////////////////////////////////////////

void MainWindow::loadDepartments()
{
    ui->cmbDepartment->clear();
    ui->cmbDepartmentCourses->clear();

    QSqlQuery query("SELECT Department_ID, Department_Name FROM Departments");

    while (query.next()) {
        int id = query.value(0).toInt();
        QString name = query.value(1).toString();

        ui->cmbDepartment->addItem(name, id);
        ui->cmbDepartmentCourses->addItem(name, id);
    }
}

void MainWindow::loadLecturers()
{
    ui->cmbLecturerCourses->clear();

    QSqlQuery query("SELECT Lecturer_ID, CONCAT(First_Name, ' ', Last_Name) FROM Lecturers");

    while (query.next()) {
        ui->cmbLecturerCourses->addItem(query.value(1).toString(), query.value(0).toInt());
    }
}

void MainWindow::loadStudentsForEnrollment()
{
    ui->cmbStudent->clear();

    QSqlQuery query("SELECT Student_ID, CONCAT(First_Name, ' ', Last_Name) FROM Students");

    while (query.next()) {
        ui->cmbStudent->addItem(query.value(1).toString(), query.value(0).toInt());
    }
}

void MainWindow::loadCoursesForEnrollment()
{
    ui->cmbCourse->clear();

    QSqlQuery query("SELECT Course_ID, CONCAT(Course_Code, ' - ', Course_Name) FROM Courses");

    while (query.next()) {
        ui->cmbCourse->addItem(query.value(1).toString(), query.value(0).toInt());
    }
}

void MainWindow::loadSemesters()
{
    ui->cmbSemester->clear();

    QSqlQuery query("SELECT Semester_ID, Semester_Name FROM Semester");

    while (query.next()) {
        ui->cmbSemester->addItem(query.value(1).toString(), query.value(0).toInt());
    }
}

void MainWindow::on_btnAddStudents_clicked()
{
    QString firstName = ui->txtFirstName->text().trimmed();
    QString lastName  = ui->txtLastName->text().trimmed();
    QString email     = ui->txtEmail->text().trimmed();
    QString phone     = ui->txtPhone->text().trimmed();
    QString address   = ui->txtAddress->text().trimmed();
    QString admission = ui->dtAdmissionDate->date().toString("yyyy-MM-dd");
    int deptId        = ui->cmbDepartment->currentData().toInt();

    if (firstName.isEmpty() || lastName.isEmpty() || email.isEmpty()) {
        ui->lblStudentStatus->setText("Please fill in all required fields.");
        ui->lblStudentStatus->setStyleSheet("color: red;");
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO Students (First_Name, Last_Name, Email, Phone, Address, Date_of_Admission, Department_ID) "
                  "VALUES (:fn, :ln, :em, :ph, :ad, :dt, :dept)");

    query.bindValue(":fn",  firstName);
    query.bindValue(":ln",  lastName);
    query.bindValue(":em",  email);
    query.bindValue(":ph",  phone);
    query.bindValue(":ad",  address);
    query.bindValue(":dt",  admission);
    query.bindValue(":dept", deptId);

    if (query.exec()) {
        ui->lblStudentStatus->setText("Student added successfully.");
        ui->lblStudentStatus->setStyleSheet("color: lightgreen;");
        modelStudents->select();   // refresh table
    } else {
        ui->lblStudentStatus->setText("Insert Error: " + query.lastError().text());
        ui->lblStudentStatus->setStyleSheet("color: red;");
    }
}


void MainWindow::on_btnUpdateStudents_clicked()
{
    int id = ui->txtStudentId->text().toInt();
    if (id == 0) {
        ui->lblStudentStatus->setText("Select a student from the table first.");
        ui->lblStudentStatus->setStyleSheet("color: red;");
        return;
    }

    QString firstName = ui->txtFirstName->text().trimmed();
    QString lastName  = ui->txtLastName->text().trimmed();
    QString email     = ui->txtEmail->text().trimmed();
    QString phone     = ui->txtPhone->text().trimmed();
    QString address   = ui->txtAddress->text().trimmed();
    QString admission = ui->dtAdmissionDate->date().toString("yyyy-MM-dd");
    int deptId        = ui->cmbDepartment->currentData().toInt();

    QSqlQuery query;
    query.prepare("UPDATE Students SET First_Name = :fn, Last_Name = :ln, Email = :em, Phone = :ph, "
                  "Address = :ad, Date_of_Admission = :dt, Department_ID = :dept "
                  "WHERE Student_ID = :id");

    query.bindValue(":fn", firstName);
    query.bindValue(":ln", lastName);
    query.bindValue(":em", email);
    query.bindValue(":ph", phone);
    query.bindValue(":ad", address);
    query.bindValue(":dt", admission);
    query.bindValue(":dept", deptId);
    query.bindValue(":id", id);

    if (query.exec()) {
        ui->lblStudentStatus->setText("Student updated.");
        ui->lblStudentStatus->setStyleSheet("color: lightgreen;");
        modelStudents->select();
    } else {
        ui->lblStudentStatus->setText("Update Error: " + query.lastError().text());
        ui->lblStudentStatus->setStyleSheet("color: red;");
    }
}


void MainWindow::on_btnDeleteStudents_clicked()
{
    int id = ui->txtStudentId->text().toInt();

    if (id == 0) {
        ui->lblStudentStatus->setText("Select a student to delete.");
        ui->lblStudentStatus->setStyleSheet("color: red;");
        return;
    }

    QSqlQuery query;
    query.prepare("DELETE FROM Students WHERE Student_ID = :id");
    query.bindValue(":id", id);

    if (query.exec()) {
        ui->lblStudentStatus->setText("Student deleted.");
        ui->lblStudentStatus->setStyleSheet("color: lightgreen;");
        modelStudents->select();
    } else {
        ui->lblStudentStatus->setText("Delete Error: " + query.lastError().text());
        ui->lblStudentStatus->setStyleSheet("color: red;");
    }
}


void MainWindow::on_btnClearStudents_clicked()
{

    ui->txtStudentId->clear();
    ui->txtFirstName->clear();
    ui->txtLastName->clear();
    ui->txtEmail->clear();
    ui->txtPhone->clear();
    ui->txtAddress->clear();
    ui->dtAdmissionDate->setDate(QDate::currentDate());
    ui->cmbDepartment->setCurrentIndex(0);
    ui->lblStudentStatus->clear();
}


void MainWindow::on_btnAddCourses_clicked()
{

    QString name     = ui->txtCourseName->text().trimmed();
    QString code     = ui->txtCourseCode->text().trimmed();
    int credit       = ui->spnCreditHours->value();
    int deptId       = ui->cmbDepartmentCourses->currentData().toInt();
    int lecturerId   = ui->cmbLecturerCourses->currentData().toInt();

    if (name.isEmpty() || code.isEmpty()) {
        ui->lblCoursesStatus->setText("Please fill in all required fields.");
        ui->lblCoursesStatus->setStyleSheet("color: red;");
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO Courses "
                  "(Course_Name, Course_Code, Credit_Hours, Department_ID, Lecturer_ID) "
                  "VALUES (:name, :code, :credit, :dept, :lecturer)");

    query.bindValue(":name", name);
    query.bindValue(":code", code);
    query.bindValue(":credit", credit);
    query.bindValue(":dept", deptId);
    query.bindValue(":lecturer", lecturerId);

    if (query.exec()) {
        ui->lblCoursesStatus->setText("Course added successfully.");
        ui->lblCoursesStatus->setStyleSheet("color: lightgreen;");
        modelCourses->select();
    } else {
        ui->lblCoursesStatus->setText("Insert Error: " + query.lastError().text());
        ui->lblCoursesStatus->setStyleSheet("color: red;");
    }

}


void MainWindow::on_btnUpdateCourses_clicked()
{
    int id = ui->txtCourseId->text().toInt();
    QString name = ui->txtCourseName->text().trimmed();
    QString code = ui->txtCourseCode->text().trimmed();
    int credit = ui->spnCreditHours->value();
    int deptId = ui->cmbDepartmentCourses->currentData().toInt();
    int lecturerId = ui->cmbLecturerCourses->currentData().toInt();

    if (id <= 0) {
        ui->lblCoursesStatus->setText("Invalid Course ID.");
        ui->lblCoursesStatus->setStyleSheet("color: red;");
        return;
    }

    QSqlQuery query;
    query.prepare("UPDATE Courses SET "
                  "Course_Name = :name, "
                  "Course_Code = :code, "
                  "Credit_Hours = :credit, "
                  "Department_ID = :dept, "
                  "Lecturer_ID = :lecturer "
                  "WHERE Course_ID = :id");

    query.bindValue(":name", name);
    query.bindValue(":code", code);
    query.bindValue(":credit", credit);
    query.bindValue(":dept", deptId);
    query.bindValue(":lecturer", lecturerId);
    query.bindValue(":id", id);

    if (query.exec()) {
        ui->lblCoursesStatus->setText("Course updated successfully.");
        ui->lblCoursesStatus->setStyleSheet("color: lightgreen;");
        modelCourses->select();
    } else {
        ui->lblCoursesStatus->setText("Update Error: " + query.lastError().text());
        ui->lblCoursesStatus->setStyleSheet("color: red;");
    }
}


void MainWindow::on_btnDeleteCourses_clicked()
{
    int id = ui->txtCourseId->text().toInt();

    if (id == 0) {
        ui->lblCoursesStatus->setText("Select a course to delete.");
        ui->lblCoursesStatus->setStyleSheet("color: red;");
        return;
    }

    QSqlQuery query;
    query.prepare("DELETE FROM Courses WHERE Course_ID = :id");
    query.bindValue(":id", id);

    if (query.exec()) {
        ui->lblCoursesStatus->setText("Course deleted.");
        ui->lblCoursesStatus->setStyleSheet("color: lightgreen;");
        modelCourses->select();
    } else {
        ui->lblCoursesStatus->setText("Delete Error: " + query.lastError().text());
        ui->lblCoursesStatus->setStyleSheet("color: red;");
    }
}


void MainWindow::on_btnClearCourses_clicked()
{
    ui->txtCourseId->clear();
    ui->txtCourseName->clear();
    ui->txtCourseCode->clear();
    ui->spnCreditHours->setValue(1);
    ui->cmbDepartmentCourses->setCurrentIndex(0);
    ui->cmbLecturerCourses->setCurrentIndex(0);
    ui->lblCoursesStatus->clear();
}


void MainWindow::on_btnAddEnrollment_clicked()
{

    int studentId  = ui->cmbStudent->currentData().toInt();
    int courseId   = ui->cmbCourse->currentData().toInt();
    int semesterId = ui->cmbSemester->currentData().toInt();
    QString grade  = ui->txtGrade->text().trimmed();
    QString date   = ui->dtEnrollmentDate->date().toString("yyyy-MM-dd");

    if (studentId <= 0 || courseId <= 0 || semesterId <= 0) {
        ui->lblEnrollmentStatus->setText("Please fill in all required fields.");
        ui->lblEnrollmentStatus->setStyleSheet("color: red;");
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO Enrollment "
                  "(Student_ID, Course_ID, Semester_ID, Enrollment_Date, Grade) "
                  "VALUES (:student, :course, :semester, :date, :grade)");

    query.bindValue(":student", studentId);
    query.bindValue(":course", courseId);
    query.bindValue(":semester", semesterId);
    query.bindValue(":date", date);
    query.bindValue(":grade", grade.isEmpty() ? QVariant(QVariant::String) : grade);

    if (query.exec()) {
        ui->lblEnrollmentStatus->setText("Enrollment added successfully.");
        ui->lblEnrollmentStatus->setStyleSheet("color: lightgreen;");
        modelEnrollment->select();
    } else {
        ui->lblEnrollmentStatus->setText("Insert Error: " + query.lastError().text());
        ui->lblEnrollmentStatus->setStyleSheet("color: red;");
    }
}


void MainWindow::on_btnUpdateEnrollment_clicked()
{

    int id = ui->txtEnrollmentId->text().toInt();

    if (id <= 0) {
        ui->lblEnrollmentStatus->setText("Invalid Enrollment ID.");
        ui->lblEnrollmentStatus->setStyleSheet("color: red;");
        return;
    }

    int studentId  = ui->cmbStudent->currentData().toInt();
    int courseId   = ui->cmbCourse->currentData().toInt();
    int semesterId = ui->cmbSemester->currentData().toInt();
    QString date   = ui->dtEnrollmentDate->date().toString("yyyy-MM-dd");
    QString grade  = ui->txtGrade->text().trimmed();

    QSqlQuery query;
    query.prepare("UPDATE Enrollment SET "
                  "Student_ID = :student, "
                  "Course_ID = :course, "
                  "Semester_ID = :semester, "
                  "Enrollment_Date = :date, "
                  "Grade = :grade "
                  "WHERE Enrollment_ID = :id");

    query.bindValue(":student", studentId);
    query.bindValue(":course", courseId);
    query.bindValue(":semester", semesterId);
    query.bindValue(":date", date);
    query.bindValue(":grade", grade.isEmpty() ? QVariant(QVariant::String) : grade);
    query.bindValue(":id", id);

    if (query.exec()) {
        ui->lblEnrollmentStatus->setText("Enrollment updated successfully.");
        ui->lblEnrollmentStatus->setStyleSheet("color: lightgreen;");
        modelEnrollment->select();
    } else {
        ui->lblEnrollmentStatus->setText("Update Error: " + query.lastError().text());
        ui->lblEnrollmentStatus->setStyleSheet("color: red;");
    }

}


void MainWindow::on_btnDeleteEnrollment_clicked()
{
    int id = ui->txtEnrollmentId->text().toInt();

    if (id <= 0) {
        ui->lblEnrollmentStatus->setText("Invalid Enrollment ID.");
        ui->lblEnrollmentStatus->setStyleSheet("color: red;");
        return;
    }

    QSqlQuery query;
    query.prepare("DELETE FROM Enrollment WHERE Enrollment_ID = :id");
    query.bindValue(":id", id);

    if (query.exec()) {
        ui->lblEnrollmentStatus->setText("Enrollment deleted successfully.");
        ui->lblEnrollmentStatus->setStyleSheet("color: lightgreen;");
        modelEnrollment->select();
    } else {
        ui->lblEnrollmentStatus->setText("Delete Error: " + query.lastError().text());
        ui->lblEnrollmentStatus->setStyleSheet("color: red;");
    }
}




void MainWindow::on_btnClearEnrollment_clicked()
{

    ui->txtEnrollmentId->clear();
    ui->cmbStudent->setCurrentIndex(0);
    ui->cmbCourse->setCurrentIndex(0);
    ui->cmbSemester->setCurrentIndex(0);
    ui->dtEnrollmentDate->setDate(QDate::currentDate());
    ui->txtGrade->clear();
    ui->lblEnrollmentStatus->clear();

}


void MainWindow::on_btnBackFromStudents_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->pageMainMenu);
}


void MainWindow::on_btnBackFromCourses_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->pageMainMenu);
}


void MainWindow::on_btnBackFromEnrollment_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->pageMainMenu);
}

